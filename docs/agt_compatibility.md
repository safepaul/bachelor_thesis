# Action-Guard-Type compatibility tables

## ACTION_SUSPEND
Only with task type OLD

| SUSPEND | Old                                                                                                                                                                                                                                                                                                                                                                                                    |
|---------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| True    | "Suspend Immediately". Clear its backlog; if there was a running job it will finish (change state when it finishes [eventgroup] or at the action instant?). When the task gets action_released back again, that action will change parameters if necessary and call timer_reset.                                                                                                   |
| OLR     | "Suspend with a margin". apply the guard_true logic when the offset expires (stop timer and clear backlog) |
| OMCR    | Same as OLR.                                                                                                                                                                                                                                                                                                                                                                                            |
| BZERO   | "Suspend when its backlog is empty". Stop timer and clear backlog even though its empty. When its backlog hits 0 AND its last job finishes executing, change state. [XXX] add a timeout?|
| BGLOBAL | Same as BZERO, but after all backlogs hit 0.|

---

## ACTION_CONTINUE
Only with tasks of type: UNCHANGED

| CONTINUE | Unchanged                                                                                                                |
|----------|--------------------------------------------------------------------------------------------------------------------------|
| True     | No special actions. Can be optimized so that the task doesn't get affected by the transition |
| OLR      | [O] Doesn't make too much sense. Maybe start timer back again in some time to avoid congestion...                        |
| OMCR     | [O] Same as OLR.                                                                                                         |
| BZERO    | [O] Maybe to let it continue after its backlog is clear, but why?                                                        |
| BGLOBAL  | [O] Same as BZERO.                                                                                                       |

---

## ACTION_RELEASE
Only with tasks of type: NEW

| RELEASE | New                                                                                                     |
|---------|---------------------------------------------------------------------------------------------------------|
| True    | "Release at the mcr_instant". Change parameters if necessary, release a job once and restart its timer. |
| OLR     | "Release after X time". same as True|
| OMCR    | Same as OLR.|
| BZERO   | [O] Makes no sense. Its backlog should be already cleared.                                              |
| BGLOBAL | [O] Podria ser:  No arranques la tarea nueva hasta que TODAS las viejas hayan terminado su trabajo      |

---

## ACTION_UPDATE
Only with tasks of type: CHANGED

| UPDATE  | Changed                                                                                                                                |
|---------|----------------------------------------------------------------------------------------------------------------------------------------|
| True    | Clear backlog (because parameters change and priority may change (not sure about this)). Change parameters. |
| OLR     | "Update with a margin". jobs with old parameters keep on releasing until timer expires. when timer expires, clear backlog and change parameters. The following release will be with the old timer and new priority but the upcoming ones will have updated period |
| OMCR    | Same as OLR |
| BZERO   | "Change parameters after its backlog gets cleared". Clear backlog, even though it should be cleared, but still do it for action code consistency. Change parameters. xTimerChangePeriod() starts the timer back again, which should have been stopped in the guard. |
| BGLOBAL | Same as BZERO.|
