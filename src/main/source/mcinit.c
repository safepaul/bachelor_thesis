#include <stdio.h>
#include <stdlib.h>

#include "freertos/idf_additions.h"
#include "../data/tasks.h"


// creates and starts all tasks, data structures and fills them...


// the task functions have to be supplied
// mcinit may create the tasks from functions in a task.c+task.h files, from
// a list of function names or similar

// idea: first fill the data structures with the task details then, for example
// if it is marked as an initial mode task, start it active, else, start it suspended

void initialize_tasks(){
    // TODO: pull the task parameters from a data structure done in other function
    puts("\nInitializing all tasks");

    int *period = malloc(sizeof(int));
    *period = 1000;
    xTaskCreatePinnedToCore(taskPrintTime_u,
                "Print Time",
                2048,
                period,
                1,
                NULL,
                0);

}

void mcinit(){
    initialize_tasks();
}

