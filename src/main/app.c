#include <stddef.h>
#include "headers/mcinit.h"



void app_main(void){
    mcinit();

    // TODO: this will be handled by the mcmanager with the data structures
    // while (1) {
    //     vTaskDelay(2000 / portTICK_PERIOD_MS);
    //     *period = 100;
    //     vTaskDelay(2000 / portTICK_PERIOD_MS);
    //     *period = 1000;
    // }

}
