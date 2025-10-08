#include <stdio.h>
#include <stdlib.h>

#include "freertos/idf_additions.h"
#include "../data/tasks.h"


// creates and starts all tasks, data structures and fills them...


// the task functions have to be supplied
// mcinit may create the tasks from functions in a task.c+task.h files, from
// a list of function names or similar

// idea: first fill the data structures with the task details then, for example
// if it is marked as an initial mode task, start it active, else, start it suspended

// for the demo app, make a real-time Data Structure visualizer

void ds_init(){
    // mcmgr checks
    //  - which is the destination mode
    //  - check (maybe said in the transition table) what changes in each task (active->suspended, active->active witch diff params, stays the same) for the transition
    //  - checks how the change has to be done for the specific transition (immediately suspend current tasks, let them run until finish...)
    //  - check if the change has been done as expected *(for later)
    // I need: 
    //  - what modes they are active on VS what tasks are active in each mode -> what is better?
    //  - what parameters and parameter values each task has in each mode they are active on (the shape of tfParams, a struct?) (a dedicate data struture for this?)
    //  - to be able to address the task in order to change the priorities and maybe sth else (record the task handle?)
    //  - to initialize the transition table *(for later)
    //  - (the stack size?)
    //  - (their priorities in each mode?)
    //  - (transition function?)
}

void initialize_tasks(){
    // TODO: pull the task parameters from a data structure done in other function
    puts("\nInitializing all tasks");

    int *period = malloc(sizeof(int));
    *period = 1000;
    xTaskCreate(taskPrintString_u, "Print Time", 2048, period, 1, NULL);

    int *increment = malloc(sizeof(int));
    *increment = 15;
    xTaskCreate(taskPrintCounter_u, "Print counter", 2048, increment, 1, NULL);
}

void mcinit(){
    initialize_tasks();
}

