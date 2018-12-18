/*
 * simple hugepage memory management
 * no patterner alogrithm
 * just alloc and free
 */
#include <inttypes.h>

#ifndef _HUGEPAGE_ENV_
#define _HUGEPAGE_ENV_

//hugepage environment init
void
hugepage_env_init(uint64_t _tot_bytes);

//hugepage environment destory
void
hugepage_env_destory(void);

//hugepage alloc 
void*
hp_alloc(int m_nb, int m_size);

//hugepage free
void hp_free(void **ptr, int _size);

#endif
