#include <stdio.h>
#include <stdlib.h>

#include "freertos/idf_additions.h"
#include "../include/tasks.h"
#include "../include/dict.h"

/*
 * A function that creates and fills a data structure that holds what tasks are active in each mode and their parameters (tfParams included).
 * */
void fill_task_info(){ // TODO: pass a HashTable_t argument for which table to fill
    // mcmgr checks
    //  - which is the destination mode
    //  - check (maybe said in the transition table) what changes in each task (active->suspended, active->active witch diff params, stays the same) for the transition
    //  - checks how the change has to be done for the specific transition (immediately suspend current tasks, let them run until finish...)
    //  - check if the change has been done as expected *(for later)
    //
    // # First make a prototype of the logic of the function. Polish afterwards with the code generation.
    //
    // prototype: DS indexed by mode, containing in each "row" a struct with info about the tasks that should be active in the specific mode
    //  -> gets filled one by one, when the tasks are being created (must be after creating the task because it stores arg_values and task_handle)
    //  -> I dont know if the user needs to give an id to each task. Maybe the DS has to be filled in some kind of order, or maybe some specific
    //      task has to be addressed for some reason, or (most likely) the transition function needs to address some specific tasks because of
    //      the way the transitions have to be made. Not an issue for this DS but also not a problem to implement in the future if its needed.
    //      A different kind of DS may have to be built here.
    //
    // active_tasks_in_mode() -> array of structs (active tasks), structs having: task_handle; pointer_to_args;
    // |MODE|   |active_task_data| |active_task_data| |...|
    //
    // dictionary/hash_table: key (mode_id):value (array of structs)
    

}

/*
 * A function that ...
 * */
void create_transition_info(){

}

void mcinit(){
    // Data structure holding information about the active tasks in each mode
    dict_create();
    // Create the tasks and fill the Data structure with the task information
    fill_task_info();


    /*
    // TODO: pull the task parameters from a data structure done in other function
    puts("\nInitializing all tasks");


    int *period = malloc(sizeof(int));
    *period = 1000;
    TaskHandle_t printStringHandle = NULL;
    xTaskCreate(taskPrintString_u, "Print String", 2048, period, 1, &printStringHandle);
    // TODO: function that inserts this in the task info data structure, only if the creation succeeded. handle errors and return values.


    int *increment = malloc(sizeof(int));
    *increment = 15;
    xTaskCreate(taskPrintCounter_u, "Print counter", 2048, increment, 1, NULL);
    */
}

