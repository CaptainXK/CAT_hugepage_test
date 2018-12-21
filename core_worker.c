#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <core_worker.h>
#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>
#include <CAS_FUNC.h>
#include <stdlib.h>//exit()

#define MAX_THD_NB 32

#define RUN_INFI 1

//shared data
struct shared_data{
    int core_cnt;
    int start_work;
    int thd_count;
    int head;
    int data_cnt;
    uint8_t * data;
};
typedef struct shared_data shared_data;
volatile shared_data test_data;

//thread var
struct thd_param{
    int thd_id;
    int tar_cpu_id;
};
typedef struct thd_param thd_param;

pthread_cond_t cond;
pthread_mutex_t cond_mutex;
pthread_t thds[MAX_THD_NB];
volatile int force_quit = 0;
struct sigaction gl_sa;

//quit signal handler
void
signal_handler(int signo){
    printf("[Got a signal]\n");
    
    if(signo == SIGINT || signo == SIGTERM){
        printf("[Force Quit]\n");
        force_quit = 1;
    }
}

//register quit signal handler
void
register_signal()
{
    gl_sa.sa_handler = signal_handler;

    sigemptyset(&gl_sa.sa_mask);

    sigaction(SIGINT, &gl_sa, 0);
    sigaction(SIGTERM, &gl_sa, 0);

    printf("[Signal handler register done]\n");
}

//worker function
void* thd_func(void * _param)
{
    thd_param * tp = (thd_param*)_param;
    int tar_cpu = tp->tar_cpu_id;
    int thd_id = tp->thd_id;

    //core migrate
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(tar_cpu, &cpu_set);
    if( pthread_setaffinity_np(pthread_self(), sizeof(cpu_set), &cpu_set) ){
        fprintf(stderr, "worker#%d:[Can not migrate to CPU#%d]\n", thd_id, tar_cpu);
        exit(1);
    }
    else{
        printf("worker#%d:[I am on CPU#%d]\n", thd_id, sched_getcpu());
    }
    
    //signal to master core
    pthread_cond_signal(&cond);

    int old_head;
    int old_thd_count;
    int tot_data = test_data.data_cnt;
    int thd_stride = test_data.data_cnt / test_data.core_cnt;

#if RUN_INFI
    while(force_quit == 0){
#endif
        while(test_data.start_work == 0 && force_quit == 0){
            continue;
        }

        old_head = 0;
        old_thd_count=0;
        
        //start to work
        //CAS op to find work area 
        do{
            old_head = test_data.head;
        }while(!DO_CAS(&(test_data.head), old_head, old_head + thd_stride));

        for(int i = old_head; i < (old_head + thd_stride) && i < tot_data; ++i){
            test_data.data[i] += 1;
        }

        do{
            old_thd_count = test_data.thd_count;
        }while(!DO_CAS(&(test_data.thd_count), old_thd_count, old_thd_count + 1));

#if RUN_INFI
        //sychronize with master and other thread
        while(test_data.thd_count != 0 && force_quit == 0)
            continue;
    }
#endif

}

//init env of multi core workers
void
init_core_worker(uint8_t **_shared_data, int _data_cnt, int core_nb, int core_list[])
{
    //set shared data
    test_data.data = *_shared_data; 
    test_data.data_cnt = _data_cnt;
    test_data.start_work = 0;
    test_data.thd_count = 0;
    test_data.head = 0;
    test_data.core_cnt = core_nb;

    //empty data
    for(int i = 0; i < _data_cnt; ++i){
        test_data.data[i] = 0;
    }

    //register signal
    register_signal();
    
    //init cond and mutex
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&cond_mutex, NULL);

    //create all workers
    for(int i = 0; i < core_nb; i++){
        thd_param tp_tmp;
        tp_tmp.thd_id = i;
        tp_tmp.tar_cpu_id = core_list[i];

        thds[i] = pthread_create(&thds[i], NULL, thd_func, (void*)&tp_tmp);
        pthread_cond_wait(&cond, &cond_mutex); 
    }
}

void reset_data()
{
    test_data.thd_count = 0;
    test_data.head = 0;
}

//start all worker
void
all_start()
{
    printf("[Master core working, PID=%d]\n", getpid());
#if RUN_INFI
	printf("[Infinite running mode]\n");
#else
	printf("[Once running mode]\n");
#endif

#if RUN_INFI
    while(force_quit == 0){
#endif
        //start all thread
        test_data.start_work = 1;    
    
        //poll working status
        while(test_data.thd_count != test_data.core_cnt && force_quit == 0);
        
        //pause all thread
        test_data.start_work = 0;    

        //reset data
    //    printf("Master reset data\n");
        reset_data();
#if RUN_INFI
    }
#endif
    
    printf("[Master core stop]\n");
}

//void stop all worker
void 
all_stop()
{
    int error_count = 0;
    int data_cnt = test_data.data_cnt;

    for(int i = 0; i < data_cnt; ++i){
        if(test_data.data[i] == 0)
            error_count++;
    }

    printf("error rate %d/%d\n", error_count, data_cnt);
}



