
#include <stdint.h>
#include <stdio.h>

#include "freertos/idf_additions.h"

#include "tasks.h"
#include "mcmanager.h"




void Telemetry_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[Telemetry - %lu]\n", xTaskGetTickCount());
    }
}

void Sensor_Fusion_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[SensorFusion - %lu]\n", xTaskGetTickCount());
    }
}

void Control_Loop_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[ControlLoop - %lu]\n", xTaskGetTickCount());
    }
}

void Vision_Processing_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[VisualProcessing - %lu]\n", xTaskGetTickCount());
    }
}

void Data_Logging_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[DataLogging - %lu]\n", xTaskGetTickCount());
    }
}

void Diagnostics_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[Diagnostics - %lu]\n", xTaskGetTickCount());
    }
}

void Failsafe_Monitor_utask(void *pvParameters)
{
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;

    while (1)
    {
        mcm_wait_for_release(task_id);
        printf("[FailsafeMonitor - %lu]\n", xTaskGetTickCount());
    }
}
