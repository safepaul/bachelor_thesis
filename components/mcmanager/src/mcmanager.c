#include "mcmanager.h"

#include "freertos/idf_additions.h"
#include "stdlib.h"
#include "esp_log.h"
#include <stdint.h>
#include <stdio.h>



// Component tag for ESP-IDF logging
static const char TAG[] = "MCMANAGER";

/*
    Array holding transition info for the MCManager to perform the right actions on the tasks of the transition taskset.
    A hard limit is set as dynamic data structures are not an option. Sorted by transition id.
    NOTE: make it a pointer array? Benefits vs drawbacks...
    When learning if this is a good change, think about how can I get the most value for time spent researching...
*/
static transition_t transitions[MAX_TRANSITIONS];

// Pointer to tail of transitions array in order to simply assign the transition id to the transition tasks
static transition_t *transitions_tail = transitions;

// Array holding all of the tasks of the transition taskset in every transition. Sorted by transition id.
static transition_task_t transition_tasks[MAX_TRANS_TASKS];

// Pointer to tail of transitions array in order to simply assign the taskset pointer to the transitions
static transition_task_t *trans_tasks_tail = transition_tasks;

// Array holding information about each task in order to start and manage them through the lifetime of the program
static task_info_t task_info[MAX_TASKS] = { 0 };


void mcm_create_transition(uint32_t id, uint16_t source, uint16_t dest){
    
    // TODO: safety checks: same src and dst...

    // bounds checking
    // XXX: dont quite like this. I am not assigning the ids myself for now, so maybe no pointers for the transition array...
    if((transitions_tail - transitions) >= MAX_TRANSITIONS){
        ESP_LOGE(TAG, "Reached maximum transitions allowed (%d). Aborting.", MAX_TRANSITIONS);
        abort();
    }

    // NOTE: if it's necessary that the program gives the id's of the transitions instead of the specification file, I think
    // it can be done by trusting the order of insertion. For example, create and insert the transition to the array: the transition
    // id is known but the generator doesn't know it, so when the trans_tasks are created, the id that they are assigned to is the
    // one of the transition at the end of the transition array (I dont know if I can guarantee that the order will be right).
    // Can be done with (transitions + ntrans).

    // TODO: assign taskset pointer
    // TODO: loop the tasks starting at the taskset pointer until the trans_id of the trans_task is not equal to the one of the current transition

    // set the taskset pointer to the tail of the transition tasks list so it's the beginning of the tasks that will be added
    // as the transition is created first and the tasks are added afterwards
    // 
    // insert the transition to the next empty spot in the array and update the tail
    *transitions_tail = (transition_t)
        { .taskset = trans_tasks_tail,
          .id = id,
          .source = source,
          .dest = dest};

    transitions_tail++;



}

void mcm_add_task_to_transition(uint16_t task_id, uint16_t transition_id, task_params_t params, job_primitives_t primitives){

    // TODO: safety checks.

    // bounds checking
    if((trans_tasks_tail - transition_tasks) >= MAX_TRANS_TASKS){
        ESP_LOGE(TAG, "Reached maximum transition tasks allowed (%d). Aborting.", MAX_TRANS_TASKS);
        abort();
    }

    *trans_tasks_tail = (transition_task_t){    .task_id = task_id,
                                                .transition_id = transition_id,
                                                .params = params,
                                                .primitives = primitives
                                           };

    trans_tasks_tail++;

}









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
void handle_task(transition_task_t task, eTaskState state){

    // Here I understand that when the MCR is triggered, I should write the logic for moving a task from its Blocked/Suspended state to a Ready state because tasks are not going to wake themselves up.
    // So maybe I have to check for Suspended and Blocked tasks that are in the transition taskset and work with that



    // check the state of the task

    // New
    if (state == eBlocked || state == eSuspended) {

        // Evaluate guard
        // NOTE: No guards for now

        // Apply action
        perform_action(task_info[task.task_id].handle, task.primitives.anew);



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

void mc_request(uint16_t tran_id){

    // 0. fetch transition taskset for an easy lookup
    transition_task_t *taskset = transitions[tran_id].taskset;

    // XXX: I have to know the state of the tasks at the MCR instant.
    ///////// How can a mode change be reliable time-wise if the tasks are checked at the MCR instant and they should all be handled at the same time but this is single-threaded? The state of the tasks may change in the time the tasks before are being handled...
    //
    // XXX: I have to know how many tasks there are beforehand, either during generation or calculating it during initialization. I have to fix the way I store the tasks. Addressing them by order of storage and determining the size of the taskset when the transition_id associated to it changes may not be a good idea.
    

    // TODO: Done poorly just to see if it works. Fix it
    // 
    // 1. create a snapshot of the states of the tasks in the MCR: task state array
    eTaskState state_snapshot[MAX_TASKS];

    vTaskResume(task_info[5].handle);

    state_snapshot[2] = eTaskGetState(task_info[2].handle);
    state_snapshot[5] = eTaskGetState(task_info[5].handle);

    printf("\n[MCMGR] State of task %d: %d\n", 2, state_snapshot[2]);
    printf("[MCMGR] State of task %d: %d\n", 5, state_snapshot[5]);

    // 2. perform the changes (primitives) to the tasks in the taskset
    handle_task(taskset, state_snapshot[2]);
    handle_task(taskset + 1, state_snapshot[5]);

}


/*
 *
 *
*/
void mcm_add_task_info(uint8_t task_id, TaskFunction_t func, char* name){

    task_info[task_id] = (task_info_t) { .func = func, .name = name, .handle = NULL };

}

/*
 *
 *
*/
void spawn_tasks(){

    // TODO: stop scheduler, create tasks suspended then start scheduler
    // TODO: add a loop

    // stop scheduler
    vTaskSuspendAll();

    // FIXME: Right now this loops from 0 to MAX_TASKS. It can be done better knowing the size before or some other way
    for (int i = 0; i < MAX_TASKS; i++) {   

        if (task_info[i].name != 0) {

            task_info_t current_task = task_info[i];
            TaskHandle_t current_handle = NULL;

            // NOTE: priority not necessary now but before starting everything all tasks will have to be calibrated
            xTaskCreate(current_task.func,
                        current_task.name,
                        2048,
                        NULL,
                        1,
                        &current_handle);
            vTaskSuspend(current_handle);

            task_info[i].handle = current_handle;

            
        }

    }

    // NOTE: resume scheduler but tasks still suspended until the app is completely set up. then must set up the initial mode.
    xTaskResumeAll();

}








void debug_print_transition_table(){

    for (int i = 0; i < (transitions_tail - transitions); i++) {

        printf("Transition id: %d, source: %d \n", i, transitions[i].source);
        printf("Transition id: %d, destination: %d \n", i, transitions[i].dest);

    }

}

void debug_print_trans_tasks_table(){

    printf("Transition TASK id: %d\n", transition_tasks[0].task_id);
    printf("Transition TASK period: %d\n", transition_tasks[0].params.period);
    printf("Transition TASK associated to transition id: %d\n", transition_tasks[0].transition_id);
    printf("Transition TASK action new: %d\n", transition_tasks[0].primitives.anew);
    printf("Transition TASK guard new: %d\n", transition_tasks[0].primitives.gnew);
    printf("Transition TASK guard new value: %d\n", transition_tasks[0].primitives.gnewval);

    printf("Transition TASK id: %d\n", transition_tasks[1].task_id);
    printf("Transition TASK period: %d\n", transition_tasks[1].params.period);
    printf("Transition TASK associated to transition id: %d\n", transition_tasks[1].transition_id);
    printf("Transition TASK action new: %d\n", transition_tasks[1].primitives.anew);
    printf("Transition TASK guard new: %d\n", transition_tasks[1].primitives.gnew);
    printf("Transition TASK guard new value: %d\n", transition_tasks[1].primitives.gnewval);

    // printf("Transition TASK id: %d\n", transition_tasks[2].task_id);
    // printf("Transition TASK period: %d\n", transition_tasks[2].params.period);
    // printf("Transition TASK associated to transition id: %d\n", transition_tasks[2].transition_id);

}

void debug_print_task_info_table(){

    printf("[DEBUG] info of task 2:    name: %s, function: %p, handle: %p\n", task_info[2].name, task_info[2].func, task_info[2].handle);
    printf("[DEBUG] info of task 5:    name: %s, function: %p, handle: %p\n", task_info[5].name, task_info[5].func, task_info[5].handle);

}


