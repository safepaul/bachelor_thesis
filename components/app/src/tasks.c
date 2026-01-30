#include "tasks.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include <stdio.h>


/*
 *  
 *
*/
void taskZero_utask(void *pvParameters){

    while (1) {
        
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        printf("[T0 - %lu] Hello! Task ZERO speaking\n", xTaskGetTickCount());

    }

}



/*
 *  
 *
*/
void taskOne_utask(void *pvParameters){

    while (1) {
        
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        printf("[T1 - %lu] Hello! Task ONE speaking\n", xTaskGetTickCount());

    }

}




/*
 *  
 *
*/
void taskTest_utask(void *pvParameters){

    while (1) {
        
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        printf("[TEST - %lu] Hello! THIS IS A TEST\n", xTaskGetTickCount());

    }

}
