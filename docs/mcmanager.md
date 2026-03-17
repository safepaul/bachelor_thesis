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

**Description**:

### 

**

**Description**:






# Functions

### mcm_mc_request

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
    - Initializes the starting system configuration **config** to the one coming from the generator. 
    - Sets global variable **system_state** to SYSTEM_STATE_NORMAL.
    - Sets global variable **current_mode** to the *initial_mode* argument.


### mcm_perform_transition  

static mcm_trans_result_t mcm_perform_transition(const mcm_transition_t *transition, const uint8_t target_mode)

**Description**: Performs the passed-on transition. Returns either MCM_TRANS_SYNC_ALL or MCM_TRANS_SYNC_PENDING.


### mcm_process_task  

static bool mcm_process_task(const mcm_transition_task_t *task)

**Description**: Processes a task. Checks its guard and performs its action if possible.

- Nuances:
    - Returns True if the processed task triggered an asynchronous action. Returns False if the action was performed immediately.


### mcm_wait_for_release

void mcm_wait_for_release(const uint8_t task_id)

**Description**: Stalls the execution of the calling task until it receives a release signal.

- Nuances:
    - Internally, this function uses FreeRTOS Task Notifications to stall and release the tasks.


### mcm_get_backlog

void mcm_get_backlog(const uint8_t task_id)

**Description**: Returns the backlog of the specified task.

- Nuances:
    - Designed to be platform agnostic.




# Primitives
## Guards
### GUARD_TRUE
**Description**: No conditions required for performing the action. Perform immediately.

### GUARD_BACKLOG_ZERO
**Description**: Perform the action when the backlog of the task reaches 0.

## Actions
