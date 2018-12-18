#ifndef _CORE_WORKER_
#define _CORE_WORKER_

#include <inttypes.h>

void
init_core_worker(uint8_t **_shared_data, int _data_cnt, int core_nb, int core_list[]);

void
all_start();

void
all_stop();

#endif

