#ifndef __URLENCODE__H
#define __URLENCODE__H

#include "global.h"

#define quotebuff(urlstr, sz, safe) quote(urlstr, sz, safe)
#define quotestr(urlstr, safe) quote(urlstr, strlen(urlstr),safe)

#define unquotebuff(urlstr, sz) unquote(urlstr, sz)
#define unquotestr(urlstr) unquote(urlstr, strlen(urlstr))

#ifdef __cplusplus
extern "C"{
#endif

extern char* quote(const uchar* urlstr, uint sz, const char *safe) ;
extern uchar* unquote(const char* urlstr, uint sz) ; 

extern char *urlencode(const char *str) ;

#ifdef __cplusplus
}
#endif

#endif
