#include "mcm_trans.h"
#include "mcm_types.h"
#include "gen_data.h"
#include "portmacro.h"




void mcm_trans_handle_guard_none(const mcm_transition_task_t *task, const uint8_t dest_mode_id)
{
    const uint8_t task_id = task->task_id;
    const mcm_action_t action = task->primitives.action;
    mcm_task_state_t *state = &mcm_tasks[task_id].state;

    // if it was already waiting (with all its jobs finished or inactive),
    // perform action
    if (*state == STATE_WAITING_FOR_RELEASE)
        mcm_perform_action(task_id, action, dest_mode_id);
    // if it was executing, clean its backlog to let the running job finish,
    // and perform action
    else if (*state == STATE_RELEASED)
    {
        mcm_clean_backlog(task_id);
        mcm_perform_action(task_id, action, dest_mode_id);
    }
    // this should not happen
    // TODO: maybe check its state and decide what to do depending on the state
    else
    {
        abort();
    }
}

void mcm_trans_handle_guard_offset(const mcm_transition_task_t *task, const uint8_t dest_mode_id, const TickType_t mcr_instant)
{
    const uint8_t task_id = task->task_id;
    const uint8_t guard = task->primitives.guard;
    const uint8_t guard_value = task->primitives.guard_value;
    const uint8_t action = task->primitives.action;
    const TickType_t last_release = mcm_tasks[task_id].last_release;
    mcm_task_state_t *state = &mcm_tasks[task_id].state;

    TickType_t delay = 0;

    if (guard == GUARD_OFFSETMCR)
        delay = mcm_calculate_offset_delay(guard_value, mcr_instant);
    else
        delay = mcm_calculate_offset_delay(guard_value, last_release);

    // if already waiting, create the one-shot timer with the calculated
    // offset (done after) it it's executing, clean its backlog to let the
    // last job finish and create the one-shot timer
    if (*state == STATE_RELEASED)
        mcm_clean_backlog(task_id);

    // if the difference is 0 or less, release instantly.
    if (delay == 0)
        mcm_perform_action(task_id, action, dest_mode_id);
    else
    {
        // creating the timer for the job to be relased after the lr delay
        TimerHandle_t offset_timer_handle = xTimerCreate("offsetlr_timer", delay, pdFALSE, (void *)task, callback_offset_timer);
        xTimerStart(offset_timer_handle, 0);
        *state = STATE_WAITING_FOR_OFFSETLR;
    }
}

void mcm_trans_handle_guard_backlog(const mcm_transition_task_t *task, const uint8_t dest_mode_id)
{
    const uint8_t task_id = task->task_id;
    const uint8_t action = task->primitives.action;
    const uint8_t guard = task->primitives.guard;
    mcm_task_state_t *state = &mcm_tasks[task_id].state;

    // if the task is already waiting, it means it finished executing and is
    // waiting for its period or it's a new task, so we can release it
    // instantly
    if (*state == STATE_WAITING_FOR_RELEASE)
        mcm_perform_action(task_id, action, dest_mode_id);

    // if the task is released it means that at the mcr instant it has backlog >= 1
    // so we have set its state to waiting and check back again
    else if (*state == STATE_RELEASED)
    {
        if (guard == GUARD_BACKLOG_ZERO)
            *state = STATE_WAITING_FOR_BACKLOG_Z;
        else
            *state = STATE_WAITING_FOR_BACKLOG_G;
    }
    // this should not happen
    else
    {
        abort();
        // TODO: maybe if an mcr happens while some task is waiting for
        // its timer we have to take other actions.
    }
}




void mcm_trans_poll_backlogs(const uint8_t trans_id, const uint8_t current_mode, const uint8_t guard)
{
    // loop until all tasks are handled
    while (1)
    {
        uint8_t handled_tasks = 0;

        // loop through all tasks again, but only cares for the backlog states
        for (int i = 0; i < transitions[trans_id].taskset_size; i++)
        {
            const mcm_transition_task_t *task = &transitions[trans_id].taskset[i];
            const mcm_task_state_t task_state = mcm_tasks[task->task_id].state;

            uint16_t backlog;
            if      (task_state == STATE_WAITING_FOR_BACKLOG_G)
                backlog = mcm_trans_calculate_global_backlog(current_mode);
            else if (task_state == STATE_WAITING_FOR_BACKLOG_Z) 
                backlog = mcm_tasks[task->task_id].backlog;
            else
                handled_tasks++;
        }

        // if all tasks have been handled, exit loop
        if (handled_tasks == transitions[trans_id].taskset_size)
            break;

        // TODO: add a timeout
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// --- static functions ---

static uint16_t mcm_trans_calculate_global_backlog(const uint8_t current_mode)
{
    uint16_t global_backlog = 0;
    for (uint8_t i = 0; i < modes[current_mode].n_tasks; i++)
        global_backlog += mcm_tasks[modes[current_mode].tasks[i].id].backlog;
    return global_backlog;
}
