#include <limits.h>
#include <stdio.h>
#include <time.h>
#include "freertos/idf_additions.h"

/*
 * Task function that prints a string once every 'sleep' seconds.
 * */
void taskPrintString_u(void *tfParams){
    unsigned int *sleep = (unsigned int *)tfParams;
    while (1) {
        printf("\n Hello, I am printing: HELLO WORLD!\n");
        vTaskDelay( *sleep / portTICK_PERIOD_MS);
    }
}


/*
 * Task function that prints a counter's value once every second.
 * The counter increments its value by 'inc' every iteration.
 * */
void taskPrintCounter_u(void *tfParams){
    int *inc = (int *)tfParams;
    int counter = 0;
    while (1) {
        if (counter > INT_MAX || counter < INT_MIN) {
            puts("The counter exceeded the integer size limit, resetting back to 0");
            counter = 0;
        }

        printf("Value of the counter: %d\n", counter);
        counter += *inc;
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
