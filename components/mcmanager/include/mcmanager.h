#ifndef MCMANAGER_H
#define MCMANAGER_H

#include "freertos/idf_additions.h"
#include "portmacro.h"
#include <stdint.h>


// guards and actions
#define ACTION_NONE         (uint8_t) 0
#define ACTION_CONTINUE     (uint8_t) 1
#define ACTION_SUSPEND      (uint8_t) 2
#define ACTION_UPDATE       (uint8_t) 3
#define ACTION_RELEASE      (uint8_t) 4

#define GUARD_NONE          (uint8_t) 0
#define GUARD_TRUE          (uint8_t) 1
#define GUARD_OFFSETMCR     (uint8_t) 2
#define GUARD_BACKLOG       (uint8_t) 3


// general bounds
#define MAX_TRANS_TASKS (uint16_t)   N_TRANS * N_TASKS 




// structure defining the parameters of a task (in a specific transition)
typedef struct {

    uint16_t    period;
    UBaseType_t priority;

} task_params_t;

// Structure serving as a template that holds the values for what action to perform on a job depending on its type
// and the guard associated with it. Some fields may not be necessary for a specific type of job; those fields will
// have value ACTION_NONE, GUARD_NONE and -1 for action, guard and guard value respectively XXX: subject to change.
typedef struct {

    uint8_t     action;
    uint8_t     guard;
    int16_t     guard_value;

} job_primitives_t;

typedef struct {

    job_primitives_t    primitives;
    task_params_t       params;
    uint16_t            transition_id;
    uint8_t             task_id;

} trans_task_t;

typedef struct {

    const trans_task_t *taskset;
    uint8_t                 taskset_size;
    uint8_t                 source_mode;
    uint8_t                 dest_mode;
    uint8_t                 id;

} transition_t;



void initial_setup();
void apply_primitive(const trans_task_t *task);
void perform_action(TaskHandle_t handle, uint8_t action);
void mc_request(uint8_t target_mode);





typedef struct {

    TaskHandle_t handle;
    uint8_t action;

} offsetmcr_info_t;

void callback_offsetmcr_timer( TimerHandle_t xTimer );


#endif // !MCMANAGER_H
