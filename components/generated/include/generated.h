#ifndef GENERATED_H
#define GENERATED_H


#include "freertos/idf_additions.h"
#include <stdint.h>


#define GLOBAL_STACK_DEPTH 2048


#define ACTION_CONTINUE (uint8_t) 0
#define ACTION_ABORT (uint8_t) 1
// #define ACTION_ABORT_K (uint8_t) 2
#define ACTION_UPDATE (uint8_t) 3
#define ACTION_RELEASE (uint8_t) 4

#define TYPE_NEW            (uint8_t) 100
#define TYPE_OLD            (uint8_t) 101
#define TYPE_CHANGED        (uint8_t) 102
#define TYPE_UNCHANGED      (uint8_t) 103




// Generated following a deterministic order
typedef enum { 

    MODE_INIT,
    MODE_NORMAL,
    MODE_COUNT

} mode_e;

typedef enum { 

    TASK_PRINTSTRING,
    TASK_PRINTCOUNTER,
    TASK_PRINTZEROES,
    TASK_PRINTONES,
    TASK_COUNT

} task_e;

typedef struct {

    const char* name;
    task_e      id;

} task_map_t;

typedef struct {            // does it make sense for this struct to be here and not in mcinit.h?
    
    // UBaseType_t     priority;
    // const uint32_t  stack_depth;
    // const char*     name;
    // wcet, period <- in args?, criticality <- priority or different?
    void*           args_p;
    bool            active;

} task_info_t;

// Only taking into account the type of the task for making changes, not actions taken into a specific task for now.
// [!] Not making the distinction between jobs and tasks for now
typedef struct {
    
    uint8_t     type;
    uint8_t     action;

} transition_t;






extern const task_map_t g_task_map[TASK_COUNT];

extern const task_info_t g_task_table[TASK_COUNT][MODE_COUNT];

extern const TaskFunction_t g_task_funcs[TASK_COUNT];

extern const transition_t g_transition_table[MODE_COUNT][MODE_COUNT][TASK_COUNT];


// TASKS

void tf_printString(void *tfParams);

void tf_printCounter(void *tfParams);

void tf_printZeroes(void *tfParams);

void tf_printOnes(void *tfParams);



#endif // !GENERATED_H
