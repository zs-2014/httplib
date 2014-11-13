#ifndef __RESPONSE__H
#define __RESPONSE__H

#include "global.h"

typedef struct Node
{
	char *value ;
	char *key;
}NODE ;

typedef struct HttpResponseHeader
{
    char version[9] ;
    char code[4] ;
    char reason[32] ;
	int size ;
    int count ;
    char *headbuff ;
	NODE *key_val ;
}HttpResponseHeader;

typedef struct HTTPRESPONSE
{
   HttpResponseHeader httprsphdr ;
   int rspfd ;

   //extraData 包含的是在接受http响应头时，\r\n 后面的body中的数据此缓冲区在http response header中被释放
   char *extraData ;
   int extraSize ;

}HTTPRESPONSE ;

#ifdef __cplusplus
extern "C"{
#endif

extern int setResponseHeaderBuff(HTTPRESPONSE *httprsp, char *hdrbuff, int isCopy) ;
extern int parseHttpResponseHeader(HTTPRESPONSE *httprsp) ;

extern int freeHttpResponse(HTTPRESPONSE *httprsp) ;
extern HTTPRESPONSE *initHttpResponse(HTTPRESPONSE *httprsp) ;

#ifdef __cplusplus
}
#endif

#endif
