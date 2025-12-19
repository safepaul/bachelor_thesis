#include "mcmanager.h"
#include "gen_data.h"



void app_main(void){

    generate_data();
    debug_print_transition_table();
    debug_print_trans_tasks_table();

}
