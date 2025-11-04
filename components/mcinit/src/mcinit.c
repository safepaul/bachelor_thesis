#include "freertos/idf_additions.h"
#include <stdio.h>
#include <string.h>

#include "freertos/projdefs.h"
#include "portmacro.h"

#include "../include/mcinit.h"



TaskHandle_t g_task_handles[TASK_COUNT];


task_e task_name_to_index(const char* name){

    for (int i = 0; i < TASK_COUNT; i++) {

        if( !strcmp(g_task_map[i].name, name) ){

            return g_task_map[i].id;

        }

    }
    
    return TASK_COUNT;      // did not find the name
}


const char* task_index_to_name(task_e t){

    if (t < 0 || t > TASK_COUNT) {

        return "";          // no index found

    }

    for (int i = 0; i < TASK_COUNT; i++) {

        if( g_task_map[i].id == t ){

            return g_task_map[i].name;

        }

    }

    return "";              // this means no index was found

}


const task_info_t *get_task_info(task_e t, mode_e m){

    if (t > TASK_COUNT || m > MODE_COUNT || t < 0 || m < 0) {

        return NULL;

    }

    return &g_task_table[t][m];

}


void create_tasks(){


    for (int i = 0; i < TASK_COUNT; i++) {

        const char *task_name = task_index_to_name(i);

        if ( !(strcmp("", task_name)) ) {

            abort();

        }

        const task_info_t *t_info = get_task_info(i, MODE_INIT);          // for now, all tasks start in MODE_INIT (mode 0)
        TaskHandle_t handle = NULL;
        BaseType_t ret;

        ret = xTaskCreate(g_task_funcs[i], task_name, GLOBAL_STACK_DEPTH, t_info->args_p, 1, &handle);

        if ( ret == pdPASS ) {
            
            g_task_handles[i] = handle;

        } else {

            abort();

        }

    }

}



void mcinit(){

    create_tasks();

    printf("Handle task 0: %p\n", g_task_handles[0]);
    printf("Handle task 1: %p\n", g_task_handles[1]);

}


