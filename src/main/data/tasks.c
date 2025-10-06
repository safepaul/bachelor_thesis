#include <stdio.h>
#include <time.h>
#include "freertos/idf_additions.h"

/*
 * Task function that prints something once every 'sleep' seconds.
 *
 * Notes:
 *  - No error checking
 *
 * */
void taskPrintTime_u(void *tfParams){
    int *sleep = (int *)tfParams;
    while (1) {
        printf("\n Hello, I am printing: HELLO WORLD!\n");
        vTaskDelay( *sleep / portTICK_PERIOD_MS);
    }
}
