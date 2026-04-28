#!/usr/bin/env python3
import json
import random
import sys

# --- FUNCIONES DE VALIDACIÓN DE ENTRADA ---

def get_valid_int(prompt, min_val, max_val):
    """Pide un entero hasta que el usuario introduzca un valor válido en el rango."""
    while True:
        try:
            value = int(input(prompt))
            if min_val <= value <= max_val:
                return value
            else:
                print(f"  ❌ Error: El valor debe estar entre {min_val} y {max_val}.")
        except ValueError:
            print("  ❌ Error: Por favor, introduce un número entero válido.")

def get_valid_choice(prompt, valid_choices):
    """Pide una cadena hasta que el usuario introduzca una opción válida."""
    while True:
        choice = input(prompt).strip().lower()
        if choice in valid_choices:
            return choice
        else:
            print(f"  ❌ Error: Opción no válida. Opciones permitidas: {', '.join(valid_choices)}.")

# --- LÓGICA DE GENERACIÓN ---

def generate_scaffold():
    print("\n--- Generador de Scaffold (Plantilla Vacía) ---")
    
    # Validamos que no excedan los límites estáticos del sistema
    num_tasks = get_valid_int("¿Cuántas tareas tendrá el sistema? (1-24): ", 1, 24)
    num_modes = get_valid_int("¿Cuántos modos tendrá el sistema? (1-6): ", 1, 6)

    # 1. Generar Tareas
    tasks = [{"id": i, "name": f"Task_{i}"} for i in range(num_tasks)]

    # 2. Generar Modos
    modes = []
    for i in range(num_modes):
        modes.append({
            "id": i,
            "name": f"MODE_{i}",
            "active_tasks": [] # Array vacío para que el usuario lo rellene a mano
        })

    # 3. Generar Transiciones (Topología de Anillo Básica)
    transitions = []
    for i in range(num_modes):
        dest_mode = (i + 1) % num_modes  # Conecta el último con el primero
        transitions.append({
            "trans_id": i,
            "source_mode": i,
            "dest_mode": dest_mode,
            "taskset": [] # Array vacío para que el usuario lo rellene a mano
        })

    # 4. Ensamblar el JSON
    model = {
        "tasks": tasks,
        "modes": modes,
        "transitions": transitions
    }

    # 5. Exportar a archivo
    filename = "model_scaffold.json"
    try:
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(model, f, indent=2, ensure_ascii=False)
        
        print(f"\n[ÉXITO] Esqueleto generado correctamente en '{filename}'.")
        print("-> Abre el archivo con tu editor y rellena los arrays vacíos de 'active_tasks' y 'taskset'.")
    except IOError as e:
        print(f"\n[ERROR] No se pudo guardar el archivo: {e}")

def generate_random():
    print("\n--- Generador Aleatorio (Stress Test) ---")
    
    # Validamos entradas estrictamente
    num_tasks = get_valid_int("¿Cuántas tareas máximas tendrá el sistema? (1-24): ", 1, 24)
    num_modes = get_valid_int("¿Cuántos modos tendrá el sistema? (1-6): ", 1, 6)
    full_mesh_input = get_valid_choice("¿Generar Grafo Completo (todos conectados con todos)? (s/n): ", ['s', 'n'])
    full_mesh = full_mesh_input == 's'

    # 1. Generar Tareas y sus Parámetros Base
    tasks = []
    base_params = {}
    for i in range(num_tasks):
        tasks.append({"id": i, "name": f"Task_{i}"})
        base_params[i] = {
            "period": random.randint(10, 1000),
            "priority": random.randint(1, 10)
        }

    # 2. Generar Modos y asignar tareas
    modes = []
    mode_task_map = {} 

    for i in range(num_modes):
        active_tasks = []
        tasks_in_this_mode = {}
        
        for t in range(num_tasks):
            if random.random() < 0.60:
                if random.random() < 0.50:
                    params = base_params[t].copy()
                else:
                    params = {
                        "period": random.randint(10, 1000),
                        "priority": random.randint(1, 10)
                    }
                
                active_tasks.append({"id": t, "parameters": params})
                tasks_in_this_mode[t] = params
                
        if not active_tasks:
            params = base_params[0].copy()
            active_tasks.append({"id": 0, "parameters": params})
            tasks_in_this_mode[0] = params

        mode_task_map[i] = tasks_in_this_mode
        modes.append({
            "id": i,
            "name": f"MODE_{i}",
            "active_tasks": active_tasks
        })

    # 3. Generar Transiciones
    transitions = []
    trans_id = 0

    for src in range(num_modes):
        for dest in range(num_modes):
            if src == dest:
                continue 
            
            if not full_mesh and dest != (src + 1) % num_modes:
                continue

            taskset = []
            src_tasks = mode_task_map[src]
            dest_tasks = mode_task_map[dest]
            
            union_task_ids = set(src_tasks.keys()).union(set(dest_tasks.keys()))

            for t_id in union_task_ids:
                in_src = t_id in src_tasks
                in_dest = t_id in dest_tasks
                
                if in_src and not in_dest:
                    guard = random.choice(["true", "backlog_zero"])
                    taskset.append({
                        "id": t_id, "type": "O",
                        "primitives": {"action": "suspend", "guard": guard, "guard_value": -1}
                    })
                
                elif not in_src and in_dest:
                    guard = random.choice(["true", "offsetmcr", "offsetlr"])
                    g_val = -1 if guard == "true" else random.randint(50, 500)
                    taskset.append({
                        "id": t_id, "type": "N",
                        "primitives": {"action": "release", "guard": guard, "guard_value": g_val}
                    })
                
                elif in_src and in_dest:
                    p_src = src_tasks[t_id]
                    p_dest = dest_tasks[t_id]
                    
                    if p_src == p_dest:
                        taskset.append({
                            "id": t_id, "type": "U",
                            "primitives": {"action": "continue", "guard": "true", "guard_value": -1}
                        })
                    else:
                        taskset.append({
                            "id": t_id, "type": "C",
                            "primitives": {"action": "update", "guard": "true", "guard_value": -1}
                        })

            taskset = sorted(taskset, key=lambda x: x["id"])

            transitions.append({
                "trans_id": trans_id,
                "source_mode": src,
                "dest_mode": dest,
                "taskset": taskset
            })
            trans_id += 1

    # 4. Ensamblar JSON
    model = {
        "transitions": transitions,
        "tasks": tasks,
        "modes": modes
    }

    # 5. Guardar Archivo
    filename = f"model_stress_{num_tasks}T_{num_modes}M.json"
    try:
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(model, f, indent=2, ensure_ascii=False)
        print(f"\n[ÉXITO] Generado '{filename}' con {len(transitions)} transiciones.")
    except IOError as e:
        print(f"\n[ERROR] No se pudo guardar el archivo: {e}")

def main():
    while True:
        print("\n==================================================")
        print("  MC Manager - Herramienta de Modelado JSON")
        print("==================================================")
        print("1) Generar Scaffold (Plantilla vacía para rellenar a mano)")
        print("2) Generar Modelo Aleatorio (Para Stress Testing)")
        print("3) Salir")
        
        opcion = get_valid_choice("\nElige una opción (1, 2 o 3): ", ['1', '2', '3'])
        
        if opcion == '1':
            generate_scaffold()
        elif opcion == '2':
            generate_random()
        elif opcion == '3':
            print("\nSaliendo... ¡Hasta luego!")
            sys.exit(0)

if __name__ == "__main__":
    main()
