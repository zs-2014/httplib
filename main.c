#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include  "util.h"
int main(int argc, char *argv[])
{
    char buff[20] ;
    printf("itoa = %s\n",itoa(123, buff)) ;
    printf("itoa = %s\n", itoa(1234, buff)) ;
    printf("itoa = %s\n", itoa(0, buff)) ;
    return 0 ;
}
