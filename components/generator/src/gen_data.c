#include "stddef.h"
#include "tasks.h"
#include "freertos/idf_additions.h"
#include "gen_data.h"
#include "mcmanager.h"
#include "mcm_types.h"


TaskHandle_t task_handles[N_TASKS];
SemaphoreHandle_t semaphore_handles[N_TASKS];
TimerHandle_t task_timer_handles[N_TASKS];
TimerHandle_t offset_timer_handles[N_TASKS];



const uint8_t mode_transitions[N_MODES * N_MODES] =
{
	[0] = NO_TRANSITION,
	[1] = 0,
	[2] = 1,
	[3] = NO_TRANSITION,
};

static const mcm_transition_task_t trans_0_taskset[2] = {

	(mcm_transition_task_t){ .transition_id = 0, .id = 0, .type = MCM_TASK_TYPE_CHANGED, .primitives = (mcm_task_primitives_t){ .action = ACTION_UPDATE, .guard = GUARD_OFFSETLR, .guard_value = 500, } },
	(mcm_transition_task_t){ .transition_id = 0, .id = 1, .type = MCM_TASK_TYPE_OLD, .primitives = (mcm_task_primitives_t){ .action = ACTION_SUSPEND, .guard = GUARD_BACKLOG_ZERO, .guard_value = -1, } },

};

static const mcm_transition_task_t trans_1_taskset[2] = {

	(mcm_transition_task_t){ .transition_id = 1, .id = 0, .type = MCM_TASK_TYPE_CHANGED, .primitives = (mcm_task_primitives_t){ .action = ACTION_UPDATE, .guard = GUARD_TRUE, .guard_value = -1, } },
	(mcm_transition_task_t){ .transition_id = 1, .id = 1, .type = MCM_TASK_TYPE_NEW, .primitives = (mcm_task_primitives_t){ .action = ACTION_RELEASE, .guard = GUARD_OFFSETMCR, .guard_value = 3000, } },

};

const mcm_transition_t transitions[N_TRANS] = {

	[0] = { .id = 0, .source_mode = 0, .dest_mode = 1, .taskset_size = 2, .taskset = trans_0_taskset },
	[1] = { .id = 1, .source_mode = 1, .dest_mode = 0, .taskset_size = 2, .taskset = trans_1_taskset },

};




static const mcm_mode_task_t mode_0_tasks[2] = {

	[0] = { .id = 0, .parameters = (mcm_task_params_t){ .period = 2000, .priority = 10 } }, 
	[1] = { .id = 1, .parameters = (mcm_task_params_t){ .period = 750, .priority = 10 } }, 

};

static const mcm_mode_task_t mode_1_tasks[1] = {

	[0] = { .id = 0, .parameters = (mcm_task_params_t){ .period = 500, .priority = 10 } }, 

};

const mcm_mode_t modes[N_MODES] =  {

	[0] = { .id = 0, .n_tasks = 2, .tasks = mode_0_tasks  },
	[1] = { .id = 1, .n_tasks = 1, .tasks = mode_1_tasks  },

};

mcm_task_t tasks[N_TASKS] =  {

	[0] = { .id = 0, .last_release = 0 },
	[1] = { .id = 1, .last_release = 0 },

};

void create_tasks(){

	xTaskCreate( taskZero_utask, "taskZero", 2048, (void*)(uintptr_t)0, 10, &task_handles[0] );
	xTaskCreate( taskOne_utask, "taskOne", 2048, (void*)(uintptr_t)1, 10, &task_handles[1] );

}

void create_semaphores()
{
	semaphore_handles[0] = xSemaphoreCreateCounting(LIMIT_BACKLOG , 0);
	semaphore_handles[1] = xSemaphoreCreateCounting(LIMIT_BACKLOG , 0);
}

void create_timers()
{
	task_timer_handles[0] = xTimerCreate( "taskZero_task_timer", pdMS_TO_TICKS(2000), pdTRUE, (void*)(uintptr_t)0, mcm_task_timer_callback_func );
	offset_timer_handles[0] = xTimerCreate( "taskZero_offset_timer", 1, pdTRUE, (void*)(uintptr_t)0, mcm_offset_timer_callback_func );
	task_timer_handles[1] = xTimerCreate( "taskOne_task_timer", pdMS_TO_TICKS(750), pdTRUE, (void*)(uintptr_t)1, mcm_task_timer_callback_func );
	offset_timer_handles[1] = xTimerCreate( "taskOne_offset_timer", 1, pdTRUE, (void*)(uintptr_t)1, mcm_offset_timer_callback_func );
}

mcm_config_t sys_config = 
{
	.n_tasks = N_TASKS,
	.n_modes = N_MODES,
	.n_trans = N_TRANS,
	.tasks = tasks,
	.modes = modes,
	.transitions = transitions,
	.mode_transitions = mode_transitions,
	.task_handles = task_handles,
	.task_timer_handles = task_timer_handles,
	.offset_timer_handles = task_timer_handles,
	.semaphore_handles = semaphore_handles,
};

void mcm_init()
{
	create_semaphores();
	create_timers();
	mcm_initial_setup(&sys_config, MODE_INIT);
	create_tasks();
}

