#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "buffer.h"
#include "global.h"

static int get_grow(buffer_t *buff)
{
    static int i = 1 ;
    return 512*i++ ;
}

static buffer_t *realloc_buffer(buffer_t *buff, uint sz)
{
    if(buff ->size > sz) 
    {
        return buff ;
    }
    uchar *tmp = (uchar *)MALLOC(sizeof(uchar)*sz) ;
    if(tmp == NULL)
    {
        return NULL ;
    }
    if(buff ->buff != NULL)
    {
        memcpy(tmp, buff ->buff, buff ->curr_sz) ;
        FREE(buff ->buff) ;
    }
    buff ->buff = tmp ;
    buff ->size = sz ;
    return buff ;
}

int init_buffer(buffer_t *buff)
{
    if(buff == NULL)
    {
        return -1 ;
    }
    buff ->buff = NULL ;
    buff ->curr_sz = 0 ;
    buff ->size = 0 ;
    return 0 ;
}

int free_buffer(buffer_t *buff)
{
    if(buff == NULL)
    {
        return -1 ;
    }
    FREE(buff ->buff) ;
    buff ->curr_sz = 0 ;
    buff ->size = 0 ;
    return 0 ;
}

int append_buffer(buffer_t *buff, const uchar *val, uint sz)
{
    if(buff == NULL || val == NULL)    
    {
        return -1 ;
    }
    if(buff ->curr_sz + sz > buff ->size)
    {
        if(realloc_buffer(buff, buff ->curr_sz + sz + get_grow(buff)) == NULL)
        {
            return -1;
        }
    }
    memcpy(buff ->buff + buff ->curr_sz, val ,sz) ;
    buff ->curr_sz += sz ;
    return 0 ;
}

buffer_t *lstrip_buffer(buffer_t *buff, uchar ch)
{
    if(buff == NULL)
    {
        return NULL ;
    }
    for( ; buff ->curr_sz > 0 ; buff ->curr_sz--) 
    {
        if(buff ->buff[buff ->curr_sz - 1] != ch)
        {
            return buff ;
        }
    }
}

const uchar* get_buffer_data(const buffer_t *buff)
{
    return buff != NULL ? (buff ->curr_sz == 0 ? (uchar *)null_str_ptr : buff ->buff) : (uchar *)null_str_ptr ;
}

int drop_data(buffer_t *buff, uint sz) 
{
    if(buff != NULL)
    {
       buff ->curr_sz = buff ->curr_sz > sz ? buff ->curr_sz - sz:0 ;
    }
    return 0 ;
}

uint get_buffer_size(const buffer_t *buff) 
{
   return buff == NULL ? 0 : buff ->curr_sz ; 
}
