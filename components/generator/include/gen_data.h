#ifndef GEN_DATA_H
#define GEN_DATA_H


#define MODE_INIT (uint8_t) 0
#define MODE_EMERGENCY (uint8_t) 1

#define N_TASKS 2
#define N_TRANS 2
#define N_MODES 2
#define LIMIT_BACKLOG (uint8_t) 5

void mcm_init();

#endif //GEN_DATA_H