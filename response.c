#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "response.h"
#include "util.h"
#include "global.h"

#define VERSION(m, n) (((m)-'0')*10 + (n) - '0')
#define MIN_VERSION VERSION('1', '0') 

int initHttpResponseHeader(HttpResponseHeader *httprsphdr)
{
    if(httprsphdr == NULL)
    {
        return -1 ;
    }
    memset(httprsphdr, 0, sizeof(*httprsphdr)) ;
    httprsphdr ->hdrbuff = NULL ;
    httprsphdr ->size = 0 ;
    httprsphdr ->key_val = NULL ;
    return 0 ;
}

int freeHttpResponseHeader(HttpResponseHeader * httphdr)
{
    if(!httphdr)
    {
        return 0 ;
    }
    if(httphdr ->headbuff)
    {
        FREE(httphdr ->headbuff) ;
        httphdr ->headbuff = NULL ;
    }

    if(httphdr ->key_val)
    {
        FREE(httphdr ->key_val) ;
        httphdr ->key_val = NULL ;
    }
    //FREE(httphdr) ;
    //httphdr = NULL ;
    return 0 ;
}

int checkVersion(HttpResponseHeader *httphdr)
{
    if(httphdr == NULL)
    {
        return -1;
    }
    char *p = httphdr ->version;
    char *p1 = strchr(p, '/') ;
    if(p1 == NULL ||strncasecmp("HTTP", p, p1 - p) != 0)
    {
        return -1;
    }
    if(!isdigit(*++p1) || *++p1 != '.'|| !isdigit(*++p1))
    {
        return -1 ;
    }
    if(VERSION(*(p1 - 2), *p1) < MIN_VERSION)
    {
        return -1;
    }
    return 0 ;
}

int checkCode(HttpResponseHeader * httphdr)
{
    if(httphdr == NULL)
    {
        return -1;
    }
    int code = atoi(httphdr ->code) ;
    if(code < 100 || code > 500)
    {
        return -1;
    }
    return 0;
}

int checkReason(HttpResponseHeader *httphdr)
{
    if(httphdr == NULL)
    {
        return -1;
    }
    return 0;
}

int checkStatusLine(HttpResponseHeader *httphdr)
{
    if(httphdr == NULL)
    {
        return -1 ; 
    } 
    return !(!checkVersion(httphdr) && !checkCode(httphdr) && !checkReason(httphdr)) ;
}

#define COPY_FIELD(beg, delimit, dst, sz) ({\
    char *end = strstr(beg, delimit) ;\
    int errflag = 0 ;\
    if(end <= beg || end - beg >= sz)\
    {\
        return -1 ;\
    }\
    strncpy(dst, beg, end - beg) ;\
    end - beg + strlen(delimit) ;\
})

int parseStatusLine(HttpResponseHeader *httphdr ,const char *hdrbuff)
{
    if(hdrbuff == NULL || httphdr == NULL)
    {
        return -1 ;
    }
    const char *p = hdrbuff ;
    p += COPY_FIELD(p, " ", httphdr ->version, sizeof(httphdr ->version))  ;
    p += COPY_FIELD(p, " ", httphdr ->code,    sizeof(httphdr ->code)) ; 
    p += COPY_FIELD(p, "\r\n", httphdr ->reason, sizeof(httphdr ->reason)) ;
    return p - hdrbuff ;
}

static char *deleteContinueLineFlag(char *hdrbuff)
{
    if(hdrbuff == NULL)
    {
        return NULL ;
    }
    char *tmp = hdrbuff ;
    while(*tmp != '\0') 
    {
        char *p = tmp ;
        if(*tmp++ == '\r' && *tmp == '\n')
        {
            tmp++ ;
          //此行是续行
           if(*tmp == ' ' || *tmp == '\t') 
           {
                tmp++ ;
                strcpy(p, tmp) ;
                tmp = p + 1;
           }
           //\r\n\r\n 意味着http头的结束
           else if(*tmp++ == '\r' && *tmp == '\n')
           {
                *++tmp = '\0' ;
                return hdrbuff;
           }
        }
    }
    return NULL;
}

int parseKeyValue(HttpResponseHeader *httphdr)
{ 
    if(httphdr == NULL)
    {
        return -1 ;
    }
    char *p = strstr(httphdr ->headbuff, "\r\n") ;
    if(p == NULL) 
    {
        return -1 ;
    }
    p += 2;
    while(*p != '\0')
    {
        char *key_b = p ;
        char *key_e = strchr(key_b, ':');
        if(key_e != NULL)
        {
            char *val_b = key_e + 1 ; 
            char *val_e = strstr(val_b, "\r\n");
            if(val_e == NULL)
            {
                return -1 ;
            }
            *key_e = '\0' ;
            *val_e = '\0' ;
            p = val_e + 2 ;
            if(httphdr ->count >= httphdr ->size)
            {
               httphdr ->key_val = (NODE *)REALLOC(httphdr ->key_val, sizeof(NODE)*(httphdr ->size + 20));
               if(httphdr ->key_val == NULL)
               {
                    return -1 ;
               }
               httphdr ->size += 20  ;
            }
            httphdr ->key_val[httphdr ->count].key = key_b ;
            httphdr ->key_val[httphdr ->count++].value = val_b ;
        }
        else if(key_e == NULL && strcmp(key_b, "\r\n") == 0)
        {
            return 0 ;
        }
        else
        {
            return -1 ;
        }
    }
    return -1 ;
}

HttpResponseHeader *parseHttpResponseHeader(const char *hdrbuff)
{
    if(hdrbuff == NULL)
    {
        return NULL ;
    }
    HttpResponseHeader *httphdr = (HttpResponseHeader *)MALLOC(sizeof(HttpResponseHeader)) ;
    if(httphdr == NULL)
    {
        return NULL ;
    }
    httphdr ->headbuff = strdup(hdrbuff) ;
    if(httphdr ->headbuff == NULL)
    {
        goto _fails ;
    }
    deleteContinueLineFlag(httphdr ->headbuff) ;
    // 第一次分配100个键值对
    httphdr ->key_val = (NODE *)MALLOC(sizeof(NODE)*30) ;
    if(httphdr ->key_val == NULL)
    {
        goto _fails ;
    }
    httphdr ->size = 30;
    httphdr ->count = 0 ;
    //提取开始行
    int i = parseStatusLine(httphdr, httphdr ->headbuff);
    if(i < 0)
    {
       goto _fails ; 
    }
    if(checkStatusLine(httphdr) != 0) 
    {
        goto _fails ;
    }
    
    if(parseKeyValue(httphdr) != 0)
    {
        goto _fails ;
    }
    return httphdr ;
_fails:
    freeHttpResponseHeader(httphdr) ;
    FREE(httphdr) ;
    httphdr = NULL ;
    return NULL ;
}

HTTPRESPONSE *initHttpResponse(HTTPRESPONSE *httprsp)
{
   if(httprsp == NULL) 
   {
        return -1 ;
   }
   httprsp ->rspfd = -1 ;
   initHttpResponseHeader(&httprsp ->httprsphdr) ;
   return httprsphdr ;
}

int setResponseSocket(HTTPRESPONSE *httprsp, int sockfd)
{
    if(httprsp == NULL || sockfd < 0)
    {
        return -1 ;
    }
    httprsp ->rspfd = sockfd ;
    return 0 ;
}

int readResponse(HTTPRESPONSE *httprsp, void *buff, int sz)
{
    if(httprsp == NULL || buff == NULL || sz <= 0 || httprsp ->rspfd < 0)
    {
        return 0 ;
    }
   
}

int readResponseFully(HTTPRESPONSE *httprsp, void *buff, int sz)
{
    if(httprsp == NULL || buff == NULL || sz <= 0 || httprsp ->rspfd < 0)
    {
        return 0 ;
    }
}

#if 1
void printHttpResponseHeader(HttpResponseHeader *httphdr)
{
    if(!httphdr)
    {
        return ;
    }
    int i = 0;
    printf("version=%s\ncode=%s\nreason=%s\n", httphdr ->version, httphdr ->code, httphdr ->reason);
    for (i = 0 ;i < httphdr ->count; i++)
    {
       printf("%s:%s\n", httphdr ->key_val[i].key, httphdr ->key_val[i].value) ; 
    }
}
int main(int argc, char *argv[])
{
    char buff[] = "HTTP/1.2 200 OK\r\nkey1:value1\r\nkey2:value2\r\nkey3:values,123\r\n\r\n" ;
    HttpResponseHeader *hdr = parseHttpResponseHeader(buff) ;
    printHttpResponseHeader(hdr) ;
    freeHttpResponseHeader(hdr) ;
    return 0 ;
}
#endif
