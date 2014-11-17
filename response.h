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

   //transfer-encoding 为chunk时，记录距离下一个chunk还有多少数据要读
   int nextChunkSize ;
   int chunkCount ;
   char buff[8192] ;
   int currSz ;
   int currPos ;

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
