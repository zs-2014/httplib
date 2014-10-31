#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_url.h"
int main(int argc, char *argv[])
{
    init() ;
    if(argc != 2)
    {
        printf("please input the string you want to test\n") ;
        return -1 ;
    }
    char *p1 = quotestr(argv[1]) ;
    uchar *p2 = unquotestr(argv[1]) ;
    printf("before:%s\nafter:%s\nunquote = %s\n",argv[1], p1, p2) ;
    free(p1) ;
    free(p2) ;
    return 0 ;
}
