#ifndef MCM_TYPES_H
#define MCM_TYPES_H

#include <stdint.h>
#include "portmacro.h"


enum 
{
    ACTION_NONE = 0,
    ACTION_CONTINUE,
    ACTION_UPDATE,
    ACTION_RELEASE,
    ACTION_SUSPEND,
    ACTION_COUNT
}; typedef uint8_t mcm_action_t;

enum
{
    GUARD_NONE = 0,
    GUARD_TRUE,
    GUARD_OFFSETMCR,
    GUARD_OFFSETLR,
    GUARD_BACKLOG_ZERO,
    GUARD_BACKLOG_GLOBAL,
    GUARD_COUNT
}; typedef uint8_t mcm_guard_t ;

enum 
{
    STATE_RELEASED = 0,
    STATE_WAITING_FOR_RELEASE,
    STATE_WAITING_FOR_BACKLOG_Z,
    STATE_WAITING_FOR_BACKLOG_G,
    STATE_WAITING_FOR_OFFSETMCR,
    STATE_WAITING_FOR_OFFSETLR,
    STATE_COUNT
}; typedef uint8_t mcm_task_state_t;

typedef struct
{
    UBaseType_t priority;
    uint16_t    period;
} mcm_task_params_t;

typedef struct
{
    TickType_t  last_release;
    uint8_t     backlog;
    uint8_t     state;
    uint8_t     id;
} mcm_task_t;

typedef struct 
{
    uint8_t     action;
    uint8_t     guard;
    int16_t     guard_value;
} mcm_task_primitives_t;

typedef struct 
{
    mcm_task_primitives_t   primitives;
    uint16_t                transition_id;
    uint8_t                 task_id;
} mcm_transition_task_t;

typedef struct 
{
    const mcm_transition_task_t      *taskset;
    uint8_t     taskset_size;
    uint8_t     source_mode;
    uint8_t     dest_mode;
    uint8_t     id;
} mcm_transition_t;

typedef struct 
{
    mcm_task_params_t   parameters;
    uint8_t         id;
} mcm_mode_task_t;

typedef struct 
{
    const mcm_mode_task_t   *tasks;
    uint8_t             n_tasks;
    uint8_t             id;
} mcm_mode_info_t;


#endif // !MCM_TYPES_H
