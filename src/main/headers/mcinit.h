#ifndef MCINIT_H
#define MCINIT_H


/*
 * Creates and fills the relevant data structures with the relevant data
 * */
void ds_init();

/*
 * Creates the tasks and sets their status according to the system initial state
 * */
void initialize_tasks();

/*
 * Initialization function. Creates the data structures for [...], the tasks, [...]
 * */
void mcinit();


#endif // !MCINIT_H

