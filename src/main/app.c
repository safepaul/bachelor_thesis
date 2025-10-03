#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#include "unistd.h"


/* -------------------------------------------------------------------
 * Task function that prints the time once every 'sleep' seconds.
 * ------------------------------------------------------------------- */
void taskPrintTime(void *tfParams){
    int *sleep = (int *)tfParams;
    while (1) {
        printf("\n Current time: -- %lld --\n", time(NULL));        // (?) Not portable 
        vTaskDelay( *sleep / portTICK_PERIOD_MS);                    // No error checking
    }
}

void app_main(void){
    // this will probably be handled by the MC initialized and MC manager in the future
    int *period = malloc(sizeof(int));
    *period = 1000;
    xTaskCreatePinnedToCore(taskPrintTime,
                "Print Time",
                2048,
                period,
                1,
                NULL,
                0);

    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        *period = 100;
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        *period = 1000;
    }

}
