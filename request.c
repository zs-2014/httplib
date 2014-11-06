#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "response.h"
#include "request.h"
#include "cookie.h"
#include "http_url.h"
#include "util.h"
#include "httpheader.h"
#include "buffer.h"
#include "global.h"

#define DEFAULT_PORT "80"
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
    return initHttpHeader(&httpreq ->header) ;
}

int freeHttpRequest(HTTPREQUEST *httpreq)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    freeURL(httpreq ->url) ; 
    freeHttpHeader(&httpreq ->header) ;
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

int sendRequestWithGET(HTTPREQUEST *httpreq, int timeout)
{
    if(httpreq == NULL || httpreq ->url == NULL) 
    {
        return -1 ;
    }

    URL *url = httpreq ->url ;
    int fd = connectToServer(url->host, url ->port == NULL ?DEFAULT_PORT:url ->port, timeout) ;
    if(fd < 0)
    {
       return -1; 
    }
    //开始拼凑请求头
    BUFFER buff ;
    initBuffer(&buff) ;
    appendBuffer(&buff, "GET ", strlen("GET ")) ;
    appendBuffer(&buff, "/", strlen("/")) ;
    if(url ->path != NULL && strlen(url ->path) == 0)
    {
        appendBuffer(&buff, url ->path, strlen(url ->path)) ;
    }
    appendBuffer(&buff, "?", strlen("?")) ; 
    if(url ->query != NULL || strlen(url ->query) != 0)
    {
        appendBuffer(&buff, url ->query, strlen(url ->query)) ;
    }
    if(!isEmpty(&data))
    {
        FOREACH(key, val, &data)
        {
            appendBuffer(&buff, "&", 1) ;
            appendBuffer(&buff, key, strlen(key)) ;
            appendBuffer(&buff, "=", 1)
            appendBuffer(&buff, val, strlen(val)) ;
        }
    }
    lstripBuffer(&buff, '?') ;
    //method path version \r\n
    appendBuffer(&buff, " ", 1) ;
    appendBuffer(&buff, "HTTP/", 5) ;
    appendBuffer(&buff, httpreq ->version, strlen(httpreq ->version)) ;
    appendBuffer(&buff, " \r\n", 2) ; 
    appendBuffer(&buff, header2String(&httpreq ->header), headerLen(&httpreq ->header)) ;
    appendBuffer(&buff, "\r\n", 2) ;
}

int sendRequestWithPOST(HTTPREQUEST *httpreq, int timeout)
{
    
}
int sendRequest(HTTPREQUEST *httpreq, int timeout)
{
    if(httpreq == NULL || httpreq ->url == NULL)
    {
        return -1 ;
    }

    if(httpreq ->method == GET)
    {
        return sendRequestWithGET(httpreq, timeout) ;
    }
    else
    {
       //post数据 
    }
}

HttpResponseHeader *getResponse(HTTPREQUEST *httpreq)
{
    return NULL ;
}


#if 1
int main(int argc, char *argv[])
{
    
    return 0 ;
}
#endif
