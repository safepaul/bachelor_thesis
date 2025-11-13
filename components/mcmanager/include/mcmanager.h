#ifndef MCMANAGER_H
#define MCMANAGER_H

#include <stdint.h>


// TODO: check all structures for padding


//   -  TASK STRUCTURES  -

// structure defining the parameters of a task
typedef struct {

    uint16_t period;

} task_params_t;


// structure defining the relevant information of a task
typedef struct task_info {

    uint16_t id;
    task_params_t params;
    struct task_info *next;

} task_info_t;


//  -  MODE STRUCTURES  -

// structure defining the relevant information of a mode
// `tasks` is a pointer to the first node of the task list
typedef struct mode_info {

    uint16_t id;
    task_info_t *tasks;
    struct mode_info *next;

} mode_info_t;


//  -  TRANSITION STRUCTURES --

// TODO: develop a hashmap for the transition list

typedef enum {

    // TODO: Add support for all of the actions
    ACTION_CONTINUE,
    ACTION_ABORT,
    ACTION_UPDATE,
    ACTION_COUNT

} Action_e;

typedef enum {

    // TODO: Add the names for all of the available guards and add support for all of them
    GUARD_NULL,
    GUARD_COUNT

} Guard_e;

// TODO: This is a generic structure. Make specific ones for each type of task (4 in total) and the generator 
// generates the specified one. Type is transition_task_t but value is the specific one. I've seen this done
// in the linux kernel with tasks.
typedef struct transition_task {

    uint16_t id;
    // type
    task_params_t *new_params; // XXX: pointer to the new parameters in the mode DS &mode[dest],task[id]. Assumes the mode DS is created first. Maybe it's not worth the generator logic complication.
    Action_e action_new;
    Guard_e guard_new;
    int32_t guard_new_value;

    Action_e action_pend;
    Guard_e guard_pend;
    int32_t guard_pend_value;

    Action_e action_exec;
    Guard_e guard_exec;
    int32_t guard_exec_value;

    struct transition_task *next;

} transition_task_t;


typedef struct { // XXX: this will be a hashmap

    uint16_t id;
    uint16_t source;
    uint16_t dest;
    transition_task_t *tasks; // Linked list

} transition_t;



// Creates a mode in the mode list
void mcm_create_mode(uint16_t mode_id);

// Creates a new task and adds it to the end of the specified Mode
void mcm_add_task_to_mode(uint16_t mode_id, uint16_t task_id, task_params_t task_params);

//
void mcm_create_transition(transition_t transition);

//
void mcm_add_task_to_transition(transition_task_t task);












/*
 *
 * 
 * */
void mode_change_request();




#endif // !MCMANAGER_H
