#include "mcmanager.h"
#include "gen_data.h"







void app_main(void){

    generate_data();
    spawn_tasks();

    mc_request(1);

    // debug_print_transition_table();
    // debug_print_trans_tasks_table();
    debug_print_task_info_table();

}
