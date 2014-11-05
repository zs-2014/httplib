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


int appendBuff(BUFFER *buff, const uchar *val, uint sz)
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


#if 1
void printBuffer(BUFFER *buff)
{
    printf("buff:currSz = %u size = %u\n", buff ->currSz, buff ->size) ;
}
void appendBuffTest()
{
   BUFFER buff ;
   initBuffer(&buff) ;
   appendBuff(&buff, "hello,world",strlen("hello,world")) ;
   printBuffer(&buff) ;
   appendBuff(&buff, "hello,world",strlen("hello,world")) ;
   printBuffer(&buff) ;
   char bf[1024] = {0} ;
   appendBuff(&buff, bf, sizeof(bf)) ;
   printBuffer(&buff) ;
   freeBuffer(&buff) ;
}

int main(int argc, char *argv[])
{
    appendBuffTest() ; 
    return 0 ; 
}
#endif
