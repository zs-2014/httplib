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
    uint curr_sz ;
    uint size ;
    uint data_len ;
}data_t;

#define FOREACH(name, value, dataobj) \
char *name = NULL ;\
char *value = NULL ;\
int __i__ = 0 ;\
for(__i__ = 0; __i__ < (dataobj) ->curr_sz && (name = (dataobj) ->data[__i__].key, value = (dataobj) ->data[__i__].val,1); __i__++)

#ifdef __cplusplus
extern "C" {
#endif

extern int init_data(data_t *data) ;
extern int is_empty(data_t *data) ;
extern int add_data(data_t *data, const uchar *key, int key_sz, const uchar *val, int val_sz) ;
extern int delete_data(data_t *data, const uchar *key, int key_sz) ;
extern int update_data(data_t *data, const uchar *key, int key_sz, const uchar *val, int val_sz) ;

#ifdef __cplusplus
}
#endif

#endif
