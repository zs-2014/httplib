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
    uint dataLen ;
}DATA;

#define FOREACH(name, value, dataobj) \
char *name = NULL ;\
char *value = NULL ;\
int __i__ = 0 ;\
for(__i__ = 0; __i__ < (dataobj) ->currSz && (name = (dataobj) ->data[__i__].key, value = (dataobj) ->data[__i__].val,1); __i__++)

#ifdef __cplusplus
extern "C" {
#endif

extern int initData(DATA *data) ;
extern int isEmpty(DATA *data) ;
extern int addData(DATA *data, const uchar *key, int keySz, const uchar *val, int valSz) ;
extern int deleteData(DATA *data, const uchar *key, int keySz) ;
extern int updateData(DATA *data, const uchar *key, int keySz, const uchar *val, int valSz) ;

#ifdef __cplusplus
}
#endif

#endif
