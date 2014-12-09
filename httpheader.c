#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "global.h"
#include "util.h"
#include "httpheader.h"

#define DEFAULT_GROW_SIZE  256

int init_http_header(http_request_header_t *httphdr)
{
    if(httphdr == NULL)
    {
        return -1 ; 
    }
    httphdr ->curr_sz = 0 ;
    httphdr ->size = 0 ;
    httphdr ->hdr_buff = 0 ;
    return 0 ;
}

int free_http_header(http_request_header_t *httphdr)
{
    if(httphdr == NULL)
    {
        return -1; 
    }
    FREE(httphdr ->hdr_buff) ;
    httphdr ->hdr_buff = NULL ;
    httphdr ->curr_sz = 0 ;
    httphdr ->size = 0 ;
}

static int get_grow(http_request_header_t *httphdr)
{
    return DEFAULT_GROW_SIZE ;
}

static http_request_header_t * realloc_header(http_request_header_t *httphdr, uint sz)
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
    if(httphdr ->hdr_buff != NULL)
    {
        memcpy(tmp, httphdr ->hdr_buff, httphdr ->curr_sz) ;
        FREE(httphdr ->hdr_buff) ;
    }
    httphdr ->hdr_buff = tmp ;
    httphdr ->size = sz ;
    return httphdr ;
}

static int search(http_request_header_t *httphdr, const char *key, char **key_b, char **val_b, char **val_e)
{
    int key_len = strlen(key) ;
    *key_b = strcasestr(httphdr ->hdr_buff , key) ; 
    while(*key_b != NULL)
    {
        if((*key_b)[key_len] == ':' && (*key_b == httphdr ->hdr_buff || (*key_b)[-1] == '\n')) 
        {
            *val_b = *key_b + key_len + 1; 
            *val_e = strstr(*val_b, "\r\n") ;
            *val_e = (*val_e != NULL ? *val_e : NULL ) ;
            break ;
        }
        else
        {
           *key_b = strcasestr(*key_b + key_len + 1, key) ; 
        }
    }
    return 0 ;
}

int add_header(http_request_header_t *httphdr, const char *key, const char *val)
{
    if(httphdr == NULL || key == NULL || val == NULL) 
    {
        return -1 ; 
    }
    int key_len = strlen(key) ;
    int val_len = strlen(val) ;
    //  len(":") == 1 len("\r\n") == 2
    if(httphdr ->curr_sz + key_len + val_len + 3 >= httphdr ->size)
    {
        if(realloc_header(httphdr, httphdr ->curr_sz + key_len + val_len + 3 + get_grow(httphdr)) == NULL) 
        {
            return -1 ;
        }
    }
    memcpy(httphdr ->hdr_buff + httphdr ->curr_sz, key, key_len) ;
    httphdr ->curr_sz += key_len ;
    httphdr ->hdr_buff[httphdr ->curr_sz] = ':' ;
    httphdr ->curr_sz += 1 ;
    memcpy(httphdr ->hdr_buff + httphdr ->curr_sz, val, val_len) ;
    httphdr ->curr_sz += val_len ;
    strcpy(httphdr ->hdr_buff + httphdr ->curr_sz, "\r\n") ;
    httphdr ->curr_sz += 2 ;
    return 0 ;
}

int delete_header(http_request_header_t *httphdr, const char *key)
{
    if(httphdr == NULL || httphdr ->hdr_buff == NULL || key == NULL)
    {
        return -1 ;
    }
    int key_len = strlen(key) ;
    char *key_b = NULL ;
    char *val_b = NULL ;
    char *val_e = NULL ;
    search(httphdr, key, &key_b, &val_b, &val_e) ;
    if(key_b == NULL || val_b == NULL || val_e == NULL)
    {
        return -1 ;
    }
    memcpy(key_b, val_e + 2, httphdr ->curr_sz - (val_e - httphdr ->hdr_buff + 2)) ;  
    httphdr ->curr_sz -= val_e - key_b + 2;
    httphdr ->hdr_buff[httphdr ->curr_sz] = '\0' ;
    return 0 ;
}

int update_header(http_request_header_t *httphdr, const char *key, const char *new_value)
{
    if(httphdr == NULL || key == NULL || new_value == NULL) 
    {
        return -1 ;
    }
    return delete_header(httphdr, key) || add_header(httphdr, key, new_value) ;
}

const char *header2String(http_request_header_t *httphdr) 
{
    return httphdr != NULL ? (httphdr ->curr_sz == 0 ? null_str_ptr : httphdr ->hdr_buff) : null_str_ptr ;
}

uint header_len(http_request_header_t *httphdr)
{
    return httphdr != NULL ? httphdr ->curr_sz : 0 ;
}

int has_header(http_request_header_t *httphdr, const char *key) 
{
    if(httphdr == NULL || key == NULL || httphdr ->hdr_buff == NULL)
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

