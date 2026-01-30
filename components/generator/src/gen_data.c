#include "gen_data.h"
#include "mcmanager.h"
#include "stddef.h"
#include "tasks.h"


TaskHandle_t task_handles[N_TASKS];
TimerHandle_t timer_handles[N_TASKS];



const uint8_t mode_transitions[N_MODES][N_MODES] = {

	[0][0] = NO_TRANSITION,
	[0][1] = 0,
	[1][0] = 1,
	[1][1] = NO_TRANSITION,


};

static const trans_task_t trans_0_taskset[2] = {

	(trans_task_t){ .transition_id = 0, .task_id = 0, .primitives = (job_primitives_t){ .action = ACTION_UPDATE, .guard = GUARD_TRUE, .guard_value = -1, } },
	(trans_task_t){ .transition_id = 0, .task_id = 1, .primitives = (job_primitives_t){ .action = ACTION_RELEASE, .guard = GUARD_OFFSETMCR, .guard_value = 3000, } },

};

static const trans_task_t trans_1_taskset[2] = {

	(trans_task_t){ .transition_id = 1, .task_id = 0, .primitives = (job_primitives_t){ .action = ACTION_UPDATE, .guard = GUARD_TRUE, .guard_value = -1, } },
	(trans_task_t){ .transition_id = 1, .task_id = 1, .primitives = (job_primitives_t){ .action = ACTION_SUSPEND, .guard = GUARD_TRUE, .guard_value = -1, } },

};

const transition_t transitions[N_TRANS] = {

	[0] = { .id = 0, .source_mode = 0, .dest_mode = 1, .taskset_size = 2, .taskset = trans_0_taskset },
	[1] = { .id = 1, .source_mode = 1, .dest_mode = 0, .taskset_size = 2, .taskset = trans_1_taskset },

};




static const mode_task_t mode_0_tasks[2] = {

	[0] = { .id = 0, .parameters = (task_params_t){ .period = 200, .priority = 10 } }, 
	[1] = { .id = 1, .parameters = (task_params_t){ .period = 500, .priority = 10 } }, 

};

static const mode_task_t mode_1_tasks[1] = {

	[0] = { .id = 0, .parameters = (task_params_t){ .period = 100, .priority = 10 } }, 

};

const mode_info_t modes[N_MODES] =  {

	[0] = { .id = 0, .n_tasks = 2, .tasks = mode_0_tasks  },
	[1] = { .id = 1, .n_tasks = 1, .tasks = mode_1_tasks  },

};

void create_tasks(){

	xTaskCreate( taskZero_utask, "taskZero", 2048, NULL, 1, &task_handles[0] );
	xTaskCreate( taskOne_utask, "taskOne", 2048, NULL, 1, &task_handles[1] );

}

void create_timers(){

	timer_handles[0] = xTimerCreate( "taskZero_timer", 1, pdTRUE, (void*)(uintptr_t)0, task_timer_callback );
	timer_handles[1] = xTimerCreate( "taskOne_timer", 1, pdTRUE, (void*)(uintptr_t)1, task_timer_callback );

}



