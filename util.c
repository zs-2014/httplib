#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include "util.h"

static int set_nonblock(int fd)
{
    int ret = fcntl(fd, F_GETFL, 0) ;   
    if(ret == -1)
    {
        return -1 ;
    }
    ret |= O_NONBLOCK ;
    return fcntl(fd, F_SETFL, ret) ; 
}

static int cancel_nonblock(fd)
{
    int ret = fcntl(fd, F_GETFL, 0) ;   
    if(ret == -1)
    {
        printf("fail to get f_getfl\n") ;
        return -1 ;
    }
    ret &= ~O_NONBLOCK ;
    return fcntl(fd, F_SETFL, ret) ; 

}
static int connect_with_timeout(int clifd, struct sockaddr *addr, socklen_t addrlen, int timeout)
{ 
    if(clifd < 0 || addr == NULL || addrlen == 0)
    {
        return -1 ;
    }

    if(timeout <= 0)
    {
        return connect(clifd, addr, addrlen) ;
    }
    if(set_nonblock(clifd) < 0) 
    {
        return -1 ;
    }
    int ret = connect(clifd, addr, addrlen) ;
    int err = errno ;
    if(ret == 0) //有可能连接立即就建立起来了
    {
        return 0 ;
    }
    if(err != EINPROGRESS) //非阻塞状态的socket connect 会立即返回这个错误
    {
        return -1 ;
    }
    if(cancel_nonblock(clifd) < 0) 
    {
        return -1 ;
    }
    struct timeval tm ;
    tm.tv_sec = timeout/1000 ;
    tm.tv_usec = (timeout - tm.tv_sec * 1000)*1000 ;
    fd_set wset ;
    FD_ZERO(&wset) ;
    FD_SET(clifd, &wset) ;
    ret = select(clifd+1, NULL, &wset, NULL, &tm) ;
    if(ret > 0)
    {
        int error = -1 ;
        int len = sizeof(error) ;
        getsockopt(clifd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
        if(error == 0)
        {
            printf("connect success\n") ;
            return 0 ;
        }
        else
        {
            printf("connect fail \n") ;
            return -1 ;
        }
    }
    else
    {
        printf("timeout \n") ;
    }
    return -1 ;
}

int create_server_socket(const char *ip, const char *port, int lsncnt)
{ 
    if(ip == NULL || port == NULL)
    {
        return -1 ;
    }

    struct addrinfo *result = NULL ;
    struct addrinfo hints ;
    hints.ai_family = AF_UNSPEC ;
    hints.ai_protocol = 0 ;
    hints.ai_socktype = SOCK_STREAM ;
    hints.ai_addr = NULL ;
    hints.ai_addrlen = 0 ;
    hints.ai_next = NULL ;
    hints.ai_flags = 0 ;
    int ret = getaddrinfo(ip, port, &hints, &result) ;
    struct addrinfo *tmp = result ;
    int fd = -1 ;
    for(tmp = result; tmp != NULL ;tmp = tmp ->ai_next)
    {  
        fd = socket(tmp ->ai_family, tmp ->ai_socktype, tmp ->ai_protocol) ;
        if(fd == -1)
            continue ;
        if(bind(fd, tmp ->ai_addr, tmp ->ai_addrlen) == 0 && listen(fd, lsncnt > 0 ? lsncnt:5) == 0) 
        {
            break ;
        }
        close(fd) ;
        fd = -1 ;
    }
    freeaddrinfo(result) ;
    return fd ;
}

int connect_to_server(const char *server, const char *port, int timeout)
{
    if(server == NULL || port == NULL)  
    {
         return -1 ;
    }
    struct addrinfo *result = NULL ;
    struct addrinfo hints ;
    hints.ai_family = AF_UNSPEC ;
    hints.ai_protocol = 0 ;
    hints.ai_socktype = SOCK_STREAM ;
    hints.ai_addr = NULL ;
    hints.ai_addrlen = 0 ;
    hints.ai_next = NULL ;
    hints.ai_flags = 0 ;
    int ret = getaddrinfo(server, port, &hints, &result) ;
    struct addrinfo *tmp = result ;
    int cfd = -1 ;
    for(tmp = result; tmp != NULL ;tmp = tmp ->ai_next)
    {  
        char ip[256] = {0} ;
        inet_ntop(tmp ->ai_family, &(((struct sockaddr_in *)tmp ->ai_addr) ->sin_addr), ip, sizeof(ip) -1) ;
        printf("ip = %s port = %d\n", ip, ntohs(((struct sockaddr_in *)tmp ->ai_addr) ->sin_port)) ;
        cfd = socket(tmp ->ai_family, tmp ->ai_socktype, tmp ->ai_protocol) ;
        if(cfd == -1)
            continue ;
        if(connect_with_timeout(cfd, tmp ->ai_addr, tmp ->ai_addrlen, timeout) == 0)
            break ;
        close(cfd) ;
        cfd = -1 ;
    }
    freeaddrinfo(result) ;
    return cfd ;
}

const char *casefind(const char *src, const char *needle)
{
    if(src == NULL || needle == NULL) 
    {
        return NULL ;
    }
    while(*src != '\0') 
    {
        if(strcasecmp(src, needle) == 0) 
        {
            return src ;
        }
        src++ ;
    }
    return NULL ;
}

char *itoa(int64_t num, char *buff)
{
    if(buff == NULL) 
    {
        return NULL ;
    }
    char *tmp_buff = buff ;
    char *tmp_buff1 = buff ;
    if(num < 0)
    {
        *tmp_buff++ = '-' ;
        num = 0 - num ;
    }
    do
    {
       *tmp_buff++ = num%10 + '0'; 
       num = num/10 ;
    }while(num != 0) ;
    *tmp_buff = '\0' ;
    while(tmp_buff1 < --tmp_buff)
    {
        char c = *tmp_buff ;
        *tmp_buff = *tmp_buff1 ;
        *tmp_buff1 = c ;
        tmp_buff1++ ;
    }
    return buff ;
}

int skip_char(const char *str, int ch)
{
   if(str == NULL) 
   {
        return 0 ;
   }
   const char *tmp = str ;
   while(*tmp != '\0' && *tmp == ch)
   {
        tmp++ ; 
   }
   return tmp - str ;
}

int send_data(int fd, const void *buff, int sz)
{
    if(fd < 0 || buff == NULL|| sz == 0)
    {
        return 0 ;
    }
    int n_send = 0 ;
    int ret = 0 ;
    const char *tmp = (const char *)buff ;
    while((ret = write(fd, tmp + n_send, sz - n_send)) != 0)
    { 
        if(ret + n_send == sz)
        {
            return sz ;
        }
        else if(ret < 0)
        {
            if(errno == EINTR)
            {
                continue ;
            }
           return n_send ; 
        }
        else
        {
            n_send += ret ;
        }
    }
    return n_send ;
}

int write_all(int fd, void *buff, int sz)
{
    if(fd < 0 || buff == NULL || sz == 0) 
    {
        return 0 ; 
    }
    int n_send = 0 ;
    do
    {
        int ret = write(fd, ((char *)buff) + n_send, sz - n_send) ;
        if(ret + n_send == sz)  
        {
            return sz ;
        }
        else if(ret > 0)
        {
            n_send += ret;
        }
        else
        {
            if(errno == EINTR)
            {
                continue ;
            }
            return -1 ;
        }
    }while(1) ;
}

int read_fully(int fd, void *buff, int sz)
{
    if(fd < 0 || buff == NULL || sz == 0) 
    {
        return 0 ;
    }
    int n_recv = 0 ;
    int ret = 0 ;
    while((ret = read(fd, ((char *)buff) + n_recv, sz - n_recv)) != 0)
    {
        if(ret + n_recv == sz) 
        {
            return sz ;
        }
        else if(ret > 0 )
        {
           n_recv += ret ; 
        }
        else
        {
            if(errno == EINTR)
            {
                continue ;
            }
            return -1;
        }
    }
    return n_recv ;
}

char *read_until(int fd, int *total_len, int *len, const char *flagstr)
{
    if(fd < 0 || total_len == NULL || len == NULL) 
    {
        return NULL ;
    }
//这个size不能超过8K
#define BUFF_GROW_SIZE 512 

    char *buff = (char *)CALLOC(1, BUFF_GROW_SIZE + 1) ; 
    int sz = BUFF_GROW_SIZE;
    if(buff == NULL)
    {
        return NULL ;
    }
    int n_recv = 0 ;
    int ret = 0 ;
    while((ret = read(fd, buff + n_recv, sz - n_recv)) != 0) 
    {
       if(ret > 0) 
       {
            buff[ret + n_recv] = '\0' ;
            char *p = strstr(buff + n_recv, flagstr) ;
            n_recv += ret ;
            if(p != NULL)
            {
                *len = p - buff + strlen(flagstr) ;
                *total_len = n_recv ;
                return buff ;
            }
            else if(p == NULL && n_recv == sz) 
            {
                buff = (char *)REALLOC(buff, sz + BUFF_GROW_SIZE + 1) ;
                if(buff == NULL)
                {
                    FREE(buff) ;
                    return NULL ;
                }
                sz = sz + BUFF_GROW_SIZE;
                continue ;
            }
       }
       else
       {
            if(errno == EINTR) 
            {
                continue ;
            }
            break ;
       }
    }
    FREE(buff) ;
    *len = 0 ;
    *total_len = 0 ;
    return NULL ;
}
