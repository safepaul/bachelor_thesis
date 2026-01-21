#include "gen_data.h"
#include "mcmanager.h"
#include "stddef.h"
#include "tasks.h"




void generate_data(){


	mcm_add_task_info( 2, &taskTwo_utask, "taskTwo" );
	mcm_add_task_info( 5, &taskFive_utask, "taskFive" );

	mcm_create_transition(0, 0, 1);
	mcm_add_task_to_transition(2, 0, (task_params_t){ .period = 200, .priority = 10 }, (job_primitives_t){.anew = ACTION_RELEASE, .gnew = GUARD_NONE, .gnewval = -1, .aexec = ACTION_CONTINUE, .gexec = GUARD_NONE, .gexecval = -1});
	mcm_add_task_to_transition(5, 0, (task_params_t){ .period = 500, .priority = 10 }, (job_primitives_t){.anew = ACTION_RELEASE, .gnew = GUARD_NONE, .gnewval = -1, .aexec = ACTION_NONE, .gexec = GUARD_NONE, .gexecval = -1});
	mcm_create_transition(1, 1, 0);
	mcm_add_task_to_transition(2, 1, (task_params_t){ .period = 100, .priority = 10 }, (job_primitives_t){.anew = ACTION_RELEASE, .gnew = GUARD_NONE, .gnewval = -1, .aexec = ACTION_CONTINUE, .gexec = GUARD_NONE, .gexecval = -1});
	mcm_add_task_to_transition(5, 1, (task_params_t){ .period = 0, .priority = 10 }, (job_primitives_t){.anew = ACTION_NONE, .gnew = GUARD_NONE, .gnewval = -1, .aexec = ACTION_CONTINUE, .gexec = GUARD_NONE, .gexecval = -1});

}
