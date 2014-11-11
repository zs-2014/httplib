#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "global.h"
#include "util.h"
#include "httpheader.h"

#define DEFAULT_GROW_SIZE  256

int initHttpHeader(HEADER *httphdr)
{
    if(httphdr == NULL)
    {
        return -1 ; 
    }
    httphdr ->currSz = 0 ;
    httphdr ->size = 0 ;
    httphdr ->hdrBuff = 0 ;
    return 0 ;
}

int freeHttpHeader(HEADER *httphdr)
{
    if(httphdr == NULL)
    {
        return -1; 
    }
    FREE(httphdr ->hdrBuff) ;
    httphdr ->hdrBuff = NULL ;
    httphdr ->currSz = 0 ;
    httphdr ->size = 0 ;
}

static int getGrow(HEADER *httphdr)
{
    return DEFAULT_GROW_SIZE ;
}

static HEADER * reallocHeader(HEADER *httphdr, uint sz)
{
    if(httphdr == NULL)    
    {
        return NULL ;
    }
    if(httphdr ->size > sz)
    {
        return httphdr ;
    }
    char *tmp = (char *)MALLOC(sizeof(char)*sz) ;
    if(tmp == NULL)
    {
        return NULL ;
    }
    if(httphdr ->hdrBuff != NULL)
    {
        memcpy(tmp, httphdr ->hdrBuff, httphdr ->currSz) ;
        FREE(httphdr ->hdrBuff) ;
    }
    httphdr ->hdrBuff = tmp ;
    httphdr ->size = sz ;
    return httphdr ;
}

static int search(HEADER *httphdr, const char *key, char **key_b, char **val_b, char **val_e)
{
    int keyLen = strlen(key) ;
    *key_b = strcasestr(httphdr ->hdrBuff , key) ; 
    while(*key_b != NULL)
    {
        if((*key_b)[keyLen] == ':' && (*key_b == httphdr ->hdrBuff || (*key_b)[-1] == '\n')) 
        {
            *val_b = *key_b + keyLen + 1; 
            *val_e = strstr(*val_b, "\r\n") ;
            *val_e = (*val_e != NULL ? *val_e : NULL ) ;
            break ;
        }
        else
        {
           *key_b = strcasestr(*key_b + keyLen + 1, key) ; 
        }
    }
    return 0 ;
}

int addHeader(HEADER *httphdr, const char *key, const char *val)
{
    if(httphdr == NULL || key == NULL || val == NULL) 
    {
        return -1 ; 
    }
    int keyLen = strlen(key) ;
    int valLen = strlen(val) ;
    //  len(":") == 1 len("\r\n") == 2
    if(httphdr ->currSz + keyLen + valLen + 3 >= httphdr ->size)
    {
        if(reallocHeader(httphdr, httphdr ->currSz + keyLen + valLen + 3 + getGrow(httphdr)) == NULL) 
        {
            return -1 ;
        }
    }
    memcpy(httphdr ->hdrBuff + httphdr ->currSz, key, keyLen) ;
    httphdr ->currSz += keyLen ;
    httphdr ->hdrBuff[httphdr ->currSz] = ':' ;
    httphdr ->currSz += 1 ;
    memcpy(httphdr ->hdrBuff + httphdr ->currSz, val, valLen) ;
    httphdr ->currSz += valLen ;
    strcpy(httphdr ->hdrBuff + httphdr ->currSz, "\r\n") ;
    httphdr ->currSz += 2 ;
    return 0 ;
}

int deleteHeader(HEADER *httphdr, const char *key)
{
    if(httphdr == NULL || httphdr ->hdrBuff == NULL || key == NULL)
    {
        return -1 ;
    }
    int keyLen = strlen(key) ;
    char *key_b = NULL ;
    char *val_b = NULL ;
    char *val_e = NULL ;
    search(httphdr, key, &key_b, &val_b, &val_e) ;
    if(key_b == NULL || val_b == NULL || val_e == NULL)
    {
        return -1 ;
    }
    memcpy(key_b, val_e + 2, httphdr ->currSz - (val_e - httphdr ->hdrBuff + 2)) ;  
    httphdr ->currSz -= val_e - key_b + 2;
    httphdr ->hdrBuff[httphdr ->currSz] = '\0' ;
    return 0 ;
}

int updateHeader(HEADER *httphdr, const char *key, const char *newValue)
{
    if(httphdr == NULL || key == NULL || newValue == NULL) 
    {
        return -1 ;
    }
    return deleteHeader(httphdr, key) || addHeader(httphdr, key, newValue) ;
}

const char *header2String(HEADER *httphdr) 
{
    return httphdr != NULL ? (httphdr ->currSz == 0 ? nullStrPtr : httphdr ->hdrBuff) : nullStrPtr ;
}
uint headerLen(HEADER *httphdr)
{
    return httphdr != NULL ? httphdr ->currSz : 0 ;
}
int hasHeader(HEADER *httphdr, const char *key) 
{
    if(httphdr == NULL || key == NULL || httphdr ->hdrBuff == NULL)
    {
        return 0 ;
    }
    char *key_b = NULL ;
    char *val_b = NULL ;
    char *val_e = NULL ;
    search(httphdr, key, &key_b, &val_b, &val_e) ;
    if(key_b == NULL || val_b == NULL || val_e == NULL)
    {
        return 0 ;
    }
    return 1 ;
}
void printHeader(HEADER *httphdr)
{
    printf("[%s]\n", httphdr ->hdrBuff) ;
}

#if 0
void addHeaderTest()
{
    HEADER header ;
    initHttpHeader(&header) ;
    addHeader(&header, "content-type", "application/json ;charset=utf-8"); 
    printHeader(&header) ;
    addHeader(&header, "content-length", "12045") ;
    printHeader(&header) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) -1) ;
    addHeader(&header, "Cookie", buff) ;
    printHeader(&header) ;
    freeHttpHeader(&header) ;
}

void deleteHeaderTest()
{
    HEADER header ;
    initHttpHeader(&header) ;
    addHeader(&header, "content-type1", "application/json ;charset=utf-8"); 
    printHeader(&header) ;
    addHeader(&header, "content-length", "12045") ;
    printHeader(&header) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) -1) ;
    addHeader(&header, "Cookie", buff) ;
    printHeader(&header) ;
    //freeHttpHeader(&header) ;
    
    deleteHeader(&header, "Cookie") ;
    printHeader(&header) ;
    deleteHeader(&header, "Content-length") ;
    printHeader(&header) ;
    deleteHeader(&header, "content-length");
    printHeader(&header) ;

    deleteHeader(&header, "content-type") ; 
    printHeader(&header) ;
    freeHttpHeader(&header) ;
}

void updateHeaderTest()
{
    HEADER header ;
    initHttpHeader(&header) ;
    addHeader(&header, "content-type", "application/json ;charset=utf-8"); 
    printHeader(&header) ;
    addHeader(&header, "content-length", "12045") ;
    printHeader(&header) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) -1) ;
    addHeader(&header, "Cookie", buff) ;
    printHeader(&header) ;

    updateHeader(&header, "content-length", "12345") ;
    printHeader(&header) ;
    updateHeader(&header, "content-type", "plain/text");
    printHeader(&header) ;
    updateHeader(&header, "Cookie", "12ksfjskdjksdjl") ;
    printHeader(&header) ;
    freeHttpHeader(&header) ;
}
int main(int argc, char *argv[])
{
    //addHeaderTest() ;
    deleteHeaderTest() ;
   //updateHeaderTest() ;
    return 0 ;
}
#endif
