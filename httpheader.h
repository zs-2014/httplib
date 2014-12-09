#ifndef __HTTP_HEADER__H
#define __HTTP_HEADER__H

#include "global.h"

typedef struct Http_header
{
    int curr_sz ;
    int size ;
    char *hdr_buff ;
}http_request_header_t;

#ifdef __cplusplus
extern "C" {
#endif

extern int init_http_header(http_request_header_t *httphdr) ;
extern int free_http_header(http_request_header_t *httphdr) ;
extern int add_header(http_request_header_t *httphdr, const char *key, const char *value) ;
extern int delete_header(http_request_header_t *httphdr, const char *key) ;
extern int update_header(http_request_header_t *httphdr, const char *key, const char *values) ;
extern const char *header2String(http_request_header_t *httphdr) ;
extern uint header_len(http_request_header_t *httphdr) ;
extern int has_header(http_request_header_t *httphdr, const char *key) ;
#ifdef __cplusplus
}
#endif

#endif
