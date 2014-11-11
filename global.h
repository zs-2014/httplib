#ifndef __GLOBAL__H
#define __GLOBAL__H

#define const

#define FREE(ptr) free(ptr)
#define MALLOC(sz) malloc(sz) 
#define CALLOC(n, sz) calloc(n, sz)
#define REALLOC(ptr, sz) realloc(ptr, sz)

#define MAX(a, b) (a) > (b) ? (a):(b)
#define MIN(a, b) (a) > (b) ? (b):(a)


typedef unsigned int uint ;
typedef unsigned char uchar ;


static char *nullStrPtr = "" ; 
#ifdef __cplusplus
extern "C"{
#endif 


#ifdef __cplusplus
}
#endif



#endif
