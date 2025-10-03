#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"


/* -------------------------------------------------------------------
 * Task function that prints the time once every 'sleep' seconds.
 * ------------------------------------------------------------------- */
void taskPrintTime(void *tfParams){
    // int sleep = *( (int *) tfParams );                              // No error checking
    int sleep = ( (int)(uintptr_t) tfParams );                       // No error checking
    while (1) {
        printf("\n Current time: -- %lld --\n", time(NULL));        // (?) Not portable 
        vTaskDelay( sleep / portTICK_PERIOD_MS);                    // No error checking
    }
}

void app_main(void){
    long period = 1000;
    xTaskCreate(taskPrintTime,
                "Print Time",
                2048,
                (void *)period,
                1,
                NULL);
}
