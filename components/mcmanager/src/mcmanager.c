#include "../include/mcmanager.h"

#include "freertos/idf_additions.h"
#include "stdlib.h"
#include "esp_log.h"
#include <stdint.h>


// Component tag for ESP-IDF logging
static const char* TAG = "MCMANAGER";



void mcm_create_mode(uint8_t mode_id){

    // data structure 
    // i dont know how many entries(modes) there will be beforehand
    // indexed by id (mode id, an integer)
    // contains tasks with their parameters
    // i dont know how many tasks in each mode
    // i know the type and the amount of parameters of the tasks
    // i know there will be < 100 modes
    // i know there will be < 1000 tasks

}







void mode_change_request(){

}

