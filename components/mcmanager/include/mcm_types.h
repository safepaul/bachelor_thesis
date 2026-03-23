#ifndef MCM_TYPES_H
#define MCM_TYPES_H

#include <stdint.h>
#include "freertos/idf_additions.h"
#include "portmacro.h"


enum 
{
    SYSTEM_STATE_NORMAL = 0,
    SYSTEM_STATE_TRANSITIONING,
    SYSTEM_STATE_TRANSIENT,
    SYSTEM_STATE_COUNT
}; typedef uint8_t mcm_system_state_t;

enum 
{
    MCM_TASK_TYPE_CHANGED = 0,
    MCM_TASK_TYPE_UNCHANGED,
    MCM_TASK_TYPE_NEW,
    MCM_TASK_TYPE_OLD,
    MCM_TASK_TYPE_COUNT
}; typedef uint8_t mcm_task_type_t;

enum 
{
    ACTION_CONTINUE = 0,
    ACTION_UPDATE,
    ACTION_RELEASE,
    ACTION_SUSPEND,
    ACTION_COUNT
}; typedef uint8_t mcm_action_t;

enum
{
    GUARD_TRUE = 0,
    GUARD_OFFSETMCR,
    GUARD_OFFSETLR,
    GUARD_BACKLOG_ZERO,
    GUARD_COUNT
}; typedef uint8_t mcm_guard_t;

enum
{
    MCM_TRANS_SYNC_ALL = 0,
    MCM_TRANS_SYNC_PENDING
}; typedef uint8_t mcm_trans_result_t;

typedef struct
{
    UBaseType_t priority;
    uint16_t    period;
} mcm_task_params_t;

typedef struct 
{
    uint8_t     action;
    uint8_t     guard;
    int16_t     guard_value;
} mcm_task_primitives_t;

typedef struct
{
    TickType_t  last_release;
    uint8_t     id;
} mcm_task_t;

typedef struct 
{
    mcm_task_primitives_t   primitives;
    uint16_t                transition_id;
    uint8_t                 type;
    uint8_t                 id;
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
    uint8_t             id;
} mcm_mode_task_t;

typedef struct 
{
    const mcm_mode_task_t   *tasks;
    uint8_t                 n_tasks;
    uint8_t                 id;
} mcm_mode_t;

typedef struct
{
    uint8_t n_tasks;
    uint8_t n_modes;
    uint8_t n_trans;
    
    mcm_task_t              *tasks;
    const mcm_mode_t        *modes;
    const mcm_transition_t  *transitions;
    const uint8_t           *mode_transitions;
    
    TaskHandle_t *task_handles;
    TimerHandle_t *task_timer_handles;
    TimerHandle_t *offset_timer_handles;
    SemaphoreHandle_t *semaphore_handles;
} mcm_config_t;


#endif // !MCM_TYPES_H
