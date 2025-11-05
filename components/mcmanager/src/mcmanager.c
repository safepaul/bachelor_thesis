#include "mcmanager.h"
#include "freertos/idf_additions.h"
#include "generated.h"
#include "mcinit.h"
#include "stdlib.h"
#include "esp_log.h"


// Component tag for ESP-IDF logging
static const char* TAG = "MCMANAGER";

static mode_e current_mode = MODE_INIT;



// this may not be needed if just this file uses the current_mode variable
mode_e get_current_mode(){

    return current_mode;

}


const transition_t (*get_transition(mode_e old, mode_e new))[TASK_COUNT]{

    if ( old < 0 || old >= MODE_COUNT || new < 0 || new >= MODE_COUNT ) {
        
        ESP_LOGE(TAG, "Mode arguments in function 'get_transition()' are out of bounds");
        abort();

    }

    return &g_transition_table[old][new];

}


void change_mode(mode_e new){

    mode_e old_mode = get_current_mode();

    // Change the current mode to the new mode
    current_mode = new;

    const transition_t (*trans_info)[TASK_COUNT] = get_transition(old_mode, new);

    // perform actions depending on the type of task
    for (int i = 0; i < TASK_COUNT; i++) {

        // assumption: task with id 0 in the enum will be also 0 in the params table
        switch ( (*trans_info)[i].type ) {

            case TYPE_NEW:

                // RELEASE, which means activate the task. We assume it was started but suspended
                vTaskResume(g_task_handles[i]);

                break;

            case TYPE_OLD:

                // ABORT, which means suspend the task. We assume it was active before
                vTaskSuspend(g_task_handles[i]);

                break;

            case TYPE_CHANGED:

                // UPDATE, which means change the parameters while it's still running
                *((unsigned int*)g_task_table[i][MODE_INIT].args_p) = *((unsigned int*)g_task_table[i][new].args_p);

                break;

            case TYPE_UNCHANGED:

                // CONTINUE, which means do nothing

                break;

            default:

            
                ESP_LOGE(TAG, "Trying to perform actions on a task of unknown type: %d\n", (*trans_info)[i].type );
                abort();

                break;

        
        }

    }

}



