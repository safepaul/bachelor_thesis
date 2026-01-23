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
        h.write("#define MODE_NONE (uint8_t) 0\n")
        h.write("#define MODE_INIT (uint8_t) 1\n")
        h.write("\n\n")
        h.write("// useful constants for the program\n")
        h.write(f"#define N_TASKS {n_tasks}\n")
        h.write(f"#define N_TRANS {n_trans}\n")
        h.write(f"#define N_MODES {n_modes}\n")
        h.write("\n")
        h.write("// macro for declaring that a transition doesn't exist or hasn't been found\n")
        h.write("#define NO_TRANSITION (uint8_t) 255\n")
        h.write("\n\n")



        h.write("// public array containing all transitions  \n")
        h.write("extern const transition_t transitions[N_TRANS];\n\n")

        h.write("// lookup table (2d array) containing the id's of the transitions corresponding to each mode pair\n")
        h.write("// mode_transitions[i][j] means the transition id that goes from mode i to mode j \n")
        h.write("extern const uint8_t mode_transitions[N_MODES][N_MODES];\n\n")


        # NOTE: I can maybe make this array constant, but not such big of a deal
        h.write("// array holding task handles\n")
        h.write("extern TaskHandle_t task_handles[N_TASKS];\n")

        h.write("\n")
        h.write("// function for spawning tasks, for their handles and initialization\n")
        h.write("void spawn_tasks();\n")




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

        s.write("TaskHandle_t task_handles[N_TASKS];\n\n\n\n")


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

        ## taskset specific arrays, one per transition, containing data on what and how to perform the actions to a specific task in a specific transition
        for transition in data.get("transitions"):
            trans_id = transition.get("trans_id")
            taskset = transition.get("taskset")
            taskset_size = len(taskset)

            s.write(f"static const task_trans_data_t trans_{trans_id}_taskset[{taskset_size}] = {{\n\n")

            for task in taskset:
                parameters = task.get("parameters")
                primitives = task.get("primitives")

                s.write(f"\t(task_trans_data_t){{ .transition_id = {trans_id}, .task_id = {task.get("task_id")}, .params = (task_params_t){{ .period = {parameters.get("period")}, .priority = {parameters.get("priority")} }}, .primitives = (job_primitives_t){{ ")
                s.write(f".anew = {primitives.get("anew", "ACTION_NONE")}, ")
                s.write(f".gnew = {primitives.get("gnew", "GUARD_NONE")}, ")
                s.write(f".gnewval = {primitives.get("gnewval", -1)}, ")
                s.write(f".aexec = {primitives.get("aexec", "ACTION_NONE")}, ")
                s.write(f".gexec = {primitives.get("gexec", "GUARD_NONE")}, ")
                s.write(f".gexecval = {primitives.get("gexecval", -1)}, ")
                s.write("} },\n")

            s.write("\n};\n\n")


        ## main array, containing all transitions
        s.write("const transition_t transitions[N_TRANS] = {\n\n")

        for transition in data.get("transitions"):
            trans_id = transition.get("trans_id")
            source_mode = transition.get("source_mode")
            dest_mode = transition.get("dest_mode")
            taskset_size = len(transition.get("taskset"))

            s.write(f"\t[{trans_id}] = {{ .id = {trans_id}, .source_mode = {source_mode}, .dest_mode = {dest_mode}, .taskset_size = {taskset_size}, .taskset = trans_{trans_id}_taskset }},\n")


        s.write("\n};\n\n")



        ## function for spawning the tasks, all suspended with some placeholder values, until the system is set-up and can move to the initial mode
        s.write("void spawn_tasks(){\n\n")

    
        s.write("\t// stop scheduler\n")
        s.write("\tvTaskSuspendAll();\n\n")



        # s.write("\tfor(int i = 0; i < N_TASKS; i++){\n\n")

        # h.write("extern TaskHandle_t task_handles[N_TASKS];\n")

        for task in data.get("tasks"):
            func = task.get("func")
            name = task.get("name")
            id = task.get("id")

            s.write(f"\txTaskCreate( {func}, \"{name}\", 2048, NULL, 1, &task_handles[{id}] );\n")
            s.write(f"\tvTaskSuspend( task_handles[{id}] );\n")

        s.write("\n\txTaskResumeAll();\n")

        s.write("\n}\n")







        s.write("\n\n")
        pass




if __name__ == "__main__":
    generate()

