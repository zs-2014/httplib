#ifndef __RESPONSE__H
#define __RESPONSE__H

#include "global.h"

typedef struct Node
{
	char *value ;
	char *key;
}NODE ;

typedef struct http_response_header_t
{
    char version[9] ;
    char code[4] ;
    char reason[32] ;
	int size ;
    int count ;
    char *headbuff ;
	NODE *key_val ;
}http_response_header_t;

typedef struct http_response_t
{
   http_response_header_t httprsphdr ;
   int rspfd ;

   //transfer-encoding 为chunk时，记录距离下一个chunk还有多少数据要读
   int next_chunk_size ;
   int chunk_count ;
   char buff[8192] ;
   int curr_sz ;
   int curr_pos ;

}http_response_t ;

#ifdef __cplusplus
extern "C"{
#endif

extern int set_response_header_buff(http_response_t *httprsp, char *hdrbuff, int is_copy) ;
extern int parse_http_response_header(http_response_t *httprsp) ;

extern int free_http_response(http_response_t *httprsp) ;
extern http_response_t *init_http_response(http_response_t *httprsp) ;

#ifdef __cplusplus
}
#endif

#endif
