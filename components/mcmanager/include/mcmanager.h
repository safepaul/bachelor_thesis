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
#define MAX_TRANSITIONS (uint16_t)   64
#define MAX_TASKS       (uint8_t)    32
#define MAX_TRANS_TASKS (uint16_t)   MAX_TRANSITIONS * MAX_TASKS 




// // needed to force 8bit size on both actions and guards to save space. Actions and Guards defined with #define directives
// typedef uint8_t Action_t;
// typedef uint8_t Guard_t;





// TODO: using same stack size (2048). This may need to be adjusted for each task.
// TODO: check all structures for padding

// structure defining the parameters of a task (in a specific transition)
typedef struct {

    uint16_t    period;
    UBaseType_t priority;

} task_params_t;

// structure defining the information that the program needs to start user tasks according to the specifications of the user (generated)
typedef struct {

    TaskFunction_t  func;
    char*           name;
    TaskHandle_t    handle;

} task_info_t;

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

    uint8_t            task_id;
    uint16_t            transition_id;
    task_params_t       params;
    job_primitives_t    primitives;

} transition_task_t;

typedef struct transition {

    uint16_t            id;
    uint16_t            source;
    uint16_t            dest;
    transition_task_t   *taskset;

} transition_t;







void mcm_create_transition(uint32_t id, uint16_t source, uint16_t dest);
void mcm_add_task_to_transition(uint16_t task_id, uint16_t transition_id, task_params_t params, job_primitives_t primitives);
void mcm_add_task_info(uint8_t task_id, TaskFunction_t func, char* name);
void spawn_tasks();

void perform_action(TaskHandle_t handle, uint8_t action);
void handle_task(transition_task_t task, eTaskState state);
void mc_request(uint16_t transition_id);















// NOTE: These may not be needed as the transitions have all the information necessary for the mode changes.
// Although maybe in the future I realize I need DS's for the modes and tasks
//
// void mcm_create_mode(uint16_t mode_id);
// void mcm_add_task_to_mode(uint16_t mode_id, uint16_t task_id, task_params_t task_params);
// void mcm_add_task_to_transition(transition_task_t task);















// TODO: add a debug mode
void debug_print_transition_table();
void debug_print_trans_tasks_table();
void debug_print_task_info_table();

#endif // !MCMANAGER_H
