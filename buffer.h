#ifndef __BUFFER__H
#define __BUFFER__H

#include "global.h"

typedef struct buffer
{
    uchar *buff ;
    uint currSz ;
    uint size ;
}BUFFER ;

#ifdef __cplusplus
extern "C"{
#endif

extern int initBuffer(BUFFER *buff) ;
extern int freeBuffer(BUFFER *buff) ;
extern int appendBuff(BUFFER *buff, const uchar *val, uint sz) ;

#ifdef __cplusplus
}
#endif


#endif
