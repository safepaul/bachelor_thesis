#ifndef GENERATED_H
#define GENERATED_H


#include "freertos/idf_additions.h"


#define GLOBAL_STACK_DEPTH 2048


// Generated following a deterministic order
typedef enum { 
    MODE_INIT,
    MODE_NORMAL,
    MODE_COUNT
} mode_e;

typedef enum { 
    TASK_PRINTSTRING,
    TASK_PRINTCOUNTER,
    TASK_COUNT
} task_e;

typedef struct {

    const char* name;
    task_e      id;

} task_map_t;

typedef struct {            // does it make sense for this struct to be here and not in mcinit.h?
    
    TaskHandle_t    handle;
    UBaseType_t     priority;
    // const uint32_t  stack_depth;
    // const char*     name;
    void*           args;

} task_info_t;


extern const task_map_t g_task_map[TASK_COUNT];

extern const task_info_t g_task_table[TASK_COUNT][MODE_COUNT];

extern const TaskFunction_t g_task_funcs[TASK_COUNT];


void taskPrintString_u(void *tfParams);

void taskPrintCounter_u(void *tfParams);



#endif // !GENERATED_H
