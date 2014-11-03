#include<string.h> 
#include<stdio.h>
#include<stdlib.h>
#include <ctype.h>

#include "global.h"
#include "http_url.h" 
#include "urlencode.h"

#define NEXT_PROTOCOL  0X00000001
#define NEXT_HOST_PORT 0X00000002
#define NEXT_PATH      0X00000003
#define NEXT_PARAM     0X00000004
#define NEXT_QUERY     0X00000005
#define NEXT_FRAGMENT  0X00000006 
#define NEXT_END       0X00000007

static int parseProtocol(URL *pUrl, char *pUrlStr)
{
    if(pUrl == NULL|| pUrlStr == NULL)
    {
        return -1 ;
    }
    char *protocol_b = pUrlStr ;
    char *protocol_e = strstr(pUrlStr, "://") ;
    if(protocol_e == NULL)
    {
        pUrl ->protocol = NULL ;
        pUrl ->next = NEXT_HOST_PORT;
        return 0 ;
    }
    else
    {
       pUrl ->protocol = protocol_b;
       *protocol_e = '\0' ;
       pUrl ->next = NEXT_HOST_PORT ;
       return protocol_e - protocol_b + 3 ;
    }
}

static int parseHostAndPort(URL *pUrl, char *pUrlStr)
{
    if(pUrl == NULL || pUrlStr == NULL)
    {
        return -1 ;
    }
    char *host_b = pUrlStr ;
    char *host_e = strchr(host_b, ':') ;
    //端口是默认的
    if(host_e == NULL)
    {
        pUrl ->port = NULL ;
        host_e = strchr(host_b, '/') ;
        //后面没有路径
        if(host_e == NULL)
        {
            pUrl ->next = NEXT_END ;
            pUrl ->host = host_b ;
            return strlen(host_b) ;
        }
        //后面跟的还有路径这个参数
        else
        {
            *host_e = '\0' ;
            pUrl ->host = host_b ;
            pUrl ->next = NEXT_PATH ;
            return host_e - host_b + 1 ;
        }
    }
    //指定了端口
    else
    {
        pUrl ->host = host_b ;
        *host_e = '\0' ;
        char *port_b = host_e + 1;
        char *port_e = strchr(port_b, '/');
        //端口后面没有路径了
        if(port_e == NULL)
        {
           pUrl ->port = port_b ; 
           pUrl ->next = NEXT_END ;
           return strlen(host_b) ;
        }
        //端口后面还有路径
        else
        {
           *port_e = '\0' ;
           pUrl ->port = port_b ;
           pUrl ->next = NEXT_PATH ;
           return port_e - host_b + 1;
        }
    }
}

static int parsePath(URL *pUrl, char *pUrlStr)
{
    if(pUrl == NULL || pUrlStr == NULL)
    {
        return -1 ;
    }

    char *path_b = pUrlStr ;
    char *path_e = strpbrk(path_b, ";?#") ;
    if(path_e != NULL)
    {
        pUrl ->path = path_b ;
        pUrl ->next = (*path_e == ';'? NEXT_PARAM :(*path_e == '?' ? NEXT_QUERY: NEXT_FRAGMENT) );
        *path_e = '\0' ;
        return path_e - path_b + 1 ;
    }
    else
    {
        pUrl ->next = NEXT_END ;
        pUrl ->path = path_b ;
        return strlen(path_b) ;
    }
}

static int parseParams(URL *pUrl, char *pUrlStr)
{
    if(pUrl == NULL || pUrlStr == NULL) 
    {
        return -1 ;
    }

    char *param_b = pUrlStr ;
    char *param_e = strpbrk(param_b, "?#");
    //路径后面的特殊参数后面还有字符串
    if(param_e != NULL)
    {
        pUrl ->param = param_b ;
        pUrl ->next = (*param_e == '?' ? NEXT_QUERY:NEXT_FRAGMENT) ;
        *param_e = '\0' ; 
        return param_e - param_b + 1 ;
    }
    //路径后面没有字符串了
    else
    {
        pUrl ->param = param_b ;
        pUrl ->next = NEXT_END ;
        return strlen(param_b) ;
    }
}

static int parseQueryString(URL *pUrl, char *pUrlStr)
{
    if(pUrl == NULL || pUrlStr == NULL)
    {
        return -1 ;
    }
    char *query_b = pUrlStr ;
    char *query_e = strchr(query_b, '#') ;
    if(query_e == NULL)
    {
        pUrl ->query = query_b ;
        pUrl ->next = NEXT_END ;
        return strlen(query_b) ;
    }
    else
    {
        *query_e = '\0' ;
        pUrl ->query = query_b ;
        pUrl ->next = NEXT_FRAGMENT ;
        return query_e - query_b + 1;
    }
}

static int parseFragment(URL *pUrl, char *pUrlStr)
{
    if(pUrl == NULL || pUrlStr == NULL)
    {
        return -1 ;
    }
    pUrl ->next = NEXT_END ;
    pUrl ->frag = pUrlStr ;
    return strlen(pUrlStr) ;
}

int freeURL(URL *pUrl)
{
    if(pUrl == NULL)
    {
        return 0;
    }
    if(pUrl ->urlbuff)
    {
        FREE(pUrl ->urlbuff) ;
    }
    pUrl ->urlbuff = NULL ;
    pUrl = NULL ;
    return 0 ;
}

static int checkURL(const URL *pUrl)
{
    if(pUrl == NULL)
    {
        return -1 ;
    }
    if(pUrl ->protocol != NULL)
    {
       if(strcasecmp(pUrl ->protocol, "http") != 0 && strcasecmp(pUrl ->protocol, "https") != 0) 
       {
            return -1 ;
       }
    }
    if(pUrl ->host == NULL)
    {
        return -1 ;
    }

    if(pUrl ->port != NULL)
    {
       int port = atoi(pUrl ->port) ;
       if(port < 0 || port > 65535)
       {
            return -1 ;
       }
       const char *p = pUrl ->port ;
       while(*p != '\0')
       {
            if(!isdigit(*p++))
            {
                return -1 ;
            }
       }
    }
    
    if(pUrl ->path != NULL && strlen(pUrl ->path) == 0)
    {
        return -1 ;
    }
    
    if(pUrl ->query != NULL && strlen(pUrl ->query) == 0)
    {
        return -1 ;
    }

    if(pUrl ->frag != NULL && strlen(pUrl ->frag) == 0)
    {
        return -1 ;
    }

    return 0 ;
}
//protocol://hostname[:port]/path/[;parameters][?query]#fragment
URL* parseURL(const char *pUrlStr)
{
    if(pUrlStr == NULL)
    {
        return NULL ;
    }
    URL *pUrl = (URL *)MALLOC(sizeof(URL)) ;
    if(pUrl == NULL)
    {
        return NULL ;
    }
    memset(pUrl, 0, sizeof(URL)) ;
    pUrl ->urlbuff = (char *)unquotestr(pUrlStr) ;
    if(pUrl ->urlbuff == NULL)
    {
        goto _fails;
    }
    pUrl ->next = NEXT_PROTOCOL ;
    int offset = 0 ;
    while(pUrl ->next != NEXT_END)
    {
        switch(pUrl ->next)
        {
            case NEXT_PROTOCOL:
                offset += parseProtocol(pUrl, pUrl ->urlbuff+offset) ;
                break ;
            case NEXT_HOST_PORT: 
                offset += parseHostAndPort(pUrl, pUrl ->urlbuff + offset) ;
                break ;
            case NEXT_PATH: 
                offset += parsePath(pUrl, pUrl ->urlbuff + offset) ;
                break ;
            case NEXT_PARAM:
                offset += parseParams(pUrl, pUrl ->urlbuff + offset) ;
                break ;
            case NEXT_QUERY:
                offset += parseQueryString(pUrl, pUrl ->urlbuff + offset);
                break ;
            case NEXT_FRAGMENT:
                offset += parseFragment(pUrl, pUrl ->urlbuff + offset) ;
                break ;
        }
    }
    if(checkURL(pUrl) == 0)
    {
        return pUrl ;
    }
_fails:
    freeURL(pUrl) ;   
    return NULL ;
}

int printURL(const URL *pUrl)
{
    if(pUrl == NULL) 
    {
         return -1 ;
    }
    printf("protocol:%s\nhost:%s\nport:%s\npath:%s\nparams:%s\nquery:%s\nfragment:%s\n", 
            pUrl ->protocol, pUrl ->host,pUrl ->port, pUrl ->path, pUrl ->param, pUrl ->query, pUrl ->frag) ;
}

#if 1

int main(int argc, char *argv[])
{
    URL *pUrl = parseURL("http://172.100.101.145:8088/index.html/;name=zs?key=value#this is in the fragment") ;
    printURL(pUrl) ;
    freeURL(pUrl);
    return 0 ;
}

#endif
