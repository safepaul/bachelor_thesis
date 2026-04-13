import sys
import os
import json


# open the JSON file and extract its contents to 'data'
with open("model.json") as spec:
    data = json.load(spec)

    g_transition_list = data.get("transitions")
    g_transition_count = len(g_transition_list)
    g_task_list = data.get("tasks")
    g_task_count = len(g_task_list)
    g_mode_list = data.get("modes")
    g_mode_count = len(g_mode_list)


######################
#  UTILITY FUNCTIONS
######################

# Returns the initial mode or None if no mode with id 0 was found
def fetch_initial_mode(mode_list):
    for mode in mode_list:
        if mode.get("id") == 0:
            return mode

# Returns the id of the transition from source to dest if exists; if not, it returns None 
def fetch_transition(transition_list, source, dest):
    for tr in transition_list:
        if tr.get("source_mode") == source and tr.get("dest_mode") == dest:
            return tr.get("trans_id")
    
    return None

# Returns if a task is in the initial mode (mode id = 0) along with its parameters
def is_task_in_mode_init(task_id, mode_init):
    active_tasks = mode_init.get("active_tasks")
    for task in active_tasks:
        params = task.get("parameters")
        if task.get("id") == task_id:
            return (True, params.get("period"), params.get("priority")) 

    return (False, 1, 1)

# Expands the type of a task from a character to a macro name definition
def task_type_expand(type_character):
    if type_character == 'C':
        return "MCM_TASK_TYPE_CHANGED"
    elif type_character == 'U':
        return "MCM_TASK_TYPE_UNCHANGED"
    elif type_character == 'N':
        return "MCM_TASK_TYPE_NEW"
    elif type_character == 'O':
        return "MCM_TASK_TYPE_OLD"

    return None

def job_action_expand(action):
    if action == 'release':
        return "ACTION_RELEASE"
    elif action == 'suspend':
        return "ACTION_SUSPEND"
    elif action == 'update':
        return "ACTION_UPDATE"
    elif action == 'continue':
        return "ACTION_CONTINUE"

    return None

def job_guard_expand(guard):
    if guard == 'true':
        return "GUARD_TRUE"
    elif guard == 'backlog_zero':
        return "GUARD_BACKLOG_ZERO"
    elif guard == 'offsetmcr':
        return "GUARD_OFFSETMCR"
    elif guard == 'offsetlr':
        return "GUARD_OFFSETLR"

    return None

def check_guard_value_compatibility(guard, guard_value):
    if guard == "true":
        if guard_value == -1:
            return CheckOk
    elif guard == "backlog_zero":
        if guard_value == -1:
            return CheckOk
    elif guard == "offsetmcr":
        if guard_value >= 0:
            return CheckOk
    elif guard == "offsetlr":
        if guard_value >= 0:
            return CheckOk
    
    return CheckNotOk


def check_agt_compatibility(task):
    task_id = task.get("id")
    task_type = task.get("type")
    primitives = task.get("primitives")
    action = primitives.get("action")
    guard = primitives.get("guard")
    guard_value = primitives.get("guard_value")

    ## A_RELEASE -> Type NEW (T - OL - OM)
    ## A_SUSPEND -> Type OLD (T - OL - OM - B) 
    ## A_UPDATE -> Type CHANGED (T - OL - OM - B)  
    ## A_CONTINUE -> Type UNCHANGED (T) 
    if task_type == 'C':
        if action == "update":
            # TODO: fix and finish this
            if guard is in ["true", "backlog_zero", "offsetmcr", "offsetlr"]
                ret = check_guard_value_compatibility(guard, guard_value)

            else:
                return f"Task: {task_id}: Invalid guard '{guard}'"
        else:
            return f"Task: {task_id}: Invalid action '{action}' for task type '{task_type_expand(task_type)}'"

        # passed all checks
        return None

    elif task_type == 'U':

    elif task_type == 'N':

    elif task_type == 'O':

    else:
        return f"Task: {task_id}: Invalid task type '{task_type}'"
    

def print_validation_errors(error_list):
    print("error")

######################
# VALIDATION FUNCTION
######################

def validate():
    errors = []

    # AGT validation
    for transition in g_transition_list:
        for task in transition.get("taskset"):
             errors.append(check_agt_compatibility(task))

    print_validation_errors(errors)


######################
# GENERATION FUNCTION
######################

def generate():
    with open("include/gen_data.h", "w") as h:

        h.write("#ifndef GEN_DATA_H\n")
        h.write("#define GEN_DATA_H\n")

        h.write("\n")

        n_tasks = len(data.get("tasks"))
        n_trans = len(data.get("transitions"))
        n_modes = len(data.get("modes"))

        h.write("\n")

        # mode defines
        for mode in data.get("modes"):
            h.write(f"#define {mode.get("name")} (uint8_t) {mode.get("id")}\n")

        h.write("\n")

        h.write(f"#define N_TASKS {n_tasks}\n")
        h.write(f"#define N_TRANS {n_trans}\n")
        h.write(f"#define N_MODES {n_modes}\n")
        h.write("#define LIMIT_BACKLOG (uint8_t) 5\n")

        h.write("\n")

        h.write("void mcm_init();\n")

        h.write("\n")

        h.write("#endif //GEN_DATA_H")

        pass

    with open("src/gen_data.c", "w") as s:


        initial_mode = fetch_initial_mode(g_mode_list)

        ######################
        #     INCLUDES
        ######################

        s.write("#include \"tasks.h\"\n")
        s.write("#include \"gen_data.h\"\n")
        s.write("#include \"mcmanager.h\"\n")
        # s.write("#include \"stddef.h\"\n")
        # s.write("#include \"freertos/idf_additions.h\"\n")
        # s.write("#include \"mcm_types.h\"\n")
        s.write("\n")

        ######################
        #    DECLARATIONS
        ######################

        s.write("TaskHandle_t task_handles[N_TASKS];\n")
        s.write("SemaphoreHandle_t semaphore_handles[N_TASKS];\n")
        s.write("TimerHandle_t task_timer_handles[N_TASKS];\n")
        s.write("TimerHandle_t offset_timer_handles[N_TASKS];\n")
        s.write("\n")

        ######################
        #    DEFINITIONS
        ######################

        # ---  MODE TRANSITIONS ---
        s.write("const uint8_t mode_transitions[N_MODES * N_MODES] =\n")
        s.write("{\n")

        for i in range(g_mode_count * g_mode_count):
            transition_id = fetch_transition(g_transition_list, i//g_mode_count, i%g_mode_count)
            if transition_id == None:
                transition_id = "NO_TRANSITION"

            s.write(f"\t[{i}] = {transition_id},\n")

        s.write("};\n\n")

        ## --- TRANSITION TASKSETS --- 
        for tr in g_transition_list:
            transition_id = tr.get("trans_id")
            taskset = tr.get("taskset")
            taskset_size = len(taskset)

            s.write(f"static const mcm_transition_task_t trans_{transition_id}_taskset[{taskset_size}] =\n")
            s.write("{\n")

            for task in taskset:
                task_id = task.get("id")
                task_type = task_type_expand(task.get("type"))
                primitives = task.get("primitives")
                action = job_action_expand(primitives.get("action"))
                guard = job_guard_expand(primitives.get("guard"))
                guard_value = primitives.get("guard_value")

                s.write(f"\t(mcm_transition_task_t){{ .transition_id = {transition_id}, .id = {task_id}, .type = {task_type}, .primitives = (mcm_task_primitives_t){{ ")
                s.write(f".action = {action}, ")
                s.write(f".guard = {guard}, ")
                s.write(f".guard_value = {guard_value}, ")
                s.write("} },\n")

            s.write("};\n\n")


        ## --- TRANSITIONS ---
        s.write("const mcm_transition_t transitions[N_TRANS] =\n")
        s.write("{\n")

        for transition in g_transition_list:
            transition_id = transition.get("trans_id")
            source_mode = transition.get("source_mode")
            dest_mode = transition.get("dest_mode")
            taskset_size = len(transition.get("taskset"))

            s.write(f"\t[{transition_id}] = {{ .id = {transition_id}, .source_mode = {source_mode}, .dest_mode = {dest_mode}, .taskset_size = {taskset_size}, .taskset = trans_{transition_id}_taskset }},\n")

        s.write("};\n\n")


        ## --- MODE TASKS --- 
        for mode in g_mode_list:
            mode_id = mode.get("id")
            active_tasks = mode.get("active_tasks")
            n_active_tasks = len(active_tasks)

            s.write(f"static const mcm_mode_task_t mode_{mode_id}_tasks[{n_active_tasks}] =\n")
            s.write("{\n")

            count = 0
            for task in active_tasks:
                parameters = task.get("parameters")
                period = parameters.get("period")
                priority = parameters.get("priority")
                task_id = task.get("id")

                s.write(f"\t[{count}] = {{ .id = {task_id}, .parameters = (mcm_task_params_t){{ .period = {period}, .priority = {priority} }} }}, \n") 
                count += 1

            s.write("};\n\n")


        ## --- MODES ---
        s.write("const mcm_mode_t modes[N_MODES] =\n")
        s.write("{\n")

        for mode in g_mode_list:
            mode_id = mode.get("id")
            mode_name = mode.get("name")
            n_active_tasks = len(mode.get("active_tasks"))

            s.write(f"\t[{mode_id}] = {{ .id = {mode_id}, .n_tasks = {n_active_tasks}, .tasks = mode_{mode_id}_tasks  }},\n")

        s.write("};\n\n")


        ## --- TASKS ---
        s.write("mcm_task_t tasks[N_TASKS] =\n")
        s.write("{\n")

        for task in range(g_task_count):
            s.write(f"\t[{task}] = {{ .id = {task}, .last_release = 0 }},\n")

        s.write("};\n\n")



        ######################
        #     C FUNCTIONS
        ######################

        ## --- CREATE TASKS ---
        s.write("void create_tasks()\n")
        s.write("{\n")

        for task in g_task_list:
            task_name = task.get("name")
            task_id = task.get("id")
            is_initial, _, priority = is_task_in_mode_init(task_id, initial_mode)
            
            if(is_initial):
                s.write(f"\txTaskCreate( {task_name}_utask, \"{task_name}\", 2048, (void*)(uintptr_t){task_id}, {priority}, &task_handles[{task_id}] );\n")
            else:
                s.write(f"\txTaskCreate( {task_name}_utask, \"{task_name}\", 2048, (void*)(uintptr_t){task_id}, 1, &task_handles[{task_id}] );\n")

        s.write("}\n\n")


        # ---  CREATE SEMAPHORES  ---
        s.write("void create_semaphores()\n")
        s.write("{\n")

        for task in g_task_list:
            task_id = task.get("id")
            s.write(f"\tsemaphore_handles[{task_id}] = xSemaphoreCreateCounting(LIMIT_BACKLOG , 0);\n")

        s.write("}\n\n")

        ## ---  CREATE TIMERS  ---
        s.write("void create_timers()\n")
        s.write("{\n")

        for task in g_task_list:
            task_id = task.get("id")
            is_initial, period, _ = is_task_in_mode_init(task_id, initial_mode);

            # -- TASK timers --
            if(is_initial):
                s.write(f"\ttask_timer_handles[{task_id}] = xTimerCreate( \"{task.get("name")}_task_timer\", pdMS_TO_TICKS({period}), pdTRUE, (void*)(uintptr_t){task_id}, mcm_task_timer_callback_func );\n")
            else:
                s.write(f"\ttask_timer_handles[{task_id}] = xTimerCreate( \"{task.get("name")}_task_timer\", 1, pdTRUE, (void*)(uintptr_t){task_id}, mcm_task_timer_callback_func );\n")

            # OFFSET timers
            s.write(f"\toffset_timer_handles[{task_id}] = xTimerCreate( \"{task.get("name")}_offset_timer\", 1, pdFALSE, (void*)(uintptr_t){task_id}, mcm_offset_timer_callback_func );\n")
            s.write("\n")

        s.write("}\n\n")


        ######################
        #        MISC
        ######################

        ## ---  CONFIGURATION STRUCTURE  ---
        s.write("mcm_config_t sys_config = \n")
        s.write("{\n")

        s.write("\t.n_tasks = N_TASKS,\n")
        s.write("\t.n_modes = N_MODES,\n")
        s.write("\t.n_trans = N_TRANS,\n")
        s.write("\t.tasks = tasks,\n")
        s.write("\t.modes = modes,\n")
        s.write("\t.transitions = transitions,\n")
        s.write("\t.mode_transitions = mode_transitions,\n")
        s.write("\t.task_handles = task_handles,\n")
        s.write("\t.task_timer_handles = task_timer_handles,\n")
        s.write("\t.offset_timer_handles = offset_timer_handles,\n")
        s.write("\t.semaphore_handles = semaphore_handles,\n")

        s.write("};\n\n")

        ## ---  INITIALIZATION FUNCTION  ---
        s.write("void mcm_init()\n")
        s.write("{\n")

        s.write("\tcreate_semaphores();\n")
        s.write("\tcreate_timers();\n")
        s.write(f"\tmcm_initial_setup(&sys_config, {initial_mode.get("name")});\n")
        s.write("\tcreate_tasks();\n")
        s.write("\tmcm_start_initial_tasks();\n")

        s.write("}\n\n")

        pass




if __name__ == "__main__":
    validate()
    generate()

