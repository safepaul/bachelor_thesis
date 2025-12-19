#include "../include/mcmanager.h"

#include "freertos/idf_additions.h"
#include "stdlib.h"
#include "esp_log.h"
#include <stdint.h>
#include <stdio.h>


#define MAX_TRANSITIONS (uint16_t)   64
#define MAX_TASKS       (uint8_t)    32
#define MAX_TRANS_TASKS (uint16_t)   MAX_TRANSITIONS * MAX_TASKS 


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


void mcm_create_transition(uint32_t id, uint16_t source, uint16_t dest){
    
    // TODO: safety checks: same src and dst...

    // bounds checking
    // XXX: dont quite like this. I am not assigning the ids myself for now, so maybe no pointers for the transition array...
    if((transitions_tail - transitions) >= MAX_TRANSITIONS){
        ESP_LOGE(TAG, "Reached maximum transitions allowed (%d). Aborting.", MAX_TRANSITIONS);
        abort();
    }

    // NOTE: if it's necessary that the program gives the id's of the transitions instead of the specification file, I think
    // it can be done by trusting the order of insertion. For example, create and instert the transition to the array: the transition
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
