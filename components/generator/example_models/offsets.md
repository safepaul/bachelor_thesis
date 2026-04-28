Just one transition. Mode 0 -> Mode 1

- Actions:
    - [O] CONTINUE (unchanged)
    - [X] SUSPEND  (old)
    - [X] RELEASE  (new)
    - [X] UPDATE   (changed)

- Showcase all possible offsets:
    - 5 tasks, 2 tasks with OLR and another 3 with OMCR 
    - Observations:
        - 

- What data do I extract and how will I extract it?
    - What data:
        1. The **mcr_instant**. It's the reference point for the releases.
        2. The timestamp when each task should be released (show calculations) (5 timestamps) AND record when the release actually happens to compare.
    - How?
        1. 
        2.



## Model Description
- 1 transition
- 2 modes
- 5 tasks:
    - Task_0:
        - Action: Suspend
        - Guard:  OffsetLR   =   3000ms offset
        - Period: 1000ms
    - Task_1:
        - Action: Suspend
        - Guard:  OffsetMCR  =   500ms offset
        - Period: 1500ms
    - Task_2 (not active -> active):
        - Action: Release
        - Guard:  OffsetMCR  =   2000ms offset
        - Period: 500ms
    - Task_3:
        - Action: Update
        - Guard:  OffsetLR   =   2000ms offset
        - Period: 1000ms -> 3000ms
    - Task_4:
        - Action: Update
        - Guard:  OffsetMCR  =   1000ms offset
        - Period: 3000ms -> 500ms

- MCR occurs at second (10)
