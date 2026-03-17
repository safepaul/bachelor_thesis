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
 *  STATIC VARIABLES
 **********************/
static uint32_t mcr_instant = 0;
static uint8_t current_mode;
static mcm_system_state_t system_state;
static mcm_config_t *config;

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
static void mcm_stop_task_timers();

/**********************
 *  GLOBAL FUNCTIONS
 **********************/
void mcm_initial_setup(mcm_config_t *sys_config, const uint8_t initial_mode)
{
    MCM_LOGI("Setting up initial system state");

    config = sys_config;
    system_state = SYSTEM_STATE_NORMAL;
    current_mode = initial_mode;

    mcm_start_initial_task_timers();
}

void mcm_mc_request(const uint8_t target_mode)
{
    // TODO: create a mode_to_string to print mode names here
    MCM_LOGI("Mode change requested");

    mcr_instant = pdTICKS_TO_MS(xTaskGetTickCount());
    const uint8_t transition_id = mcm_fetch_transition_id(current_mode, target_mode);
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
    mcm_stop_task_timers();

    MCM_LOGI("Processing tasks");
    system_state = SYSTEM_STATE_TRANSITIONING;
    bool has_async = false;

    for (int i = 0; i < transition->taskset_size; i++) 
    {
        const mcm_transition_task_t *task = &transition->taskset[i];

        MCM_LOGI("Processing task %d", task->task_id);
        has_async |= mcm_process_task(task);
        MCM_LOGI("Task %d processed", task->task_id);
    }

    return has_async ? MCM_TRANS_SYNC_PENDING : MCM_TRANS_SYNC_ALL;
}

static bool mcm_process_task(const mcm_transition_task_t *task)
{
    const uint8_t guard = task->primitives.guard;

    MCM_LOGI("Evaluating guard for task %d", task->task_id);
    switch (guard) 
    {
        case GUARD_TRUE:
            MCM_LOGI("Guard TRUE. Performing action immediately for task %d.", task->task_id);
            mcm_perform_action(task);
        break;

        case GUARD_BACKLOG_ZERO:
        break;

        case GUARD_BACKLOG_GLOBAL:
        break;

        case GUARD_OFFSETLR:
        break;

        case GUARD_OFFSETMCR:
        break;

        default:
            MCM_LOGE("Trying to evaluate a guard that doesn't exist");
            abort();
        break;
    }

    return false;
}

static void mcm_perform_action(const mcm_transition_task_t *task)
{
    const uint8_t action = task->primitives.action;

    switch (action) 
    {
        case ACTION_CONTINUE:
            MCM_LOGI("Action CONTINUE. Starting timer back again.");
            xTimerStart(config->timer_handles[task->task_id], 0);
        break;

        case ACTION_UPDATE:
        break;

        case ACTION_RELEASE:
        break;

        case ACTION_SUSPEND:
        break;

        default:
            MCM_LOGE("Trying to perform an action that doesn't exist");
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

static void mcm_release_job(const uint8_t task_id)
{
    MCM_LOGI("Releasing job of task %d", task_id);
    xSemaphoreGive(config->semaphore_handles[task_id]);
}

static void mcm_start_initial_task_timers()
{
    MCM_LOGI("Starting initial task timers");
    
    for (int i = 0; i < config->modes[current_mode].n_tasks; i++)
    {
        uint8_t task_id = config->modes[current_mode].tasks[i].id; 
        mcm_release_job(task_id);
        xTimerStart(config->timer_handles[task_id], 0);
    }
}

static void mcm_stop_task_timers()
{
    MCM_LOGI("Stopping task timers");
    for (int i = 0; i < config->modes[current_mode].n_tasks; i++) 
    {
        uint8_t task_id = config->modes[current_mode].tasks[i].id;
        xTimerStop(config->timer_handles[task_id], 0);
    }    
    MCM_LOGI("Task timers stopped")
}

static void mcm_clear_backlog(const uint8_t task_id)
{
    while (xSemaphoreTake(config->semaphore_handles[task_id], 0) == pdTRUE) {}
}
