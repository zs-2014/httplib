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

extern BUFFER *lstripBuffer(BUFFER *buff, uchar ch) ;
extern int initBuffer(BUFFER *buff) ;
extern int freeBuffer(BUFFER *buff) ;
extern int appendBuffer(BUFFER *buff, const uchar *val, uint sz) ;
extern const uchar* getBufferData(const BUFFER *buff) ;
extern uint getBufferSize(const BUFFER *buff) ;

#ifdef __cplusplus
}
#endif


#endif
