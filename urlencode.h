#ifndef __URLENCODE__H
#define __URLENCODE__H

#include "global.h"

#define quotebuff(urlstr, sz) quote(urlstr, sz)
#define quotestr(urlstr) quote(urlstr, strlen(urlstr))

#define unquotebuff(urlstr, sz) unquote(urlstr, sz)
#define unquotestr(urlstr) unquote(urlstr, strlen(urlstr))


#ifdef __cplusplus
extern "C"{
#endif

extern char* quote(const uchar* urlstr, uint sz) ;
extern uchar* unquote(const char* urlstr, uint sz) ; 

extern char *urlencode(const char *str) ;

#ifdef __cplusplus
}
#endif


#endif
