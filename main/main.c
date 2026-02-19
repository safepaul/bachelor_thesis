#include "freertos/idf_additions.h"
#include "mcmanager.h"
#include "gen_data.h"
#include <stdio.h>
#include <unistd.h>







void app_main(void){

    // tasks are created and wait at wfa()
    create_tasks();

    // timers are created but not started
    create_timers();

    // set mode to MODE_INIT (0). Starts the timers
    initial_setup();

    // after 6 seconds, mode change request MODE_INIT -> MODE_EMERGENCY (0 -> 1)
    sleep(6);
    printf("[DEBUG] executing mc request: source = %d,  dest = %d \n", MODE_INIT, MODE_EMERGENCY);

    mc_request(MODE_EMERGENCY);


    // after 6 seconds, mode change request MODE_NONE -> MODE_INIT (0 -> 1)
    // ACTION_UPDATE-GUARD_TRUE and ACTION_RELEASE-GUARD_OFFSETMCR are called. A Timer should be created and fired after some time, and the task released just afterwards, in a timely manner (exactly 3 seconds)
    sleep(6);
    printf("[DEBUG] executing mc request: source = %d,  dest = %d \n", MODE_EMERGENCY, MODE_INIT);
    mc_request(MODE_INIT);

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
