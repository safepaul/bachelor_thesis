# Generator


# System Model
## Transitions
- Guard values for offsets MUST be specified in milliseconds.

## Modes
- The initial mode MUST have id 0.

## Tasks
- Task id's MUST be sorted from 0 to N_TASKS - 1


## General
- Generates the counting semaphores array:
    - Index is the semaphore 'i' corresponding with task with id 'i'.
