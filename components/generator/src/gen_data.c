#include "gen_data.h"
#include "mcmanager.h"
#include "stddef.h"
#include "tasks.h"


TaskHandle_t task_handles[N_TASKS];



const uint8_t mode_transitions[N_MODES][N_MODES] = {

	[0][0] = NO_TRANSITION,
	[0][1] = 0,
	[1][0] = 1,
	[1][1] = NO_TRANSITION,


};

static const task_trans_data_t trans_0_taskset[2] = {

	(task_trans_data_t){ .transition_id = 0, .task_id = 0, .params = (task_params_t){ .period = 200, .priority = 10 }, .primitives = (job_primitives_t){ .anew = ACTION_RELEASE, .gnew = GUARD_NONE, .gnewval = -1, .aexec = ACTION_CONTINUE, .gexec = GUARD_NONE, .gexecval = -1, } },
	(task_trans_data_t){ .transition_id = 0, .task_id = 1, .params = (task_params_t){ .period = 500, .priority = 10 }, .primitives = (job_primitives_t){ .anew = ACTION_RELEASE, .gnew = GUARD_NONE, .gnewval = -1, .aexec = ACTION_NONE, .gexec = GUARD_NONE, .gexecval = -1, } },

};

static const task_trans_data_t trans_1_taskset[2] = {

	(task_trans_data_t){ .transition_id = 1, .task_id = 0, .params = (task_params_t){ .period = 100, .priority = 10 }, .primitives = (job_primitives_t){ .anew = ACTION_RELEASE, .gnew = GUARD_NONE, .gnewval = -1, .aexec = ACTION_CONTINUE, .gexec = GUARD_NONE, .gexecval = -1, } },
	(task_trans_data_t){ .transition_id = 1, .task_id = 1, .params = (task_params_t){ .period = 0, .priority = 10 }, .primitives = (job_primitives_t){ .anew = ACTION_NONE, .gnew = GUARD_NONE, .gnewval = -1, .aexec = ACTION_CONTINUE, .gexec = GUARD_NONE, .gexecval = -1, } },

};

const transition_t transitions[N_TRANS] = {

	[0] = { .id = 0, .source_mode = 0, .dest_mode = 1, .taskset_size = 2, .taskset = trans_0_taskset },
	[1] = { .id = 1, .source_mode = 1, .dest_mode = 0, .taskset_size = 2, .taskset = trans_1_taskset },

};

void spawn_tasks(){

	// stop scheduler
	vTaskSuspendAll();

	xTaskCreate( taskZero_utask, "taskZero", 2048, NULL, 1, &task_handles[0] );
	vTaskSuspend( task_handles[0] );
	xTaskCreate( taskOne_utask, "taskOne", 2048, NULL, 1, &task_handles[1] );
	vTaskSuspend( task_handles[1] );

	xTaskResumeAll();

}


