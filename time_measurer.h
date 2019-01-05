#ifndef _TIME_MEASURER_
#define _TIME_MEASURER_
#include <time.h>
#include <stdio.h>
#include <inttypes.h>

enum{
	s=0,
	ms=1,
	us=2,
	ns=3
};

char* unit_str[4] = {"s", "ms", "us", "ns"};

struct timespec begin, end;
uint64_t dura;

static inline
void time_start(){
    clock_gettime(CLOCK_REALTIME, &begin);
}

static inline
void time_stop(){
    clock_gettime(CLOCK_REALTIME, &end);
}

static inline
void time_report(int _unit){
    dura = ((int64_t)end.tv_sec*1000000000 + end.tv_nsec) - ((int64_t)begin.tv_sec*1000000000 + begin.tv_nsec);//ns

	double d_dura = (double)dura;

	switch(_unit){
		case 0:d_dura /= 1000;//s
		case 1:d_dura /= 1000;//ms
		case 2:d_dura /= 1000;//us
		case 3://ns
		default:
			   break;
	}

    printf("Total time elapsed : %lf %s\n", d_dura, unit_str[_unit]);
}

#endif
