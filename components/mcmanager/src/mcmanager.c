#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "mcm_types.h"
#include "portmacro.h"

#include "stdlib.h"
#include <stdbool.h>
#include <stdint.h>

#include "gen_data.h"
#include "mcm_types.h"
#include "mcm_log.h"
#include "mcm_trans.h"
#include "mcmanager.h"

// Records the instant of the most recent mode change request
static TickType_t mcr_instant = 0;

static uint8_t current_mode = MODE_INIT;

void mcm_initial_setup()
{
    // start the timers
    for (int i = 0; i < modes[MODE_INIT].n_tasks; i++)
    {
        uint8_t *task_id = &modes[MODE_INIT].tasks[i].id;
        xTimerStart(timer_handles[*task_id], pdMS_TO_TICKS(5));
    }
}

void mcm_mc_request(const uint8_t target_mode)
{
    mcr_instant = xTaskGetTickCount();
    uint8_t transition_id = mode_transitions[current_mode][target_mode];

    // does the transition exist?
    // XXX: If I generate all of the data, how come a const data can suddenly not exist?
    if (transition_id == NO_TRANSITION)
        abort();

    mcm_perform_transition(transition_id);
    current_mode = target_mode;
}

void mcm_perform_transition(const uint8_t trans_id)
{
    const mcm_transition_t *trans = &transitions[trans_id];
    const mcm_transition_task_t *taskset = trans->taskset;
    const uint8_t dest_mode_id = transitions[trans_id].dest_mode;

    // stop all taskset timers; we dont want new jobs releasing while processing
    for (int i = 0; i < trans->taskset_size; i++)
    {
        TimerHandle_t *thandle = &timer_handles[taskset[i].task_id];
        xTimerStop(*thandle, 0);
    }

    for (int i = 0; i < trans->taskset_size; i++)
    {
        const mcm_transition_task_t *task = &taskset[i];
        const uint8_t guard = task->primitives.guard;

        if (guard == GUARD_TRUE || guard == GUARD_NONE)
            mcm_trans_handle_guard_none(task, dest_mode_id);
        else if (guard == GUARD_BACKLOG_ZERO || guard == GUARD_BACKLOG_GLOBAL)
            mcm_trans_handle_guard_backlog(task, dest_mode_id);
        else if (guard == GUARD_OFFSETLR || guard == GUARD_OFFSETMCR)
            mcm_trans_handle_guard_offset(task, dest_mode_id, mcr_instant);
        else
            abort();
    }

    //mcm_trans_poll_backlogs(trans_id, current_mode, guard);
}



void mcm_perform_action(const uint8_t task_id, const uint8_t action, const uint8_t mode_id)
{
    TimerHandle_t timer_handle = timer_handles[task_id];

    switch (action)
    {
        case ACTION_NONE:
        case ACTION_CONTINUE:
            // start timer again, as they are stopped during the mcr()
            xTimerStart(timer_handle, 0);
        break;

        case ACTION_SUSPEND:
            // stop its timer. it will reset when it gets released again due to parameter change
            xTimerStop(timer_handle, 0);
        break;

        case ACTION_UPDATE:
            // change parameters.
            mcm_change_parameters(task_id, mode_id);
        break;

        case ACTION_RELEASE:
            // TODO: Right now it's redundant, check if it's necessary beforehand
            mcm_change_parameters(task_id, mode_id);
            mcm_release_job(task_id);
        break;

        default:
            MCM_LOGE("Trying to perform an action that doesn't exist to task %d", task_id);
            abort();
        break;
    }
}

void mcm_wait_for_release(const uint8_t task_id)
{
    mcm_task_t *task = &mcm_tasks[task_id];

    // if when it executes the waiting function it still has jobs in its backlog...
    if (task->backlog > 0) 
    {
        // don't change its state. Stay in released or waiting_for_backlog
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        task->backlog -= 1;
        task->last_release = xTaskGetTickCount();
    }
    else 
    {
        task->state = STATE_WAITING_FOR_RELEASE;
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        task->backlog -= 1;
        task->state = STATE_RELEASED;
        task->last_release = xTaskGetTickCount();
    }

}

void mcm_release_job(const uint8_t task_id)
{
    uint8_t *backlog = &mcm_tasks[task_id].backlog;

    if (*backlog == BACKLOG_MAX)
    {
        MCM_LOGW("Trying to release job of task %d but its backlog reached max. Not releasing it.", task_id);
    }
    else
    {
        *backlog += 1;
        xTaskNotifyGive(task_handles[task_id]);
    }
}

void mcm_clean_backlog(const uint8_t task_id)
{
    MCM_LOGI("Cleaning backlog of task %d", task_id);

    ulTaskNotifyValueClear(task_handles[task_id], 0xFFFFFFFF);
    mcm_tasks[task_id].backlog = 0;
}

void mcm_change_parameters(const uint8_t task_id, const uint8_t mode_id)
{
    TaskHandle_t task_handle = task_handles[task_id];
    TimerHandle_t timer_handle = timer_handles[task_id];
    const mcm_task_params_t *params = &modes[mode_id].tasks[task_id].parameters;

    vTaskPrioritySet(task_handle, params->priority);
    xTimerChangePeriod(timer_handle, pdMS_TO_TICKS(params->period), 0);
}

TickType_t mcm_calculate_offset_delay(const uint16_t offset, const TickType_t offset_reference)
{
    // release at offset_reference + offset
    TickType_t release_timestamp = offset_reference + pdMS_TO_TICKS(offset);
    TickType_t current_time = xTaskGetTickCount();

    // delay the task for the difference between the timestamp and the actual time
    if ((int32_t)(release_timestamp - current_time) <= 0)
        return 0;

    return release_timestamp - current_time;
}

void callback_offset_timer(TimerHandle_t xTimer)
{

    mcm_transition_task_t *task = (mcm_transition_task_t *)pvTimerGetTimerID(xTimer);

    // here a notification is sent because we want to execute the job just
    // after the delay. If the job is not released here, the timer will
    // change period AND reset itself
    mcm_release_job(task->task_id);

    mcm_perform_action(task->task_id, task->primitives.action, transitions[task->transition_id].dest_mode);

    // TODO: right now using auto-deletion. I could use an array of offset
    // timers for each task and recycle them by, instead of creating and
    // deleting them every time, just change their period. It is faster as
    // no heap allocations are done at runtime. Will do later.
    xTimerDelete(xTimer, 0);
}

void task_timer_callback(TimerHandle_t xTimer)
{
    uint8_t task_id = (uint8_t)(uintptr_t)pvTimerGetTimerID(xTimer);
    mcm_release_job(task_id);
}
