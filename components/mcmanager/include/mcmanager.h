#ifndef MCMANAGER_H
#define MCMANAGER_H

#include "freertos/idf_additions.h"
#include "portmacro.h"
#include <stdint.h>


// guards and actions
#define ACTION_NONE      (uint8_t) 0
#define ACTION_CONTINUE  (uint8_t) 1
#define ACTION_ABORT     (uint8_t) 2
#define ACTION_UPDATE    (uint8_t) 3
#define ACTION_RELEASE   (uint8_t) 4

#define GUARD_NONE      (uint8_t) 0


// general bounds
#define MAX_TRANS_TASKS (uint16_t)   N_TRAN * N_TASKS 




// structure defining the parameters of a task (in a specific transition)
typedef struct {

    uint16_t    period;
    UBaseType_t priority;

} task_params_t;

// Structure serving as a template that holds the values for what action to perform on a job depending on its type
// and the guard associated with it. Some fields may not be necessary for a specific type of job; those fields will
// have value ACTION_NONE, GUARD_NONE and -1 for action, guard and guard value respectively XXX: subject to change.
typedef struct {

    uint8_t     anew;
    uint8_t     gnew;
    int16_t     gnewval;

    uint8_t     aexec;
    uint8_t     gexec;
    int16_t     gexecval;

} job_primitives_t;

typedef struct {

    job_primitives_t    primitives;
    task_params_t       params;
    uint16_t            transition_id;
    uint8_t             task_id;

} task_trans_data_t;

typedef struct transition {

    const task_trans_data_t *taskset;
    uint8_t                 taskset_size;
    uint8_t                 source_mode;
    uint8_t                 dest_mode;
    uint8_t                 id;

} transition_t;



void initial_setup();
void perform_action(TaskHandle_t handle, uint8_t action);
void handle_task(task_trans_data_t task, eTaskState state);
void mc_request(uint8_t target_mode);
















// TODO: add a debug mode
void debug_print_transition_table();
void debug_print_trans_tasks_table();
void debug_print_task_info_table();

#endif // !MCMANAGER_H
