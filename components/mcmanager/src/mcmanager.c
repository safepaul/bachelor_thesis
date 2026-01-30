#include "mcmanager.h"
#include "esp_rom_sys.h"
#include "freertos/projdefs.h"
#include "gen_data.h"

#include "freertos/idf_additions.h"
#include "portmacro.h"
#include "stdlib.h"
#include "esp_log.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>



// Component tag for ESP-IDF logging
static const char TAG[] = "MCMANAGER";

// Variable that keeps track of the current active mode
static uint8_t current_mode;

// Variable that records the mcr_instant of the most recent mode change request. Useful for software timers for the offsetmcr guard.
static TickType_t mcr_instant = 0;






/*
 *
 *
*/
void initial_setup(){
    // XXX: I have to start the timers here (and suspend the tasks that are not active in the initial mode?)
    // For the timers, as this function is intended to prepare the systemfor the mode MODE_INIT, set all task parameters before startup so, before starting the timers (which I think is the last thing to do, as that starts the tasks and the actual system), the periods of the timers (of the tasks) have to be set up as I have decided that the timers will start with period 0 during creation.
    // I have to turn off the timers when I change the mode
    // - With xTimerChangePeriod (+ xTimerReset)
    // - I think with xTimerStop, the timer stops mid-counting so with xTimerReset I can reset it back to 0
    // - must think of a way to recognize pending tasks (if they are even necessary to recognize)

    current_mode = MODE_INIT;


    // stop scheduler to avoid some tasks starting before others during processing
    vTaskSuspendAll();

    // set the period and priority of the timers (task periods) and tasks
    for (int i = 0; i < modes[MODE_INIT].n_tasks; i++) {

        mode_task_t init_task_i = modes[MODE_INIT].tasks[i];
        
        xTimerChangePeriod( timer_handles[  init_task_i.id],  init_task_i.parameters.period,   0);
        vTaskPrioritySet(task_handles[  init_task_i.id],   init_task_i.parameters.priority  );
        esp_rom_printf("starting task %d timer\n", i);
        xTimerStart(timer_handles[ init_task_i.id ], 0);

    }

    xTaskResumeAll();


}



/*
 *
 * 
*/
void mc_request(uint8_t target_mode){

    // calculate the mcr instant just after receiving the mc request
    mcr_instant = xTaskGetTickCount();

    // stop the scheduler so that the states of the tasks don't change while the program processes the transition of the tasks that come before (or even the state snapshot)
    // this must be the first call to avoid a higher priority task taking over the cpu (not too sure about this, mcr_instant gets delayed...)
    vTaskSuspendAll();


    // fetch the transition that corresponds to the 'source -> dest' mode pair
    uint8_t current_trans_id = mode_transitions[current_mode][target_mode];

    //// does the transition exist?
    if (current_trans_id == NO_TRANSITION)
        abort();

    const transition_t *current_trans = &transitions[ current_trans_id ];


    //perform the task changes
    for (int i = 0; i < current_trans->taskset_size; i++) {
        apply_primitive(&current_trans->taskset[i]);
    }

    
    // after all tasks have been handled correctly, change the current mode and resume the scheduler
    current_mode = target_mode;

    xTaskResumeAll();

    ESP_LOGI(TAG, "Mode change successfully performed!");

}



/*
 *
 *
*/
void apply_primitive(const trans_task_t *task){


    // NOTE: I removed the distinction between jobs executing or not executing because as I also removed the distinction between pending and other job states, just by knowing the type of task we know they can only be in one state:
    //  - if the task is of type NEW: so it can only be in a non-executing state (suspended)
    //  - if the task is of type UNCHANGED, CHANGED, or OLD: the task HAS TO BE already in an executing state (ready, blocked, running).
    // The program loses granularity but for simplicity I'll leave it like this, adding features will be fine.
    // The program itself doesnt care about the type of the task, it just cares about performing the changes. The user that generates the YAML must take care of that. If a task is of type OLD, it should follow its corresponding "ACTION_ABORT" action or similar
    // although this hurts program readability. May complicate things if I have to assert in the generator or check for errors. I'll add that later if necessary.


    TaskHandle_t handle = task_handles[task->task_id];
    uint8_t action = task->primitives.action;
    uint8_t guard = task->primitives.guard;
    int16_t guard_value = task->primitives.guard_value;

    esp_rom_printf("task id: %d\nhandle: %p\naction: %d\nguard: %d\nguard_value: %d\n", task->task_id, handle, action, guard, guard_value);

    switch (guard) {

        case GUARD_TRUE:
        case GUARD_NONE:

            perform_action(handle, action);

            break;

        case GUARD_OFFSETMCR:
            // the time the action should be performed at -> timestamp after scheduler started
            TickType_t release_time = (mcr_instant + pdMS_TO_TICKS(guard_value));
            esp_rom_printf("mcr_instant: %d\nrelease_time: %d\n", mcr_instant, release_time);

            // guard_value  >= 1  in milliseconds.
            // calculating mcr_instant + offset AND substracting the time it passed between the mcr and the time this gets processed, for a more precise time
            TickType_t start_time = release_time - xTaskGetTickCount();
            esp_rom_printf("start_time: %d\n", start_time);

            // if the offset has elapsed, perform action immediately. 
            // NOTE: better polished system would probably start the timer right after the mcr_insant, somehow, to avoid the timer elapsing
            if (start_time <= 0) {
                perform_action(handle, action);
                break;
            }

            // creating the timer
            TimerHandle_t timer = xTimerCreate("offsetmcr_timer",
                        start_time,
                        pdFALSE,
                        (void *) task,
                        callback_offsetmcr_timer);

            esp_rom_printf("timer returned : %p\n", timer);


            // NOTE: timers are created using heap memory, so the creation may fail
            if (timer == NULL)
                abort();




            // maximum waiting time for the timer queue
            TickType_t remaining_time = release_time - xTaskGetTickCount();
            esp_rom_printf("remaining_time: %d\n", remaining_time);

            // NOTE: The timer countdown starts immediately, but the scheduler is stopped and there may be some tasks left to process. If the timer ticks while processing, the time of release won't be that precise, although processing time may be a bit negligible?
            //
            // if the offset has elapsed, perform action immediately. 
            if (remaining_time <= 0) {
                perform_action(handle, action);
                break;
            } else {
                // second parameter of xTimerStart is maximum blocking time for the timer if timer command queue is full. Waiting only for the time remaining
                // As I understand, this function is blocking, so may not be good for rtos time criticality
                // if timer waits for too long, perform the action.
                
                if (xTimerStart(timer, remaining_time) == pdFAIL){
                    perform_action(handle, action);
                    break;
                }

            }

            break;

        // olvidarse de constante, preguntar long cola < 0?
        case GUARD_BACKLOG:
            // XXX: idea:
            // he visto que con notificaciones, el codigo de la tarea se queda quieto hasta que lo notifiquen (por lo que se podrian iniciar sin tener que suspenderlas después instantaneo).
            // Se pueden acumular las notificaciones por lo que funciona como contador de backlog y esas tareas estarian en pending. Las notificaciones se envian con software timers cuando hago release?
            // Como detecto que ciertas tasks están en pending? hace falta siquiera para el backlog saber que estan en pending?
            // LEEME: hacerme preguntas -> seguir el rollo del video de ICS para psicologia. Aplicar ics bien.
            // queue size -> max_task_notifications o parecido
            // como hay varias operaciones posibles para el backlog, hacer defines con cada operación, pero esto crea el problema de que tendria que crear un case: en el switch para cada uno, a no ser que consiga diferenciarlos de alguna manera o encuente otro modo de hacerlo

            


            

            break;

        default:
            abort();
            break;

    }


}



/*
 * 
 *
*/
void perform_action(uint8_t task_id, uint8_t action, uint8_t mode_id){
    TaskHandle_t task_handle = task_handles[task_id];
    TimerHandle_t timer_handle = timer_handles[task_id];

    // Se deja que la tarea termine siempre

    // TODO: Check if parameter changes are needed and perform that too. Maybe adding a task type field avoids having to check if the task is C-U-N-O and makes it easier to perform the actions
    switch (action) {
        case ACTION_NONE:
            // do nothing?
            break;

        case ACTION_CONTINUE:
            // do nothing?
            break;

        case ACTION_SUSPEND:
            // stop and reset the timer (how to set back the code to the first line? reset task somehow?)
            xTimerStop(timer_handle, 0);
            xTimerReset(timer_handle, 0);
            vTaskSuspend(task_handle);
            break;

        case ACTION_UPDATE:
            // just change the parameters.
            // priority
            vTaskPrioritySet(task_handle, modes[mode_id].tasks[task_id].parameters.priority);
            // period
            xTimerChangePeriod(timer_handle, modes[mode_id].tasks[task_id].parameters.period, 0);

            break;

        case ACTION_RELEASE:
            // first change parameters
            // priority
            vTaskPrioritySet(task_handle, modes[mode_id].tasks[task_id].parameters.priority);
            // period
            xTimerChangePeriod(timer_handle, modes[mode_id].tasks[task_id].parameters.period, 0);



            vTaskResume(task_handle);
            // reset timer just in case
            xTimerReset(timer_handle, 0);
            xTimerStart(timer_handle, 0);
            break;

        default:
            abort();
            break;

    }

}



/*
 * Callbacks used for the software timers necessary for the offsetmcr guard.
 * They expect a task pointer to be passed as an argument
 *
 * NOTE: I believe I am passing the task pointer as an argument that should be a timer identifier. I don't know the consequences to that
*/
void callback_offsetmcr_timer( TimerHandle_t xTimer ){

    esp_rom_printf("Offsetmcr timer expired. Executing...\n");

    const trans_task_t *task = (const trans_task_t *) pvTimerGetTimerID(xTimer);

    perform_action(task_handles[task->task_id], task->primitives.action);

}


/*
 *
 *
*/
void task_timer_callback( TimerHandle_t xTimer ){


    uint8_t task_id = (uint8_t)(uintptr_t)pvTimerGetTimerID(xTimer);

    esp_rom_printf("notifying task %d from task timer callback!\n", task_id);

    xTaskNotifyGive( task_handles[ task_id ] );

}
