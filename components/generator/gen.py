import sys
import os


# TODO: change paths from relative to absolute
# Add the tools/lib/ folder containing the yaml parser to path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(current_dir, 'tools/lib'))

import yaml

def generate():
    with open("include/gen_data.h", "w") as h:


        with open("model.yaml") as spec:
            data = yaml.safe_load(spec)

        n_tasks = len(data.get("tasks"))
        n_trans = len(data.get("transitions"))
        n_modes = len(data.get("modes"))

        h.write("#ifndef GEN_DATA_H\n")
        h.write("#define GEN_DATA_H\n\n")

        h.write("\n\n")
        h.write("#include \"mcmanager.h\"\n\n\n")
        h.write("\n// macros for the modes\n")

        # generate mode macros
        for mode in data.get("modes"):
                h.write(f"#define {mode.get("name")} (uint8_t) {mode.get("id")}\n")

        h.write("\n\n")
        h.write("// useful constants for the program\n")
        h.write(f"#define N_TASKS {n_tasks}\n")
        h.write(f"#define N_TRANS {n_trans}\n")
        h.write(f"#define N_MODES {n_modes}\n")

        h.write("\n")
        h.write("// macro for declaring that a transition doesn't exist or hasn't been found\n")
        h.write("#define NO_TRANSITION (uint8_t) 255\n")
        h.write("\n")
        h.write("// macro for declaring that a guard has no value\n")
        h.write("#define NO_GUARD_VALUE (int16_t) -1\n")
        h.write("\n\n")


        ## --- mode table ---
        h.write("// array containing all modes and information related to them  \n")
        h.write("extern const mode_info_t modes[N_MODES];\n\n")

        ## --- transition table ---
        h.write("// array containing all transitions  \n")
        h.write("extern const transition_t transitions[N_TRANS];\n\n")

        h.write("// lookup table (2d array) containing the id's of the transitions corresponding to each mode pair\n")
        h.write("// mode_transitions[i][j] means the transition id that goes from mode i to mode j \n")
        h.write("extern const uint8_t mode_transitions[N_MODES][N_MODES];\n\n")


        # NOTE: I can maybe make this array constant, but not such big of a deal
        h.write("// array holding task handles\n")
        h.write("extern TaskHandle_t task_handles[N_TASKS];\n")
        h.write("\n")
        h.write("// array holding timer handles\n")
        h.write("extern TimerHandle_t timer_handles[N_TASKS];\n")

        h.write("\n")
        h.write("// function for spawning tasks, for their handles and initialization\n")
        h.write("void create_tasks();\n")
        h.write("\n")
        h.write("// function for creating the task timers, afterwards started and managed by the mcmanager on startup\n")
        h.write("void create_timers();\n")




        h.write("\n#endif //GEN_DATA_H")

        pass

    with open("src/gen_data.c", "w") as s:

        # open the YAML file and extract its contents to 'data'
        with open("model.yaml") as spec:
            data = yaml.safe_load(spec)


        # include directives
        s.write("#include \"gen_data.h\"\n")
        s.write("#include \"mcmanager.h\"\n")
        s.write("#include \"stddef.h\"\n")
        s.write("#include \"tasks.h\"\n")
        s.write("\n\n")

        s.write("TaskHandle_t task_handles[N_TASKS];\n")
        s.write("TimerHandle_t timer_handles[N_TASKS];\n\n\n\n")

        # returns the id of the transition from source to dest if exists; if not, it returns NULL
        ## TODO: must check if there are more than one transition with the same source+dest pair
        def fetch_transition(source, dest):
            transitions = data.get("transitions")

            for transition in transitions:
                if transition.get("source_mode") == source :
                    if transition.get("dest_mode") == dest:
                        return transition.get("trans_id")

            return None


        s.write("const uint8_t mode_transitions[N_MODES][N_MODES] = {\n\n")

        for source in data.get("modes"):
            src_id = source.get("id")
            for dest in data.get("modes"):
                dest_id = dest.get("id")
                fetch_result = fetch_transition(src_id, dest_id)
                if fetch_result == None:
                    fetch_result = "NO_TRANSITION"

                s.write(f"\t[{src_id}][{dest_id}] = {fetch_result},\n")


        s.write("\n\n};\n\n")


        # code generation


        ## --- taskset specific arrays --- 
        ## one per transition, containing data on what and how to perform the actions to a specific task in a specific transition
        for transition in data.get("transitions"):
            trans_id = transition.get("trans_id")
            taskset = transition.get("taskset")
            taskset_size = len(taskset)

            s.write(f"static const trans_task_t trans_{trans_id}_taskset[{taskset_size}] = {{\n\n")

            for task in taskset:
                primitives = task.get("primitives")

                s.write(f"\t(trans_task_t){{ .transition_id = {trans_id}, .task_id = {task.get("task_id")}, .primitives = (job_primitives_t){{ ")
                s.write(f".action = {primitives.get("action", "ACTION_NONE")}, ")
                s.write(f".guard = {primitives.get("guard", "GUARD_NONE")}, ")
                s.write(f".guard_value = {primitives.get("guard_value", -1)}, ")
                s.write("} },\n")

            s.write("\n};\n\n")


        ## --- transition table ---
        s.write("const transition_t transitions[N_TRANS] = {\n\n")

        for transition in data.get("transitions"):
            trans_id = transition.get("trans_id")
            source_mode = transition.get("source_mode")
            dest_mode = transition.get("dest_mode")
            taskset_size = len(transition.get("taskset"))

            s.write(f"\t[{trans_id}] = {{ .id = {trans_id}, .source_mode = {source_mode}, .dest_mode = {dest_mode}, .taskset_size = {taskset_size}, .taskset = trans_{trans_id}_taskset }},\n")


        s.write("\n};\n\n")


        s.write("\n\n\n")

        ## --- mode tasks specific arrays --- 
        ## one per mode, containing information about the parameters of the tasks that are active in a specific mode 
        for mode in data.get("modes"):
            mode_id = mode.get("id")
            tasks = mode.get("active_tasks")
            n_tasks = len(tasks)

            s.write(f"static const mode_task_t mode_{mode_id}_tasks[{n_tasks}] = {{\n\n")

            count = 0
            for task in tasks:
                parameters = task.get("parameters")
                s.write(f"\t[{count}] = {{ .id = {task.get("id")}, .parameters = (task_params_t){{ .period = {parameters.get("period")}, .priority = {parameters.get("priority")} }} }}, \n") 
                count += 1

            s.write("\n};\n\n")


        ## --- mode table ---
        s.write("const mode_info_t modes[N_MODES] =  {\n\n")

        for mode in data.get("modes"):
            mode_id = mode.get("id")
            mode_name = mode.get("name")
            n_tasks = len(mode.get("active_tasks"))

            s.write(f"\t[{mode_id}] = {{ .id = {mode_id}, .n_tasks = {n_tasks}, .tasks = mode_{mode_id}_tasks  }},\n")

        s.write("\n};\n\n")



        ## function for spawning the tasks, all suspended with some placeholder values, until the system is set-up and can move to the initial mode
        s.write("void create_tasks(){\n\n")

        for task in data.get("tasks"):
            name = task.get("name")
            id = task.get("id")

            s.write(f"\txTaskCreate( {name}_utask, \"{name}\", 2048, NULL, 1, &task_handles[{id}] );\n")

        s.write("\n}\n\n")




        ## function for creating the task timers, afterwards started and managed by the mcmanager on startup.
        s.write("void create_timers(){\n\n")

        for task in data.get("tasks"):
            id = task.get("id")

            s.write(f"\ttimer_handles[{id}] = xTimerCreate( \"{task.get("name")}_timer\", 1, pdTRUE, (void*)(uintptr_t){id}, task_timer_callback );\n")

        s.write("\n}\n\n")






        s.write("\n\n")
        pass




if __name__ == "__main__":
    generate()

