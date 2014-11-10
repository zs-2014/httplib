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

static int setNonblock(int fd)
{
    int ret = fcntl(fd, F_GETFL, 0) ;   
    if(ret == -1)
    {
        return -1 ;
    }
    ret |= O_NONBLOCK ;
    return fcntl(fd, F_SETFL, ret) ; 
}

static int cancelNonblock(fd)
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
    if(setNonblock(clifd) < 0) 
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
    if(cancelNonblock(clifd) < 0) 
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

int connectToServer(const char *server, const char *port, int timeout)
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

char *itoa(int num, char *buff)
{
    if(buff == NULL) 
    {
        return NULL ;
    }
    char *tmpBuff = buff ;
    char *tmpBuff1 = buff ;
    if(num < 0)
    {
        *tmpBuff++ = '-' ;
        num = 0 - num ;
    }
    do
    {
       *tmpBuff++ = num%10 + '0'; 
       num = num/10 ;
    }while(num != 0) ;
    *tmpBuff = '\0' ;
    while(tmpBuff1 < --tmpBuff)
    {
        char c = *tmpBuff ;
        *tmpBuff = *tmpBuff1 ;
        *tmpBuff1 = c ;
        tmpBuff1++ ;
    }
    return buff ;
}

int sendData(int fd, const void *buff, int sz)
{
    if(fd < 0 || buff == NULL|| sz == 0)
    {
        return 0 ;
    }
    int nSend = 0 ;
    int ret = 0 ;
    const char *tmp = (const char *)buff ;
    while((ret = write(fd, tmp + nSend, sz - nSend)) != 0)
    { 
        if(ret + nSend == sz)
        {
            return sz ;
        }
        else if(ret < 0)
        {
            if(errno == EINTR)
            {
                continue ;
            }
           return nSend ; 
        }
        else
        {
            nSend += ret ;
        }
    }
    return nSend ;
}

int readFully(int fd, void *buff, int sz)
{
    if(fd < 0 || buff == NULL || sz == 0) 
    {
        return 0 ;
    }
    int nRecv = 0 ;
    int ret = 0 ;
    while((ret = read(fd, ((char *)buff) + nRecv, sz - nRecv)) != 0)
    {
        if(ret + nRecv == sz) 
        {
            return sz ;
        }
        else if(ret > 0 )
        {
           nRecv += ret ; 
        }
        else
        {
            if(errno == EINTR)
            {
                continue ;
            }
            return nRecv ;
        }
    }
    return nRecv ;
}

int readUntil(int fd, void *buff, int sz, int *len, const char *flagstr)
{
    if(fd < 0 || buff == NULL || sz <= 0 || len == NULL) 
    {
        return 0 ;
    }
    char *tmpBuff = (char *)buff ;
    int nRecv = 0 ;
    int ret = 0 ;
    while((ret = read(fd, tmpBuff + nRecv, sz - nRecv - 1)) != 0)
    {
        if(ret + nRecv == sz - 1) 
        {
            tmpBuff[sz - 1] = '\0' ;  
            char *p = strstr(tmpBuff + nRecv, flagstr) ;
            if(p != NULL)
            {
                *len = p - tmpBuff + strlen(flagstr);
            }
            else
            {
                *len = 0 ;
            }
            return sz - 1 ; 
        }
        else if(ret > 0 )
        {
            tmpBuff[nRecv + ret] = '\0' ;
            char *p = strstr(tmpBuff + nRecv, flagstr) ;
            nRecv += ret ; 
            if(p != NULL)
            {
                *len = p - tmpBuff + strlen(flagstr); 
                return nRecv ;
            }
        }
        else
        {
            if(errno == EINTR)
            {
                continue ;
            }
            return nRecv ;
        }
    }
    *len = 0 ;
    return nRecv ;
}

#if 1
int main(int argc,char *argv[])
{
    int fd = connectToServer(argv[1], argv[2], atoi(argv[3])) ;
    printf("%d\n", fd)  ;
    char buff[] = {"GET / HTTP/1.1\r\nContent-Type:plain/text\r\nUser-Agent:curlib2.7\r\n\r\n"} ; 
    printf("write = %d\n", write(fd,buff, strlen(buff))) ;
    char readbuff[64*1024] = {0} ;
    int len = 0 ;
    int total = readUntil(fd, readbuff, sizeof(readbuff)-1, &len, "\r\n\r\n") ;
    printf("len = [%d]\ntotal=%d\n", len, total) ;
    printf("%s\n", readbuff) ;
    close(fd) ;
    perror("error msg") ;
	return 0 ;
}
#endif
