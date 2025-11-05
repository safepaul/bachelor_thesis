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
    { .name = "Print Zeroes", .id = TASK_PRINTZEROES },
    { .name = "Print Ones", .id = TASK_PRINTONES },

};

// TEMP
int arg1 = 1000;
int arg2 = 5;
int arg3 = 2500;

const task_info_t g_task_table[TASK_COUNT][MODE_COUNT] = {

    [TASK_PRINTSTRING] = 
        {

            [MODE_INIT] = {  .args_p = &arg1, .active = 1 },
            [MODE_NORMAL] = {  .args_p = &arg3, .active = 1 } 

        },

    [TASK_PRINTCOUNTER] = 
        {

            [MODE_INIT] = {  .args_p = &arg2, .active = 1 },
            [MODE_NORMAL] = {  .args_p = &arg2, .active = 1 }

        },

    [TASK_PRINTZEROES] = 
        {

            [MODE_INIT] = {  .args_p = NULL, .active = 1 },
            [MODE_NORMAL] = {  .args_p = NULL, .active = 0 }

        },

    [TASK_PRINTONES] = 
        {

            [MODE_INIT] = {  .args_p = NULL, .active = 0 },
            [MODE_NORMAL] = {  .args_p = NULL, .active = 1 }

        },

};


const transition_t g_transition_table[MODE_COUNT][MODE_COUNT][TASK_COUNT] = {
    
    [MODE_INIT] = 
        {
            [MODE_INIT] = { },
            [MODE_NORMAL] =
                {
                    [TASK_PRINTSTRING]      = { .type = TYPE_CHANGED, .action = ACTION_UPDATE },
                    [TASK_PRINTCOUNTER]     = { .type = TYPE_UNCHANGED, .action = ACTION_CONTINUE },
                    [TASK_PRINTZEROES]      = { .type = TYPE_OLD, .action = ACTION_ABORT },
                    [TASK_PRINTONES]     = { .type = TYPE_NEW, .action = ACTION_RELEASE }
                },
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
            [MODE_NORMAL] = { }
        }

};































/*
 * Task function that prints a string once every 'sleep' seconds.
 *
 * */
void tf_printString(void *tfParams){

    unsigned int *sleep = (unsigned int *)tfParams;

    while (1) {

        printf("\n Hello, I am printing: HELLO WORLD! every %ud ticks\n", *sleep);
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

/*
 * Task function that prints a row of zeroes every five seconds (0's)
 *
 * */
void tf_printZeroes(void *tfParams){

    while (1) {
        
        puts("0000000000000000000000000000000000000000000000000000");
        vTaskDelay(5000 / portTICK_PERIOD_MS);

    }

}

/*
 * Task function that prints a row of zeroes every five seconds (0's)
 *
 * */
void tf_printOnes(void *tfParams){

    while (1) {
        
        puts("1111111111111111111111111111111111111111111111111111");
        vTaskDelay(5000 / portTICK_PERIOD_MS);

    }

}
