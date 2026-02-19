#ifndef MCMANAGER_H
#define MCMANAGER_H

#include "freertos/idf_additions.h"
#include "portmacro.h"
#include <stdbool.h>
#include <stdint.h>


// guards and actions
#define ACTION_NONE         (uint8_t) 0
#define ACTION_CONTINUE     (uint8_t) 1
#define ACTION_UPDATE       (uint8_t) 2
#define ACTION_RELEASE      (uint8_t) 3
#define ACTION_SUSPEND      (uint8_t) 4

#define GUARD_NONE               (uint8_t) 0
#define GUARD_TRUE               (uint8_t) 1
#define GUARD_OFFSETMCR          (uint8_t) 2
#define GUARD_OFFSETLR           (uint8_t) 3
#define GUARD_BACKLOG_ZERO       (uint8_t) 4
#define GUARD_BACKLOG_GLOBAL     (uint8_t) 5


#define MAX_TRANS_TASKS (uint16_t)   N_TRANS * N_TASKS 




// structure defining the parameters of a task (in a specific transition)
typedef struct {

    uint16_t    period;
    UBaseType_t priority;

} task_params_t;


// the mcmanager has to know which tasks are active, their backlog... inside the mcm_task_t structures
//  - **active** tasks are known during each mode transition, so the "active" fields have to change during that transition managing
//  - **backlog** is known during execution in the mode change, so it must change in the timer code or the task code or inside something that executes periodically (?)
typedef struct {

    uint8_t backlog;
    bool is_waiting;

    TickType_t last_release;
    uint16_t last_period;
    uint8_t id;

} mcm_task_t;

typedef struct {

    uint8_t     action;
    uint8_t     guard;
    int16_t     guard_value;

} job_primitives_t;

typedef struct {

    job_primitives_t    primitives;
    uint16_t            transition_id;
    uint8_t             task_id;

} trans_task_t;

typedef struct {

    const trans_task_t      *taskset;
    uint8_t                 taskset_size;
    uint8_t                 source_mode;
    uint8_t                 dest_mode;
    uint8_t                 id;

} transition_t;

typedef struct {

    task_params_t   parameters;
    uint8_t         id;

} mode_task_t;

typedef struct {

    const mode_task_t   *tasks;
    uint8_t             n_tasks;
    uint8_t             id;

} mode_info_t;



void initial_setup();
void apply_primitive(const trans_task_t *task);
void perform_action(uint8_t task_id, uint8_t action, uint8_t mode_id);
void mc_request(uint8_t target_mode);



void callback_offset_timer( TimerHandle_t xTimer );

void task_timer_callback( TimerHandle_t xTimer );



// --- Private API ---

/*
 *
 *
*/
void mcm_wait_for_activation(uint8_t task_id);

/*
 *
 *
*/
void mcm_release_job(uint8_t task_id);


// --- Public API ---


#endif // !MCMANAGER_H
