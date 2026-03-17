#include "gen_data.h"
#include "mcmanager.h"
#include <stdio.h>
#include <unistd.h>

void app_main(void)
{
    mcm_init();

    /*
    
    // after 6 seconds, mode change request MODE_INIT -> MODE_EMERGENCY (0 -> 1)
    sleep(6);
    printf("[DEBUG] executing mc request: source = %d,  dest = %d \n",
           MODE_INIT, MODE_EMERGENCY);

    mcm_mc_request(MODE_EMERGENCY);

    // after 6 seconds, mode change request MODE_NONE -> MODE_INIT (0 -> 1)
    // ACTION_UPDATE-GUARD_TRUE and ACTION_RELEASE-GUARD_OFFSETMCR are called. A
    // Timer should be created and fired after some time, and the task released
    // just afterwards, in a timely manner (exactly 3 seconds)
    sleep(6);
    printf("[DEBUG] executing mc request: source = %d,  dest = %d \n",
           MODE_EMERGENCY, MODE_INIT);
    mcm_mc_request(MODE_INIT);

    */
}
