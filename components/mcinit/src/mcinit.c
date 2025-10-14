#include "mcinit.h"
#include "esp_log.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct TaskParams_t (*params)[N_MODES] = NULL;
const char *TAG = "MCINIT.C";

/*
 * Create a 2d array that holds the parameters of each task in every mode, or NULL if the task should
 * not be active in a certain mode.
 * */
void params_create(){
    params = malloc(sizeof(struct TaskParams_t [N_TASKS][N_MODES]));
    if (params != NULL) {
        ESP_LOGI(TAG, "params data structure allocated successfully");
    } else {
        ESP_LOGE(TAG, "Memory allocation for the params data structure did not succeed");
        abort();
    }

    //initialize the entries to all zeroes
    memset(params, 0, sizeof(struct TaskParams_t [N_TASKS][N_MODES]));
}

/*
 * Add the parameters of a task in a specific mode in the params data structure
 * Two options of indexing the tasks:
 *      1. the user supplies a task_id (integer) of value 0 to N_TASKS-1
 *      [2.] the parser creates an id for each task automatically
 *      3. the tasks are indexed by a string and the parser creates a data structure that maps
 *      those strings to indexes in the array.
 * */
void params_add_entry(uint16_t task_id, uint16_t mode_id, struct TaskParams_t task_params){
    if (!(params[task_id][mode_id].filled)) {
        params[task_id][mode_id] = task_params;
        params[task_id][mode_id].filled = true;
        ESP_LOGI(TAG, "params entry: %ud-%ud; filled successfully!", task_id, mode_id);
    } else {
        ESP_LOGE(TAG, "Trying to fill params entry: %ud-%ud; but it is already filled!", task_id, mode_id);
        abort();
    }
}

void params_fill(){

}

struct TaskParams_t params_lookup(uint16_t task_id, uint16_t mode_id){
    if (!(params[task_id][mode_id].filled)) {
        ESP_LOGE(TAG, "Trying to lookup params entry: %ud-%ud; but it is not filled!", task_id, mode_id);
        abort();
    }
    ESP_LOGI(TAG, "Returning params entry: %ud-%ud; successfully!", task_id, mode_id);
    return params[task_id][mode_id];
}

void mcinit(){
    params_create();
    struct TaskParams_t example_params = { .args = NULL, .filled = 0, .priority = 10 };
    params_add_entry(0, 0, example_params);
    printf("Priority of entry at 0-0: %d\n", params[0][0].priority);
}
