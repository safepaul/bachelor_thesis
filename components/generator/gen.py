# generate .c and .h files

# MG: create a .c, .h file and include the .h in the .c
# TODO: change paths from relative to absolute


import sys
import os

# Add the tools/lib/ folder containing the yaml parser to path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(current_dir, 'tools/lib'))

import yaml

def generate():
    with open("include/gen_data.h", "w") as h:
        # Include guards
        h.write("#ifndef GEN_DATA_H\n")
        h.write("#define GEN_DATA_H\n\n")


        # h.write("extern const char* task_names[];\n")
        # h.write("extern const task_info_t task_info[];\n")

        h.write("void generate_data();\n")
        # h.write("void spawn_tasks();\n")


        # Include guards
        h.write("\n#endif //GEN_DATA_H")
        pass

    with open("src/gen_data.c", "w") as s:
        s.write("#include \"gen_data.h\"\n")
        s.write("#include \"mcmanager.h\"\n")
        s.write("#include \"stddef.h\"\n")
        s.write("#include \"tasks.h\"\n")
        s.write("\n\n")



        # open the YAML file and extract its contents to 'data'
        with open("model.yaml") as spec:
            data = yaml.safe_load(spec)










        s.write("\n\n")


        s.write("void generate_data(){\n\n")


        # task information
        for task in data.get("tasks"):

            s.write("\n")

            s.write(f"\tmcm_add_task_info( {task.get("id")}, &{task.get("func")}, \"{task.get("name")}\" );")


        s.write("\n\n")

        # transition information
        for tr in data.get("transitions"):



            ## CREATING TRANSITION
            # print(type(tr))
            tr_id = tr.get("trans_id")
            # print(id)
            tr_src = tr.get("source_mode")
            tr_dst = tr.get("dest_mode")
            s.write(f"\tmcm_create_transition({tr_id}, {tr_src}, {tr_dst});\n")


            ## CREATING ALL TRANSITION TASKS of the transition
            for task in tr.get("taskset"):
                # id, parameters and primitives
                
                # ID
                task_id = task.get("task_id")
                print(f"task id: {task_id}")
                print(type(task))
                print(type(task.get("parameters")))

                # PARAMETERS and PRIMITIVES

                parameters = task.get("parameters")
                
                task_period = parameters.get("period", 0)
                task_priority = parameters.get("priority", 0)
                s.write(f"\tmcm_add_task_to_transition({task_id}, {tr_id}, (task_params_t)")
                s.write(f"{{ .period = {task_period}, .priority = {task_priority} }}")



                # if the member doesnt exist, default to null
                primitives = task.get("primitives")
                anew = primitives.get("anew", "ACTION_NONE")
                gnew = primitives.get("gnew", "GUARD_NONE")
                gnewval = primitives.get("gnewval", -1)
                aexec = primitives.get("aexec", "ACTION_NONE")
                gexec = primitives.get("gexec", "GUARD_NONE")
                gexecval = primitives.get("gexecval", -1)


                s.write(f", (job_primitives_t){{.anew = {anew}")
                s.write(f", .gnew = {gnew}")
                s.write(f", .gnewval = {gnewval}")
                s.write(f", .aexec = {aexec}")
                s.write(f", .gexec = {gexec}")
                s.write(f", .gexecval = {gexecval}")



                
                s.write("});\n")



        
        













        s.write("\n}")
        pass




if __name__ == "__main__":
    generate()

