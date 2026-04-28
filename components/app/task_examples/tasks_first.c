#include <stdint.h>
#include <stdio.h>

#include "freertos/idf_additions.h"

#include "tasks.h"
#include "mcmanager.h"


void taskZero_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[T0 - %lu] Hello! Task ZERO speaking\n", xTaskGetTickCount());
    }
}

void taskOne_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[T1 - %lu] Hello! Task ONE speaking\n", xTaskGetTickCount());
    }
}
