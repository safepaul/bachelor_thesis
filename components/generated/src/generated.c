#include <limits.h>
#include <stdio.h>
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "portmacro.h"

#include "../../components/mcinit/include/mcinit.h"
#include "../include/generated.h"



const TaskFunction_t g_task_funcs[TASK_COUNT] = { 

    [TASK_PRINTSTRING]  = taskPrintString_u,
    [TASK_PRINTCOUNTER] = taskPrintCounter_u

};

const task_map_t g_task_map[TASK_COUNT] = {

    { .name = "Print String", .id = TASK_PRINTSTRING },
    { .name = "Print Counter", .id = TASK_PRINTCOUNTER },

};

// TEMP
int arg1 = 1000;
int arg2 = 5;

const task_info_t g_task_table[TASK_COUNT][MODE_COUNT] = {

    // TASK_PRINTSTRING
    {
        [MODE_INIT] = { .handle = NULL, .priority = 1, .args = &arg1 },
        [MODE_NORMAL] = { .handle = NULL, .priority = 1, .args = NULL } },

    // TASK_PRINTCOUNTER
    {
        [MODE_INIT] = { .handle = NULL, .priority = 1, .args = &arg2 },
        [MODE_NORMAL] = { .handle = NULL, .priority = 1, .args = NULL }
    }

};



































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
