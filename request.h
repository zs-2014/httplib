#ifndef __REQUEST__H
#define __REQUEST__H

#include "http_url.h"
#include "httpheader.h"
#include "cookie.h"
#include "data.h"
#include "response.h"

#define GET  0X00000001
#define POST 0X00000002

typedef struct HttpRequest
{
    //对url解析的结果
    URL *url ;
    HEADER header ;
    COOKIE cookie ; 
    DATA data;
    int method;
    char version[4] ;
} HTTPREQUEST;
#ifdef __cplusplus
extern "C" {
#endif


extern HTTPRESPONSE* sendRequest(HTTPREQUEST *httpreq, int method, int timeout) ;
extern HTTPRESPONSE* sendRequestWithGET(HTTPREQUEST *httpreq, int timeout) ;
extern HTTPRESPONSE* sendRequestWithPOST(HTTPREQUEST *httpreq, int timeout) ;
extern int addRequestData(HTTPREQUEST *httpreq, const uchar *key, int keySz, const uchar *val, int valSz) ;
extern int addRequestHeader(HTTPREQUEST *httpreq, const char *key, const char *value) ;
extern int setHttpVersion(HTTPREQUEST *httpreq, const char *version) ;

#ifdef __cplusplus
}
#endif


#endif
