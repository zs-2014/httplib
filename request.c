#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "response.h"
#include "request.h"
#include "http_url.h"

#define MAX(a, b) (a) > (b) ? (a):(b)
#define MIN(a, b) (a) > (b) ? (b):(a)

#define VERSION(m, n) (((m)-'0')*10 + (n) - '0')
#define MIN_VERSION VERSION('1', '0') 

#define MALLOC(sz) malloc(sz)
#define CALLOC(n, sz) calloc(n, sz)
#define REALLOC(ptr, sz) realloc(ptr, sz)
#define FREE(ptr) free(ptr)

HTTPREQUEST * newHTTPREQUEST()
{
    HTTPREQUEST *httpreq = MALLOC(sizeof(HTTPREQUEST)) ;
    if(httpreq == NULL)
    {
        return NULL ;
    }
    httpreq ->cookie = NULL ;
    httpreq ->url = NULL ;
    return httpreq ;

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
    httpreq ->cookie = cookie ;
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

int addRequestData(HTTPREQUEST *httpreq, const char *key, const char *value)
{
    return 0 ; 
}

int sendRequest(HTTPREQUEST *httpreq, int timeout)
{
    return 0 ;
}

HttpResponseHeader *getResponse(HTTPREQUEST *httpreq)
{
    return NULL ;
}
