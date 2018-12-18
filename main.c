#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>//strtoul
#include <unistd.h>
#include <assert.h>
#include <string.h>//strtok_r()

#include <hugepage_env.h>
#include <core_worker.h>
#include <time_measurer.h>

#define MAX_CORE_NB 32

struct Global_Config{
    int core_nb;
    int core_list[MAX_CORE_NB];
    uint64_t tot_bytes;
};
typedef struct Global_Config Global_Config;

/*
 * CMD usage
 */
void show_usage()
{
    printf("Usage:\n");
    printf("\t-m : memory size in byte\n");
    printf("\t-n : core number\n");
    printf("\t-l : core list, format is look like '-l \"1,2,3....\"'\n");
    printf("\t-h : show helper\n");
}

/*
 * parse core list
 */
void parse_core_list(char * _cmd, int _core_list[])
{
    char *str, *token;
    char *saveptr;
    int idx = 0;

    for(str = _cmd; ; str = NULL){
        token = strtok_r(str, "\",", &saveptr);
        if(token == NULL)
            break;
        else
            _core_list[idx++] = (int)atoi(token);
    }

}

/*
 * parse cmd to get hugepage size
 */
void parse_cmd(int argc, char** argv, Global_Config * _gl_conf)
{
    int opt;
    char * endptr;

    while( (opt = getopt(argc, argv, "hm:n:l:")) != -1 ){
        switch(opt){
            case 'm':
                _gl_conf->tot_bytes = (uint64_t)strtoul(optarg, &endptr, 10);
                switch(*endptr){
                    case 'm':
                    case 'M':
                        _gl_conf->tot_bytes *= 1024;
                    case 'k':
                    case 'K':
                        _gl_conf->tot_bytes *= 1024;
                }
                printf("%lu bytes totally\n", _gl_conf->tot_bytes);
                break;
            case 'n':
                _gl_conf->core_nb = (int)atoi(optarg);
                printf("%d cores totally\n", _gl_conf->core_nb);
                break;
            case 'l':
                printf("core list : %s\n", optarg);
                parse_core_list(optarg, _gl_conf->core_list);
                break;
            case 'h':
            default:
                show_usage();
                break;
        }
    }
}

int main(int argc, char** argv){
    uint8_t * shared_data;
    Global_Config gl_conf;
    
    //parse hugepage size
    parse_cmd(argc, argv, &gl_conf);
    assert(gl_conf.tot_bytes != 0);

    //init hugepage memory
    hugepage_env_init(gl_conf.tot_bytes);

    //alloc hugepage size
    shared_data = (uint8_t*)hp_alloc(gl_conf.tot_bytes, sizeof(uint8_t));
    assert(shared_data != NULL);

    //init all core workers
    init_core_worker(&shared_data, gl_conf.tot_bytes, gl_conf.core_nb, gl_conf.core_list);

	time_start();

    //start all core
    all_start();

	time_stop();

	time_report();

    //all stop
    all_stop();

    return 0;
}

