#ifndef MCINIT_H
#define MCINIT_H

#include "portmacro.h"


#define N_TASKS 5
#define N_MODES 3


struct TaskParams_t {
    bool filled;
    UBaseType_t priority;
    void *args;
    // const configSTACK_DEPTH_TYPE stack_depth;
};

/*
 * Initialization function [...]
 * */
void mcinit();

/*
 * Creates and zero-initializes the params data structure
 * */
void params_create();

/*
 * Adds the parameters of a task in a specific mode to its respective entry in the params data structure
 * */
void params_add_entry(uint16_t task_id, uint16_t mode_id, struct TaskParams_t params);

/*
 * Returns an entry from the params data structure
 * */
struct TaskParams_t params_lookup(uint16_t task_id, uint16_t mode_id);

/*
 * Fills the params data structure with the info about every task for every mode using params_add_entry()
 * */
void params_fill();


#endif // !MCINIT_H

