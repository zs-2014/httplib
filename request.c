#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h> 

#include "response.h"
#include "request.h"
#include "cookie.h"
#include "http_url.h"
#include "util.h"
#include "httpheader.h"
#include "buffer.h"
#include "global.h"

#define MAX_BOUNDRY_BYTE 20

#define DEFAULT_PORT "80"

#define STR_POST    "POST"
#define STR_POST_LEN strlen(STR_POST)

#define STR_GET     "GET"
#define STR_GET_LEN strlen(STR_GET)
#define VERSION(m, n) (((m)-'0')*10 + (n) - '0')
#define MIN_VERSION VERSION('1', '0') 

int init_http_request(http_request_t *httpreq)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }

    httpreq ->method = GET ;
    httpreq ->url = NULL ;
    httpreq ->version[0] = '1' ;
    httpreq ->version[1] = '.' ;
    httpreq ->version[2] = '1' ;
    memset(httpreq ->filename, 0, sizeof(httpreq ->filename)) ;
    httpreq ->curr_file_count = 0 ;
    memset(httpreq ->boundary, 0, sizeof(httpreq ->boundary)) ;
    return init_data(&httpreq ->data)||init_http_header(&httpreq ->header) || init_http_header(&httpreq ->header) ;
}

int free_http_request(http_request_t *httpreq)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    free_url(httpreq ->url) ; 
    free_http_header(&httpreq ->header) ;
    int i = 0 ;
    for(i = 0 ; i < httpreq ->curr_file_count ;i++)
    {
        FREE(httpreq ->filename[i]) ;
        httpreq ->filename[i] = NULL ;
    }
    httpreq ->curr_file_count = 0 ;
    return 0 ;
}

int set_request_method(http_request_t *httpreq, int method)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    httpreq ->method = (method == GET ?GET :POST) ;
}

int set_http_request_url(http_request_t *httpreq, const char *urlstr)
{
    if(httpreq == NULL || urlstr == NULL)
    {
        return -1 ;
    }
    httpreq ->url = parse_url(urlstr) ;
    if(httpreq ->url == NULL)
    {
        return -1 ;
    }
    return 0 ; 
}

int set_cookie(http_request_t *httpreq, http_cookie_t *cookie)
{
    return add_request_header(httpreq, "Cookie", cookie2String(cookie)) ;
}

int set_content_type(http_request_t *httpreq, const char *content_type)
{
    return add_request_header(httpreq, "Content-Type", content_type);
}
int set_user_agent(http_request_t *httpreq, const char *user_agent)
{
    return add_request_header(httpreq, "User-Agent", user_agent) ;
}

int set_http_version(http_request_t *httpreq, const char *version)
{
    if(httpreq == NULL || version == NULL)    
    {
        return -1 ;
    }
    int len = strlen(version) ;
    if(len != 3 || VERSION(version[0], version[2]) < MIN_VERSION)
    {
        printf("[%s]:invalid http version\n", version) ;
    }
    httpreq ->version[0] = version[0] ;
    httpreq ->version[1] = version[1] ;
    httpreq ->version[2] = version[2] ;
    return 0 ;
}

int add_request_header(http_request_t *httpreq, const char *key, const char *value)
{

    if(httpreq == NULL) 
    {
        return -1 ;
    }
    if(!has_header(&httpreq ->header, key))
    {
        return add_header(&httpreq ->header, key, value); 
    }
    else
    {
        return 0 ;
    }

}

int add_request_data(http_request_t *httpreq, const uchar *key, int key_sz, const uchar *val, int val_sz)
{
    if(httpreq == NULL || key == NULL || val == NULL)
    {
        return -1 ;
    }
    return add_data(&httpreq ->data, key, key_sz, val, val_sz) ;
}

int add_post_file(http_request_t *httpreq, const char *name, const char *filename)
{
   if(httpreq == NULL || filename == NULL) 
   {
        return -1 ;
   }
   if(access(filename, R_OK) != 0)
   {
        printf("the [%s] is not exists or not allowed to read\n", filename) ;
        return -1 ;
   }
   if(httpreq ->curr_file_count >= MAX_FILE_COUNT)
   {
        printf("has beyond the max file count\n") ;
        return -1 ;
   }
   if((httpreq ->filename[httpreq ->curr_file_count] = strdup(filename)) == NULL)
   {
        printf("fail to copy the filename:[%s]\n", filename) ;
        return -1 ;
   }
   if((httpreq ->name[httpreq ->curr_file_count] = strdup(name)) == NULL)
   {
        printf("fail to copy the name:[%s]\n", name);
        return -1 ;
   }
   httpreq ->curr_file_count += 1 ;
   return 0 ;
}

static int add_default_request_header(http_request_t *httpreq)
{
    if(!has_header(&httpreq ->header, "Content-Type"))
    {
        add_request_header(httpreq, "Content-type", "plain/text") ;
    }
    if(!has_header(&httpreq ->header, "Host"))
    {
        add_request_header(httpreq, "Host", httpreq ->url ->host) ;
    }
    if(!has_header(&httpreq ->header, "User-Agent"))
    {
        add_request_header(httpreq, "User-Agent", "httplib/1.0") ;
    } 
    return 0 ;
}

static int calc_data_len(data_t *data)
{
    int total = 0 ;
    FOREACH(key, val, data) 
    {
        //total += strlen(key) + strlen("=") + strlen(val) + strlen("&")
        total += strlen(key) + 1 ;
        if(val != NULL)
        {
            total += strlen(val) ;
        }
        total += 1 ;
    }
    return total - 1;
}

static http_response_t *get_response(int fd)
{
    int total_len = 0 ;
    int hdr_len = 0 ;
    char *hdrbuff = read_until(fd, &total_len, &hdr_len, "\r\n\r\n") ;
    printf("%s", hdrbuff) ;
    if(hdrbuff == NULL)
    {
        return NULL ;
    }
    http_response_t *httprsp = init_http_response(MALLOC(sizeof(http_response_t))) ;
    if(httprsp == NULL)
    {
        printf("fail to create response object:%s\n", strerror(errno)) ;
        goto __fails ; 
    }
    set_response_socket(httprsp, fd) ;
    set_response_header_buff(httprsp, hdrbuff, 0) ; 
    set_response_extra_data(httprsp, hdrbuff + hdr_len, total_len - hdr_len) ;
    if(parse_http_response_header(httprsp) != 0)
    {
        printf("fail to parse http response header\n") ;
        goto __fails ;
    }
    return httprsp ;

__fails:
   free_http_response(httprsp) ; 
   FREE(httprsp) ;
   return NULL ;
}

//设置边界
int set_boundary(http_request_t *httpreq, char *boundary)
{
   if(httpreq == NULL || boundary == NULL) 
   {
        return -1 ;
   }
   memcpy(httpreq ->boundary, boundary, MIN(strlen(boundary), sizeof(httpreq ->boundary)-1)) ;
   return 0 ;
}

static char *produce_boundary(http_request_t *httpreq)
{
    if(httpreq == NULL || strlen(httpreq ->boundary) != 0) 
    {
        return NULL ;
    }
    char chs[] = {"1234567890abcdefghijklmnopqrstuvwxyz_abcdefghijklmnopqrstuvwzyz"} ;
    int chs_len = sizeof(chs) - 1;
    int i = 0 ;
    int count = MIN(sizeof(httpreq ->boundary)-1, MAX_BOUNDRY_BYTE) ;
    srand(time(NULL)) ;
    for(i=0; i< count ;i++)
    {
       httpreq ->boundary[i] = chs[rand()%chs_len] ;
    }
    return httpreq ->boundary ;
}

http_response_t *send_request(http_request_t *httpreq, int timeout, int method)
{
    if(httpreq == NULL)
    {
        return NULL ;
    }
    httpreq ->method = method ;
    if(httpreq ->method == GET)
    {
        return send_request_with_get(httpreq, timeout); 
    }
    else
    {
       return send_request_with_post(httpreq, timeout)  ;
    }
}

http_response_t *send_request_with_get(http_request_t *httpreq, int timeout)
{
    if(httpreq == NULL)
    {
        return  NULL ; 
    }
    http_url_t *url = httpreq ->url ;
    httpreq ->method = GET ;
    buffer_t buff ;
    init_buffer(&buff) ;
     //开始拼凑请求头
    append_buffer(&buff, STR_GET, STR_GET_LEN) ;
    append_buffer(&buff, " /", strlen(" /")) ;
    if(url ->path != NULL && strlen(url ->path) != 0)
    {
        append_buffer(&buff, url ->path, strlen(url ->path)) ;
    }
    append_buffer(&buff, "?", 1) ;
    if(url ->query != NULL && strlen(url ->query) != 0)
    {
        append_buffer(&buff, url ->query, strlen(url ->query)) ;
        append_buffer(&buff, "&", 1) ;
    }
    //如果是GET方法的话，data部分应该放到url中
    if(!is_empty(&httpreq ->data))
    {
        FOREACH(key, val, &httpreq ->data)
        {
            append_buffer(&buff, key, strlen(key)) ;
            append_buffer(&buff, "=", 1) ;
            if(val != NULL)
            {
                append_buffer(&buff, val, strlen(val)) ;
            }
            append_buffer(&buff, "&", 1) ;
        }
        lstrip_buffer(&buff, '&') ;
    }
    lstrip_buffer(&buff, '&') ;
    lstrip_buffer(&buff, '?') ;
    append_buffer(&buff, " ", 1) ;
    append_buffer(&buff, "HTTP/", 5) ;
    append_buffer(&buff, httpreq ->version, strlen(httpreq ->version)) ;
    append_buffer(&buff, "\r\n", 2) ; 
    add_default_request_header(httpreq) ;    
    append_buffer(&buff, header2String(&httpreq ->header), header_len(&httpreq ->header)) ;
    append_buffer(&buff, "\r\n", 2) ;
    int fd = connect_to_server(url->host, url ->port == NULL ?DEFAULT_PORT:url ->port, timeout) ;
    if(fd < 0)
    {
       perror("fd < 0") ;
       return NULL; 
    }
    if(send_data(fd, get_buffer_data(&buff), get_buffer_size(&buff)) != get_buffer_size(&buff))
    {
        printf("send [%s] fail\n", get_buffer_data(&buff)) ;
    }
    free_buffer(&buff) ;
    return get_response(fd) ;
}

#define CONTENT_DISPOSITION  "Content-Disposition: form-data; name=\""
#define CONTENT_TYPE         "Content-Type: text/plain; charset=utf-8\r\n"
#define CONTENT_ENCODING     "Content-Transfer-Encoding:8bit\r\n"

static int build_form_data(http_request_t *httpreq, int sockfd, buffer_t *buff, int issend)
{
    if(httpreq == NULL || buff == NULL || sockfd < 0)
    {
        return -1 ;
    }
    int curr_sz = get_buffer_size(buff) ;
    produce_boundary(httpreq) ;
    int boundary_len = strlen(httpreq ->boundary) ;
    FOREACH(key, val, &httpreq ->data)
    {
        //add boundary
        append_buffer(buff, "--", 2) ;
        append_buffer(buff, httpreq ->boundary, boundary_len) ;
        append_buffer(buff, "\r\n", 2) ;
        
        //add content-type disposition encoding
        append_buffer(buff, CONTENT_DISPOSITION, strlen(CONTENT_DISPOSITION)) ;
        append_buffer(buff, key, strlen(key)) ;
        append_buffer(buff, "\"\r\n", 3);
        append_buffer(buff, CONTENT_TYPE, strlen(CONTENT_TYPE)) ;
        append_buffer(buff, CONTENT_ENCODING, strlen(CONTENT_ENCODING)) ;
        append_buffer(buff, "\r\n", 2) ;

        append_buffer(buff, val, strlen(val)) ;
        append_buffer(buff, "\r\n", 2) ;
    }
    int len = get_buffer_size(buff) - curr_sz ;
    if(issend == 0)
    {
        drop_data(buff, len) ;
        return len ;
    }
    if(write_all(sockfd, get_buffer_data(buff), get_buffer_size(buff)) != get_buffer_size(buff))
    {
        perror("fail to send data\n") ;
        return -1 ;
    }
    printf("%s", get_buffer_data(buff)) ;
    return 0 ;
}

static int64_t build_body_from_file(http_request_t *httpreq, int sockfd, int issend)
{
    
    if(httpreq == NULL || sockfd < 0)
    {
        return -1 ;
    }
    produce_boundary(httpreq) ;
    int boundary_len = strlen(httpreq ->boundary) ;
    int64_t file_len = 0 ;
    int i = 0 ;
    char buff[16*1024] = {0};
    int offset = 0 ;
    for(i=0; i < httpreq ->curr_file_count ; i++)
    {
        /*
         * --boundary\r\n
         * content-disposition:form-data; name="file";filename="test.txt"\r\n
         * content-type:xxxx\r\n
         * Content-Transfer-Encoding:8bit\r\n
         * \r\n
         * ........\r\n
         */
         int ret = snprintf(buff + offset, sizeof(buff)-offset, "--%s\r\n"
                                                                 "content-disposition:form-data;name=\"%s\";filename=\"%s\"\r\n"
                                                                 "content-type:application/octet-stream\r\n"
                                                                 "content-transfer-encoding:8bit\r\n"
                                                                 "\r\n", httpreq ->boundary, httpreq ->name[i], httpreq ->filename[i]) ;
        if(ret < 0)
        {
            printf("fail to format the data\n") ;
            return -1 ;
        }
        offset += ret ;
        //strcpy(buff + offset, "--") ;
        //offset += 2 ;
        //strcpy(buff + offset, httpreq ->boundary) ;        
        //offset += strlen(httpreq ->boundary) ; 
        //strcpy(buff + offset, "\r\n") ;
        //offset += 2 ;

        //strcpy(buff + offset, CONTENT_DISPOSITION) ;
        //offset += strlen(CONTENT_DISPOSITION) ;
        //strcpy(buff + offset, httpreq ->name[i]) ;
        //offset += strlen(httpreq ->name[i]) ;
        //strcpy(buff + offset, "\"") ;
        //offset += 1 ;
        //strcpy(buff + offset, ";filename=\"") ;
        //offset += strlen(";filename=\"") ;
        //strcpy(buff + offset, httpreq ->filename[i]) ;
        //offset += strlen(httpreq ->filename[i]) ;
        //strcpy(buff + offset, "\"\r\n") ;
        //offset += strlen("\"\r\n");
        //strcpy(buff + offset, "Content-Type:application/octet-stream\r\n") ;
        //offset += strlen("Content-Type:application/octet-stream\r\n") ;
        //strcpy(buff + offset, "Content-Transfer-Encoding:8bit\r\n\r\n") ;
        //offset += strlen("Content-Transfer-Encoding:8bit\r\n\r\n") ; 
        struct stat st ;
        if(stat(httpreq ->filename[i], &st) == -1)
        {
            perror("fail to get fail size") ;
            return -1 ; 
        }
        if(issend == 0)
        {
            file_len += offset ;
            //文件内容后面需要跟上 \r\n，所以需要+2
            file_len += st.st_size + 2;
            offset = 0 ;
            continue ;
        }
        int fd = open(httpreq ->filename[i], O_RDONLY);
        if(fd < 0)
        {
            return -1 ;
        }
        //可以使用sendfile
        buff[offset] = 0 ;
        printf("%s", buff) ;
        do
        {
            ret = read_fully(fd, buff + offset, sizeof(buff)-offset);
            if(ret < 0)
            {
                close(fd);
                return -1 ;
            }
            st.st_size -= ret ;
            ret += offset ;
            offset = 0 ;
            if(write_all(sockfd, buff, ret) != ret)
            {
               perror("error\n") ; 
               close(fd);
               return -1 ;
            }
        }while(st.st_size > 0) ;
        close(fd); 
        strcpy(buff + offset, "\r\n") ;
        offset += 2 ;
    }
    if(issend == 0)
    {
        return file_len ; 
    }
    if(write_all(sockfd, buff, offset) != offset)
    {
       perror("error\n") ; 
       return -1 ;
    }
    printf("%s", buff) ;
    return 0 ;
}

static int64_t build_body(http_request_t *httpreq, int sockfd, buffer_t *buff, int issend)
{
    if(httpreq == NULL || buff == NULL || sockfd < 0)
    {
        return -1;
    }
    int formdata_len = build_form_data(httpreq, sockfd, buff, issend) ;
    int boundary_len = strlen(httpreq ->boundary) ;
    int64_t file_len = build_body_from_file(httpreq, sockfd, issend) ;
    if(file_len == -1)
    {
        return -1 ;
    }
    if(issend == 0)
    {
       //--boundary--\r\n 
       return file_len + formdata_len + 2 + boundary_len + 2 + 2 ;
    } 
    //数据传送完毕，发送结束标记 --boundary--\r\n
    char end_data[1024] = {0} ;
    strcpy(end_data, "--") ;
    strcat(end_data, httpreq ->boundary) ;
    strcat(end_data, "--\r\n") ;
    if(write_all(sockfd, end_data, boundary_len + 6) != boundary_len + 6)
    {
        return -1 ;
    }
    printf("%s", end_data) ;
    return 0 ;
}

http_response_t *send_request_with_post(http_request_t *httpreq, int timeout)
{
    if(httpreq == NULL || httpreq ->url == NULL) 
    {
        return NULL;
    }
    buffer_t buff ;
    init_buffer(&buff) ;
    httpreq ->method = POST ;
    http_url_t *url = httpreq ->url ;
     //开始拼凑请求头
    append_buffer(&buff, STR_POST, STR_POST_LEN) ;
    append_buffer(&buff, " /", strlen(" /")) ;
    if(url ->path != NULL && strlen(url ->path) != 0)
    {
        append_buffer(&buff, url ->path, strlen(url ->path)) ;
    }
    append_buffer(&buff, "?", 1) ;
    if(url ->query != NULL && strlen(url ->query) != 0)
    {
        append_buffer(&buff, url ->query, strlen(url ->query)) ;
        append_buffer(&buff, "&", 1) ;
    }
    lstrip_buffer(&buff, '&') ;
    lstrip_buffer(&buff, '?') ;
    append_buffer(&buff, " ", 1) ;
    append_buffer(&buff, "HTTP/", 5) ;
    append_buffer(&buff, httpreq ->version, strlen(httpreq ->version)) ;
    append_buffer(&buff, "\r\n", 2) ; 
    produce_boundary(httpreq) ;
    char tmpbuff[1024] = {0} ;
    strcpy(tmpbuff, "multipart/form-data; boundary=") ;
    strcat(tmpbuff, httpreq ->boundary) ;
    add_request_header(httpreq, "Content-Type", tmpbuff) ;
    //上传文件一定得加上Content-Length，否则无法解析出来
    char len_buff[50] = {0} ;
    int64_t total_len = build_body(httpreq, 0, &buff, 0) ;
    add_request_header(httpreq, "Content-Length", itoa(total_len,len_buff)) ;
    add_default_request_header(httpreq) ;    
    append_buffer(&buff, header2String(&httpreq ->header), header_len(&httpreq ->header)) ;
    append_buffer(&buff, "\r\n", 2) ;
    int fd = connect_to_server(url->host, url ->port == NULL ?DEFAULT_PORT:url ->port, timeout) ;
    if(fd < 0)
    {
       perror("fd < 0") ;
       return NULL; 
    }
    if(build_body(httpreq, fd, &buff, 1) != 0) 
    {
        printf("fail to build body\n") ;
        free_buffer(&buff) ;
        return NULL ;
    }
    free_buffer(&buff) ;
    return get_response(fd) ;
}
