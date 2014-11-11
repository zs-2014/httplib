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
}HTTPRESPONSE ;

#ifdef __cplusplus
extern "C"{
#endif

extern int setResponseHeaderBuff(HTTPRESPONSE *httprsp, char *hdrbuff) ;
extern int parseHttpResponseHeader(HTTPRESPONSE *httprsp) ;
extern HTTPRESPONSE *initHttpResponse(HTTPRESPONSE *httprsp) ;

#ifdef __cplusplus
}
#endif

#endif
