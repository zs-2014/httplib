#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cookie.h"

#define  DEFAULT_COOKIE_BUFF_SIZE 512

//如果申请空间失败，则原空间不会释放
static char *reallocCookie(COOKIE *cookie, uint sz)
{
    if(cookie == NULL || cookie ->size > sz)
    {
        return NULL ;
    }

    char *tmp = (char *)CALLOC(1, (sz*sizeof(char))) ;
    if(tmp == NULL )
    {
         return NULL ;
    } 

    memcpy(tmp, cookie ->cookieBuff, cookie ->currSize) ;
    FREE(cookie ->cookieBuff) ;
    cookie ->size = sz ;
    cookie ->cookieBuff = tmp ;
    return tmp ;
}
//获取增加空间的大小
static uint getGrow(COOKIE *cookie)
{
    return DEFAULT_COOKIE_BUFF_SIZE ;
}

int freeCookie(COOKIE *cookie)
{
    if(cookie == NULL)
    {
        return 0 ;
    }
    if(cookie ->cookieBuff != NULL)
    {
        FREE(cookie ->cookieBuff) ;
        cookie ->cookieBuff = NULL ; 
        cookie ->size = 0 ;
    }
    FREE(cookie) ;
    cookie = NULL ;
    return 0 ;
}

COOKIE *mallocCookie(uint buffSz) 
{ 
    COOKIE *cookie = (COOKIE *)MALLOC(sizeof(COOKIE)) ;
    if(cookie == NULL)
    {
        return NULL ;
    }
    cookie ->currSize = 0 ;
    cookie ->size = sizeof(char)*(buffSz == 0 ? DEFAULT_COOKIE_BUFF_SIZE : buffSz) ;
    cookie ->cookieBuff = (char *)CALLOC(1, cookie ->size) ;
    if(cookie ->cookieBuff == NULL)
    {
        freeCookie(cookie) ;
        return NULL ;
    }
    return cookie;
}

int addKeyValue(COOKIE *cookie, const char *key, const char *value) 
{
    if(cookie == NULL || key == NULL || value == NULL)
    {
        return -1 ;
    }
    uint keyLen = strlen(key) ;
    uint valLen = strlen(value) ;
    if(keyLen == 0)
    {
        return -1 ;
    }

    //len(";=") == 1
    if(keyLen + valLen + 2 + cookie ->currSize >=  cookie ->size )
    {
        //空间不够，重新增加空间
        if(reallocCookie(cookie, keyLen + valLen + 2 + cookie ->currSize + getGrow(cookie)) == NULL)
        {
            return -1 ;
        }
    }
    memcpy(cookie ->cookieBuff + cookie ->currSize, key, keyLen) ;
    cookie ->currSize += keyLen ;
    cookie ->cookieBuff[cookie ->currSize] = '=' ;
    cookie ->currSize += 1 ;

    memcpy(cookie ->cookieBuff + cookie ->currSize, value, valLen) ;
    cookie ->currSize += valLen ;
    cookie ->cookieBuff[cookie ->currSize] = ';' ;
    cookie ->currSize += 1 ;
    cookie ->cookieBuff[cookie ->currSize] = '\0' ;
    return 0 ;
}

static int search(COOKIE *cookie, const char *key, char **key_b, char **val_b, char **val_e)
{
    int keyLen = strlen(key) ;
    *key_b = strstr(cookie ->cookieBuff, key) ;
    while(*key_b != NULL) 
    {
        if((*key_b)[keyLen] == '=' ) 
        {
            *val_b = *key_b + keyLen + 1 ;
            *val_e = strchr(*val_b, ';') ; 
            *val_e = *val_e == NULL ? *val_b + strlen(*key_b) : *val_e;
            break ;
        }
        else
        {
            *key_b = strstr(*key_b + 1, key) ;
        }
    }
    return 0 ;
}

int deleteKey(COOKIE *cookie, const char *key) 
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
    memcpy(key_b, val_e + 1, cookie ->currSize - (val_e - cookie ->cookieBuff)) ;  
    cookie ->currSize -= val_e - key_b + 1 ;
    cookie ->cookieBuff[cookie ->currSize] = '\0' ;
    return 0 ;
}

int updateKey(COOKIE *cookie, const char *key, const char *newValue) 
{
    if(cookie == NULL || key == NULL || newValue == NULL)
    {
        return -1 ;
    }
    
    char *key_b = NULL ; 
    uint keyLen = strlen(key) ;
    char *val_b = NULL ;
    char *val_e = NULL ;
    search(cookie, key, &key_b, &val_b, &val_e) ;
    if(key_b == NULL)
    {
        return addKeyValue(cookie, key, newValue) ;
    }
    int valLen = val_e - val_b ;
    int newValLen = strlen(newValue) ;
    if(cookie ->currSize + newValLen >= cookie ->size)
    {
        if(reallocCookie(cookie, cookie ->currSize + newValLen + getGrow(cookie)) == NULL)
        {
            return -1 ;
        }
        return updateKey(cookie, key, newValue) ;
    }
    // hello=world;xx=xxxx   ----->hello=word;xx=xxxx  currSize=12
    //       |    |
    //   val_b    val_e (val_e - val_b == 5) 
    //                                                 val_e
    //                                                 |
    //memcpy(val_b, newVal, newValLen)  --->hello=wordd;xx=xxxx
    //                                                |
    //                                                (val_b + newValLen)
    //                                                val_e - cookie ->cookieBuff == 11  
    //                                      hello=wordd;  ->hello=word;xx=xxxxx
    //memcpy(val_b + newValLen, val_e, cookie ->currSize - (val_e - cookie ->cookieBuff)) ;
    //cookie ->cookieBuff[cookie ->currSize] = '\0' ---> hello=word;xx=xxxx
    if(valLen > newValLen)
    {
        memcpy(val_b, newValue, newValLen) ; 
        memcpy(val_b + newValLen, val_e, cookie ->currSize - (val_e - cookie ->cookieBuff)) ;
        cookie ->currSize -= valLen - newValLen ;
        cookie ->cookieBuff[cookie ->currSize] = '\0';
        return 0 ;
    }
    else if(valLen == newValLen)
    {
        memcpy(val_b, newValue, valLen) ; 
        return 0 ;
    }
    else
    {
        char *end = cookie ->cookieBuff + cookie ->currSize ;  
        while(end >= val_e)
        {
           end[newValLen-valLen] = *end ;
           end-- ;
        }
        memcpy(val_b, newValue, newValLen) ;
        cookie ->currSize += newValLen - valLen ; 
    }
    return 0 ;
}

//获取键对应的值
char *getValue(COOKIE *cookie, const char *key, char *val) 
{
    if(cookie == NULL || key == NULL || val == NULL)
    {
        return NULL;
    }

    int keyLen = strlen(key) ;
    char *key_b = strstr(cookie ->cookieBuff, key) ;
    char *val_b = NULL ;
    char *val_e = NULL ;
    while(key_b != NULL)
    {
        if(key_b[keyLen] == '=')
        {
            val_b = key_b + keyLen + 1 ;
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

int printCookie(COOKIE *cookie)
{
    if(cookie == NULL)
    {
        return -1 ;
    }
    printf("cookiebuff = %s\n", cookie ->cookieBuff) ;
    return 0 ;
}

#if 1
void updateTest()
{
    COOKIE *cookie = mallocCookie(10) ;
    addKeyValue(cookie, "path", "/") ;
    addKeyValue(cookie, "name1", "zs1") ;
    addKeyValue(cookie, "name", "test1 for update Test") ;
    printCookie(cookie) ;

    updateKey(cookie, "name", "test1 for update Test") ;
    printCookie(cookie) ;
    
    updateKey(cookie, "name", "test1 for update Test  xxxxxxx") ;
    printCookie(cookie) ;

    updateKey(cookie, "name", "") ;
    printCookie(cookie) ;

    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    updateKey(cookie, "name", buff);
    printCookie(cookie) ;
    freeCookie(cookie) ;
}

void delTest()
{
    COOKIE *cookie = mallocCookie(10) ;
    addKeyValue(cookie, "path", "/") ;
    addKeyValue(cookie, "name1", "zs1") ;
    addKeyValue(cookie, "name", "test1 for update Test") ;
    printCookie(cookie) ;

    deleteKey(cookie, "name") ;
    printCookie(cookie) ;
    
    updateKey(cookie, "name", "test1 for update Test  xxxxxxx") ;
    printCookie(cookie) ;

    deleteKey(cookie, "name") ;
    printCookie(cookie) ;

    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    deleteKey(cookie, "name");
    printCookie(cookie) ;

    freeCookie(cookie) ;

}

void addTest()
{
    COOKIE *cookie = mallocCookie(10) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    addKeyValue(cookie, "name", "val") ;
    printCookie(cookie) ;
    addKeyValue(cookie, "key", "value") ;
    printCookie(cookie) ;
    addKeyValue(cookie, "key", buff) ;
    printCookie(cookie) ;

}
int main(int argc, char *argv[])
{
    updateTest() ;
    delTest() ;
    addTest() ;
    return 0 ;
}

#endif
