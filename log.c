#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"

static void free_logger(logger *lgr)
{
    if(lgr != NULL)
    {
        if(lgr ->logger_name)
        {
            FREE(lgr ->logger_name) ;
            lgr ->logger_name = NULL ;
        }
        if(lgr ->errmsg)
        {
            FREE(lgr ->errmsg) ;
            lgr ->errmsg = NULL ;
        }
        pthread_mutex_destroy(&lgr ->lock) ;
    }
    lgr == NULL ;
}

static void close_stream(stream *stm)
{
    if(stm != NULL)
    { 
        if(stm ->stm_type == STREAM_TYPE_FILE)
        {
            close(stm ->file_stm.fd) ;
            FREE(stm ->file_stm.file_name) ;
        }
        else if(stm ->stm_type == STREAM_TYPE_TCP_SOCKET)
        {
            close(stm ->tcp_sock_stm.fd) ; 
            FREE(stm ->tcp_sock_stm.sock_addr) ;
        }
        else if(stm ->stm_type == STREAM_TYPE_UDP_SOCKET)
        {
            close(stm ->udp_sock_stm.fd) ;
            FREE(stm ->udp_sock_stm.sock_addr) ;
        }
    }
}

static void init(stream_array *stm_ay)
{ 
    if(stm_ay != NULL)
    {
        stm_ay ->stm = NULL ;
        stm_ay ->curr_sz = 0 ;
        stm_ay ->size = 0 ;
    }
}

static int realloc_stream_array(stream_array *stm_ay, int sz)
{
    if(stm_ay == NULL)
    {
        return -1 ;
    } 
    if(sz < stm_ay ->size)
    {
        return 0 ;
    }
    stream *stm = (stream *)MALLOC(sizeof(stream)*sz) ;
    if(stm == NULL)
    {
        return -1 ;
    }
    if(stm_ay ->curr_sz > 0)
    {
        memcpy(stm, stm_ay ->stm, sizeof(stream)*stm_ay->curr_sz) ;
        FREE(stm_ay ->stm) ;
    }
    stm_ay ->stm = stm ;
    return 0 ;
}

//检查array的大小，如果size满了，则重新分配
static int check_array_size(stream_array *stm_ay)
{
    if(stm_ay == NULL ||(stm_ay ->curr_sz >= stm_ay ->size && realloc_stream_array(stm_ay, stm_ay ->size + 20) != 0))
    {
        return -1 ;
    }
    return 0 ;
}

static int write_file_stream(stream *stm, LOG_LEVEL lvl, void *data, int size)
{
    if(stm == NULL || data == NULL || size < 0) 
    {
        return -1 ;
    }

}

static int add_file_stream_to_ay(stream_array *stm_ay, const char *file_name, LOG_LEVEL lvl, filter_log is_write)
{
    if(check_array_size(stm_ay) == -1 || file_name == NULL)
    {
        return -1 ;
    }
    int fd = open(file_name, O_APPEND) ;
    if(fd < 0)
    {
        return -1 ;
    }
    stream *stm = stm_ay ->stm + stm_ay ->curr_sz ;
    stm ->file_stm.fd= fd ;
    stm ->level = lvl ;
    stm ->file_stm.file_name = strdup(file_name) ;
    stm ->stm_type = STREAM_TYPE_FILE ;
    stm ->write_log = write_file_stream ;
    stm ->is_write = is_write ;
    if(stm ->file_stm.file_name == NULL)
    {
        goto __fails ; 
    }
    if(fstat(fd, &stm ->file_stm.st) == -1)
    {
        goto __fails ;
    }
    stm_ay ->curr_sz += 1 ;
    return 0 ; 

__fails:
    close_stream(stm) ;
    return -1 ;
}

static int add_std_stream_to_ay(stream_array *stm_ay, const char *stm_name, LOG_LEVEL lvl, filter_log is_write)
{
    if(check_array_size(stm_ay) == -1 || stm_name == NULL)     
    {
        return -1 ;
    }
    stream *stm = stm_ay ->stm + stm_ay ->curr_sz ;
    stm ->level = lvl ;
    stm ->is_write = is_write ;
    stm ->stm_type = STREAM_TYPE_STD ;
    if(strcmp(stm_name, "stdout") == 0)
    {  
       stm ->std_stm.fd = fileno(stdout)  ;
       stm_ay ->curr_sz += 1 ;
    }
    else if(strcmp(stm_name, "stderr") == 0)
    {
        stm ->std_stm.fd = fileno(stderr) ;       
        stm_ay ->curr_sz += 1 ;
    }
    return -1 ;
}

logger *new_logger(const char *logger_name) 
{
    if(logger_name == NULL)
    {
        return NULL ;
    }
    logger *lgr = (logger *)MALLOC(sizeof(logger)) ;
    if(lgr == NULL)
    {
        goto __fails ;
    }
    lgr ->log_level = LEVEL_DEBUG;
    lgr ->logger_name = strdup(logger_name) ;
    if(lgr ->logger_name == NULL)
    {
        goto __fails ;
    }
    if(pthread_mutex_init(&lgr ->lock, NULL) == -1) 
    {
        goto __fails ;
    }
    return lgr ;

__fails:
    free_logger(lgr) ;
    return NULL ;
}

void set_logger_level(logger *lgr, LOG_LEVEL lvl) 
{
    if(lgr != NULL)        
    {
        lgr ->log_level = lvl ;
    }
}

int add_stdout_stream(logger *lgr, LOG_LEVEL lvl, filter_log is_write) 
{
   if(lgr == NULL) 
   {
        return -1 ;
   }
   return add_std_stream_to_ay(&lgr ->stm_ay, "stdout", lvl, is_write) ;
}

int add_stderr_stream(logger *lgr, LOG_LEVEL lvl, filter_log is_write)
{
    if(lgr == NULL)
    {
        return -1 ;
    }
    return add_std_stream_to_ay(&lgr ->stm_ay, "stderr", lvl, is_write) ;
}

int add_file_stream(logger *lgr, const char *file_name, LOG_LEVEL lvl, filter_log is_write)
{
    if(lgr == NULL)
    { 
        return -1 ;
    }
    return add_file_stream_to_ay(&lgr ->stm_ay, file_name, lvl, is_write) ;
}

static int  lock_logger(logger *lgr)
{
    if(lgr != NULL)
    {
        return pthread_mutex_lock(&lgr ->lock) ;
    }
    return -1 ;
}

static int unlock_logger(logger *lgr)
{
    if(lgr != NULL)
    {
        return pthread_mutex_unlock(&lgr ->lock) ;
    }
    return -1 ;
}

static int __write_log(logger *lgr, LOG_LEVEL lvl, void *data, int size)
{
    if(lgr == NULL || data == NULL || size < 0) 
    {
        return -1 ;
    }
    stream *stm = lgr ->stm_ay.curr_sz != 0 ? lgr ->stm_ay.stm:NULL ;
    int i = 0 ;
    for(i=0; i < lgr ->stm_ay.curr_sz ;i++)
    {
       if(stm[i].level >= lvl && (stm[i].is_write == NULL ||stm[i].is_write(stm[i].level, lvl))) 
       {
            //int  fd = (stm[i].stm_type == STREAM_TYPE_FILE       ? stm[i].file_stm.fd     :
            //          (stm[i].stm_type == STREAM_TYPE_STD        ? stm[i].std_stm.fd      :
            //          (stm[i].stm_type == STREAM_TYPE_TCP_SOCKET ? stm[i].tcp_sock_stm.fd :
            //          (stm[i].stm_type == STREAM_TYPE_UDP_SOCKET ? stm[i].udp_sock_stm.fd : -1)))) ;
            if(stm[i].write_log(stm + i, lvl, data, size) != size) 
            {
                return -1 ;
            }
       }
    }
}

//时间,thread-id,file:lineno [level-name] 
int write_log(logger *lgr, LOG_LEVEL lvl, void *data, int size)
{
    lock_logger(lgr) ;
    int ret = __write_log(lgr, lvl, data, size) ;
    unlock_logger(lgr) ;
    return ret ;
}
