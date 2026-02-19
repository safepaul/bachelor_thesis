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
    // XXX:
    // I have to turn off the timers when I change the mode
    // - With xTimerChangePeriod (+ xTimerReset)
    // - I think with xTimerStop, the timer stops mid-counting so with xTimerReset I can reset it back to 0
    //
    // NOTE: can create the timers directly with the initial values and skip part of this setup

    current_mode = MODE_INIT;


    // stop scheduler to avoid some tasks starting before others during processing
    // vTaskSuspendAll();


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


    // xTaskResumeAll();

}


/*
 *
 * 
*/
void mc_request(uint8_t target_mode){

    // calculate the mcr instant just after receiving the mc request
    mcr_instant = xTaskGetTickCount();

    printf("-------------- MCR INSTANT: %lu\n\n", mcr_instant);

    // stop the scheduler so that the states of the tasks don't change while the program processes the transition of the tasks that come before (or even the state snapshot)
    // this must be the first call to avoid a higher priority task taking over the cpu (not too sure about this, mcr_instant gets delayed...)
    // vTaskSuspendAll();


    // fetch the transition that corresponds to the 'source -> dest' mode pair
    uint8_t current_trans_id = mode_transitions[current_mode][target_mode];

    //// does the transition exist?
    if (current_trans_id == NO_TRANSITION)
        abort();

    const transition_t *current_trans = &transitions[ current_trans_id ];


    // perform the task changes
    for (int i = 0; i < current_trans->taskset_size; i++) {
        apply_primitive(&current_trans->taskset[i]);
    }

    
    // after all tasks have been handled correctly, change the current mode and resume the scheduler
    current_mode = target_mode;

    // xTaskResumeAll();

    ESP_LOGI(TAG, "Mode change successfully performed!");

}



/*
 *
 *
*/
void apply_primitive(const trans_task_t *task){


    // NOTE: I removed the distinction between jobs executing or not executing because as I also removed the distinction between pending and other job states, just by knowing the type of task we know they can only be in one state:
    //  - if the task is of type NEW: so it can only be in a non-executing state (suspended)
    //  - if the task is of type UNCHANGED, CHANGED, or OLD: the task HAS TO BE already in an executing state (ready, blocked, running).
    // The program loses granularity but for simplicity I'll leave it like this, adding features will be fine.
    // The program itself doesnt care about the type of the task, it just cares about performing the changes. The user that generates the YAML must take care of that. If a task is of type OLD, it should follow its corresponding "ACTION_ABORT" action or similar
    // although this hurts program readability. May complicate things if I have to assert in the generator or check for errors. I'll add that later if necessary.


    TaskHandle_t handle = task_handles[task->task_id];
    uint8_t action = task->primitives.action;
    uint8_t guard = task->primitives.guard;
    int16_t guard_value = task->primitives.guard_value;

    printf("task id: %d\nhandle: %p\naction: %d\nguard: %d\nguard_value: %d\n", task->task_id, handle, action, guard, guard_value);

    switch (guard) {

        case GUARD_TRUE:
        case GUARD_NONE:

            perform_action(task->task_id, action, transitions[task->transition_id].dest_mode);

            break;

        case GUARD_OFFSETMCR:
            // NOTE: because the scheduler is stopped, we dont have to worry about taking into account the processing time of the tasks that come before others sequentially
            
            // stop the timer, because after the mcr is done and the scheduler starts again, the offset may be greater than the last_period, so while the action is
            // delayed, executions of the previous timer will occur. The timer will be restarted when calling xTimerChangePeriod or reset in the actions section.
            xTimerStop(timer_handles[task->task_id], 0);

            // calculate release timestamp
            TickType_t mcr_release_timestamp = mcr_instant  +  pdMS_TO_TICKS(guard_value); 

            // delay the task for the difference between the timestamp and the actual time.
            TickType_t mcr_delay = mcr_release_timestamp - xTaskGetTickCount();

            // creating the timer for the job to be relased after the mcr delay
            TimerHandle_t omcr_timer_handle = xTimerCreate("offsetmcr_timer", mcr_delay, pdFALSE, (void *)task, callback_offset_timer);


            // if the difference is 0 or less, release instantly. this could happen if the offset is smaller than the old period ->
            // last_period = 100, last_release = 1000, mcr_instant at 1060, offset = 30:: offset + lr = 1030, mcr_instant 1060, difference = -30
            if (mcr_delay <= 0) {
                xTimerStart(omcr_timer_handle, 0);
                break;
            }

            // releases the task after the lr_delay
            xTimerStart(omcr_timer_handle, 0);
            break;

            break;

        case GUARD_BACKLOG_ZERO:

            TickType_t delay_local = 0;

            while (1) {

                // IF task backlog = 0, break from the loop and apply action
                if ( mcm_tasks[task->task_id].backlog == 0 )
                    break;
                // ELSE wait until it finishes, or timeout and clean
                else {

                    // TODO: add a timeout routine and clean whatever is needed
                    if (delay_local >= 40) {
                        abort();
                    }
                    

                    // try again in 10ms XXX: is this too much waiting time?
                    vTaskDelay(pdMS_TO_TICKS(10));

                    delay_local = delay_local + 10;
                    
                }
                
            }

            perform_action(task->task_id, action, transitions[task->transition_id].dest_mode);



            

            break;


        case GUARD_BACKLOG_GLOBAL:
            // Check if the sum of the backlogs of all active tasks is 0 

            TickType_t delay_global = 0;

            while (1) {

                uint16_t global_backlog = 0;

                // sum all backlogs of the active tasks
                for (uint8_t i = 0; i < modes[current_mode].n_tasks; i++) {
                    global_backlog += mcm_tasks[ modes[current_mode].tasks[i].id ].backlog;
                }

                // IF global backlog = 0, break from the loop and apply action
                if ( global_backlog == 0 )
                    break;
                // ELSE wait until it finishes, or timeout and clean
                else {

                    // TODO: add a timeout routine and clean whatever is needed
                    if (delay_global >= 200) {  //XXX: is this too much waiting time?
                        abort();
                    }

                    // try again in 20ms XXX: is this too much waiting time?
                    vTaskDelay(pdMS_TO_TICKS(20));

                    delay_global = delay_global + 10;
                    
                }
                
            }

            // TODO: add a macro for this? it repeats in the whole switch statement
            perform_action(task->task_id, action, transitions[task->transition_id].dest_mode);

            break;


        case GUARD_OFFSETLR:
            // meaning:
            //  - apply action X milliseconds after its last release
            //
            // use cases:
            //  - for changed or unchanged tasks:
            //      - as the last_release will be known, this is used for jobs that changed their parameters and want to be executed instantly after the mode change, or
            //      maybe having their first execution after the mode change to have the new period, instead of their first execution after the mode change being the old period.
            //      If old_period was 50, last_release was 1000 and new period is 200, they want the new execution at 1200 instead of at 1050.
            //
            //  - for new tasks: (probably wont be used here)
            //      - ACTION_RELEASE. last release of new tasks will be (CURRENT_TICKS mod LAST_RELEASE), meaning, the last time it would have been released
            //      if it had been active all the time. And add the offset to that so release in (ticksToMs(last_release) + offset) milliseconds.
            //
            //  - for old tasks (should not be used with old tasks, just with ACTION_NONE)

            // stop the timer, because after the mcr is done and the scheduler starts again, the offset may be greater than the last_period, so while the action is
            // delayed, executions of the previous timer will occur. The timer will be restarted when calling xTimerChangePeriod or reset in the actions section.
            xTimerStop(timer_handles[task->task_id], 0);


            // TODO: add support for new tasks (calculate theoretical last release with modulo) (probably unneccessary)
            //
            // release at last_release + offset
            TickType_t lr_release_timestamp = mcm_tasks[task->task_id].last_release  +  pdMS_TO_TICKS(guard_value); 

            // delay the task for the difference between the timestamp and the actual time.
            TickType_t lr_delay = lr_release_timestamp - xTaskGetTickCount();


            // creating the timer for the job to be relased after the lr delay
            TimerHandle_t olr_timer_handle = xTimerCreate("offsetlr_timer", lr_delay, pdFALSE, (void *)task, callback_offset_timer);


            // if the difference is 0 or less, release instantly. this could happen if the offset is smaller than the old period ->
            // last_period = 100, last_release = 1000, mcr_instant at 1060, offset = 30:: offset + lr = 1030, mcr_instant 1060, difference = -30
            if (lr_delay <= 0) {
                xTimerStart(olr_timer_handle, 0);
                break;
            }

            // releases the task after the lr_delay
            xTimerStart(olr_timer_handle, 0);
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
void perform_action(uint8_t task_id, uint8_t action, uint8_t mode_id){
    TaskHandle_t task_handle = task_handles[task_id];
    TimerHandle_t timer_handle = timer_handles[task_id];

    // TODO: Check if parameter changes are needed and perform that too. Maybe adding a task type field avoids having to check if the task is C-U-N-O and makes it easier to perform the actions
    switch (action) {
        case ACTION_NONE:
            // do nothing?
            break;

        case ACTION_CONTINUE:
            // do nothing?
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
void mcm_wait_for_activation(uint8_t task_id){

    // if execution reaches here, it means the task is waiting for its activation
    mcm_tasks[task_id].is_waiting = true;


    /*
     * ???: what conditions have to be met to release a job that called wfa()?
     *  - 1. the task should be active in the current mode.
     *  - 2. on first release, they depend on their guard. After that, they depend on their period, and maybe on other parameters
     *
     *
     *  all tasks call ulTaskNotifyTake, only the active ones get notified?
     *   - [?] do I use task states for anything? -> ulTaskNotifyTake will leave them in the blocked state
     *
    */


    // jobs wait for their activation here
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    // one activation/release means one less job in the backlog
    mcm_tasks[task_id].backlog -= 1;

    // after the task takes its notification, it means it is not waiting anymore
    mcm_tasks[task_id].is_waiting = false;

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
    mcm_tasks[task_id].backlog += 1;
    xTaskNotifyGive( task_handles[ task_id ] );

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
