#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cookie.h"

#define  DEFAULT_COOKIE_BUFF_SIZE 128 

//如果申请空间失败，则原空间不会释放
static char *realloc_cookie(http_cookie_t *cookie, uint sz)
{
    if(cookie == NULL)
    {
        return NULL ;
    }

    if(cookie ->size > sz)
    {
        return cookie ->cookie_buff ;
    }

    char *tmp = (char *)CALLOC(1, (sz*sizeof(char))) ;
    if(tmp == NULL )
    {
         return NULL ;
    } 

    if(cookie ->cookie_buff != NULL)
    {
        memcpy(tmp, cookie ->cookie_buff, cookie ->curr_size) ;
        FREE(cookie ->cookie_buff) ;
    }
    cookie ->size = sz ;
    cookie ->cookie_buff = tmp ;
    return tmp ;
}

//获取增加空间的大小
static uint get_grow(http_cookie_t *cookie)
{
    return DEFAULT_COOKIE_BUFF_SIZE ;
}

//在session buffer中查找一个key val
static int search(http_cookie_t *cookie, const char *key, char **key_b, char **val_b, char **val_e)
{
    int key_len = strlen(key) ;
    *key_b = strstr(cookie ->cookie_buff, key) ;
    while(*key_b != NULL) 
    {
        if((*key_b)[key_len] == '=' && (*key_b == cookie ->cookie_buff || ((*key_b)[-1] == ';'))) 
        {
            *val_b = *key_b + key_len + 1 ;
            *val_e = strchr(*val_b, ';') ; 
            *val_e = *val_e == NULL ? NULL : *val_e;
            break ;
        }
        else
        {
            *key_b = strstr(*key_b + key_len + 1, key) ;
        }
    }
    return 0 ;
}

static del_option(http_cookie_t *cookie, const char *option)
{
    if(cookie == NULL || option == NULL)
    {
        return -1 ; 
    }
    int oplen = strlen(option) ;
    if(oplen == 0)
    {
        return -1 ;
    }

    char *option_b = strstr(cookie ->cookie_buff, option) ;
    //如果查找到了这个session选项
    while(option_b != NULL)
    {
        if(option_b != cookie ->cookie_buff)
        {
            //保证session是完全匹配的
            if(option_b[-1] == ';' && option_b[oplen] == ';')
            {
                memcpy(option_b, option_b + oplen + 1, cookie ->curr_size - (option_b - cookie ->cookie_buff + oplen + 1)) ;
                cookie ->curr_size -= oplen + 1 ;
                cookie ->cookie_buff[cookie ->curr_size] = '\0' ;
                return 0 ; 
            }
            else
            {
                //不匹配的话，继续查找下去
                option_b = strstr(option_b + oplen, option) ;
            }
        }
        //如果这个session选项在开头
        else if (option_b[oplen] == ';')
        {
           memcpy(option_b, option_b + oplen + 1, cookie ->curr_size - (oplen + 1)) ;
           cookie ->curr_size -= oplen + 1 ;
           return 0 ;
        }
        else
        {
           option_b = strstr(option_b + oplen, option) ;
        }
   }
    return 0 ;
}

int init_cookie(http_cookie_t *cookie)
{
    if(cookie == NULL) 
    {
            return -1 ;
    }
    cookie ->cookie_buff = NULL ;
    cookie ->size = 0 ;
    cookie ->curr_size = 0 ;
    return 0 ;
}

http_cookie_t *cookie_copy(http_cookie_t *dst, const http_cookie_t *src)
{
    if(src == NULL || dst == NULL)
    {
        return NULL ; 
    }
    if(dst ->size <= src ->curr_size && realloc_cookie(dst, src ->size) == NULL)
    {
        return NULL ;
    }

    memcpy(dst ->cookie_buff, src ->cookie_buff, src ->curr_size) ;
    dst ->curr_size = src ->curr_size;
    return dst ;
}

int free_cookie(http_cookie_t *cookie)
{
    if(cookie == NULL && cookie ->cookie_buff != NULL)
    {
        FREE(cookie ->cookie_buff) ;
        cookie ->cookie_buff = NULL ; 
        cookie ->size = 0 ;
        cookie ->curr_size = 0 ;
    }
    return 0 ;
}

int add_key_value(http_cookie_t *cookie, const char *key, const char *value) 
{
    if(cookie == NULL || value == NULL)
    {
        return -1 ;
    }
    uint key_len = (key != NULL ?strlen(key):0) ;
    uint val_len = strlen(value) ;

    //len(";=") == 1
    if(key_len + val_len + 2 + cookie ->curr_size >=  cookie ->size )
    {
        //空间不够，重新增加空间
        if(realloc_cookie(cookie, key_len + val_len + 2 + cookie ->curr_size + get_grow(cookie)) == NULL)
        {
            return -1 ;
        }
    }
    if(key_len != 0)
    { 
        memcpy(cookie ->cookie_buff + cookie ->curr_size, key, key_len) ;
        cookie ->curr_size += key_len ;
        cookie ->cookie_buff[cookie ->curr_size] = '=' ;
        cookie ->curr_size += 1 ;
    }

    memcpy(cookie ->cookie_buff + cookie ->curr_size, value, val_len) ;
    cookie ->curr_size += val_len ;
    cookie ->cookie_buff[cookie ->curr_size] = ';' ;
    cookie ->curr_size += 1 ;
    cookie ->cookie_buff[cookie ->curr_size] = '\0' ;
    return 0 ;
}

int delete_key(http_cookie_t *cookie, const char *key) 
{  
    if(cookie == NULL || key == NULL)
    {
        return -1 ;
    }
    char *key_b = NULL ;
    char *val_b = NULL ;
    char *val_e = NULL ;
    search(cookie, key, &key_b, &val_b, &val_e)  ;
    if(key_b == NULL || val_b == NULL || val_e == NULL)
    {
        return 0 ;
    }
    // 1  ';'
    memcpy(key_b, val_e + 1, cookie ->curr_size - (val_e - cookie ->cookie_buff + 1)) ;  
    cookie ->curr_size -= val_e - key_b + 1 ;
    cookie ->cookie_buff[cookie ->curr_size] = '\0' ;
    return 0 ;
}

int update_key(http_cookie_t *cookie, const char *key, const char *new_value) 
{
    if(cookie == NULL || key == NULL || new_value == NULL)
    {
        return -1 ;
    }
    delete_key(cookie, key) ;
    return add_key_value(cookie, key, new_value) ;
}

int set_path(http_cookie_t *cookie, const char *path)
{
    return add_key_value(cookie, "path", path) ;
}

int set_domain(http_cookie_t *cookie, const char *domain)
{
    return add_key_value(cookie, "domain", domain) ;
}

int add_secure_option(http_cookie_t *cookie) 
{
    return add_key_value(cookie, NULL, "secure") ;
}

int del_secure_option(http_cookie_t *cookie)
{
    return del_option(cookie, "secure") ;
}

int add_httponly_option(http_cookie_t *cookie)
{
    return add_key_value(cookie, NULL, "httponly") ;
}

int del_httponly_option(http_cookie_t *cookie)
{
    return del_option(cookie, "httponly") ;
}

//获取键对应的值
char *copy_value(http_cookie_t *cookie, const char *key, char *val) 
{
    if(cookie == NULL || key == NULL || val == NULL)
    {
        return NULL;
    }

    int key_len = strlen(key) ;
    char *key_b = strstr(cookie ->cookie_buff, key) ;
    char *val_b = NULL ;
    char *val_e = NULL ;
    while(key_b != NULL)
    {
        if(key_b[key_len] == '=')
        {
            val_b = key_b + key_len + 1 ;
            val_e = strchr(val_b, ';') ; 
            val_e = val_e == NULL ? val_b + strlen(key_b): val_e;
            break ;
        }
        else
        {
            key_b = strstr(key_b + 1, key) ;
        }
    }
    if(key_b == NULL)
    {
        return NULL; 
    }

    return memcpy(val, val_b ,val_e - val_b);
}

const char *cookie2String(http_cookie_t *cookie)
{
    return cookie != NULL ?cookie ->cookie_buff :NULL ;
}

