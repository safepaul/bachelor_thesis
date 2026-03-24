#ifndef MCMANAGER_H
#define MCMANAGER_H

#include <stdbool.h>
#include <stdint.h>

#include "mcm_types.h"


#define MCM_MAX_TASKS   (uint8_t)   24
#define BACKLOG_MAX     (uint8_t)   5
#define NO_TRANSITION   (uint8_t)   255


void mcm_initial_setup(mcm_config_t *sys_config, const uint8_t initial_mode);
void mcm_wait_for_release(const uint8_t task_id);
void mcm_mc_request(const uint8_t target_mode);
void mcm_task_timer_callback_func(TimerHandle_t xTimer);
void mcm_offset_timer_callback_func(TimerHandle_t xTimer);


#endif // !MCMANAGER_H
