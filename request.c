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
    return initCookie(&httpreq ->cookie) || initHttpHeader(&httpreq ->header) ;
}

int freeHttpRequest(HTTPREQUEST *httpreq)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    freeURL(httpreq ->url) ; 
    freeCookie(&httpreq ->cookie) ;
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
    if(httpreq == NULL)
    {
        return -1 ;
    }
    if(cookieCopy(&httpreq ->cookie, cookie) == NULL) 
    {
        return -1 ;
    }
    return 0 ;
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

int sendRequest(HTTPREQUEST *httpreq, int timeout)
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
    BUFFER buff ;
    initBuffer(&buff) ;
    //开始拼凑请求头
    return 0 ;
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
