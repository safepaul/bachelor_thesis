// EXAMPLE of a generated transition

/*
typedef struct transition {

    uint32_t            id;
    uint16_t            source;
    uint16_t            dest;
    transition_task_t   *taskset;

} transition_t;
*/


#include "mcmanager.h"
#include "stddef.h"
#include "testing.h"


void generate_data(){

    mcm_create_transition(0, 99, 1);

    mcm_add_task_to_transition(0, 0, 
                               (task_params_t){.period = 100},
                               (job_primitives_t)
                               {.anew = ACTION_NONE,
                                .gnew = GUARD_NONE,
                                .aexec = ACTION_NONE,
                                .gexec = GUARD_NONE,
                                .apend = ACTION_NONE,
                                .gpend = GUARD_NONE,
                                .gval = 69});

    mcm_add_task_to_transition(1, 0, 
                               (task_params_t){.period = 200},
                               (job_primitives_t)
                               {.anew = ACTION_NONE,
                                .gnew = GUARD_NONE,
                                .aexec = ACTION_NONE,
                                .gexec = GUARD_NONE,
                                .apend = ACTION_NONE,
                                .gpend = GUARD_NONE,
                                .gval = 420});

    mcm_add_task_to_transition(2, 0, 
                               (task_params_t){.period = 300},
                               (job_primitives_t)
                               {.anew = ACTION_NONE,
                                .gnew = GUARD_NONE,
                                .aexec = ACTION_NONE,
                                .gexec = GUARD_NONE,
                                .apend = ACTION_NONE,
                                .gpend = GUARD_NONE,
                                .gval = 1337});

}

