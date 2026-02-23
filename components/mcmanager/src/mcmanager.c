#include "mcmanager.h"
#include "esp_rom_sys.h"
#include "freertos/projdefs.h"
#include "gen_data.h"

#include "freertos/idf_additions.h"
#include "portmacro.h"
#include "stdlib.h"
#include "esp_log.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>



// Component tag for ESP-IDF logging
static const char TAG[] = "MCMANAGER";

// Variable that keeps track of the current active mode
static uint8_t current_mode;

// Variable that records the mcr_instant of the most recent mode change request. Useful for software timers for the offsetmcr guard.
static TickType_t mcr_instant = 0;






/*
 *
 *
*/
void initial_setup(){
    // NOTE: can create the timers directly with the initial values and skip part of this setup

    current_mode = MODE_INIT;

    // set the period and priority of the timers (task periods) and tasks
    for (int i = 0; i < modes[MODE_INIT].n_tasks; i++) {

        mode_task_t init_task_i = modes[MODE_INIT].tasks[i];
        
        xTimerChangePeriod( timer_handles[ init_task_i.id ],  pdMS_TO_TICKS(init_task_i.parameters.period),   0);
        vTaskPrioritySet(task_handles[ init_task_i.id ],   init_task_i.parameters.priority  );

    }

    // start the timers
    for (int i = 0; i < modes[MODE_INIT].n_tasks; i++) {

        mode_task_t init_task_i = modes[MODE_INIT].tasks[i];

        printf("starting task %d timer\n", i);
        xTimerStart(timer_handles[ init_task_i.id ], 0);

    }

}


/*
 *
 * 
*/
void mc_request(uint8_t target_mode){

    // calculate the mcr instant just after receiving the mc request
    mcr_instant = xTaskGetTickCount();

    // fetch the transition that corresponds to the 'source -> dest' mode pair
    uint8_t current_trans_id = mode_transitions[current_mode][target_mode];

    //// does the transition exist?
    if (current_trans_id == NO_TRANSITION)
        abort();

    // perform transition (state-based)
    mcm_perform_transition(current_trans_id);
    
    // after all tasks have been handled correctly, change the current mode and resume the scheduler
    current_mode = target_mode;

    ESP_LOGI(TAG, "Mode change successfully performed!");

}



/*
 *
 *
*/
void mcm_perform_transition(uint8_t trans_id){

    // stop all taskset timers (all active tasks); we dont want new jobs releasing while processing.
    for (int i = 0; i < transitions[trans_id].taskset_size; i++) {
        uint8_t task_id = transitions[trans_id].taskset[i].task_id;
        xTimerStop(timer_handles[task_id], 0);
    }

    for (int i = 0; i < transitions[trans_id].taskset_size; i++) {

        const trans_task_t *task = &transitions[trans_id].taskset[i];

        switch (task->primitives.guard) {

            case GUARD_TRUE:
            case GUARD_NONE:

                // if it was already waiting (with all its jobs finished or inactive), perform action
                if (mcm_tasks[task->task_id].state == STATE_WAITING_FOR_RELEASE) {
                    perform_action(task->task_id, task->primitives.action, transitions[task->transition_id].dest_mode);
                    break;
                }
                // if it was executing, clean its backlog to 1 to let the running job finish, and perform action
                else if (mcm_tasks[task->task_id].state == STATE_RELEASED) {
                    mcm_clean_backlog(task->task_id);
                    perform_action(task->task_id, task->primitives.action, transitions[task->transition_id].dest_mode);
                    break;
                }
                // this should not happen
                else{
                    abort();
                }

                break;

            // the code is the same
            case GUARD_BACKLOG_ZERO:
            case GUARD_BACKLOG_GLOBAL:

                // if the task is already waiting, it means it finished executing and is waiting for its period or it's a new task, so we can release it instantly
                if (mcm_tasks[task->task_id].state == STATE_WAITING_FOR_RELEASE) {
                    perform_action(task->task_id, task->primitives.action, transitions[task->transition_id].dest_mode);
                }
                // if the task is released it means that at the mcr instant it has backlog >= 1, so we have set its state to waiting and check back again
                else if (mcm_tasks[task->task_id].state == STATE_RELEASED) {
                    if (task->primitives.guard == GUARD_BACKLOG_ZERO)
                        mcm_tasks[task->task_id].state = STATE_WAITING_FOR_BACKLOG_Z;
                    else
                        mcm_tasks[task->task_id].state = STATE_WAITING_FOR_BACKLOG_G;

                }
                // this should not happen
                else {
                    abort();
                    // TODO: maybe if an mcr happens while some task is waiting for its timer we have to take other actions.
                }

                break;

            // almost the same code
            case GUARD_OFFSETMCR:
            case GUARD_OFFSETLR:

                TickType_t delay = 0;

                if (task->primitives.guard == GUARD_OFFSETMCR)
                    delay = mcm_calculate_offset_delay(task->primitives.guard_value, mcr_instant);
                else
                    delay = mcm_calculate_offset_delay(task->primitives.guard_value, mcm_tasks[task->task_id].last_release);


                // if already waiting, create the one-shot timer with the calculated offset (done after)
                // it it's executing, clean its backlog to let the last job finish and create the one-shot timer
                if (mcm_tasks[task->task_id].state == STATE_RELEASED) {
                    mcm_clean_backlog(task->task_id);
                }


                // if the difference is 0 or less, release instantly. 
                if (delay <= 0) {
                    perform_action(task->task_id, task->primitives.action, transitions[task->transition_id].dest_mode);
                    break;
                }

                // creating the timer for the job to be relased after the lr delay
                TimerHandle_t offset_timer_handle = xTimerCreate("offsetlr_timer", delay, pdFALSE, (void *)task, callback_offset_timer);

                // releases the task after the lr_delay
                xTimerStart(offset_timer_handle, 0);

                // set the new state
                mcm_tasks[task->task_id].state = STATE_WAITING_FOR_OFFSETLR;

                break;

            default:
                abort();
                break;

        }

    }

    // the way the loop exits is, it loops through all tasks and if it is "handled", it adds one to the count. If count == n_tasks, it exits. There may be an issue where the loop releases the last
    // task but its state hasn't changed yet, so it has to loop once more. Probably a bad design but may work.
    uint8_t handled_tasks;

    // loop until all tasks are handled
    while (1) {

        handled_tasks = 0;
        
        // loop through all tasks again, but only cares for the backlog states
        for (int i = 0; i < transitions[trans_id].taskset_size; i++) {

            const trans_task_t *task = &transitions[trans_id].taskset[i];
            
            // chooses sequentially
            switch (mcm_tasks[task->task_id].state) {

                case STATE_WAITING_FOR_BACKLOG_G:

                    uint16_t global_backlog = 0;

                    // sum all backlogs of the active tasks
                    for (uint8_t i = 0; i < modes[current_mode].n_tasks; i++) {
                        global_backlog += mcm_tasks[ modes[current_mode].tasks[i].id ].backlog;
                    }

                    // apply action if the global backlog is 0 and is not running
                    if (global_backlog == 0 && mcm_tasks[task->task_id].state == STATE_WAITING_FOR_RELEASE) {

                        // TODO: stop the timer?
                        perform_action(task->task_id, task->primitives.action, transitions[task->transition_id].dest_mode);

                    }
                    // if it's not 0, check in next iteration

                    break;

                case STATE_WAITING_FOR_BACKLOG_Z:

                    // apply action if the task backlog is 0 and is not running
                    if ( mcm_tasks[task->task_id].backlog == 0 && mcm_tasks[task->task_id].state == STATE_WAITING_FOR_RELEASE) {
                        // TODO: add a macro for this? it repeats in the whole switch statement
                        perform_action(task->task_id, task->primitives.action, transitions[task->transition_id].dest_mode);

                    }
                    // if it's not 0, check in next iteration

                    break;
                    
                // if not backlog state, check next
                default:
                    handled_tasks++;
                    break;

            }

        }

        // if all tasks have been handled, exit loop
        if (handled_tasks == transitions[trans_id].taskset_size)
            break;

        // wait 5ms, this blocks the main task, i dont know if this will give problems. 
        // Theoretically, until this loops finishes with all tasks, there shouldn't be an additional mcr 
        // TODO: add a timeout
        vTaskDelay(pdMS_TO_TICKS(5));

    }

}

/*
 * 
 *
*/
void perform_action(uint8_t task_id, uint8_t action, uint8_t mode_id){
    TaskHandle_t task_handle = task_handles[task_id];
    TimerHandle_t timer_handle = timer_handles[task_id];

    switch (action) {
        case ACTION_NONE:
        case ACTION_CONTINUE:
            // start timer again, as they are stopped during the mcr()
            xTimerStart(timer_handle, 0);
            break;

        case ACTION_SUSPEND:
            // stop (and reset?) its timer
            // xTimerReset(timer_handle, 0);
            xTimerStop(timer_handle, 0);
            
            break;

        case ACTION_UPDATE:
            // just change the parameters.
            // priority
            vTaskPrioritySet(task_handle, modes[mode_id].tasks[task_id].parameters.priority);
            // period
            xTimerChangePeriod(timer_handle, pdMS_TO_TICKS(modes[mode_id].tasks[task_id].parameters.period), 0);

            break;

        case ACTION_RELEASE:
            // FIRST change parameters
            // priority
            vTaskPrioritySet(task_handle, modes[mode_id].tasks[task_id].parameters.priority);
            // period
            xTimerChangePeriod(timer_handle, pdMS_TO_TICKS(modes[mode_id].tasks[task_id].parameters.period), 0);

            mcm_release_job(task_id);

            break;

        default:
            abort();
            break;

    }

}



/*
 *
 *
*/
void mcm_wait_for_release(uint8_t task_id){

    // if execution reaches here, it means the task is waiting for its release
    mcm_tasks[task_id].state = STATE_WAITING_FOR_RELEASE;

    // jobs wait for their activation here
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    // one activation/release means one less job in the backlog
    mcm_tasks[task_id].backlog -= 1;

    // after the task takes its notification, it means it is not waiting anymore
    mcm_tasks[task_id].state = STATE_RELEASED;

    // update the last_release and last_period values values only once we know they have been released to avoid updating them when the task is notified instead of when it's released
    // ---> just after the task takes the notification.
    mcm_tasks[task_id].last_release = xTaskGetTickCount();
    // last period = timer's period = task's period in the current mode
    mcm_tasks[task_id].last_period = modes[current_mode].tasks[task_id].parameters.period;

}


/*
 *
 *
*/
void mcm_release_job(uint8_t task_id){
    // FIRST Notify (release) the task

    // increment the value of the internal backlog of the mcm_task. This will be reduced by 1 each time the task calls xTaskNotifyTake inside wfa()
    // Here I'm supposing:
    //  - tasks can be notified more than once before they finish executing their job (creating a backlog), so a task will never TAKE unless it's GIVEN before,
    //  so backlog should never be < 0.
    //  - If we always wait until backlog = 0, the program will never change mode unless all backlogs are 0, so backlog global will always be = 0 for the mode change to happen.
    //  
    //  I increment before the call because if I call then increment, the decrement in wfa() may occur before the increment and cause backlog to be 255 for a moment.
    //  Maybe if something happens between the increment and the notification, it could signal a backlog increment when there should not be and the program may get stuck
    //  in an infinite waiting time (or a timeout), but thay may be very unlikely (?)
    
    // releasing doesnt mean instantly executing, but adding a job to its backlog
    mcm_tasks[task_id].backlog += 1;
    xTaskNotifyGive( task_handles[ task_id ] );

}

/*
 *
 *
*/
void mcm_clean_backlog(uint8_t task_id){

    ulTaskNotifyValueClear(task_handles[task_id], 0xFFFFFFFF);
    mcm_tasks[task_id].backlog = 0;

}

/*
 *
 *
*/
TickType_t mcm_calculate_offset_delay(uint16_t offset, TickType_t offset_reference){

    // release at offset timestamp + offset
    TickType_t release_timestamp = offset_reference  +  pdMS_TO_TICKS(offset); 

    // delay the task for the difference between the timestamp and the actual time.
    TickType_t delay = release_timestamp - xTaskGetTickCount();

    return delay;
}


/*
 * Timer used to release the jobs that have an offset type guard during a mode change. Using a timer to avoid blocking the main calling task.
 * Using the supposed timer id xTimer as a means to pass a trans_task_t pointer argument. It may have consequences.
 *
*/
void callback_offset_timer( TimerHandle_t xTimer ){

    trans_task_t *task = (trans_task_t *) pvTimerGetTimerID(xTimer);

    // here a notification is sent because we want to execute the job just after the delay. If the job is not released here, the timer will change
    // period AND reset itself, so the job cycle would be (execution_before_mcr -> mcr = period_time_elapsed + new_period) instead of ( |execution_after_delay| -> new_period )
    mcm_release_job(task->task_id);

    perform_action(task->task_id, task->primitives.action, transitions[task->transition_id].dest_mode);


    // TODO: right now using auto-deletion. I could use an array of offset timers for each task and recycle them by, instead of creating and deleting them every time, just change their
    // period. It is faster as no heap allocations are done at runtime. Will do later.
    xTimerDelete(xTimer, 0);
}


/*
 * xTimer identifies the timer, which has the same value as the id of the task it is assigned to: timer id 0 <-> task id 0
 *
*/
void task_timer_callback( TimerHandle_t xTimer ){

    // pvTimerGetTimerID returns a void pointer, so in order to perform an integer casting (or any integer operations) to a pointer,
    // the pointer has to be casted to an integer value that can hold the pointer's value beforehand.
    uint8_t task_id = (uint8_t)(uintptr_t)pvTimerGetTimerID(xTimer);

    mcm_release_job(task_id);



    // NOTE: I am also supposing that the only way a task can get overcharged with releases (notifications) is because during its execution, something blocked the task and it 
    // could not execute completely before its period concluded. Right now, if a task gets notified after it has finished but before its period has concluded, a new job will
    // be released instantly because it is waiting in wfa() and only cares about the notifications, not about the period. This may be good or bad, I don't know.
    //
    // NOTE: I have to set a limit for the backlog, as if the system has to wait for all task to finish and some task somehow got 1000 notifications, the system cannot assume
    // waiting for 1000 executions of just a single job

}
