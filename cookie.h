#ifndef __COOKIE__H
#define __COOKIE__H
#include "global.h"

typedef struct cookie
{
    
    char *cookieBuff ;
    uint size ;
    uint currSize ;

}COOKIE;

#ifdef __cpluscplus
extern "C" {
#endif

int freeCookie(COOKIE *cookie) ;
int initCookie(COOKIE *cookie) ;
int deleteKey(COOKIE *cookie, const char *key) ;
int addKey(COOKIE *cookie, const char *key, const char *value) ;
int updateKey(COOKIE *cookie, const char *key, const char *newVal) ;

int addSecureOption(COOKIE *cookie) ;
int delSecureOption(COOKIE *cookie) ;

int addHttponlyOption(COOKIE *cookie) ;
int delHttponlyOption(COOKIE *cookie) ;

COOKIE *cookieCopy(COOKIE *dst, const COOKIE *src) ;
char *copyValue(COOKIE *cookie, const char *key, char *val) ;

#ifdef __cpluscplus
}
#endif

#endif
