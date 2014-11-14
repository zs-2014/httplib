#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "buffer.h"
#include "global.h"

static int getGrow(BUFFER *buff)
{
    static int i = 1 ;
    return 512*i++ ;
}

static BUFFER *reallocBuffer(BUFFER *buff, uint sz)
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
        memcpy(tmp, buff ->buff, buff ->currSz) ;
        FREE(buff ->buff) ;
    }
    buff ->buff = tmp ;
    buff ->size = sz ;
    return buff ;
}

int initBuffer(BUFFER *buff)
{
    if(buff == NULL)
    {
        return -1 ;
    }
    buff ->buff = NULL ;
    buff ->currSz = 0 ;
    buff ->size = 0 ;
    return 0 ;
}

int freeBuffer(BUFFER *buff)
{
    if(buff == NULL)
    {
        return -1 ;
    }
    FREE(buff ->buff) ;
    buff ->currSz = 0 ;
    buff ->size = 0 ;
    return 0 ;
}

int appendBuffer(BUFFER *buff, const uchar *val, uint sz)
{
    if(buff == NULL || val == NULL)    
    {
        return -1 ;
    }
    if(buff ->currSz + sz > buff ->size)
    {
        if(reallocBuffer(buff, buff ->currSz + sz + getGrow(buff)) == NULL)
        {
            return -1;
        }
    }
    memcpy(buff ->buff + buff ->currSz, val ,sz) ;
    buff ->currSz += sz ;
    return 0 ;
}

BUFFER *lstripBuffer(BUFFER *buff, uchar ch)
{
    if(buff == NULL)
    {
        return NULL ;
    }
    for( ; buff ->currSz > 0 ; buff ->currSz--) 
    {
        if(buff ->buff[buff ->currSz - 1] != ch)
        {
            return buff ;
        }
    }
}

const uchar* getBufferData(const BUFFER *buff)
{
    return buff != NULL ? (buff ->currSz == 0 ? (uchar *)nullStrPtr : buff ->buff) : (uchar *)nullStrPtr ;
}

uint getBufferSize(const BUFFER *buff) 
{
   return buff == NULL ? 0 : buff ->currSz ; 
}
