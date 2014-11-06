#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cookie.h"

#define  DEFAULT_COOKIE_BUFF_SIZE 128 

//如果申请空间失败，则原空间不会释放
static char *reallocCookie(COOKIE *cookie, uint sz)
{
    if(cookie == NULL)
    {
        return NULL ;
    }

    if(cookie ->size > sz)
    {
        return cookie ->cookieBuff ;
    }

    char *tmp = (char *)CALLOC(1, (sz*sizeof(char))) ;
    if(tmp == NULL )
    {
         return NULL ;
    } 

    if(cookie ->cookieBuff != NULL)
    {
        memcpy(tmp, cookie ->cookieBuff, cookie ->currSize) ;
        FREE(cookie ->cookieBuff) ;
    }
    cookie ->size = sz ;
    cookie ->cookieBuff = tmp ;
    return tmp ;
}

//获取增加空间的大小
static uint getGrow(COOKIE *cookie)
{
    return DEFAULT_COOKIE_BUFF_SIZE ;
}

static int search(COOKIE *cookie, const char *key, char **key_b, char **val_b, char **val_e)
{
    int keyLen = strlen(key) ;
    *key_b = strstr(cookie ->cookieBuff, key) ;
    while(*key_b != NULL) 
    {
        if((*key_b)[keyLen] == '=' && (*key_b == cookie ->cookieBuff || ((*key_b)[-1] == ';'))) 
        {
            *val_b = *key_b + keyLen + 1 ;
            *val_e = strchr(*val_b, ';') ; 
            *val_e = *val_e == NULL ? NULL : *val_e;
            break ;
        }
        else
        {
            *key_b = strstr(*key_b + keyLen + 1, key) ;
        }
    }
    return 0 ;
}

static delOption(COOKIE *cookie, const char *option)
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

    char *option_b = strstr(cookie ->cookieBuff, option) ;
    while(option_b != NULL)
    {
        if(option_b != cookie ->cookieBuff)
        {
            if(option_b[-1] == ';' && option_b[oplen] == ';')
            {
                memcpy(option_b, option_b + oplen + 1, cookie ->currSize - (option_b - cookie ->cookieBuff + oplen + 1)) ;
                cookie ->currSize -= oplen + 1 ;
                cookie ->cookieBuff[cookie ->currSize] = '\0' ;
                return 0 ; 
            }
            else
            {
                option_b = strstr(option_b + 1, option) ;
            }
        }
        else
        {
           memset(option_b, 0, oplen) ; 
           cookie ->currSize = 0 ;
           return 0 ;
        }
    }
    return 0 ;
}



int initCookie(COOKIE *cookie)
{
    if(cookie == NULL) 
    {
            return -1 ;
    }
    //cookie ->cookieBuff = (char *)malloc(sizeof(char)*DEFAULT_COOKIE_BUFF_SIZE) ;
    //if(cookie ->cookieBuff == NULL)
    //{
    //    return -1 ;
    //}
    cookie ->cookieBuff = NULL ;
    cookie ->size = 0 ;
    cookie ->currSize = 0 ;
    return 0 ;
}

COOKIE *cookieCopy(COOKIE *dst, const COOKIE *src)
{
    if(src == NULL || dst == NULL)
    {
        return NULL ; 
    }
    if(dst ->size <= src ->currSize && reallocCookie(dst, src ->size) == NULL)
    {
        return NULL ;
    }

    memcpy(dst ->cookieBuff, src ->cookieBuff, src ->currSize) ;
    dst ->currSize = src ->currSize;
    return dst ;
}

int freeCookie(COOKIE *cookie)
{
    if(cookie == NULL && cookie ->cookieBuff != NULL)
    {
        FREE(cookie ->cookieBuff) ;
        cookie ->cookieBuff = NULL ; 
        cookie ->size = 0 ;
        cookie ->currSize = 0 ;
    }
    return 0 ;
}

int addKeyValue(COOKIE *cookie, const char *key, const char *value) 
{
    if(cookie == NULL || value == NULL)
    {
        return -1 ;
    }
    uint keyLen = (key != NULL ?strlen(key):0) ;
    uint valLen = strlen(value) ;

    //len(";=") == 1
    if(keyLen + valLen + 2 + cookie ->currSize >=  cookie ->size )
    {
        //空间不够，重新增加空间
        if(reallocCookie(cookie, keyLen + valLen + 2 + cookie ->currSize + getGrow(cookie)) == NULL)
        {
            return -1 ;
        }
    }
    if(keyLen != 0)
    { 
        memcpy(cookie ->cookieBuff + cookie ->currSize, key, keyLen) ;
        cookie ->currSize += keyLen ;
        cookie ->cookieBuff[cookie ->currSize] = '=' ;
        cookie ->currSize += 1 ;
    }

    memcpy(cookie ->cookieBuff + cookie ->currSize, value, valLen) ;
    cookie ->currSize += valLen ;
    cookie ->cookieBuff[cookie ->currSize] = ';' ;
    cookie ->currSize += 1 ;
    cookie ->cookieBuff[cookie ->currSize] = '\0' ;
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
    memcpy(key_b, val_e + 1, cookie ->currSize - (val_e - cookie ->cookieBuff + 1)) ;  
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
    //deleteKey(cookie, key) ;
    //addKeyValue(cookie, key, newValue) ;
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

int addSecureOption(COOKIE *cookie) 
{
    return addKeyValue(cookie, NULL, "secure") ;
}

int delSecureOption(COOKIE *cookie)
{
    return delOption(cookie, "secure") ;
}

int addHttponlyOption(COOKIE *cookie)
{
    return addKeyValue(cookie, NULL, "httponly") ;
}

int delHttponlyOption(COOKIE *cookie)
{
    return delOption(cookie, "httponly") ;
}

//获取键对应的值
char *copyValue(COOKIE *cookie, const char *key, char *val) 
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

const char *cookie2String(COOKIE *cookie)
{
    return cookie != NULL ?cookie ->cookieBuff :NULL ;
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
    COOKIE cookie ;
    initCookie(&cookie) ;
    addKeyValue(&cookie, "path", "/") ;
    addKeyValue(&cookie, "name1", "zs1") ;
    addKeyValue(&cookie, "name", "test1 for update Test") ;
    printCookie(&cookie) ;

    updateKey(&cookie, "name", "test1 for update Test") ;
    printCookie(&cookie) ;
    
    updateKey(&cookie, "name", "test1 for update Test  xxxxxxx") ;
    printCookie(&cookie) ;

    updateKey(&cookie, "name", "") ;
    printCookie(&cookie) ;

    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    updateKey(&cookie, "name", buff);
    printCookie(&cookie) ;
    freeCookie(&cookie) ;
}

void delTest()
{
    COOKIE cookie ;
    initCookie(&cookie) ;
    addKeyValue(&cookie, "path", "/") ;
    addKeyValue(&cookie, "name1", "zs1") ;
    addKeyValue(&cookie, "name", "test1 for update Test") ;
    printCookie(&cookie) ;

    deleteKey(&cookie, "name") ;
    printCookie(&cookie) ;
    
    updateKey(&cookie, "name", "test1 for update Test  xxxxxxx") ;
    printCookie(&cookie) ;

    deleteKey(&cookie, "name") ;
    printCookie(&cookie) ;

    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    deleteKey(&cookie, "name");
    printCookie(&cookie) ;

    freeCookie(&cookie) ;

}

void addTest()
{
    COOKIE cookie ;
    initCookie(&cookie) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    addKeyValue(&cookie, "name", "val") ;
    printCookie(&cookie) ;
    addKeyValue(&cookie, "key", "value") ;
    printCookie(&cookie) ;
    addKeyValue(&cookie, "key", buff) ;
    printCookie(&cookie) ;

    freeCookie(&cookie) ;
}

void OptionTest()
{
    COOKIE cookie ;
    initCookie(&cookie) ;
    addKeyValue(&cookie, "name", "zs") ;
    addSecureOption(&cookie) ;
    printCookie(&cookie);
    addHttponlyOption(&cookie) ;
    printCookie(&cookie) ;
    addKeyValue(&cookie, "name1", "zs") ;

    delHttponlyOption(&cookie) ;
    printCookie(&cookie) ;
    delSecureOption(&cookie) ;
    printCookie(&cookie) ;

    freeCookie(&cookie) ;
}
void getValTest()
{
    COOKIE cookie;
    initCookie(&cookie) ;
    addKeyValue(&cookie, "name", "value") ;
    addKeyValue(&cookie, "name1", "value1") ;
    addKeyValue(&cookie, "name2", "value2") ;
    char buff[30] = {0} ;
    char buff1[30] = {0} ;
    char buff2[30] = {0} ;
    char buff3[30] = {0} ;
    printf("name = %s\nname1 = %s\nname2 = %s\nname3=%s\n", copyValue(&cookie, "name", buff), copyValue(&cookie, "name1", buff1), copyValue(&cookie, "name2", buff2), copyValue(&cookie, "name3", buff3)) ;

}
int main(int argc, char *argv[])
{
    updateTest() ;
    delTest() ;
    addTest() ;
    getValTest() ;
    OptionTest() ;
    return 0 ;
}

#endif
