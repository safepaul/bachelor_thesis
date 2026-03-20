# Generator


# System Model
## Transitions
- Guard values for offsets MUST be specified in milliseconds.

## Modes
- The initial mode MUST have id 0.

## Tasks
- Task id's MUST be sorted from 0 to N_TASKS - 1
- Task types MUST be one of {C,U,N,O}
- In general, if a task is of type Old, its action MUST be Suspend and viceversa.
    [XXX] doubt -> should I force the system to only use certain guard-action-type combinations? it is more restrictive but
    a lot of problems may arise and it may be a logic nightmare to make sense of those strange cases in code, like logically
    applying a suspend action in a task that is not of type old makes no sense; applying suspend to a guard with offset makes
    little sense, maybe you want it to release jobs for some timea then suspend, but if you dont want that i dont see other use case
    that makes sense.
    Also I'm wondering if I should double check for those things during the action taking and so on. Like, these types of problems
    should be handled during the generation, curating the input combinations and such, and maybe if I add a mcm_create_task() thing,
    that function should check if the input combination makes sense or not. so why should I double check if I trust that the input is
    well curated before reaching the action phase.


## General
- Generates the counting semaphores array:
    - Index is the semaphore 'i' corresponding with task with id 'i'.
