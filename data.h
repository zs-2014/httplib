#ifndef __DATA__H
#define __DATA__H

#include "global.h"

typedef struct __Data 
{
    char *key ;
    char *val ;
}__DATA ;

typedef struct Data
{
    __DATA *data ;
    uint currSz ;
    uint size ;
}DATA;

#ifdef __cplusplus
extern "C" {
#endif

extern int initData(DATA *data) ;
extern int addData(DATA *data, uchar *key, int keySz, uchar *val, int valSz) ;
extern int deleteData(DATA *data, uchar *key, int keySz) ;
extern int updateData(DATA *data, uchar *key, int keySz, uchar *val, int valSz) ;

#ifdef __cplusplus
}
#endif

#endif
