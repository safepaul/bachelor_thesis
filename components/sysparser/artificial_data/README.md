- This data is artificially generated until a proper code generation tool is set up.
- The data is needed for testing some of the components, specially the dictionary and the initialization functions.
- The properly generated data is intended to come from an XML file and stored in the build tree, generated
by the sysparser component.
- The data contains information such as: the number of tasks and modes, the active tasks in each mode, the transition
table...


## Generation objectives
- generated code has to define the task logic and
    1. create the tasks and give back the handle and arg_pointer OR
    2. give the necessary information so that the mcinit handles that (in order to fill the dictionary)
