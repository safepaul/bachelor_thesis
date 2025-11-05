#include "../components/mcinit/include/mcinit.h"
#include "../components/mcmanager/include/mcmanager.h"
#include "unistd.h"



void app_main(void){
    mcinit();


    sleep(10);


    mode_change_request(MODE_NORMAL);


    sleep(5);


    mode_change_request(MODE_INIT);
}
