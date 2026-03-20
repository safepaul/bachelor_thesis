# Variables

### mcr_instant

*static uint32_t mcr_instant;*

**Description**: Holds the value of the timestamp of the most recent mode change request in milliseconds.

### current_mode

*static uint8_t current_mode;*

**Description**: Holds the current mode ID.

### system_state

*static mcm_system_state_t system_state;*

**Description**: Holds the current state of the system.

### config 

*static mcm_config_t config;*

**Description**: Holds a pointer to the system configuration, meaning the transitions, modes, tasks... coming from the generated data.

### mode_transitions

*uint8_t mode_transitions[N_MODES*N_MODES]*

**Description**: A flattened 2D array that serves as a lookup table for transition id's.


### 

**

**Description**:






# Functions

### mcm_mc_request

[XXX]: Se suspende el planificador para garantizar la atomicidad de la transición y evitar condiciones de carrera si una tarea de alta prioridad es liberada durante el bucle de actualización

void mcm_mc_request(uint8_t target_mode_id)

**Description**: Signals the system to perform a mode transition from the mode it was in to the target mode.

- Nuances:
    - At the beginning of the mcr, **system_state** is set to SYSTEM_STATE_TRANSITIONING. When the mcr finishes being processed, **system_state** is set to either SYSTEM_STATE_NORMAL or SYSTEM_STATE_TRANSIENT, depending on if asynchronous actions have to be taken or not, determined by the return value of mcm_perform_transition().
    - If a mode change request is issued while the system is in a transitioning state, [TODO]
    - If a mode change request is issued while the system is in a transient state, [TODO]


> [!IMPORTANT]
> A mode transition is considered complete (the mode has been successfully changed) when all tasks have been processed, i.e. when all immediate actions on tasks have been taken and all non-immediate actions have been scheduled or asynchronously handled, like in the cases of the offset and backlog guards.
> After a transition is complete, if there are still actions pending to be applied, the system will remain in a "transient state" until those pending actions are handled.

### mcm_initial_setup

void mcm_initial_setup(mcm_config_t *sys_config)

**Description**: Sets up the initial state of the system.

- Nuances:
    - MUST be called from mcm_init() in main().
    - MUST be called before the task creation to avoid null config pointer.
    - Initializes the starting system configuration **config** to the one coming from the generator. 
    - Sets global variable **system_state** to SYSTEM_STATE_NORMAL.
    - Sets global variable **current_mode** to the *initial_mode* argument.
    - Assigns the global mutex **transition_mutex** a semaphore handle.
    - Sets both backlog and offset bitmasks to 0.


### mcm_perform_transition  

static mcm_trans_result_t mcm_perform_transition(const mcm_transition_t *transition, const uint8_t target_mode)

**Description**: Performs the passed-on transition. Returns either MCM_TRANS_SYNC_ALL or MCM_TRANS_SYNC_PENDING.

- Nuances:
    - The function takes the **transition_mutex** so that no new jobs can be released. It gives the mutex once the transition is finished processing.


### mcm_perform_action

static void mcm_perform_action(const mcm_transition_task_t *task)

**Description**: Performs the corresponding action to the passed-on task.


### mcm_process_task  

static bool mcm_process_task(const mcm_transition_task_t *task)

**Description**: Processes a task. Checks its guard and performs its action if possible.

- Nuances:
    - Returns True if the processed task triggered an asynchronous action. Returns False if the action was performed immediately.


### mcm_wait_for_release

void mcm_wait_for_release(const uint8_t task_id)

**Description**: Stalls the execution of the calling task until it gets released by mcm_release_job().

- Nuances:
    - Internally, this function uses FreeRTOS Semaphores to stall and release the tasks.
    - It has a call to take and give the **transition_mutex** in case there is an ongoing transition.


### mcm_task_timer_callback_func 

void mcm_task_timer_callback_func(TimerHandle_t xTimer)

**Description**: Callback function that task timers use to periodically release the jobs of the tasks.

- Nuances:


### mcm_get_backlog

void mcm_get_backlog(const uint8_t task_id)

**Description**: Returns the backlog of the specified task.

- Nuances:
    - Designed to be platform agnostic.


### mcm_fetch_transition_id

static uint8_t mcm_fetch_transition_id(const uint8_t source_mode, const uint8_t dest_mode)

**Description**: Returns the transition id of the transition that goes from source_mode to dest_mode.

- Nuances:


### mcm_release_job

static void mcm_release_job(const uint8_t task_id)

**Description**: Releases a job of the task with id task_id

- Nuances:
    - Internally, it 'gives' to the semaphore that corresponds to the task.


### mcm_start_initial_task_timers

static void mcm_start_initial_task_timers()

**Description**: Starts the timers of the initial tasks. Basically "launches" the system.

- Nuances:
    - It MUST be called just once at startup.
    - It also releases the first jobs of the initial tasks, instead of them being released after their first timer period concludes.


### mcm_clear_backlog

static void mcm_clear_backlog(const uint8_t task_id)

**Description**: Clears the backlog of the specified task.

- Nuances:
    - Because of how Semaphores work, clearing the semaphore count is not straightforward, so this function takes from the semaphore repeatedly until it's empty.

### mcm_check_backlog_status

static void mcm_check_backlog_status(const uint8_t task_id)

**Description**: Checks if the task was waiting for its backlog to clear and removes its bit from the backlog bitmask if it was. It also changes the system state to NORMAL in case it was the last one of the asynchronous tasks from the last mode transition and performs the scheduled action.

- Nuances:
    - This function is meant to be used just after the task takes from the semaphore to release one of its jobs, in case it was waiting for its backlog to clear and signal it to the system.



# Primitives
## Guards
### GUARD_TRUE
**Description**: No conditions required for performing the action. Perform immediately.

### GUARD_BACKLOG_ZERO
**Description**: Perform the action when the backlog of the task reaches 0.

## Actions
