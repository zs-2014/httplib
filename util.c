#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include "util.h"

#define PRINT_ERR_MSG(Text) \
	printf("%s:%s:%d:%s:%s\n",__FILE__,__func__,__LINE__,(Text),strerror(errno));
#define PRINT_ARGUMENT_ERR_MSG(msg)\
	printf("%s:%s:%d:%s\n",__FILE__,__func__,__LINE__,(msg));

static char *replacestr_equal(char *src,
						int srcLen,
						const char *patternStr,
						const char *replaceStr,
						int len)
{
		if(strcmp(patternStr,replaceStr) == 0)
			return src ;
		int i = 0 ;	
		//for(i = 0 ;i < srcLen ;)
		while(i < srcLen )
		{
			if(strncmp(src + i ,patternStr,len) == 0)
			{
				strncpy(src + i,replaceStr,len);
				i += len ;
				continue ;
			}
			i++ ;
		}
		return src ;
}

//strlen(replaceStr) < strlen(patternStr)
static char *replacestr_less(char *src,
						int srcLen ,
						const char *patternStr,
						int patternStrLen,
						const char *replaceStr,
						int replaceStrLen)
{
	int i = 0 ;
	int count = 0 ;
	//for(i = 0 ;i < srcLen ; )
	while(i < srcLen )
	{
		if(strncmp(src + i,patternStr,patternStrLen) == 0)
		{
			strncpy(src + count,replaceStr,replaceStrLen);
			count += replaceStrLen ;
			i += patternStrLen ;		
			continue ;
		}
		src[count++] = src[i++] ;	
	}
	src[count] = '\0' ;
	return src ;
}

//strlen(replaceStr) > strlen(patternStr)
static char *replacestr_big(char *src,
					  int srcLen,
					  const char *patternStr,
					  int patternStrLen,
					  const char *replaceStr,
					  int replaceStrLen)
{
	int findCount = 0 ;
	char *p = src ;
	while((p = strstr(p,patternStr)) != NULL)
	{
		findCount++ ;
		p += patternStrLen ;
	}
	if(findCount == 0)
		return src ;
	int extraCount = findCount *(replaceStrLen - patternStrLen) ;
	int totalLen = srcLen + extraCount ;
	src[totalLen] = '\0' ;
	srcLen = srcLen ;
	int i = srcLen;
	while(i >= 0)
	{
		src[i + extraCount] = src[i] ;
		i-- ;
	}
	int j = 0 ;
	//for(i = 0 ;i < srcLen ;)
	i = 0 ;
	while(i < srcLen )
	{
		if(strncmp(src + i+extraCount,patternStr ,patternStrLen) == 0)
		{	
			strncpy(src + j ,replaceStr,replaceStrLen) ;
			i += patternStrLen ;
			j += replaceStrLen ;
			continue ;
		}
		src[j] = src[i + extraCount ] ;
		i++ ;
		j++ ;
	}
	return src ;
}
char *replacestr(char *src,const char *patternStr,const char *replaceStr)
{
	if(patternStr == NULL || replaceStr == NULL )
		return src ;
	int patternStrLen = strlen(patternStr);
	int replaceStrLen = strlen(replaceStr);
	if(patternStrLen == replaceStrLen)
	{
		return replacestr_equal(src,strlen(src),patternStr,replaceStr,patternStrLen);
	}
	else if(patternStrLen > replaceStrLen)
	{
		return replacestr_less(src,strlen(src),patternStr,patternStrLen,replaceStr,replaceStrLen);
	}
	else
	{
		return replacestr_big(src,strlen(src),patternStr,patternStrLen,replaceStr,replaceStrLen);
	}
}

char *headstrip(char *pstr,char ch)
{
	if(pstr == NULL)
	{
		return NULL ;
	}
	int i = 0 ;
	char *tmp = pstr ;
	while(tmp[i] != '\0'&&tmp[i] == ch)
	{
		i++ ;
	}

	while(tmp[i] != '\0')
	{
		tmp[0] = tmp[i] ;
		tmp++ ;
	}

	*tmp = '\0' ;
	return pstr ;
}

char *tailstrip(char *pstr,char ch)
{
	if(pstr == NULL)
	{	
		return NULL ;
	}
	int len = strlen(pstr) ;
	while(--len >= 0)
	{
		if(pstr[len] == ch)
		{
			pstr[len] = '\0' ;	
		}	
		else
		{
			break ;
		}
	}
	return pstr ;
}

char *strip(char *pstr, char ch)
{
	if(pstr == NULL)
	{
		return NULL ;
	}

	//return tailstrip(headstrip(pstr, ch), ch) ;
	int len = strlen(pstr) ;

	while(--len >= 0 && pstr[len] == ch) 
	{
		pstr[len] = '\0' ;
	}
	
	len = 0 ;
	while(pstr[len] != '\0' && pstr[len] == ch)
	{
		len++ ;
	}
	char *tmp = pstr ;	
	while(pstr[len] != '\0')
	{
		pstr[0] = pstr[len] ;
		pstr++ ;
	}
	pstr[0] = '\0' ;	
	return tmp ;
}

int startwith(const char *pstr1, const char *pstr2)
{
    if(pstr1 == NULL || pstr2 == NULL)
    {
        return 0 ;
    }
    while(*pstr1 != '\0' && *pstr2 != '\0')
    {
        if(*pstr1++ != *pstr2++)
        {
            return 0 ;
        }
    }
    return *pstr1 != '\0' || (*pstr1 =='\0' && *pstr2 == '\0') ;
}

int endwith(const char *pstr1, const char *pstr2)
{
    if(pstr1 == NULL || pstr2 == NULL)
    {
        return 0 ;
    }
    uint l1 = strlen(pstr1) ;
    uint l2 = strlen(pstr2) ;
    return l1 >= l2 && strcmp(pstr1 + (l1 - l2), pstr2) == 0 ;
}

char *upper(char *pstr)
{
    if(pstr == NULL)
    {
        return NULL ;
    }
    char *ptmp = pstr ;
    while(*ptmp != '\0')
    {
        if(*ptmp >= 'a' && *ptmp <= 'z')
        {
            //小写字母的值与对应的大写的字母的值相差 32 例如 'a' - 'A' == 32
            *ptmp = *ptmp - 32 ; 
        }
        ptmp++ ;
    }
    return pstr ;
}
char *lower(char *pstr)
{
    if(pstr == NULL)
    {
        return NULL ;
    }
    char *ptmp = pstr ;
    while(*ptmp != '\0')
    {
        if(*ptmp >= 'A' && *ptmp <= 'Z')
        {
            *ptmp = *ptmp + 32 ;
        }
        ptmp++ ;
    }

    return pstr ;
}
#if 1
int main(int argc,char *argv[])
{
	char buff[1024] = {0} ;
	strcpy(buff,argv[1]);
	replacestr(buff,argv[2],'\0') ;
	printf("after = %s\n",buff);
    printf("startwith(abc,ab) = %d\nstartwith(abc, bc)=%d\n startwith(abc, abcd)=%d\n startwith(abc, abc)=%d\n",
            startwith("abc", "ab"), startwith("abc", "bc"), startwith("abc", "abcd"), startwith("abc", "abc")) ;

    printf("endwith(abc,bc) = %d\nendwith(abc, abc)=%d\n endwith(abc, abcd)=%d\n endwith(abc, dabc)=%d\n",
            endwith("abc", "bc"), endwith("abc", "abc"), endwith("abc", "abcd"), endwith("abc", "dabc")) ;
    char buff1[123] = {"hello"} ;
    char buff2[123] = {"WORLD"} ;
    printf("lower(buff1) = %s\n upper(buff1) = %s\n", lower(buff1), upper(buff1)) ;
    printf("lower(buff2) = %s\n upper(buff2) = %s\n", lower(buff2), upper(buff2)) ;
	return 0 ;
}
#endif
