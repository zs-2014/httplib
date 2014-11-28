#ifndef __LOG__H
#define __LOG__H

#include <sys/stat.h> 
#include <pthread.h>

#define MALLOC(sz) malloc(sz)
#define ALLOC(n, sz) alloc(n, sz)
#define REALLOC(ptr, sz) realloc(ptr, sz)
#define FREE(ptr) free(ptr)

typedef enum LOG_LEVEL
{ 
    LEVEL_DEBUG = 0,
    LEVEL_INFO, 
    LEVEL_WARN,
    LEVEL_ERROR
}LOG_LEVEL ;

typedef enum STREAM_TYPE
{
    STREAM_TYPE_FILE = 0,
    STREAM_TYPE_STD ,
    STREAM_TYPE_TCP_SOCKET,
    STREAM_TYPE_UDP_SOCKET
}STREAM_TYPE ;

typedef struct file_stream
{
    int fd ;
    //类型 文件流  标准输出流
    int stream_type ;
    struct stat st ;
    char *file_name ;
}file_stream ;

typedef struct socket_stream
{
    int fd ; 
    void *sock_addr ;
    int sock_len ;
}tcp_socket_stream, udp_socket_stream ;

typedef struct std_stream
{
    int fd ;
}std_stream;

typedef int (*filter_log)(LOG_LEVEL ori_lvl, LOG_LEVEL pass_lvl) ;
typedef struct stream 
{
    STREAM_TYPE stm_type ;
    LOG_LEVEL level ;
    union
    {
        file_stream file_stm ;    
        tcp_socket_stream tcp_sock_stm ;
        udp_socket_stream udp_sock_stm ;
        std_stream std_stm ;
    }stm ;
    int (*write_log)(struct stream *stm, LOG_LEVEL lvl, void *data, int size) ;
    filter_log is_write ;
    //int (*is_write)(LOG_LEVEL orilevel, LOG_LEVEL passlevel) ;
}stream;

#define file_stm      stm.file_stm
#define udp_sock_stm  stm.udp_sock_stm
#define tcp_sock_stm  stm.tcp_sock_stm 
#define std_stm       stm.std_stm

typedef struct stream_array
{
    stream *stm;  
    int curr_sz ;
    int size ;
}stream_array;

typedef struct logger
{ 
    pthread_mutex_t lock ;
    struct Logger *parent ;
    struct logger *child ;
    stream_array stm_ay ;
    char *logger_name ;
    LOG_LEVEL log_level ;
    char *errmsg ;
}logger;


#ifdef __cplusplus
extern "C"{
#endif

extern logger *new_logger(const char *logger_name) ;
extern void set_logger_level(logger *lgr, LOG_LEVEL lvl) ;
extern int add_stdout_stream(logger *lgr, LOG_LEVEL lvl, filter_log is_write) ;
extern int add_stderr_stream(logger *lgr, LOG_LEVEL lvl, filter_log is_write) ;
extern int add_file_stream(logger *lgr, const char *file_name, LOG_LEVEL lvl, filter_log is_write) ;

extern int write_log(logger *lgr, LOG_LEVEL lvl, void *data, int size) ;
#ifdef __cpluspluse
}
#endif

#endif
