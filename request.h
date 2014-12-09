#ifndef __REQUEST__H
#define __REQUEST__H

#include "http_url.h"
#include "httpheader.h"
#include "cookie.h"
#include "data.h"
#include "response.h"

#define GET  0X00000001
#define POST 0X00000002
#define MAX_FILE_COUNT  100

typedef struct Http_request
{
    //对url解析的结果
    http_url_t *url ;
    http_request_header_t header ;
    http_cookie_t cookie ; 
    data_t data;
    int method;
    char version[4] ;
    //POST文件的时候需要用到boundry
    char boundary[50] ;

    //上传文件时得记录文件名称
    char *filename[100] ;
    char *name[100] ;
    int curr_file_count ;
} http_request_t;
#ifdef __cplusplus
extern "C" {
#endif

extern http_response_t *send_request(http_request_t *httpreq, int timeout, int method);
extern http_response_t* send_request_with_get(http_request_t *httpreq, int timeout) ;
extern http_response_t* send_request_with_post(http_request_t *httpreq, int timeout) ;
extern int set_boundary(http_request_t *httpreq, char *boundary) ;
extern int add_post_file(http_request_t *httpreq, const char *name, const char *filename) ;
extern int add_request_data(http_request_t *httpreq, const uchar *key, int key_sz, const uchar *val, int val_sz) ;
extern int add_request_header(http_request_t *httpreq, const char *key, const char *value) ;
extern int set_http_version(http_request_t *httpreq, const char *version) ;

#ifdef __cplusplus
}
#endif

#endif
