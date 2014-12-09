#ifndef __BUFFER__H
#define __BUFFER__H

#include "global.h"

typedef struct buffer
{
    uchar *buff ;
    uint curr_sz ;
    uint size ;
}buffer_t ;

#ifdef __cplusplus
extern "C"{
#endif

extern buffer_t *lstrip_buffer(buffer_t *buff, uchar ch) ;
extern int init_buffer(buffer_t *buff) ;
extern int free_buffer(buffer_t *buff) ;
extern int append_buffer(buffer_t *buff, const uchar *val, uint sz) ;
extern const uchar* get_buffer_data(const buffer_t *buff) ;
extern int drop_data(buffer_t *buff, uint sz) ;
extern uint get_buffer_size(const buffer_t *buff) ;

#ifdef __cplusplus
}
#endif

#endif
