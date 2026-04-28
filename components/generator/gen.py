import sys
import os
import json


# ==========================================
# CONSTANTES Y LÍMITES DEL SISTEMA
# ==========================================
MAX_TASKS = 24
MAX_MODES = 6
MAX_TRANSITIONS = 36

ALLOWED_ACTIONS = {'N': 'release', 'O': 'suspend', 'C': 'update', 'U': 'continue'}
ALLOWED_GUARDS = {
    'N': ['true', 'offsetlr', 'offsetmcr'],
    'O': ['true', 'offsetlr', 'offsetmcr', 'backlog_zero'],
    'C': ['true', 'offsetlr', 'offsetmcr', 'backlog_zero'],
    'U': ['true']
}

# ==========================================
# FUNCIONES AUXILIARES DE VALIDACIÓN
# ==========================================

def check_consecutive_ids(items, entity_name):
    """Valida que los IDs empiecen en 0 y sean consecutivos (0, 1, 2...)."""
    ids = sorted([item.get("id") for item in items])
    expected_ids = list(range(len(items)))
    if ids != expected_ids:
        return f"{entity_name} IDs must be consecutive integers starting from 0. Expected {expected_ids}, got {ids}."
    return None

def check_agt_and_guards(task, trans_id):
    """Valida la matriz AGT y los valores matemáticos de las guardas."""
    errors = []
    task_id = task.get("id")
    task_type = task.get("type")
    
    primitives = task.get("primitives", {})
    action = primitives.get("action")
    guard = primitives.get("guard")
    guard_value = primitives.get("guard_value")

    # 1. Validar Tipo de Tarea
    if task_type not in ALLOWED_ACTIONS:
        errors.append(f"[Trans {trans_id} | Task {task_id}] Invalid type '{task_type}'.")
        return errors

    # 2. Compatibilidad AGT
    exp_action = ALLOWED_ACTIONS[task_type]
    if action != exp_action:
        errors.append(f"[Trans {trans_id} | Task {task_id}] Action '{action}' invalid for type '{task_type}'. Expected '{exp_action}'.")
    
    if guard not in ALLOWED_GUARDS[task_type]:
        errors.append(f"[Trans {trans_id} | Task {task_id}] Guard '{guard}' invalid for type '{task_type}'.")

    # 3. Valor de la Guarda (Offset >= 0, Resto == -1)
    if guard in ["offsetmcr", "offsetlr"]:
        if type(guard_value) is not int or guard_value < 0:
            errors.append(f"[Trans {trans_id} | Task {task_id}] Guard '{guard}' must be >= 0 ms (Got {guard_value}).")
    else:
        if guard_value != -1:
            errors.append(f"[Trans {trans_id} | Task {task_id}] Guard '{guard}' must have value -1 (Got {guard_value}).")

    return errors

def validate_periods(model_data):
    """
    Verifica que ninguna tarea tenga un periodo menor o igual a cero.
    """
    is_valid = True
    
    # Recorremos todos los modos definidos en el JSON
    for mode in model_data.get("modes", []):
        mode_name = mode.get("name", f"Desconocido (ID {mode.get('id')})")
        
        # Recorremos las tareas activas dentro de cada modo
        for task in mode.get("active_tasks", []):
            task_id = task.get("id")
            parameters = task.get("parameters", {})
            
            # Si la tarea tiene el parámetro 'period', lo evaluamos
            if "period" in parameters:
                period_value = parameters["period"]
                
                # Comprobante estructural: No puede ser <= 0
                if period_value < 10:
                    print("[FATAL ERROR] Model validation failed:")
                    print(f"  ❌ Invalid period in Mode '{mode_name}' (Mode ID: {mode.get('id')}):")
                    print(f"     Task ID {task_id} has a period of {period_value}. Period must be >= 10.")
                    is_valid = False

    return is_valid

# ==========================================
# VALIDADOR PRINCIPAL
# ==========================================

def validate_model(data):
    errors = []
    
    tasks = data.get("tasks", [])
    modes = data.get("modes", [])
    transitions = data.get("transitions", [])

    # ---------------------------------------------------------
    # 1. Límites del Sistema y Nombres
    # ---------------------------------------------------------
    if len(tasks) > MAX_TASKS: errors.append(f"Exceeded MAX_TASKS limit ({len(tasks)} > {MAX_TASKS})")
    if len(modes) > MAX_MODES: errors.append(f"Exceeded MAX_MODES limit ({len(modes)} > {MAX_MODES})")
    if len(transitions) > MAX_TRANSITIONS: errors.append(f"Exceeded MAX_TRANS limit ({len(transitions)} > {MAX_TRANSITIONS})")

    err = check_consecutive_ids(tasks, "Task")
    if err: errors.append(err)
    
    err = check_consecutive_ids(modes, "Mode")
    if err: errors.append(err)

    # Validar espacios en nombres
    for t in tasks:
        if " " in t.get("name", ""): errors.append(f"Task {t['id']} name cannot contain spaces.")
    for m in modes:
        if " " in m.get("name", ""): errors.append(f"Mode {m['id']} name cannot contain spaces.")

    # ---------------------------------------------------------
    # 2. Modos y Parámetros
    # ---------------------------------------------------------
    mode_dict = {} # Estructura interna rápida: mode_id -> {task_id -> params}
    for m in modes:
        tasks_in_mode = {}
        for t in m.get("active_tasks", []):
            tid = t["id"]
            period = t["parameters"].get("period", 0)
            priority = t["parameters"].get("priority", 0)
            
            if type(period) is not int or period <= 0:
                errors.append(f"[Mode {m['id']} | Task {tid}] Period must be a positive integer > 0.")
            if type(priority) is not int or priority < 0:
                errors.append(f"[Mode {m['id']} | Task {tid}] Priority must be an integer >= 0.")
                
            tasks_in_mode[tid] = t["parameters"]
        mode_dict[m["id"]] = tasks_in_mode

    # ---------------------------------------------------------
    # 3. Transiciones y Coherencia Lógica
    # ---------------------------------------------------------
    source_modes_used = set()
    
    for tr in transitions:
        trans_id = tr.get("trans_id")
        src = tr.get("source_mode")
        dst = tr.get("dest_mode")
        
        # Transiciones inválidas (modos fantasma)
        if src not in mode_dict:
            errors.append(f"Transition {trans_id} specifies non-existent source_mode {src}.")
            continue
        if dst not in mode_dict:
            errors.append(f"Transition {trans_id} specifies non-existent dest_mode {dst}.")
            continue
            
        source_modes_used.add(src)
        
        src_tasks = set(mode_dict[src].keys())
        dst_tasks = set(mode_dict[dst].keys())
        
        # El taskset debe ser la unión exacta sin repeticiones
        expected_taskset = src_tasks.union(dst_tasks)
        actual_taskset = set(t["id"] for t in tr.get("taskset", []))
        
        if expected_taskset != actual_taskset:
            errors.append(f"Transition {trans_id} Taskset Mismatch! Expected tasks {expected_taskset}, but got {actual_taskset}.")

        # Validar Tipos de Tarea (Inferencia vs Declaración) y AGT
        for t in tr.get("taskset", []):
            tid = t["id"]
            decl_type = t.get("type")
            
            # Inferencia lógica
            if tid in dst_tasks and tid not in src_tasks:
                inferred_type = 'N'
            elif tid in src_tasks and tid not in dst_tasks:
                inferred_type = 'O'
            elif tid in src_tasks and tid in dst_tasks:
                # Comprobar si cambian los parámetros
                if mode_dict[src][tid] != mode_dict[dst][tid]:
                    inferred_type = 'C'
                else:
                    inferred_type = 'U'
            
            if decl_type != inferred_type:
                errors.append(f"[Trans {trans_id} | Task {tid}] Type Mismatch: User declared '{decl_type}', but logic dictates it must be '{inferred_type}'.")
                
            # Llamada al AGT
            errors.extend(check_agt_and_guards(t, trans_id))

    # ---------------------------------------------------------
    # 4. Modos Muertos (Dead-ends)
    # ---------------------------------------------------------
    for m_id in mode_dict.keys():
        if m_id not in source_modes_used:
            errors.append(f"Dead-end detected: Mode {m_id} has no outgoing transitions.")

    # ---------------------------------------------------------
    # Resultados
    # ---------------------------------------------------------
    if not errors:
        print("[INFO] Model validation successful. System is structurally and logically intact.")
    else:
        print("\n[FATAL ERROR] Model validation failed:")
        for e in errors:
            print(f"  ❌ {e}")
        print("\n[INFO] Aborting generation.")
        sys.exit(1)

# open the JSON file and extract its contents to 'data', then call the validation function
with open("model.json") as spec:
    data = json.load(spec)
    validate_model(data)
    validate_periods(data)


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
    generate()

