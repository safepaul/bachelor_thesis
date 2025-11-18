#include "../include/mcmanager.h"

#include "freertos/idf_additions.h"
#include "stdlib.h"
#include "esp_log.h"
#include <stdint.h>


// Component tag for ESP-IDF logging
static const char TAG[] = "MCMANAGER";

// Mode list initializer
static mode_info_t *modes = NULL;


void mcm_create_mode(uint16_t mode_id);
void mcm_add_task_to_mode(uint16_t mode_id, uint16_t task_id, task_params_t task_params);
void mcm_create_transition(transition_t transition);
void mcm_add_task_to_transition(transition_task_t task);
