    // mcmgr checks
    //  - which is the destination mode
    //  - check (maybe said in the transition table) what changes in each task (active->suspended, active->active witch diff params, stays the same) for the transition
    //  - checks how the change has to be done for the specific transition (immediately suspend current tasks, let them run until finish...)
    //  - check if the change has been done as expected *(for later)
    //
    //  XXX: As the mcmanager will have to start tasks for the first time when needed, and maybe delete some, it will probably have to have a function
    //  that creates the tasks with some certain parameters (name, priority, args...) and adds the handle to the task_name-handle mapping Data structure,
    //  among other things.
    //  Having mcmanager create the tasks eliminates the need for the code generator to generate the code to create and save the data of each task, as the
    //  function that will create the tasks on demand is the same for every task. For now, only the task_info Data Structure has to be the one filled with
    //  generated info from the XML (that I'm aware of right now).
    //  Following this concept opens the door to maybe other functionality that can be relegated to the MCManager
