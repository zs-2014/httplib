#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "urlencode.h"
#include "data.h"
#include "global.h"

#define DEFAULT_DATA_SIZE 10

static data_t* realloc_data(data_t *data, uint sz)
{
    if(data == NULL)
    {
        return NULL ;
    }
    if(data ->size > sz)
    {
        return data;
    }
    __DATA *tmp = (__DATA *)MALLOC(sizeof(__DATA) * sz) ;
    if(tmp == NULL)
    {
        return NULL ;
    }
    if(data ->data != NULL)
    {
        memcpy(tmp, data ->data, sizeof(__DATA)*data ->curr_sz)  ;
        FREE(data ->data) ;
    }
    data ->data = tmp ;
    data ->size = sz ;
    return data;
}

static int get_grow(data_t *data)
{
    return DEFAULT_DATA_SIZE ;
}

static int search(data_t *data, const char *key)
{
    int i = 0 ;
    for(i = 0 ;i < data ->curr_sz ;i++)
    {
       if(data ->data[i].key != NULL && strcmp(data ->data[i].key, key) == 0) 
       {
            return i ;
       }
    }
    return -1 ;
}

static int  __free_data(__DATA *data)
{
    if(data != NULL)
    {
        if(data ->key != NULL)
        {
            FREE(data ->key) ;
        }
        if(data ->val != NULL)
        {
            FREE(data ->val) ;
        }
    }
    return 0 ;
}

int init_data(data_t *data) 
{
    if(data == NULL)
    {
        return -1 ; 
    }
    data ->data = NULL ;
    data ->curr_sz = 0 ;
    data ->size = 0 ;
    return 0 ;
}

int free_data(data_t *data)
{
    int i = 0 ;
    for(i = 0 ; i < data ->curr_sz; i++)
    {
        __free_data(data ->data + i) ;
    }
    FREE(data ->data) ;
    data ->data = NULL ;
    return 0 ;
}
int add_data(data_t *data, const uchar *key, int key_sz, const uchar *val, int val_sz) 
{
    if(data == NULL || key == NULL || key_sz == 0 || val == NULL) 
    {
        return -1 ;
    }
   
    if(data ->curr_sz >= data ->size && realloc_data(data, data ->curr_sz + get_grow(data)) == NULL)
    {
        return -1 ;
    }
    data ->data[data ->curr_sz].key = quotebuff(key, key_sz, NULL) ;
    data ->data[data ->curr_sz].val = quotebuff(val, val_sz, NULL) ;
    data ->curr_sz += 1;
    data ->data_len += key_sz + val_sz ;
}

int delete_data(data_t *data, const uchar *key, int key_sz) 
{
    if(data == NULL || key == NULL) 
    {
        return -1 ;
    }
    char *quote_key = quotebuff(key, key_sz, NULL) ;
    uint i = search(data, quote_key) ;
    if(i == -1)
    {
        return 0 ;
    }
    __free_data(data ->data + i ) ;
    memcpy(data ->data + i, data ->data + i + 1, (data ->curr_sz - i - 1)*sizeof(__DATA)) ;
    FREE(quote_key) ;
    data ->curr_sz -= 1 ; 
    return 0 ;
}

int update_data(data_t *data, const uchar *key, int key_sz, const uchar *val, int val_sz) 
{
    if(data == NULL || key == NULL || val == NULL)
    {
        return -1 ;
    }
    char *quote_key = quotebuff(key, key_sz, NULL) ;
    uint i = 0 ;
    if(i == -1)
    {
        FREE(quote_key) ;
        return -1;  
    }
    FREE(data ->data[i].val) ;
    FREE(quote_key) ;
    data ->data[i].val = quotebuff(val, val_sz, NULL) ;
    return 0 ;
}

int is_empty(data_t *data)
{
    if(data == NULL || data ->curr_sz == 0)
    {
        return 1 ;
    }
    return 0 ;
}
