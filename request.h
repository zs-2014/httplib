#ifndef __REQUEST__H
#define __REQUEST__H

#include "http_url.h"
#include "httpheader.h"
#include "cookie.h"
#include "data.h"

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


#ifdef __cplusplus
}
#endif


#endif
