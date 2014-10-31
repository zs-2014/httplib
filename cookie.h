#ifndef __COOKIE__H

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

COOKIE *mallocCookie(uint buffSz) ;
int freeCookie(COOKIE *cookie) ;
int deleteKey(COOKIE *cookie, const char *key) ;
int addKey(COOKIE *cookie, const char *key, const char *value) ;
int updateKey(COOKIE *cookie, const char *key, const char *newVal) ;
char *getValue(COOKIE *cookie, const char *key, char *val) ;

#ifdef __cpluscplus
}
#endif

#endif
