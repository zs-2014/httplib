#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "urlencode.h"
#include "data.h"
#include "global.h"

#define DEFAULT_DATA_SIZE 10

static DATA* reallocData(DATA *data, uint sz)
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
        memcpy(tmp, data ->data, sizeof(__DATA)*data ->currSz)  ;
        FREE(data ->data) ;
    }
    data ->data = tmp ;
    data ->size = sz ;
    return data;
}

static int getGrow(DATA *data)
{
    return DEFAULT_DATA_SIZE ;
}

static int search(DATA *data, const char *key)
{
    int i = 0 ;
    for(i = 0 ;i < data ->currSz ;i++)
    {
       if(data ->data[i].key != NULL && strcmp(data ->data[i].key, key) == 0) 
       {
            return i ;
       }
    }
    return -1 ;
}

static int  __freeData(__DATA *data)
{
    if(data != NULL)
    {
        FREE(data ->key) ;
        FREE(data ->val) ;
    }
    return 0 ;
}

int initData(DATA *data) 
{
    if(data == NULL)
    {
        return -1 ; 
    }
    data ->data = NULL ;
    data ->currSz = 0 ;
    data ->size = 0 ;
    return 0 ;
}

int freeData(DATA *data)
{
    int i = 0 ;
    for(i = 0 ; i < data ->currSz; i++)
    {
        if(data ->data[i].key)
        {
            FREE(data ->data[i].key) ;
        }
        if(data ->data[i].val)
        {
            FREE(data ->data[i].val) ;
        }
    }
    FREE(data ->data) ;
    data ->data = NULL ;
    return 0 ;
}
int addData(DATA *data, const uchar *key, int keySz, const uchar *val, int valSz) 
{
    if(data == NULL || key == NULL || keySz == 0 || val == NULL || valSz == 0) 
    {
        return -1 ;
    }
   
    if(data ->currSz >= data ->size && reallocData(data, data ->currSz + getGrow(data)) == NULL)
    {
        return -1 ;
    }
    data ->data[data ->currSz].key = quotebuff(key, keySz) ;
    data ->data[data ->currSz].val = quotebuff(val, valSz) ;
    data ->currSz += 1;
}

int deleteData(DATA *data, const uchar *key, int keySz) 
{
    if(data == NULL || key == NULL) 
    {
        return -1 ;
    }
    char *quoteKey = quotebuff(key, keySz) ;
    uint i = search(data, quoteKey) ;
    if(i == -1)
    {
        return 0 ;
    }
    __freeData(data ->data + i ) ;
    memcpy(data ->data + i, data ->data + i + 1, (data ->currSz - i - 1)*sizeof(__DATA)) ;
    FREE(quoteKey) ;
    data ->currSz -= 1 ; 
    return 0 ;
}

int updateData(DATA *data, const uchar *key, int keySz, const uchar *val, int valSz) 
{
    if(data == NULL || key == NULL || val == NULL)
    {
        return -1 ;
    }
    char *quoteKey = quotebuff(key, keySz) ;
    uint i = 0 ;
    if(i == -1)
    {
        FREE(quoteKey) ;
        return -1;  
    }
    FREE(data ->data[i].val) ;
    FREE(quoteKey) ;
    data ->data[i].val = quotebuff(val, valSz) ;
    return 0 ;
}

int isEmpty(DATA *data)
{
    if(data == NULL || data ->currSz == 0)
    {
        return 1 ;
    }
    return 0 ;
}

void printData(DATA *data)
{
    if(data == NULL)
    {
        return  ;
    }
    //int i = 0 ;
    //for(i = 0 ;i < data ->currSz ;i++)
    //{
    //    printf("key:%s\nval=%s\n", data->data[i].key, data ->data[i].val) ;
    //}
    FOREACH(key, val, data)
    {
        printf("key:%s\nval=%s\n", key,val) ;
    }
}
#if 1
void AddData()
{
    DATA data ;
    initData(&data) ;
    int i = 0 ;
    for(i = 0 ;i < 20 ;i++)
    {
        addData(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
        addData(&data, "test", strlen("test"), "this is a test", strlen("this is a test")) ;
    }
    printData(&data) ;
    freeData(&data) ;
}
void delData()
{
    DATA data ;
    initData(&data) ;
    addData(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
    deleteData(&data, "测试", strlen("测试")) ;
    deleteData(&data, "测试", strlen("测试")) ;
    addData(&data, "test", strlen("test"), "this is a test", strlen("this is a test")) ;
    addData(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
    printData(&data) ;
    freeData(&data) ;
}
void update()
{
    DATA data ;
    initData(&data) ;
    addData(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
    updateData(&data, "测试", strlen("测试"), "this is a test", strlen("this is a test")) ;
    printData(&data) ;
    freeData(&data) ;

}
int main(int argc, char *argv[])
{
    //AddData() ; 
    //delData() ;
    update() ;
    return 0 ;
}
#endif
