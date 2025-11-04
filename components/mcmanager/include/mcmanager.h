#ifndef MCMANAGER_H
#define MCMANAGER_H

#include "generated.h"








/*
 * This is the "master" function. Calling the mcr() triggers the rest of the functions.
 * 
 * */
void mode_change_request(mode_e m);


/*
 * Returns the current active mode
 *
 * */
mode_e get_current_mode();


/*
 * 
 *
 * */
void change_mode(mode_e new);


/*
 * Returns the entry that gives the info about how to perform the transition MODE_OLD -> MODE_NEW
 *
 * */
const transition_t *get_transition(mode_e old, mode_e new);


#endif // !MCMANAGER_H
