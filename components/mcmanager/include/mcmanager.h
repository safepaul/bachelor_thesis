#ifndef MCMANAGER_H
#define MCMANAGER_H

#include <stdint.h>




//   -  TASK DEFINITION  -

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


//  -  MODE DEFINITION  -

// structure defining the relevant information of a mode
// `tasks` is a pointer to the first node of the task list
typedef struct mode_info {

    uint16_t id;
    task_info_t *tasks;
    struct mode_info *next;

} mode_info_t;





// Creates a mode in the mode list
void mcm_create_mode(uint16_t mode_id);

// Creates a new task and adds it to the end of the specified Mode
void mcm_add_task_to_mode(uint16_t mode_id, uint16_t task_id, task_params_t task_params);

//
void mcm_create_transition();

//
void mcm_add_task_to_transition();












/*
 * This is the "master" function. Calling the mcr() triggers the rest of the functions.
 * 
 * */
void mode_change_request();




#endif // !MCMANAGER_H
