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

static int parse_protocol(http_url_t *p_url, char *p_url_str)
{
    if(p_url == NULL|| p_url_str == NULL)
    {
        return -1 ;
    }
    char *protocol_b = p_url_str ;
    char *protocol_e = strstr(p_url_str, "://") ;
    if(protocol_e == NULL)
    {
        p_url ->protocol = NULL ;
        p_url ->next = NEXT_HOST_PORT;
        return 0 ;
    }
    else
    {
       p_url ->protocol = protocol_b;
       *protocol_e = '\0' ;
       p_url ->next = NEXT_HOST_PORT ;
       return protocol_e - protocol_b + 3 ;
    }
}

static int parse_host_and_port(http_url_t *p_url, char *p_url_str)
{
    if(p_url == NULL || p_url_str == NULL)
    {
        return -1 ;
    }
    char *host_b = p_url_str ;
    char *host_e = strchr(host_b, ':') ;
    //端口是默认的
    if(host_e == NULL)
    {
        p_url ->port = NULL ;
        host_e = strchr(host_b, '/') ;
        //后面没有路径
        if(host_e == NULL)
        {
            p_url ->next = NEXT_END ;
            p_url ->host = host_b ;
            return strlen(host_b) ;
        }
        //后面跟的还有路径这个参数
        else
        {
            *host_e = '\0' ;
            p_url ->host = host_b ;
            p_url ->next = NEXT_PATH ;
            return host_e - host_b + 1 ;
        }
    }
    //指定了端口
    else
    {
        p_url ->host = host_b ;
        *host_e = '\0' ;
        char *port_b = host_e + 1;
        char *port_e = strchr(port_b, '/');
        //端口后面没有路径了
        if(port_e == NULL)
        {
           p_url ->port = port_b ; 
           p_url ->next = NEXT_END ;
           return strlen(host_b) ;
        }
        //端口后面还有路径
        else
        {
           *port_e = '\0' ;
           p_url ->port = port_b ;
           p_url ->next = NEXT_PATH ;
           return port_e - host_b + 1;
        }
    }
}

static int parse_path(http_url_t *p_url, char *p_url_str)
{
    if(p_url == NULL || p_url_str == NULL)
    {
        return -1 ;
    }

    char *path_b = p_url_str ;
    char *path_e = strpbrk(path_b, ";?#") ;
    if(path_e != NULL)
    {
        p_url ->path = path_b ;
        p_url ->next = (*path_e == ';'? NEXT_PARAM :(*path_e == '?' ? NEXT_QUERY: NEXT_FRAGMENT) );
        *path_e = '\0' ;
        return path_e - path_b + 1 ;
    }
    else
    {
        p_url ->next = NEXT_END ;
        p_url ->path = path_b ;
        return strlen(path_b) ;
    }
}

static int parse_params(http_url_t *p_url, char *p_url_str)
{
    if(p_url == NULL || p_url_str == NULL) 
    {
        return -1 ;
    }

    char *param_b = p_url_str ;
    char *param_e = strpbrk(param_b, "?#");
    //路径后面的特殊参数后面还有字符串
    if(param_e != NULL)
    {
        p_url ->param = param_b ;
        p_url ->next = (*param_e == '?' ? NEXT_QUERY:NEXT_FRAGMENT) ;
        *param_e = '\0' ; 
        return param_e - param_b + 1 ;
    }
    //路径后面没有字符串了
    else
    {
        p_url ->param = param_b ;
        p_url ->next = NEXT_END ;
        return strlen(param_b) ;
    }
}

static int parse_query_string(http_url_t *p_url, char *p_url_str)
{
    if(p_url == NULL || p_url_str == NULL)
    {
        return -1 ;
    }
    char *query_b = p_url_str ;
    char *query_e = strchr(query_b, '#') ;
    if(query_e == NULL)
    {
        p_url ->query = query_b ;
        p_url ->next = NEXT_END ;
        return strlen(query_b) ;
    }
    else
    {
        *query_e = '\0' ;
        p_url ->query = query_b ;
        p_url ->next = NEXT_FRAGMENT ;
        return query_e - query_b + 1;
    }
}

static int parse_fragment(http_url_t *p_url, char *p_url_str)
{
    if(p_url == NULL || p_url_str == NULL)
    {
        return -1 ;
    }
    p_url ->next = NEXT_END ;
    p_url ->frag = p_url_str ;
    return strlen(p_url_str) ;
}

int free_url(http_url_t *p_url)
{
    if(p_url == NULL)
    {
        return 0;
    }
    if(p_url ->urlbuff)
    {
        FREE(p_url ->urlbuff) ;
    }
    p_url ->urlbuff = NULL ;
    p_url = NULL ;
    return 0 ;
}

static int check_url(const http_url_t *p_url)
{
    if(p_url == NULL)
    {
        return -1 ;
    }
    if(p_url ->protocol != NULL)
    {
       if(strcasecmp(p_url ->protocol, "http") != 0 && strcasecmp(p_url ->protocol, "https") != 0) 
       {
            return -1 ;
       }
    }
    if(p_url ->host == NULL)
    {
        return -1 ;
    }

    if(p_url ->port != NULL)
    {
       int port = atoi(p_url ->port) ;
       if(port < 0 || port > 65535)
       {
            return -1 ;
       }
       const char *p = p_url ->port ;
       while(*p != '\0')
       {
            if(!isdigit(*p++))
            {
                return -1 ;
            }
       }
    }
    
    //if(p_url ->path != NULL && strlen(p_url ->path) == 0)
    //{
     //   return -1 ;
    //}
    
    if(p_url ->query != NULL && strlen(p_url ->query) == 0)
    {
        return -1 ;
    }

    if(p_url ->frag != NULL && strlen(p_url ->frag) == 0)
    {
        return -1 ;
    }

    return 0 ;
}
//protocol://hostname[:port]/path/[;parameters][?query]#fragment
http_url_t* parse_url(const char *p_url_str)
{
    if(p_url_str == NULL)
    {
        return NULL ;
    }
    http_url_t *p_url = (http_url_t *)MALLOC(sizeof(http_url_t)) ;
    if(p_url == NULL)
    {
        return NULL ;
    }
    memset(p_url, 0, sizeof(http_url_t)) ;
    char *tmp = (char *)unquotestr(p_url_str) ;
    p_url ->urlbuff = quotestr(tmp, ":/?#=") ;   
    FREE(tmp) ;
    if(p_url ->urlbuff == NULL)
    {
        goto _fails;
    }
    p_url ->next = NEXT_PROTOCOL ;
    int offset = 0 ;
    while(p_url ->next != NEXT_END)
    {
        switch(p_url ->next)
        {
            case NEXT_PROTOCOL:
                offset += parse_protocol(p_url, p_url ->urlbuff+offset) ;
                break ;
            case NEXT_HOST_PORT: 
                offset += parse_host_and_port(p_url, p_url ->urlbuff + offset) ;
                break ;
            case NEXT_PATH: 
                offset += parse_path(p_url, p_url ->urlbuff + offset) ;
                break ;
            case NEXT_PARAM:
                offset += parse_params(p_url, p_url ->urlbuff + offset) ;
                break ;
            case NEXT_QUERY:
                offset += parse_query_string(p_url, p_url ->urlbuff + offset);
                break ;
            case NEXT_FRAGMENT:
                offset += parse_fragment(p_url, p_url ->urlbuff + offset) ;
                break ;
        }
    }
    if(check_url(p_url) == 0)
    {
        return p_url ;
    }
_fails:
    free_url(p_url) ;   
    return NULL ;
}

