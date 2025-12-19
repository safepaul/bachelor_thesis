#ifndef MCMANAGER_H
#define MCMANAGER_H

#include <stdint.h>




// TODO: check all structures for padding

//   -  TASK STRUCTURES  -

// structure defining the parameters of a task
typedef struct {

    uint16_t period;

} task_params_t;

//  -  TRANSITION STRUCTURES --

typedef enum {

    // TODO: Add support for all of the actions
    ACTION_NONE,
    ACTION_CONTINUE,
    ACTION_ABORT,
    ACTION_UPDATE,
    ACTION_RELEASE,
    ACTION_COUNT

} Action_e;

typedef enum {

    // TODO: Add the names for all of the available guards and add support for all of them
    GUARD_NONE,
    GUARD_COUNT

} Guard_e;

// Structure serving as a template that holds the values for what action to perform on a job depending on its type
// and the guard associated with it. Some fields may not be necessary for a specific type of job; those fields will
// have value ACTION_NONE, GUARD_NONE and -1 for action, guard and guard value respectively XXX: subject to change.
typedef struct {

    // TODO: with an uint8_t is enough for actions and guards, but enums are integers. Find a way to change that
    // and still be practical (#defines, casting... whatever)

    Action_e    anew;
    Guard_e     gnew;
    int16_t     gnewval;

    Action_e    apend;
    Guard_e     gpend;
    int16_t     gpendval;

    Action_e    aexec;
    Guard_e     gexec;
    int16_t     gexecval;

} job_primitives_t;

// TODO: This is a generic structure. Make specific ones for each type of task (4 in total) and the generator 
// generates the specified one. Type is transition_task_t but value is the specific one. I've seen this done
// in the linux kernel with tasks.
typedef struct {

    uint16_t            task_id;
    uint16_t            transition_id;
    task_params_t       params;
    job_primitives_t    primitives;

} transition_task_t;

typedef struct transition {

    uint32_t            id;
    uint16_t            source;
    uint16_t            dest;
    transition_task_t   *taskset;

} transition_t;







void mcm_create_transition(uint32_t id, uint16_t source, uint16_t dest);
void mcm_add_task_to_transition(uint16_t task_id, uint16_t transition_id, task_params_t params, job_primitives_t primitives);

















// NOTE: These may not be needed as the transitions have all the information necessary for the mode changes.
// Although maybe in the future I realize I need DS's for the modes and tasks
//
// void mcm_create_mode(uint16_t mode_id);
// void mcm_add_task_to_mode(uint16_t mode_id, uint16_t task_id, task_params_t task_params);
// void mcm_add_task_to_transition(transition_task_t task);















// TODO: add a debug mode
void debug_print_transition_table();
void debug_print_trans_tasks_table();

#endif // !MCMANAGER_H
