#include "mcmanager.h"
#include "freertos/idf_additions.h"
#include "generated.h"
#include "stdlib.h"
#include "esp_log.h"


// Component tag for ESP-IDF logging
static const char* TAG = "MCMANAGER";

static mode_e current_mode = MODE_INIT;



// this may not be needed if just this file uses the current_mode variable
mode_e get_current_mode(){

    return current_mode;

}


const transition_t *get_transition(mode_e old, mode_e new){

    if ( old < 0 || old >= MODE_COUNT || new < 0 || new >= MODE_COUNT ) {
        
        ESP_LOGE(TAG, "Mode arguments in function 'get_transition()' are out of bounds");
        abort();

    }

    return &g_transition_table[old][new];

}

// READ ME:
// from video, tips for software dev:
// 1. Dont just try to make it work. Make some observations. Note down what i'm intending to build, the
// requirements, how does it fit with what already exists in the codebase. Readability, Maintainability,
// extensibility.
// 2. Dont change too many things at once when developing some part of the project. Commit often, with
// small, tested changes, break down changes if they are too big. It's easier to keep track of the changes
// and functionality that way. Create a function - commit; but dont push. Commit often and then when the full
// feature is developed, merge the branch to master and push to origin.


void change_mode(mode_e new){

    // 4 types of tasks: Old, New, Changed, Unchanged
    // change the parameters of the tasks that are active in modes A and B
    // suspend the tasks that arent active in mode B
    // start the tasks that weren't active in A but are in B
    // XXX: What protocol to follow? -> create a manual transition table... [!]
    //      how does a transition table look like?
    //          It specifies, for instance, when to release a new job in
    //          the destination mode (e.g., immediately or after some delay),
    //          whether to complete or abort existing jobs, and whether to
    //          update the existing jobs with the new timing parameters
    //              XXX: Are there more things to take into account for the mode change?


    mode_e old_mode = get_current_mode();
    const transition_t *trans_info = get_transition(old_mode, new);

    // 1. check which tasks should be active in mode A and mode B

    // XXX: Do all this without separating in functions?
    // 2. check and perform actions for each type of task
    // function that flags tasks of the new mode as one of the four types? -> need to do that to
    // perform the actions fromo the transition_t struct
    // void flag_tasks(mode_e old, mode_e new, uint_8 *flagged);
    // 0 - 1, new
    // 1 - 0, old
    // 1 - 1, changed
    // 1 - 1, unchanged

    // 3. in ascending order, perform the changes according to the flags of the active tasks in mode B
    // void update_tasks(uint_8 *flagged);


}



