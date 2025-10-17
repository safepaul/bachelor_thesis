#ifndef MCINIT_H
#define MCINIT_H

#include "../../components/generated/include/generated.h"



extern TaskHandle_t g_task_handles[TASK_COUNT];


task_e task_name_to_index(const char* name);

const char* task_index_to_name(task_e t);

const task_info_t *get_task_info(task_e t, mode_e m);

void create_tasks();

void mcinit();


#endif // !MCINIT_H

