#include "tasks.h"
#include "freertos/idf_additions.h"
#include <stdio.h>


/*
 *  
 *
*/
void taskTwo_utask(void *pvParameters){

    while (1) {
        
        puts("[T2] Hello! Task two speaking");
        vTaskDelay(pdMS_TO_TICKS(1000));

    }

}



/*
 *  
 *
*/
void taskFive_utask(void *pvParameters){

    while (1) {
        
        puts("[T5] Hello! Task five speaking");
        vTaskDelay(pdMS_TO_TICKS(2500));

    }

}
