#include <limits.h>
#include <stdio.h>
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "portmacro.h"

#include "generated.h"



const TaskFunction_t g_task_funcs[TASK_COUNT] = { 

    [TASK_PRINTSTRING]  = tf_printString,
    [TASK_PRINTCOUNTER] = tf_printCounter,
    [TASK_PRINTZEROES] = tf_printZeroes,
    [TASK_PRINTONES] = tf_printOnes,

};

const task_map_t g_task_map[TASK_COUNT] = {

    { .name = "Print String", .id = TASK_PRINTSTRING },
    { .name = "Print Counter", .id = TASK_PRINTCOUNTER },

};

// TEMP
int arg1 = 1000;
int arg2 = 5;

const task_info_t g_task_table[TASK_COUNT][MODE_COUNT] = {

    [TASK_PRINTSTRING] = 
        { //changed

            [MODE_INIT] = {  .args_p = &arg1 },
            [MODE_NORMAL] = {  .args_p = NULL } 

        },

    [TASK_PRINTCOUNTER] = 
        { //unchanged

            [MODE_INIT] = {  .args_p = &arg2 },
            [MODE_NORMAL] = {  .args_p = NULL }

        },

    [TASK_PRINTZEROES] = 
        { // old

            [MODE_INIT] = {  .args_p = &arg2 },
            [MODE_NORMAL] = {  .args_p = NULL }

        },

    [TASK_PRINTONES] = 
        { // new

            [MODE_INIT] = {  .args_p = &arg2 },
            [MODE_NORMAL] = {  .args_p = NULL }

        },

};


const transition_t g_transition_table[MODE_COUNT][MODE_COUNT][TASK_COUNT] = 
{
    
    [MODE_INIT] = 
        {
            [MODE_INIT] = { 0 },
            [MODE_NORMAL] =
                {
                    [TASK_PRINTSTRING]      = { .type = TYPE_CHANGED, .action = ACTION_UPDATE },
                    [TASK_PRINTCOUNTER]     = { .type = TYPE_UNCHANGED, .action = ACTION_CONTINUE },
                    [TASK_PRINTZEROES]      = { .type = TYPE_OLD, .action = ACTION_ABORT },
                    [TASK_PRINTONES]     = { .type = TYPE_NEW, .action = ACTION_RELEASE }
                }
        },
    [MODE_NORMAL] = 
        {
            [MODE_INIT] = 
                {
                    [TASK_PRINTSTRING]      = { .type = TYPE_CHANGED, .action = ACTION_UPDATE },
                    [TASK_PRINTCOUNTER]     = { .type = TYPE_UNCHANGED, .action = ACTION_CONTINUE },
                    [TASK_PRINTZEROES]      = { .type = TYPE_NEW, .action = ACTION_RELEASE },
                    [TASK_PRINTONES]     = { .type = TYPE_OLD, .action = ACTION_ABORT }
                },
            [MODE_NORMAL] = { 0 }
        }

};































/*
 * Task function that prints a string once every 'sleep' seconds.
 *
 * */
void tf_printString(void *tfParams){

    unsigned int *sleep = (unsigned int *)tfParams;

    while (1) {

        printf("\n Hello, I am printing: HELLO WORLD!\n");
        vTaskDelay( *sleep / portTICK_PERIOD_MS);

    }

}

/*
 * Task function that prints a counter's value once every second.
 * The counter increments its value by 'inc' every iteration.
 *
 * */
void tf_printCounter(void *tfParams){
    
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
