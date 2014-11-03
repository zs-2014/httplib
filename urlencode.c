#include <string.h>
#include <stdlib.h>

#include "urlencode.h"

#define SAFE     0X00000001
#define UNSAFE   0X00000010
#define RESERVED 0X00000100
#define IS_SAFE(x) (map[x]&SAFE)
#define IS_UNSAFE(x) (map[x]&UNSAFE)
#define IS_RESERVED(x) (map[x]&RESERVED)

static uchar map[256] = {0};

static char *safe_char = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$-_.+!*'()" ;
static char *reserved_char = ";/?:@=&" ;
static char convert_map[] = "0123456789ABCDEFabcdef" ; 

static int init()
{
    if(IS_UNSAFE(255))
    {
        return 0 ; 
    }
    char *p = safe_char ;
    while(*p != '\0')
    {
        map[*p++] = (uchar)SAFE ;
    }

    p = reserved_char ;
    while(*p != '\0')
    {
        map[*p++] = (uchar)RESERVED ;
    }
    int i = 0 ;
    for (i = 0 ;i < sizeof(map) ;i++)
    {
        if(map[i] != SAFE && map[i] != RESERVED)  
        {
            map[i] = (uchar)UNSAFE ;
        }
    }
    return 0 ;
}

char* quote(const uchar* urlstr, uint sz)
{
    if(urlstr == NULL||sz == 0)
    {
        return NULL ;
    }
    init() ;
    char *pout = (char *)MALLOC(sz*sizeof(uchar)*3 + 1) ;
    char *ptmp = pout ;
    if (pout == NULL)
    {
        return NULL ;
    }

    while(sz-- > 0)
    {
       if(IS_SAFE(*urlstr))  
       {
            *pout++ = (char)*urlstr++ ;
            continue ;
       }
       else
       {
            *pout++ = '%' ;
            *pout++ = convert_map[*urlstr/16] ;    
            *pout++ = convert_map[*urlstr%16] ;
            urlstr++ ;
       }
    }
    *pout = '\0' ;
    return ptmp; 
}

uchar *unquote(const char *urlstr, uint sz)
{
    if(urlstr == NULL||sz == 0)
    {
        return NULL ;
    }
    init() ;
    uchar *pout = (uchar *)MALLOC(sz*sizeof(uchar)+1) ;
    uchar *ptmp = pout ;
    if (pout == NULL)
    {
        return NULL ;
    }
    while(sz > 0)
    {
        if(*urlstr == '%')//以%开头的需要还原 
        {
            urlstr++ ;
            sz-- ;
            if(sz > 0 &&IS_SAFE(*urlstr))//如果%后面的是安全字符      
            {
                    int i = 0 ;
                    uchar n1 = 0 ;
                    for(i = 0 ;i < 2&&sz > 0 ;i++, urlstr++, sz--)//计算十六进制的值
                    {
                        if (!isxdigit(*urlstr) || !isxdigit(*urlstr)) //如果不是十六进制字符
                        {
                            pout = NULL ;
                            break ;
                        } 
                        if(*urlstr >= '0' && *urlstr <= '9')
                        {
                            n1 = n1*16 + *urlstr - '0' ;
                        }
                        else if(*urlstr >= 'A' && *urlstr <= 'F')
                        {
                            n1 = n1*16 +  *urlstr - 'A'  + 10 ;
                        }
                        else if (*urlstr >= 'a' && *urlstr <= 'z')
                        {
                            n1 = n1*16 + *urlstr - 'a' + 10 ;
                        }
                   } 
                   if(pout == NULL||i != 2)//% 后面跟的字符不合法，比如%XY
                   {
                        pout = NULL ;
                        break ;
                   }
                   else
                   {
                        *pout++ = n1 ;
                   }
            }
            else//% 后面必须跟着十六进制字符
            {
                pout = NULL ;
                break ;
             }
        }
        else//不是%号则直接拷贝，无需转义
        {
            sz-- ;
            *pout++ = *urlstr++ ;
        }
    }

    if (pout == NULL)
    {
        FREE(ptmp) ;
        return NULL ;
    }
    *pout = '\0' ;
    return ptmp;
}


char *urlencode(const char *str)
{
    return quotestr(str) ;
}

#if 0
int main(int argc, char *argv[])
{
    
    return 0 ;
}

#endif
