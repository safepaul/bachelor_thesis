#include "gen_data.h"
#include "mcmanager.h"

#include <stdio.h>
#include <unistd.h>

#include "esp_timer.h"
#include "esp_private/esp_clk.h"



void app_main(void)
{
    mcm_init();


    sleep(5);
    printf("[DEBUG] executing mc request: source = %d,  dest = %d \n", MODE_0, MODE_1);


    // int64_t t_inicio = esp_timer_get_time();
    // mcm_mc_request(MODE_1);
    // int64_t t_fin = esp_timer_get_time();
    // printf("MC REQUEST TOOK %lld microseconds (?)\n", (t_fin - t_inicio));
    // resultado: MC REQUEST TOOK 103059 microseconds (?)

    uint32_t t_start = esp_cpu_get_cycle_count();
    mcm_mc_request(MODE_1);
    uint32_t t_end = esp_cpu_get_cycle_count();

    uint32_t ciclos = t_end - t_start;
    uint32_t freq_mhz = esp_clk_cpu_freq() / 1000000;
    printf("Ciclos: %lu (Aprox %lu microsegundos)\n", ciclos, ciclos / freq_mhz);
    // Ciclos: 16488792 (Aprox 103054 microsegundos)



    // sleep(5);
    // printf("[DEBUG] executing mc request: source = %d,  dest = %d \n", MODE_2, MODE_4);
    // mcm_mc_request(MODE_4);
}
