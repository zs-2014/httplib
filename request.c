#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h> 

#include "response.h"
#include "request.h"
#include "cookie.h"
#include "http_url.h"
#include "util.h"
#include "httpheader.h"
#include "buffer.h"
#include "global.h"

#define MAX_BOUNDRY_BYTE 20

#define DEFAULT_PORT "80"

#define STR_POST    "POST"
#define STR_POST_LEN strlen(STR_POST)

#define STR_GET     "GET"
#define STR_GET_LEN strlen(STR_GET)
#define VERSION(m, n) (((m)-'0')*10 + (n) - '0')
#define MIN_VERSION VERSION('1', '0') 

int initHttpRequest(HTTPREQUEST *httpreq)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }

    httpreq ->method = GET ;
    httpreq ->url = NULL ;
    httpreq ->version[0] = '1' ;
    httpreq ->version[1] = '.' ;
    httpreq ->version[2] = '1' ;
    memset(httpreq ->filename, 0, sizeof(httpreq ->filename)) ;
    httpreq ->currFileCount = 0 ;
    memset(httpreq ->boundary, 0, sizeof(httpreq ->boundary)) ;
    return initData(&httpreq ->data)||initHttpHeader(&httpreq ->header) || initHttpHeader(&httpreq ->header) ;
}

int freeHttpRequest(HTTPREQUEST *httpreq)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    freeURL(httpreq ->url) ; 
    freeHttpHeader(&httpreq ->header) ;
    int i = 0 ;
    for(i = 0 ; i < httpreq ->currFileCount ;i++)
    {
        FREE(httpreq ->filename[i]) ;
        httpreq ->filename[i] = NULL ;
    }
    httpreq ->currFileCount = 0 ;
    return 0 ;
}

int setRequestMethod(HTTPREQUEST *httpreq, int method)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    httpreq ->method = (method == GET ?GET :POST) ;
}

int setHttpRequestUrl(HTTPREQUEST *httpreq, const char *urlstr)
{
    if(httpreq == NULL || urlstr == NULL)
    {
        return -1 ;
    }
    httpreq ->url = parseURL(urlstr) ;
    if(httpreq ->url == NULL)
    {
        return -1 ;
    }
    return 0 ; 
}

int setCookie(HTTPREQUEST *httpreq, COOKIE *cookie)
{
    return addRequestHeader(httpreq, "Cookie", cookie2String(cookie)) ;
}

int setContentType(HTTPREQUEST *httpreq, const char *contentType)
{
    return addRequestHeader(httpreq, "Content-Type", contentType);
}
int setUserAgent(HTTPREQUEST *httpreq, const char *userAgent)
{
    return addRequestHeader(httpreq, "User-Agent", userAgent) ;
}

int setHttpVersion(HTTPREQUEST *httpreq, const char *version)
{
    if(httpreq == NULL || version == NULL)    
    {
        return -1 ;
    }
    int len = strlen(version) ;
    if(len != 3 || VERSION(version[0], version[2]) < MIN_VERSION)
    {
        printf("[%s]:invalid http version\n", version) ;
    }
    httpreq ->version[0] = version[0] ;
    httpreq ->version[1] = version[1] ;
    httpreq ->version[2] = version[2] ;
    return 0 ;
}

int addRequestHeader(HTTPREQUEST *httpreq, const char *key, const char *value)
{
    if(httpreq == NULL) 
    {
        return -1 ;
    }
    return addHeader(&httpreq ->header, key, value); 
}

int addRequestData(HTTPREQUEST *httpreq, const uchar *key, int keySz, const uchar *val, int valSz)
{
    if(httpreq == NULL || key == NULL || val == NULL)
    {
        return -1 ;
    }
    return addData(&httpreq ->data, key, keySz, val, valSz) ;
}

int addPostFile(HTTPREQUEST *httpreq, const char *filename)
{
   if(httpreq == NULL || filename == NULL) 
   {
        return -1 ;
   }
   if(access(filename, R_OK) != 0)
   {
        printf("the [%s] is not exists or not allowed to read\n", filename) ;
        return -1 ;
   }
   if(httpreq ->currFileCount >= MAX_FILE_COUNT)
   {
        printf("has beyond the max file count\n") ;
        return -1 ;
   }
   if((httpreq ->filename[httpreq ->currFileCount] = strdup(filename)) == NULL)
   {
        printf("fail to copy the filename:[%s]\n", filename) ;
        return -1 ;
   }
   httpreq ->currFileCount += 1 ;
   return 0 ;
}

static int addDefaultRequestHeader(HTTPREQUEST *httpreq)
{
    if(!hasHeader(&httpreq ->header, "Content-Type"))
    {
        addRequestHeader(httpreq, "Content-type", "plain/text") ;
    }
    if(!hasHeader(&httpreq ->header, "Host"))
    {
        addRequestHeader(httpreq, "Host", httpreq ->url ->host) ;
    }
    if(!hasHeader(&httpreq ->header, "User-Agent"))
    {
        addRequestHeader(httpreq, "User-Agent", "httplib/1.0") ;
    } 
    return 0 ;
}

static int calcDataLen(DATA *data)
{
    int total = 0 ;
    FOREACH(key, val, data) 
    {
        //total += strlen(key) + strlen("=") + strlen(val) + strlen("&")
        total += strlen(key) + 1 ;
        if(val != NULL)
        {
            total += strlen(val) ;
        }
        total += 1 ;
    }
    return total - 1;
}

static HTTPRESPONSE *getResponse(int fd)
{
    int totalLen = 0 ;
    int hdrLen = 0 ;
    char *hdrbuff = readUntil(fd, &totalLen, &hdrLen, "\r\n\r\n") ;
    if(hdrbuff == NULL)
    {
        return NULL ;
    }
    HTTPRESPONSE *httprsp = initHttpResponse(MALLOC(sizeof(HTTPRESPONSE))) ;
    if(httprsp == NULL)
    {
        printf("fail to create response object:%s\n", strerror(errno)) ;
        goto __fails ; 
    }
    setResponseSocket(httprsp, fd) ;
    setResponseHeaderBuff(httprsp, hdrbuff, 0) ; 
    setResponseExtraData(httprsp, hdrbuff + hdrLen, totalLen - hdrLen) ;
    if(parseHttpResponseHeader(httprsp) != 0)
    {
        printf("fail to parse http response header\n") ;
        goto __fails ;
    }
    return httprsp ;

__fails:
   freeHttpResponse(httprsp) ; 
   FREE(httprsp) ;
   return NULL ;
}

static char *produceBoundary(HTTPREQUEST *httpreq)
{
    if(httpreq == NULL) 
    {
        return NULL ;
    }
    char chs[] = {"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZYZ"} ;
    int chsLen = sizeof(chs) - 1;
    int i = 0 ;
    int count = MIN(sizeof(httpreq ->boundary)-1, MAX_BOUNDRY_BYTE) ;
    srand(time(NULL)) ;
    for(i=0; i< count ;i++)
    {
       httpreq ->boundary[i] = chs[rand()%chsLen] ;
    }
    return httpreq ->boundary ;
}

static int makeHeader(HTTPREQUEST *httpreq, BUFFER *buff, int method)
{
    if(httpreq == NULL || httpreq ->url == NULL || buff == NULL) 
    {
        return -1;
    }
    URL *url = httpreq ->url ;
     //开始拼凑请求头
    if(method == POST)
    {
        appendBuffer(buff, STR_POST, STR_POST_LEN) ;
    }
    else
    {
        appendBuffer(buff, STR_GET, STR_GET_LEN) ;
    }
    appendBuffer(buff, " /", strlen(" /")) ;
    if(url ->path != NULL && strlen(url ->path) != 0)
    {
        appendBuffer(buff, url ->path, strlen(url ->path)) ;
    }
    appendBuffer(buff, "?", 1) ;
    if(url ->query != NULL && strlen(url ->query) != 0)
    {
        appendBuffer(buff, url ->query, strlen(url ->query)) ;
        appendBuffer(buff, "&", 1) ;
    }
    if(method == GET && !isEmpty(&httpreq ->data))
    {
        FOREACH(key, val, &httpreq ->data)
        {
            appendBuffer(buff, key, strlen(key)) ;
            appendBuffer(buff, "=", 1) ;
            if(val != NULL)
            {
                appendBuffer(buff, val, strlen(val)) ;
            }
            appendBuffer(buff, "&", 1) ;
        }
        lstripBuffer(buff, '&') ;
    }
    lstripBuffer(buff, '&') ;
    lstripBuffer(buff, '?') ;
    appendBuffer(buff, " ", 1) ;
    appendBuffer(buff, "HTTP/", 5) ;
    appendBuffer(buff, httpreq ->version, strlen(httpreq ->version)) ;
    appendBuffer(buff, "\r\n", 2) ; 
    if(method == POST)
    {
        //只是上传数据，不需要上传文件
        if(httpreq ->currFileCount == 0)
        {
            int total = calcDataLen(&httpreq ->data) ;
            char length[20] = {0};
            addRequestHeader(httpreq, "Content-Length", itoa(total, length)) ;
            addRequestHeader(httpreq, "Content-Type", "application/x-www-form-urlencoded") ;
        }
        //如果上传文件的话，需要设置boundary
        else
        {
            produceBoundary(httpreq) ;
            char buff[1024] = {0} ;
            strcpy(buff, "multipart/fixed;boundary=") ;
            strcat(buff, httpreq ->boundary) ;
            addRequestHeader(httpreq, "Content-Type", buff) ;
        }
    }
    addDefaultRequestHeader(httpreq) ;    
    appendBuffer(buff, header2String(&httpreq ->header), headerLen(&httpreq ->header)) ;
    appendBuffer(buff, "\r\n", 2) ;
    return 0 ;
}

static int makeBody(HTTPREQUEST *httpreq, BUFFER *buff, int method)
{
    if(method == POST && !isEmpty(&httpreq ->data))
    {
        if(httpreq ->currFileCount != 0)
        {
            appendBuffer(buff, "--", 2) ;
            appendBuffer(buff, httpreq ->boundary, strlen(httpreq ->boundary)) ;
            appendBuffer(buff, "\r\n", 2) ;
            appendBuffer(buff, "Content-Type:application/x-www-form-urlencoded\r\n", strlen("Content-Type:application/x-www-form-urlencoded\r\n")) ;
        }
        FOREACH(key, val, &httpreq ->data)
        {
            appendBuffer(buff, key, strlen(key)) ;
            appendBuffer(buff, "=", 1) ;
            if(val != NULL)
            {
                appendBuffer(buff, val, strlen(val)) ;
            }
            appendBuffer(buff, "&", 1) ;
        }
        lstripBuffer(buff, '&') ; 
        if(httpreq ->currFileCount != 0)
        {
            appendBuffer(buff, "--", 2) ;
            appendBuffer(buff, httpreq ->boundary, strlen(httpreq ->boundary)) ;
            appendBuffer(buff, "\r\n", 2) ;
        }
    }
    return 0 ;
}

static int postFile(HTTPREQUEST *httpreq, int sockfd)
{
   if(httpreq == NULL || sockfd == -1) 
   {
        return -1 ;
   }
   if(httpreq ->method == GET)
   {
        return 0 ;
   }
   if(httpreq ->currFileCount == 0)
   {
        return 0 ;
   }
   int i = 0 ;
   char buff[8192] = {0} ;
   int offset = 0 ;
   int blen = strlen(httpreq ->boundary) ;
   for(i = 0 ;i < httpreq ->currFileCount ;i++)
   {
        int fd = open(httpreq ->filename[i], O_RDONLY) ;
        if(fd < 0)
        {
            printf("fail to open [%s]\n", httpreq ->filename[i], 0) ;
            continue ;
        }
        struct stat st ;
        memset(&st, 0, sizeof(st)) ;
        fstat(fd, &st) ;
        if(st.st_size <= 0)
        {
            printf("%s size is 0 skip it\n", httpreq ->filename[i]) ;
            close(fd) ;
            continue ;
        }
        //开头为--boundary
        strcpy(buff + offset, "--") ;
        offset += 2 ;
        strcpy(buff + offset, httpreq ->boundary) ;
        offset += blen ;
        //增加Content-Disposition 和Content-Type
        strcpy(buff + offset, "Content-Disposition:form-data;name=\"file\";filename=\"") ;
        offset += strlen("Content-Disposition:form-data;name=\"file\";filename=\"") ;
        strcpy(buff + offset, httpreq ->filename[i]) ;
        offset += strlen(httpreq ->filename[i]) ;
        strcpy(buff + offset, "\"\r\n") ;
        offset += 3 ;
        strcpy(buff + offset, "Content-Type:application/octet-stream\r\n") ;
        offset += strlen("Content-Type:application/octet-stream\r\n") ;  
        int ret = 0 ;
        while((ret = read(fd, buff + offset, sizeof(buff) - offset)) > 0)
        {  
            if(writeAll(sockfd, buff, offset + ret) != offset + ret)  
            {
                printf("fail to send data\n") ;
                return -1 ;
            }
            offset = 0 ;
        }
        //结尾为 --boundary
        strcpy(buff + offset, "--") ;
        offset += 2 ;
        strcpy(buff + offset, httpreq ->boundary) ;
        offset += blen ;
   }
   //最后一个要添加一个
   strcpy(buff + offset, "--") ;
   if(writeAll(sockfd, buff, offset) != offset)
   {
        printf("fail to send data\n") ;
        return -1 ;
   }
   return 0 ; 
}

HTTPRESPONSE *sendRequest(HTTPREQUEST *httpreq, int timeout)
{
    if(httpreq == NULL || httpreq ->url == NULL) 
    {
        return NULL ;
    }
    BUFFER buff ;
    initBuffer(&buff) ;
    makeHeader(httpreq, &buff, httpreq ->method) ;
    makeBody(httpreq, &buff, httpreq ->method) ;
    URL *url = httpreq ->url ;
    int fd = connectToServer(url->host, url ->port == NULL ?DEFAULT_PORT:url ->port, timeout) ;
    if(fd < 0)
    {
       return NULL; 
    }
    if(sendData(fd, getBufferData(&buff), getBufferSize(&buff)) != getBufferSize(&buff))
    {
        printf("send [%s] fail\n", getBufferData(&buff)) ;
    }
    freeBuffer(&buff) ;
    if(postFile(httpreq, fd) != 0)
    {
        printf("fail to post file\n") ;
        return NULL ;
    }
    return getResponse(fd) ;
}

HTTPRESPONSE *sendRequestWithGET(HTTPREQUEST *httpreq, int timeout)
{
    if(httpreq == NULL)
    {
        return  NULL ; 
    }
    httpreq ->method = GET ;
    return sendRequest(httpreq, timeout) ;
}

HTTPRESPONSE *sendRequestWithPOST(HTTPREQUEST *httpreq, int timeout)
{
    if(httpreq == NULL)
    {
        return NULL ;
    }
    httpreq ->method = POST ;
    return sendRequest(httpreq, timeout) ;
}
