#include <stdio.h>
#include <CAS_FUNC.h>

bool DO_CAS(volatile int * ptr, int oldval, int newval){
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}
