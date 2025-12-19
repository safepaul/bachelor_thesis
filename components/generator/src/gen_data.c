#include "gen_data.h"
#include "mcmanager.h"
#include "stddef.h"


void generate_data(){

	mcm_create_transition(0, 0, 1);
	mcm_add_task_to_transition(2, 0, (task_params_t){ .period = 200 }, (job_primitives_t){.anew = ACTION_RELEASE, .gnew = ACTION_RELEASE, .gnewval = -1, .apend = ACTION_CONTINUE, .gpend = GUARD_NONE, .gpendval = -1, .aexec = ACTION_NONE, .gexec = GUARD_NONE, .gexecval = -1});
	mcm_add_task_to_transition(5, 0, (task_params_t){ .period = 500 }, (job_primitives_t){.anew = ACTION_RELEASE, .gnew = ACTION_RELEASE, .gnewval = -1, .apend = ACTION_CONTINUE, .gpend = GUARD_NONE, .gpendval = -1, .aexec = ACTION_NONE, .gexec = GUARD_NONE, .gexecval = -1});
	mcm_create_transition(1, 1, 0);
	mcm_add_task_to_transition(2, 1, (task_params_t){ .period = 100 }, (job_primitives_t){.anew = ACTION_RELEASE, .gnew = ACTION_RELEASE, .gnewval = -1, .apend = ACTION_CONTINUE, .gpend = GUARD_NONE, .gpendval = -1, .aexec = ACTION_CONTINUE, .gexec = GUARD_NONE, .gexecval = -1});
	mcm_add_task_to_transition(5, 1, (task_params_t){ .period = 0 }, (job_primitives_t){.anew = ACTION_NONE, .gnew = GUARD_NONE, .gnewval = -1, .apend = ACTION_CONTINUE, .gpend = GUARD_NONE, .gpendval = -1, .aexec = ACTION_CONTINUE, .gexec = GUARD_NONE, .gexecval = -1});

}
