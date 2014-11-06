#ifndef __HTTP_HEADER__H
#define __HTTP_HEADER__H

#include "global.h"

typedef struct HttpHeader
{
    int currSz ;
    int size ;
    char *hdrBuff ;
}HEADER;

#ifdef __cplusplus
extern "C" {
#endif

extern int initHttpHeader(HEADER *httphdr) ;
extern int freeHttpHeader(HEADER *httphdr) ;
extern int addHeader(HEADER *httphdr, const char *key, const char *value) ;
extern int deleteHeader(HEADER *httphdr, const char *key) ;
extern int updateHeader(HEADER *httphdr, const char *key, const char *values) ;
extern const char *header2String(HEADER *httphdr) ;
extern uint headerLen(HEADER *httphdr) ;
extern int hasHeader(HEADER *httphdr, const char *key) ;
#ifdef __cplusplus
}
#endif

#endif
