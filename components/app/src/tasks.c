#include "tasks.h"
#include "freertos/idf_additions.h"
#include <stdio.h>


/*
 *  
 *
*/
void taskZero_utask(void *pvParameters){

    while (1) {
        
        printf("[T0 - %lu] Hello! Task ZERO speaking\n", xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(1000));

    }

}



/*
 *  
 *
*/
void taskOne_utask(void *pvParameters){

    while (1) {
        
        printf("[T1 - %lu] Hello! Task ONE speaking\n", xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(2500));

    }

}
