#include "tasks.h"
#include "freertos/idf_additions.h"
#include <stdio.h>


/*
 *  
 *
*/
void taskZero_utask(void *pvParameters){

    while (1) {
        
        puts("[T2] Hello! Task Zero speaking");
        vTaskDelay(pdMS_TO_TICKS(1000));

    }

}



/*
 *  
 *
*/
void taskOne_utask(void *pvParameters){

    while (1) {
        
        puts("[T5] Hello! Task One speaking");
        vTaskDelay(pdMS_TO_TICKS(2500));

    }

}
