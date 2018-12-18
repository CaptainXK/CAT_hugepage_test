#include <hugepage_env.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>//mmap
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>//open
#include <unistd.h>//close

struct hp_mem_info{
    int fd;
    void * ori_addr;
    void * alloc_start;
    uint64_t tot_bytes;
    uint64_t free_bytes;
};

typedef struct hp_mem_info hp_mem_info;

hp_mem_info gl_hp_mem_info;

int
hugepage_build(uint64_t _tot_bytes)
{
    //open hugepage memory file
    gl_hp_mem_info.fd = open("/mnt/huge/mem", O_CREAT|O_RDWR, S_IRWXU);
    if(gl_hp_mem_info.fd < 0){
        perror("Open hugepage file error:");
        return 1;
    }

    //mmap to virtual address apace
    void* _map_addr = NULL;
    _map_addr = mmap(0, _tot_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, gl_hp_mem_info.fd, 0);
    if( _map_addr == MAP_FAILED){
        perror("Mmap hugepage fd error:");
        goto ERROR;
    }
    
    //set global hugepage memory info
    gl_hp_mem_info.ori_addr = _map_addr;
    gl_hp_mem_info.alloc_start = _map_addr;
    gl_hp_mem_info.tot_bytes = _tot_bytes;
    gl_hp_mem_info.free_bytes = _tot_bytes;

    //build done
    return 0;

    MAPERROR:
        munmap(gl_hp_mem_info.ori_addr, _tot_bytes);
    ERROR:
        close(gl_hp_mem_info.fd); 

    //build fail
    return 1;
}

void
hugepage_env_empty()
{
    gl_hp_mem_info.fd = 0;
    gl_hp_mem_info.ori_addr = NULL;
    gl_hp_mem_info.alloc_start  = NULL;
    gl_hp_mem_info.tot_bytes = 0;
    gl_hp_mem_info.free_bytes = 0;
}

void
hugepage_env_init(uint64_t _tot_bytes)
{
    //empty global hugapage mem info
    hugepage_env_empty();

    //build hugepage mem
    int ret;
    ret = hugepage_build(_tot_bytes);
    assert(ret == 0);
}

void
hugepage_env_destory(void)
{
    hugepage_env_empty();
}

void*
hp_alloc(int m_nb, int m_size)
{
    uint64_t _tar_size = m_nb * m_size;
    void * _ret = NULL;

    if( gl_hp_mem_info.free_bytes < _tar_size )
    {
        printf("Not enough hugepage memory!\n");
    }
    else{
        _ret = gl_hp_mem_info.alloc_start;
        gl_hp_mem_info.free_bytes -= _tar_size;
        gl_hp_mem_info.alloc_start = (void*)((uint8_t*)(gl_hp_mem_info.alloc_start) + _tar_size);
    }

    return _ret;
}

void
hp_free(void ** ptr, int _bytes)
{
    gl_hp_mem_info.alloc_start = (void*)(*ptr);
    gl_hp_mem_info.free_bytes += _bytes;
    *ptr = NULL;
}
