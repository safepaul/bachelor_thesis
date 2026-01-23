#include "mcmanager.h"
#include "gen_data.h"

#include "freertos/idf_additions.h"
#include "portmacro.h"
#include "stdlib.h"
#include "esp_log.h"
#include <stdint.h>
#include <stdio.h>



// Component tag for ESP-IDF logging
static const char TAG[] = "MCMANAGER";

static uint8_t current_mode;



/*
 * 
 *
*/
void perform_action(TaskHandle_t handle, uint8_t action){

    // TODO: Check parameter changes and perform that too. Maybe adding a task type field avoids having to check if the task is C-U-N-O and makes it easier to perform the actions
    switch (action) {
        case ACTION_NONE:
            ESP_LOGI(TAG, "Performing action: NONE");
            break;

        case ACTION_CONTINUE:
            ESP_LOGI(TAG, "Performing action: CONTINUE");

            break;

        case ACTION_ABORT:

            break;

        case ACTION_UPDATE:

            break;

        // New job, state was blocked or suspended
        case ACTION_RELEASE:
            ESP_LOGI(TAG, "Performing action: RELEASE");
            vTaskResume(handle);
            break;

        default:
            ESP_LOGE(TAG, "Trying to perform non-existent action. Aborting...");
            abort();
            break;

    }

}



// The type of the task doesn't matter for the change
void handle_task(task_trans_data_t task, eTaskState state){

    // Here I understand that when the MCR is triggered, I should write the logic for moving a task from its Blocked/Suspended state to a Ready state because tasks are not going to wake themselves up.
    // So maybe I have to check for Suspended and Blocked tasks that are in the transition taskset and work with that



    // check the state of the task

    // New
    if (state == eBlocked || state == eSuspended) {

        // Evaluate guard
        // NOTE: No guards for now

        // Apply action
        perform_action(task_handles[task.task_id], task.primitives.anew);



    // Executing
    } else if (state == eRunning || state == eReady) {



    } else {

            ESP_LOGE(TAG, "State of task is not one of the known states");
            abort();

    }


}


/*
 * 
It implements all the mode-change primitives, which
include various operations such as evaluating the guards for
each job (job type) and performing the corresponding actions
(e.g., remove some pending jobs from the run queue, disable
an old task, update the job/task parameters, release new jobs)
based on the guards. Upon a triggered MCR, it looks up
the corresponding transition in its MC data structures, scans
through all tasks involved in the transition, and performs the
actions defined for their jobs based on the action guards
 * */

void mc_request(uint8_t target_mode){
    // calculate the mcr instant here
    TickType_t mcr_instant = xTaskGetTickCount();

    // stop the scheduler so that the tasks' states don't change while the program processes the transition of the tasks that come before (or even the state snapshot)
    vTaskSuspendAll();

    // fetch the transition that corresponds ot the source -> dest mode pair
    transition_t current_trans = transitions[ mode_transitions[current_mode][target_mode] ];

    // XXX: maybe dont need this as the scheduler is stopped, fetch the state one by one
    // create a snapshot of the states of the tasks in the MCR: task state array
    eTaskState state_snapshot[N_TASKS];



    // FIXME: Under maintenance. After structural fixes are made, fix this. Loop instead of hardcode and whatever is necessary
    /*
    vTaskResume(task_info[5].handle);

    state_snapshot[2] = eTaskGetState(task_info[2].handle);
    state_snapshot[5] = eTaskGetState(task_info[5].handle);

    // 2. perform the changes (primitives) to the tasks in the taskset
    handle_task(taskset, state_snapshot[2]);
    handle_task(taskset + 1, state_snapshot[5]);
    */




    
    // after all tasks have been handled correctly, change the current mode and resume the scheduler
    current_mode = target_mode;

    xTaskResumeAll();

}


// TODO: I think there will be a function that sets up all the system before starting the tasks, things like setting up the initial mode and similar. I need this for testing for now.
/*
 *
 *
*/
void initial_setup(){

    current_mode = MODE_NONE;

}
