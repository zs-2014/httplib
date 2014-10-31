#ifndef __REQUEST__H
#define __REQUEST__H

#include "url.h"

typedef struct HttpRequest
{
    //对url解析的结果
    URL *url ;
    COOKIE *cookie ; 
    //存放待发送的数据
    uchar *buff ;
} HTTPREQUEST;
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif
