#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "response.h"
#include "util.h"
#include "global.h"

#define VERSION(m, n) (((m)-'0')*10 + (n) - '0')
#define MIN_VERSION VERSION('1', '0') 

//初始化http 响应头
int initHttpResponseHeader(HttpResponseHeader *httprsphdr)
{
    if(httprsphdr == NULL)
    {
        return -1 ;
    }
    memset(httprsphdr, 0, sizeof(*httprsphdr)) ;
    httprsphdr ->headbuff = NULL ;
    httprsphdr ->size = 0 ;
    httprsphdr ->key_val = NULL ;
    return 0 ;
}

//释放http响应头
int freeHttpResponseHeader(HttpResponseHeader * httphdr)
{
    if(httphdr == NULL)
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
    return 0 ;
}

//检查响应的http版本是否正确
static int checkVersion(HttpResponseHeader *httphdr)
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

//检查http响应状态码是否合法
static int checkCode(HttpResponseHeader * httphdr)
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

//检查http 返回状态信息
static int checkReason(HttpResponseHeader *httphdr)
{
    if(httphdr == NULL)
    {
        return -1;
    }
    return 0;
}

//检查http响应的第一行
static int checkStatusLine(HttpResponseHeader *httphdr)
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

//解析http响应的第一行  比如 HTTP/1.1 200 OK
static int parseStatusLine(HttpResponseHeader *httphdr ,const char *hdrbuff)
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

//删除http响应行中续行，并把所有的续行合并成一行
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

//解析http响应头中的名称/值对
static int parseKeyValue(HttpResponseHeader *httphdr)
{ 
    if(httphdr == NULL)
    {
        return -1 ;
    }
    //跳过首行
    char *p = strstr(httphdr ->headbuff, "\r\n") ;
    if(p == NULL) 
    {
        return -1 ;
    }
    p += 2 ;
    while(*p != '\0')
    {
        char *key_b = p ;
        //名称与键之间 以 : 隔开
        char *key_e = strchr(key_b, ':');
        if(key_e != NULL)
        {
            char *val_b = key_e + 1 ; 
            //此行的结束
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
        //http 解析完毕  \r\n标志着结束
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

//解析httprsp中hdr中的http响应头
int parseHttpResponseHeader(HTTPRESPONSE *httprsp)
{
    if(httprsp == NULL)
    {
        return -1;
    }
    HttpResponseHeader *httphdr = &httprsp ->httprsphdr ; 
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
        printf("fail to parse key and value\n") ;
        goto _fails ;
    }
    return 0 ;

_fails:
    freeHttpResponseHeader(httphdr) ;
    return -1;
}

//初始化一个http响应对象
HTTPRESPONSE *initHttpResponse(HTTPRESPONSE *httprsp)
{
   if(httprsp == NULL) 
   {
        return NULL ;
   }
   httprsp ->rspfd = -1 ;
   httprsp ->extraData = NULL ;
   httprsp ->extraSize = 0 ;
   initHttpResponseHeader(&httprsp ->httprsphdr) ;
   return httprsp ;
}

int setResponseExtraData(HTTPRESPONSE *httprsp, char *extraData, int sz)
{ 
    if(httprsp == NULL || extraData == NULL)
    {
        return -1 ;
    }
    httprsp ->extraData = extraData ;
    httprsp ->extraSize = sz ;
    return 0 ;
}

int freeHttpResponse(HTTPRESPONSE *httprsp)
{
    if(httprsp == NULL)    
    {
        return -1 ;
    }
    if(httprsp ->rspfd >= 0 )
    {
        close(httprsp ->rspfd) ;
    }
    httprsp ->rspfd = -1 ;
    httprsp ->extraData = NULL ;
    httprsp ->extraSize = 0 ;
    return freeHttpResponseHeader(&httprsp ->httprsphdr) ;
}

//设置http响应对象中头缓冲
//当isCopy 为False时 hdrbuff必须是malloc分配最终由http释放
//isCopy == True 从hdrbuff中拷贝一份
int setResponseHeaderBuff(HTTPRESPONSE *httprsp, char *hdrbuff, int isCopy)
{
   if(httprsp == NULL || hdrbuff == NULL) 
   {
        return -1 ;
   }
   if(isCopy == 0)
   {
        httprsp ->httprsphdr.headbuff = hdrbuff ;
   }   
   else
   {
        httprsp ->httprsphdr.headbuff = strdup(hdrbuff) ;
   }
   return httprsp ->httprsphdr.headbuff == NULL ? -1 : 0 ;
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

#if 0 
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
    char buff[] = "HTTP/1.2 200 OK\r\nkey1: value1\r\nkey2:value2\r\nkey3:values,123\r\n\r\n" ;
    HttpResponseHeader hdr ;
    initHttpResponseHeader(&hdr) ;
    if(parseHttpResponseHeader(&hdr) == 0)
    {
        printHttpResponseHeader(&hdr) ;
        freeHttpResponseHeader(&hdr) ;
    }
    return 0 ;
}
#endif
