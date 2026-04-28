#include "mcmanager.h"
#include "tasks.h"


void taskSLR_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[TSLR - %lu] Hello! Task 0 (Suspend on Last Release) speaking\n", xTaskGetTickCount());
    }
}

void taskSMCR_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[TSMCR - %lu] Hello! Task 1 (Suspend on MCR Instant) speaking\n", xTaskGetTickCount());
    }
}

void taskRMCR_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[TRMCR - %lu] Hello! Task 2 (Release on MCR Instant) speaking\n", xTaskGetTickCount());
    }
}

void taskULR_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[TULR - %lu] Hello! Task 3 (Update on Last Release) speaking\n", xTaskGetTickCount());
    }
}

void taskUMCR_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[TUMCR - %lu] Hello! Task 4 (Update on MCR Instant) speaking\n", xTaskGetTickCount());
    }
}
