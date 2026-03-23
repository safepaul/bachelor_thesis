#ifndef GEN_DATA_H
#define GEN_DATA_H

#include "freertos/idf_additions.h"
#include "mcm_types.h"

#define MODE_INIT (uint8_t) 0
#define MODE_EMERGENCY (uint8_t) 1

#define N_TASKS 2
#define N_TRANS 2
#define N_MODES 2
#define LIMIT_BACKLOG (uint8_t) 5

extern const mcm_mode_t modes[N_MODES];
extern mcm_task_t tasks[N_TASKS];
extern const mcm_transition_t transitions[N_TRANS];
extern const uint8_t mode_transitions[N_MODES * N_MODES];
extern TaskHandle_t task_handles[N_TASKS];
extern TimerHandle_t task_timer_handles[N_TASKS];
extern TimerHandle_t offset_timer_handles[N_TASKS];
extern SemaphoreHandle_t semaphore_handles[N_TASKS];

void create_tasks();
void create_timers();
void mcm_init();

#endif //GEN_DATA_H