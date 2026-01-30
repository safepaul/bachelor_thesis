#ifndef GEN_DATA_H
#define GEN_DATA_H



#include "mcmanager.h"



// macros for the modes
#define MODE_INIT (uint8_t) 0
#define MODE_EMERGENCY (uint8_t) 1


// useful constants for the program
#define N_TASKS 2
#define N_TRANS 2
#define N_MODES 2

// macro for declaring that a transition doesn't exist or hasn't been found
#define NO_TRANSITION (uint8_t) 255

// macro for declaring that a guard has no value
#define NO_GUARD_VALUE (int16_t) -1


// array containing all modes and information related to them  
extern const mode_info_t modes[N_MODES];

// array containing all transitions  
extern const transition_t transitions[N_TRANS];

// lookup table (2d array) containing the id's of the transitions corresponding to each mode pair
// mode_transitions[i][j] means the transition id that goes from mode i to mode j 
extern const uint8_t mode_transitions[N_MODES][N_MODES];

// array holding task handles
extern TaskHandle_t task_handles[N_TASKS];

// array holding timer handles
extern TimerHandle_t timer_handles[N_TASKS];

// function for spawning tasks, for their handles and initialization
void create_tasks();

// function for creating the task timers, afterwards started and managed by the mcmanager on startup
void create_timers();

#endif //GEN_DATA_H