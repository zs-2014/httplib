#ifndef __RESPONSE__H
#define __RESPONSE__H

typedef struct Node
{
	char *value ;
	char *key;
}NODE ;

typedef struct HttpResponseHeader
{
    char version[9] ;
    char code[4] ;
    char reason[32] ;
	int size ;
    int count ;
    char *headbuff ;
	NODE *key_val ;
}HttpResponseHeader;


#ifdef __cplusplus
extern "C" 
{
#endif
extern HttpResponseHeader* parseHttpResponseHeader(const char *hdrbuff) ;
#ifdef __cplusplus
};
#endif
#endif
