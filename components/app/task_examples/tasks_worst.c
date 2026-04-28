#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"

#include "tasks.h"
#include "mcmanager.h"

void Task_0_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_0 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_1_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_1 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_2_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_2 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_3_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_3 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_4_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_4 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_5_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_5 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_6_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_6 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_7_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_7 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_8_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_8 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_9_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_9 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_10_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_10 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_11_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_11 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_12_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_12 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_13_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_13 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_14_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_14 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_15_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_15 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_16_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_16 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_17_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_17 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_18_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_18 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_19_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_19 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_20_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_20 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_21_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_21 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_22_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_22 - %lu]\n", xTaskGetTickCount());
    }
}

void Task_23_utask(void *pvParameters) {
    uint8_t task_id = (uint8_t)(uintptr_t) pvParameters;
    while (1) {
        mcm_wait_for_release(task_id);
        printf("[Task_23 - %lu]\n", xTaskGetTickCount());
    }
}
