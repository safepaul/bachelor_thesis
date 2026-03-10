#ifndef MCMANAGER_H
#define MCMANAGER_H

#include "freertos/idf_additions.h"
#include <stdbool.h>
#include <stdint.h>

#include "mcm_types.h"
#include "portmacro.h"

#define MAX_TRANS_TASKS (uint16_t)   N_TRANS * N_TASKS 
#define BACKLOG_MAX     (uint8_t)    5



/*
 *
 *
*/
void mcm_initial_setup();

/*
 *
 *
*/
void mcm_apply_primitive(const mcm_transition_task_t *task);

/*
 *
 *
*/
void mcm_mc_request(const uint8_t target_mode);

/*
 *
 *
*/
void mcm_perform_action(const uint8_t task_id, const uint8_t action, const uint8_t mode_id);

/*
 *
 *
*/
void mcm_perform_transition(const uint8_t trans_id);

/*
 *
 *
*/
void mcm_wait_for_release(const uint8_t task_id);

/*
 *
 *
*/
void mcm_release_job(const uint8_t task_id);

/*
 *
 *
*/
void mcm_clean_backlog(const uint8_t task_id);

/*
 *
 *
 */
void mcm_change_parameters(const uint8_t task_id, const uint8_t mode_id);

/*
 *
 *
 */
TickType_t mcm_calculate_offset_delay(const uint16_t offset, const TickType_t offset_reference);



/* ---  STATIC FUNCTIONS  --- */



/*
 * Timer used to release the jobs that have an offset type guard during a mode
 * change. Using a timer to avoid blocking the main calling task. Using the
 * supposed timer id xTimer as a means to pass a trans_task_t pointer argument.
 * It may have consequences.
 */
void callback_offset_timer(TimerHandle_t xTimer);

/*
 * xTimer identifies the timer, which has the same value as the id of the task
 * it is assigned to: timer id 0 <-> task id 0
 *
 */
void task_timer_callback(TimerHandle_t xTimer);


#endif // !MCMANAGER_H
