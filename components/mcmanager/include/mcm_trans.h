#ifndef MCM_TRANS_H
#define MCM_TRANS_H

#include <stdint.h>
#include "mcm_types.h"

void mcm_trans_handle_guard_none(const mcm_transition_task_t *task, const uint8_t dest_mode_id);
void mcm_trans_handle_guard_backlog(const mcm_transition_task_t *task, const uint8_t dest_mode_id);
void mcm_trans_handle_guard_offset(const mcm_transition_task_t *task, const uint8_t dest_mode_id, const TickType_t mcr_instant);
void mcm_trans_poll_backlogs(const uint8_t trans_id, const uint8_t current_mode, const uint8_t guard);
static uint16_t mcm_trans_calculate_global_backlog(const uint8_t current_mode);



#endif // !MCM_TRANS_H
