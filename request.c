#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "response.h"
#include "request.h"
#include "cookie.h"
#include "http_url.h"
#include "global.h"

#define VERSION(m, n) (((m)-'0')*10 + (n) - '0')
#define MIN_VERSION VERSION('1', '0') 

int initHttpRequest(HTTPREQUEST *httpreq)
{
    httpreq ->method = GET ;
    httpreq ->url = NULL ;
    return initCookie(&httpreq ->cookie) ;
}

int freeHttpRequest(HTTPREQUEST *httpreq)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    freeURL(httpreq ->url) ; 
    freeCookie(&httpreq ->cookie) ;
}

int setRequestMethod(HTTPREQUEST *httpreq, int method)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    if(method == GET)
    {
        httpreq ->method = GET ;
    }
    else
    {
        httpreq ->method = POST ;
    }
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
    return 0 ;
}
int setUserAgent(HTTPREQUEST *httpreq, const char *userAgent)
{
    return 0 ; 
}

int addRequstHeader(HTTPREQUEST *httpreq, const uchar *key, const char *value)
{
    return 0 ; 
}

int addRequestData(HTTPREQUEST *httpreq, const uchar *key, int keySz, const uchar *val, int valSz)
{
    if(httpreq == NULL || key == NULL || value == NULL)
    {
        return -1 ;
    }
    return addData(httpreq ->data, key, keySz, val, valSz) ;
}

int sendRequest(HTTPREQUEST *httpreq, int timeout)
{
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
