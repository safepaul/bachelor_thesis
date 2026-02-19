#include "tasks.h"
#include "mcmanager.h"
#include "gen_data.h" // remove this afterwards
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include <stdint.h>
#include <stdio.h>


/*
 *  
 *
*/
void taskZero_utask(void *pvParameters){

    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1) {
        
        mcm_wait_for_activation(task_id);

        printf("[T0 - %lu] Hello! Task ZERO speaking\n", xTaskGetTickCount());

    }

}



/*
 *  
 *
*/
void taskOne_utask(void *pvParameters){

    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1) {

        mcm_wait_for_activation(task_id);
        
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
