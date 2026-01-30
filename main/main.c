#include "freertos/idf_additions.h"
#include "mcmanager.h"
#include "gen_data.h"
#include <stdio.h>
#include <unistd.h>







void app_main(void){

    // tasks are created and suspended right afterwards
    create_tasks();

    // timers are created but not started
    create_timers();

    // set mode to MODE_INIT (1). Resumes tasks 0 and 1 (both active in mode 1)
    initial_setup();
    //
    //
    // // after 3 seconds, mode change request MODE_INIT -> MODE_NONE (1 -> 0)
    // sleep(3);
    // printf("[DEBUG] executing mc request to mode %d\n", MODE_NONE);
    // // suspends scheduler. Loops through trans_task of the taskset. calls apply_primitive(task), which checks their guard and performs the specific actions.
    // // only GUARD_NONE/TRUE and GUARD_OFFSETMCR exist for now. GUARD_NONE performs action directly. GUARD_OFFSETMCR may cause problems, but it's not called here.
    // // ACTION_UPDATE-GUARD_TRUE and ACTION_SUSPEND-GUARD_TRUE are called
    // // in the end, task 1 should be suspended and task 0 should be active. the current mode should be changed
    // mc_request(MODE_NONE);
    //
    //
    // // after 10 seconds, mode change request MODE_NONE -> MODE_INIT (0 -> 1)
    // // ACTION_UPDATE-GUARD_TRUE and ACTION_RELEASE-GUARD_OFFSETMCR are called. A Timer should be created and fired after some time, and the task released just afterwards, in a timely manner (exactly 3 seconds)
    // sleep(10);
    // printf("[DEBUG] executing mc request to mode %d\n", MODE_INIT);
    // mc_request(MODE_INIT);

}



// void app_main(void){
//
//
//     spawn_tasks_notif();
//
//     sleep(5);
//
//     xTaskNotifyGive(task_handles[0]);
//     xTaskNotifyGive(task_handles[0]);
//     xTaskNotifyGive(task_handles[0]);
//     xTaskNotifyGive(task_handles[0]);
//
// }
