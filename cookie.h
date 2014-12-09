#ifndef __COOKIE__H
#define __COOKIE__H
#include "global.h"

typedef struct cookie
{
    
    char *cookie_buff ;
    uint size ;
    uint curr_size ;

}http_cookie_t;

#ifdef __cpluscplus
extern "C" {
#endif

int free_cookie(http_cookie_t *cookie) ;
int init_cookie(http_cookie_t *cookie) ;
int delete_key(http_cookie_t *cookie, const char *key) ;
int add_key(http_cookie_t *cookie, const char *key, const char *value) ;
int update_key(http_cookie_t *cookie, const char *key, const char *new_val) ;

int add_secure_option(http_cookie_t *cookie) ;
int del_secure_option(http_cookie_t *cookie) ;

int add_httponly_option(http_cookie_t *cookie) ;
int del_httponly_option(http_cookie_t *cookie) ;

http_cookie_t *cookie_copy(http_cookie_t *dst, const http_cookie_t *src) ;
char *copy_value(http_cookie_t *cookie, const char *key, char *val) ;
const char *cookie2String(http_cookie_t *cookie) ;
#ifdef __cpluscplus
}
#endif

#endif
