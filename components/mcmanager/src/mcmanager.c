#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "portmacro.h"

#include "stdlib.h"
#include <stdbool.h>
#include <stdint.h>

#include "mcm_log.h"
#include "mcm_types.h"
#include "mcmanager.h"


/**********************
 *      DEFINES
 **********************/
#define SET_BIT(mask, bit) ((mask) |= ((uint32_t)1 << (bit)))
#define CLEAR_BIT(mask, bit) ((mask) &= ~((uint32_t)1 << (bit)))

/**********************
 *  STATIC VARIABLES
 **********************/
static TickType_t mcr_instant = 0;
static uint8_t current_mode;
static mcm_system_state_t system_state;
static mcm_config_t *config;
static SemaphoreHandle_t transition_mutex;
static uint32_t backlog_bitmask;
static uint32_t offset_bitmask;
static uint8_t last_transition;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static mcm_trans_result_t mcm_perform_transition(const mcm_transition_t *transition, const uint8_t target_mode);
static void mcm_perform_action(const mcm_transition_task_t *task);
static bool mcm_process_task(const mcm_transition_task_t *task);
static uint8_t mcm_fetch_transition_id(const uint8_t source_mode, const uint8_t dest_mode);
static uint8_t mcm_get_backlog(const uint8_t task_id);
static void mcm_release_job(const uint8_t task_id);
static void mcm_start_initial_task_timers();
static void mcm_clear_backlog(const uint8_t task_id);
static void mcm_change_parameters(const uint8_t task_id, const uint8_t dest_mode);
static void mcm_offset_timer_callback_func(TimerHandle_t xTimer);
static void mcm_check_backlog_status(const uint8_t task_id);
static TickType_t mcm_calculate_offset_delay(const uint16_t offset, const TickType_t offset_reference);
static const mcm_transition_task_t * mcm_fetch_taskset_task_by_id(const uint8_t task_id);

/**********************
 *  GLOBAL FUNCTIONS
 **********************/
void mcm_initial_setup(mcm_config_t *sys_config, const uint8_t initial_mode)
{
    MCM_LOGI("Setting up initial system state");

    config = sys_config;
    system_state = SYSTEM_STATE_NORMAL;
    current_mode = initial_mode;
    transition_mutex = xSemaphoreCreateMutex();
    backlog_bitmask = 0;
    offset_bitmask = 0;
    last_transition = NO_TRANSITION;

    mcm_start_initial_task_timers();
}

void mcm_mc_request(const uint8_t target_mode)
{
    mcr_instant = xTaskGetTickCount();
    MCM_LOGI("Mode change requested at instant %d", (int)mcr_instant);

    const uint8_t transition_id = mcm_fetch_transition_id(current_mode, target_mode);
    last_transition = transition_id;
    const mcm_transition_t *transition = &config->transitions[transition_id];

    MCM_LOGI("Performing transition")
    mcm_trans_result_t trans_result = mcm_perform_transition(transition, target_mode);

    MCM_LOGI("All tasks have been processed. Mode transition complete.");
    current_mode = target_mode;

    if (trans_result == MCM_TRANS_SYNC_ALL)
    {
        MCM_LOGI("All tasks have been synchronized with the target mode during the transition. Setting system state to NORMAL.");
        system_state = SYSTEM_STATE_NORMAL;
    }
    else if (trans_result == MCM_TRANS_SYNC_PENDING)
    {
        MCM_LOGI("Some tasks are waiting to be synchronized with the target mode. Setting system state to TRANSIENT.");
        system_state = SYSTEM_STATE_TRANSIENT;
    }
}

void mcm_wait_for_release(const uint8_t task_id)
{
    MCM_LOGI("Task %d executing Wait For Release", task_id);
    xSemaphoreTake(config->semaphore_handles[task_id], portMAX_DELAY); 

    if (system_state == SYSTEM_STATE_TRANSIENT)
    {
        mcm_check_backlog_status(task_id);
    }

    // Mutex barrier in case there is an ongoing transition
    xSemaphoreTake(transition_mutex, portMAX_DELAY);
    xSemaphoreGive(transition_mutex);

}

void mcm_task_timer_callback_func(TimerHandle_t xTimer)
{
    // Timer ID's correspond to their respective task ID's
    uint8_t task_id = (uint8_t)(uintptr_t) pvTimerGetTimerID(xTimer);
    mcm_release_job(task_id);
}


/**********************
 *  STATIC FUNCTIONS
 **********************/
static mcm_trans_result_t mcm_perform_transition(const mcm_transition_t *transition, const uint8_t target_mode)
{
    system_state = SYSTEM_STATE_TRANSITIONING;
    bool has_async = false;

    MCM_LOGI("Performing mode transition. New job releases are blocked.")
    xSemaphoreTake(transition_mutex, portMAX_DELAY);
    MCM_LOGI("Took from transition mutex");
    for (int i = 0; i < transition->taskset_size; i++) 
    {
        const mcm_transition_task_t *task = &transition->taskset[i];

        has_async |= mcm_process_task(task);
    }
    xSemaphoreGive(transition_mutex);
    MCM_LOGI("Mode transition successfully performed. New job releases are possible.")

    return has_async ? MCM_TRANS_SYNC_PENDING : MCM_TRANS_SYNC_ALL;
}

static bool mcm_process_task(const mcm_transition_task_t *task)
{
    const uint8_t guard = task->primitives.guard;
    const uint8_t task_id = task->id;

    MCM_LOGI("Processing task %d", task_id);

    switch (guard) 
    {
        case GUARD_TRUE:
            mcm_perform_action(task);
            return false;
        break;

        case GUARD_BACKLOG_ZERO:
            if (mcm_get_backlog(task_id) == 0) 
            {
                MCM_LOGI("Performing guard BACKLOG_ZERO for task %d. Backlog was already empty, performing action instantly.", task_id);
                mcm_perform_action(task);
                return false;
            }
            else
            {
                MCM_LOGI("Performing guard BACKLOG_ZERO for task %d. Backlog was not empty, stopping timer and letting backlog clear.", task_id);
                // stop timer if backlog is not empty
                xTimerStop(config->timer_handles[task_id], 0);
                // set bit of backlog tasks bitmask 
                SET_BIT(backlog_bitmask, task_id);
                return true;
            }
        break;

        case GUARD_OFFSETMCR:
        case GUARD_OFFSETLR:
            TickType_t delay;
            // calculate delay
            if (guard == GUARD_OFFSETMCR)
            {
                MCM_LOGI("Processing guard OFFSETMCR of task %d", task_id);
                delay = mcm_calculate_offset_delay(task->primitives.guard_value, mcr_instant);
            }
            else
            {
                MCM_LOGI("Processing guard OFFSETLR of task %d", task_id);
                delay = mcm_calculate_offset_delay(task->primitives.guard_value, config->tasks[task_id].last_release);
                MCM_LOGI("Delay = %lu", delay);
            }

            if (delay == 1)
            {
                mcm_perform_action(task);
            }
            else
            {
                // create timer with that offset (delete afterwards)
                // TODO: creation+deletion will probably lead to memory fragmentation. Generate them and change parameters instead.
                TimerHandle_t timer = xTimerCreate("offset_timer", delay, pdFALSE, (void*)(uintptr_t)task_id, mcm_offset_timer_callback_func);
                xTimerStart(timer, 0);

                // set bit of offset tasks bitmask 
                SET_BIT(offset_bitmask, task_id);
            }

            return true;
        break;

        default:
            abort();
        break;
    }

    return false;
}

static void mcm_perform_action(const mcm_transition_task_t *task)
{
    const uint8_t action = task->primitives.action;
    const uint8_t task_id = task->id;
    const uint8_t dest_mode = config->transitions[task->transition_id].dest_mode;
    const TimerHandle_t task_timer = config->timer_handles[task_id];

    switch (action) 
    {
        case ACTION_CONTINUE:
            MCM_LOGI("Action CONTINUE performed for task %d. No action is taken", task_id);
        break;

        case ACTION_UPDATE:
            MCM_LOGI("Action UPDATE performed for task %d at %lu. Clearing backlog and changing parameters", task_id, xTaskGetTickCount());
            mcm_clear_backlog(task_id);
            mcm_change_parameters(task_id, dest_mode);
        break;

        case ACTION_RELEASE:
            MCM_LOGI("Action RELEASE performed for task %d. Changing parameters, releasing a job and restarting its timer.", task_id);
            mcm_release_job(task_id);
            mcm_change_parameters(task_id, dest_mode);
            xTimerReset(task_timer, 0);
        break;

        case ACTION_SUSPEND:
            MCM_LOGI("Action SUSPEND performed for task %d. Stopping timer and clearing backlog.", task_id);
            xTimerStop(task_timer, 0);
            mcm_clear_backlog(task_id);
        break;

        default:
            abort();
        break;
    }
}

static uint8_t mcm_fetch_transition_id(const uint8_t source_mode, const uint8_t dest_mode)
{
    return (uint8_t) config->mode_transitions[(config->n_modes * source_mode) + dest_mode];
}

static uint8_t mcm_get_backlog(const uint8_t task_id)
{
    return (uint8_t) uxSemaphoreGetCount(config->semaphore_handles[task_id]);
}

static void mcm_clear_backlog(const uint8_t task_id)
{
    MCM_LOGI("Task %d backlog: %d", task_id, uxSemaphoreGetCount(config->semaphore_handles[task_id]));
    while (xSemaphoreTake(config->semaphore_handles[task_id], 0) == pdTRUE) {}
}

static void mcm_change_parameters(const uint8_t task_id, const uint8_t dest_mode)
{
    UBaseType_t new_prio = config->modes[dest_mode].tasks->parameters.priority;
    TickType_t new_period = pdMS_TO_TICKS(config->modes[dest_mode].tasks[task_id].parameters.period);
    MCM_LOGI("New parameters for task %d: new_prio = %d; new_period = %lu", task_id, new_prio, new_period);

    vTaskPrioritySet(config->task_handles[task_id], new_prio);
    xTimerChangePeriod(config->timer_handles[task_id], new_period, 0);
}

static void mcm_release_job(const uint8_t task_id)
{
    xSemaphoreGive(config->semaphore_handles[task_id]);
    config->tasks[task_id].last_release = xTaskGetTickCount();
}

static void mcm_start_initial_task_timers()
{
    for (int i = 0; i < config->modes[current_mode].n_tasks; i++)
    {
        uint8_t task_id = config->modes[current_mode].tasks[i].id; 
        mcm_release_job(task_id);
        xTimerStart(config->timer_handles[task_id], 0);
    }
}

static void mcm_check_backlog_status(const uint8_t task_id)
{
    const mcm_transition_task_t *task = mcm_fetch_taskset_task_by_id(task_id);
    //if (task == NULL) abort();


    // to avoid user calling an mcr() while the asychronous actions are being applied
    xSemaphoreTake(transition_mutex, portMAX_DELAY);
    // check if the task was waiting for its backlog to clear. remove its bit from the backlog bitmask if it was and perform its action
    if (backlog_bitmask & ((uint32_t)1 << task_id))
    {
        CLEAR_BIT(backlog_bitmask, task_id);
        mcm_perform_action(task);

        // check if it was the last of the transient tasks
        if (backlog_bitmask == 0 && offset_bitmask == 0)
        {
            MCM_LOGI("SETTING SYSTEM STATE TO NORMAL FROM LAST BACKLOG");
            system_state = SYSTEM_STATE_NORMAL;
        }
    }
    xSemaphoreGive(transition_mutex);
}

static void mcm_offset_timer_callback_func(TimerHandle_t xTimer)
{
    // Timer ID's correspond to their respective task ID's
    const uint8_t task_id = (uint8_t)(uintptr_t) pvTimerGetTimerID(xTimer);

    const mcm_transition_task_t *task = mcm_fetch_taskset_task_by_id(task_id);
    //if (task == NULL) abort();

    // to avoid user calling an mcr() while the asychronous actions are being applied
    xSemaphoreTake(transition_mutex, portMAX_DELAY);
    // remove its bit from the offset tasks mask and perform its action
    CLEAR_BIT(offset_bitmask, task_id);
    mcm_perform_action(task);

    // check if it was the last of the transient tasks
    if (backlog_bitmask == 0 && offset_bitmask == 0)
    {
        MCM_LOGI("SETTING SYSTEM STATE TO NORMAL FROM LAST OFFSET");
        system_state = SYSTEM_STATE_NORMAL;
    }
    xSemaphoreGive(transition_mutex);

    // delete to avoid heap starvation
    xTimerDelete(xTimer, 0);
}

static const mcm_transition_task_t * mcm_fetch_taskset_task_by_id(const uint8_t task_id)
{
    const mcm_transition_t *trans = &config->transitions[last_transition];

    // taskset[0] is not necessarily task 0. find iteratively
    for (int i = 0; i < trans->taskset_size; i++) 
        if (trans->taskset[i].id == task_id) 
            return &trans->taskset[i];

    return NULL;
}

static TickType_t mcm_calculate_offset_delay(const uint16_t offset, const TickType_t offset_reference)
{
    MCM_LOGI("\noffset:%lu \noffset_reference:%lu\n\n", pdMS_TO_TICKS(offset), offset_reference);
    // release at offset_reference + offset
    TickType_t release_timestamp = offset_reference + pdMS_TO_TICKS(offset);
    TickType_t current_time = xTaskGetTickCount();

    // delay the task for the difference between the timestamp and the actual time
    // if timestamp is less than current time, release instantly (casted to int32 to allow negative numbers and avoid underflow)
    if ((int32_t)(release_timestamp - current_time) <= 0)
        return 1;

    return release_timestamp - current_time;
}
