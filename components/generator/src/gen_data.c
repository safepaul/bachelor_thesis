#include "tasks.h"
#include "gen_data.h"
#include "mcmanager.h"

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

static const mcm_transition_task_t trans_0_taskset[5] =
{
	(mcm_transition_task_t){ .transition_id = 0, .id = 0, .type = MCM_TASK_TYPE_OLD, .primitives = (mcm_task_primitives_t){ .action = ACTION_SUSPEND, .guard = GUARD_OFFSETLR, .guard_value = 3000, } },
	(mcm_transition_task_t){ .transition_id = 0, .id = 1, .type = MCM_TASK_TYPE_OLD, .primitives = (mcm_task_primitives_t){ .action = ACTION_SUSPEND, .guard = GUARD_OFFSETMCR, .guard_value = 500, } },
	(mcm_transition_task_t){ .transition_id = 0, .id = 2, .type = MCM_TASK_TYPE_NEW, .primitives = (mcm_task_primitives_t){ .action = ACTION_RELEASE, .guard = GUARD_OFFSETMCR, .guard_value = 2000, } },
	(mcm_transition_task_t){ .transition_id = 0, .id = 3, .type = MCM_TASK_TYPE_CHANGED, .primitives = (mcm_task_primitives_t){ .action = ACTION_UPDATE, .guard = GUARD_OFFSETLR, .guard_value = 2000, } },
	(mcm_transition_task_t){ .transition_id = 0, .id = 4, .type = MCM_TASK_TYPE_CHANGED, .primitives = (mcm_task_primitives_t){ .action = ACTION_UPDATE, .guard = GUARD_BACKLOG_ZERO, .guard_value = -1, } },
};

static const mcm_transition_task_t trans_1_taskset[5] =
{
	(mcm_transition_task_t){ .transition_id = 1, .id = 0, .type = MCM_TASK_TYPE_NEW, .primitives = (mcm_task_primitives_t){ .action = ACTION_RELEASE, .guard = GUARD_OFFSETLR, .guard_value = 3000, } },
	(mcm_transition_task_t){ .transition_id = 1, .id = 1, .type = MCM_TASK_TYPE_NEW, .primitives = (mcm_task_primitives_t){ .action = ACTION_RELEASE, .guard = GUARD_OFFSETMCR, .guard_value = 500, } },
	(mcm_transition_task_t){ .transition_id = 1, .id = 2, .type = MCM_TASK_TYPE_OLD, .primitives = (mcm_task_primitives_t){ .action = ACTION_SUSPEND, .guard = GUARD_OFFSETMCR, .guard_value = 2000, } },
	(mcm_transition_task_t){ .transition_id = 1, .id = 3, .type = MCM_TASK_TYPE_CHANGED, .primitives = (mcm_task_primitives_t){ .action = ACTION_UPDATE, .guard = GUARD_OFFSETLR, .guard_value = 2000, } },
	(mcm_transition_task_t){ .transition_id = 1, .id = 4, .type = MCM_TASK_TYPE_CHANGED, .primitives = (mcm_task_primitives_t){ .action = ACTION_UPDATE, .guard = GUARD_OFFSETMCR, .guard_value = 1000, } },
};

const mcm_transition_t transitions[N_TRANS] =
{
	[0] = { .id = 0, .source_mode = 0, .dest_mode = 1, .taskset_size = 5, .taskset = trans_0_taskset },
	[1] = { .id = 1, .source_mode = 1, .dest_mode = 0, .taskset_size = 5, .taskset = trans_1_taskset },
};

static const mcm_mode_task_t mode_0_tasks[4] =
{
	[0] = { .id = 0, .parameters = (mcm_task_params_t){ .period = 1000, .priority = 10 } }, 
	[1] = { .id = 1, .parameters = (mcm_task_params_t){ .period = 1500, .priority = 10 } }, 
	[2] = { .id = 3, .parameters = (mcm_task_params_t){ .period = 1000, .priority = 10 } }, 
	[3] = { .id = 4, .parameters = (mcm_task_params_t){ .period = 3000, .priority = 10 } }, 
};

static const mcm_mode_task_t mode_1_tasks[3] =
{
	[0] = { .id = 2, .parameters = (mcm_task_params_t){ .period = 500, .priority = 10 } }, 
	[1] = { .id = 3, .parameters = (mcm_task_params_t){ .period = 3000, .priority = 10 } }, 
	[2] = { .id = 4, .parameters = (mcm_task_params_t){ .period = 500, .priority = 10 } }, 
};

const mcm_mode_t modes[N_MODES] =
{
	[0] = { .id = 0, .n_tasks = 4, .tasks = mode_0_tasks  },
	[1] = { .id = 1, .n_tasks = 3, .tasks = mode_1_tasks  },
};

mcm_task_t tasks[N_TASKS] =
{
	[0] = { .id = 0, .last_release = 0 },
	[1] = { .id = 1, .last_release = 0 },
	[2] = { .id = 2, .last_release = 0 },
	[3] = { .id = 3, .last_release = 0 },
	[4] = { .id = 4, .last_release = 0 },
};

void create_tasks()
{
	xTaskCreate( taskSLR_utask, "taskSLR", 2048, (void*)(uintptr_t)0, 10, &task_handles[0] );
	xTaskCreate( taskSMCR_utask, "taskSMCR", 2048, (void*)(uintptr_t)1, 10, &task_handles[1] );
	xTaskCreate( taskRMCR_utask, "taskRMCR", 2048, (void*)(uintptr_t)2, 1, &task_handles[2] );
	xTaskCreate( taskULR_utask, "taskULR", 2048, (void*)(uintptr_t)3, 10, &task_handles[3] );
	xTaskCreate( taskUMCR_utask, "taskUMCR", 2048, (void*)(uintptr_t)4, 10, &task_handles[4] );
}

void create_semaphores()
{
	semaphore_handles[0] = xSemaphoreCreateCounting(LIMIT_BACKLOG , 0);
	semaphore_handles[1] = xSemaphoreCreateCounting(LIMIT_BACKLOG , 0);
	semaphore_handles[2] = xSemaphoreCreateCounting(LIMIT_BACKLOG , 0);
	semaphore_handles[3] = xSemaphoreCreateCounting(LIMIT_BACKLOG , 0);
	semaphore_handles[4] = xSemaphoreCreateCounting(LIMIT_BACKLOG , 0);
}

void create_timers()
{
	task_timer_handles[0] = xTimerCreate( "taskSLR_task_timer", pdMS_TO_TICKS(1000), pdTRUE, (void*)(uintptr_t)0, mcm_task_timer_callback_func );
	offset_timer_handles[0] = xTimerCreate( "taskSLR_offset_timer", 1, pdFALSE, (void*)(uintptr_t)0, mcm_offset_timer_callback_func );

	task_timer_handles[1] = xTimerCreate( "taskSMCR_task_timer", pdMS_TO_TICKS(1500), pdTRUE, (void*)(uintptr_t)1, mcm_task_timer_callback_func );
	offset_timer_handles[1] = xTimerCreate( "taskSMCR_offset_timer", 1, pdFALSE, (void*)(uintptr_t)1, mcm_offset_timer_callback_func );

	task_timer_handles[2] = xTimerCreate( "taskRMCR_task_timer", 1, pdTRUE, (void*)(uintptr_t)2, mcm_task_timer_callback_func );
	offset_timer_handles[2] = xTimerCreate( "taskRMCR_offset_timer", 1, pdFALSE, (void*)(uintptr_t)2, mcm_offset_timer_callback_func );

	task_timer_handles[3] = xTimerCreate( "taskULR_task_timer", pdMS_TO_TICKS(1000), pdTRUE, (void*)(uintptr_t)3, mcm_task_timer_callback_func );
	offset_timer_handles[3] = xTimerCreate( "taskULR_offset_timer", 1, pdFALSE, (void*)(uintptr_t)3, mcm_offset_timer_callback_func );

	task_timer_handles[4] = xTimerCreate( "taskUMCR_task_timer", pdMS_TO_TICKS(3000), pdTRUE, (void*)(uintptr_t)4, mcm_task_timer_callback_func );
	offset_timer_handles[4] = xTimerCreate( "taskUMCR_offset_timer", 1, pdFALSE, (void*)(uintptr_t)4, mcm_offset_timer_callback_func );

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
	.offset_timer_handles = offset_timer_handles,
	.semaphore_handles = semaphore_handles,
};

void mcm_init()
{
	create_semaphores();
	create_timers();
	mcm_initial_setup(&sys_config, MODE_0);
	create_tasks();
	mcm_start_initial_tasks();
}

