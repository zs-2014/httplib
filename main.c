#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include  "util.h"
#include "cookie.h"
#include "urlencode.h"
#include "http_url.h"
#include "request.h"
#include "response.h"
#include "data.h"
#include "buffer.h"
#include "httpheader.h"
#include "request.h"
#include "response.h"
#include "log.h"

int print_cookie(http_cookie_t *cookie)
{
    if(cookie == NULL)
    {
        return -1 ;
    }
    printf("cookiebuff = %s\n", cookie ->cookie_buff) ;
    return 0 ;
}
void update_test()
{
    http_cookie_t cookie ;
    init_cookie(&cookie) ;
    add_key_value(&cookie, "path", "/") ;
    add_key_value(&cookie, "name1", "zs1") ;
    add_key_value(&cookie, "name", "test1 for update Test") ;
    print_cookie(&cookie) ;

    update_key(&cookie, "name", "test1 for update Test") ;
    print_cookie(&cookie) ;
    
    update_key(&cookie, "name", "test1 for update Test  xxxxxxx") ;
    print_cookie(&cookie) ;

    update_key(&cookie, "name", "") ;
    print_cookie(&cookie) ;

    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    update_key(&cookie, "name", buff);
    print_cookie(&cookie) ;
    free_cookie(&cookie) ;
}

void del_test()
{
    http_cookie_t cookie ;
    init_cookie(&cookie) ;
    add_key_value(&cookie, "path", "/") ;
    add_key_value(&cookie, "name1", "zs1") ;
    add_key_value(&cookie, "name", "test1 for update Test") ;
    print_cookie(&cookie) ;

    delete_key(&cookie, "name") ;
    print_cookie(&cookie) ;
    
    update_key(&cookie, "name", "test1 for update Test  xxxxxxx") ;
    print_cookie(&cookie) ;

    delete_key(&cookie, "name") ;
    print_cookie(&cookie) ;

    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    delete_key(&cookie, "name");
    print_cookie(&cookie) ;

    free_cookie(&cookie) ;
}

void add_test()
{
    http_cookie_t cookie ;
    init_cookie(&cookie) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    add_key_value(&cookie, "name", "val") ;
    print_cookie(&cookie) ;
    add_key_value(&cookie, "key", "value") ;
    print_cookie(&cookie) ;
    add_key_value(&cookie, "key", buff) ;
    print_cookie(&cookie) ;

    free_cookie(&cookie) ;
}

void Option_test()
{
    http_cookie_t cookie ;
    init_cookie(&cookie) ;
    add_httponly_option(&cookie) ;
    print_cookie(&cookie) ;

    add_key_value(&cookie, "name", "zs") ;
    add_secure_option(&cookie) ;
    print_cookie(&cookie);
    add_key_value(&cookie, "name1", "zs") ;

    del_httponly_option(&cookie) ;
    print_cookie(&cookie) ;
    del_secure_option(&cookie) ;
    print_cookie(&cookie) ;

    free_cookie(&cookie) ;
}
void get_val_test()
{
    http_cookie_t cookie;
    init_cookie(&cookie) ;
    add_key_value(&cookie, "name", "value") ;
    add_key_value(&cookie, "name1", "value1") ;
    add_key_value(&cookie, "name2", "value2") ;
    char buff[30] = {0} ;
    char buff1[30] = {0} ;
    char buff2[30] = {0} ;
    char buff3[30] = {0} ;
    printf("name = %s\nname1 = %s\nname2 = %s\nname3=%s\n", copy_value(&cookie, "name", buff), copy_value(&cookie, "name1", buff1), copy_value(&cookie, "name2", buff2), copy_value(&cookie, "name3", buff3)) ;

}

int cookie_main(int argc, char *argv[])
{
    //update_test() ;
    //del_test() ;
    //add_test() ;
    //get_val_test() ;
    Option_test() ;
    return 0 ;
}

void print_header(http_request_header_t *httphdr)
{
    printf("[%s]\n", httphdr ->hdr_buff) ;
}
void add_header_test()
{
    http_request_header_t header ;
    init_http_header(&header) ;
    add_header(&header, "content-type", "application/json ;charset=utf-8"); 
    print_header(&header) ;
    add_header(&header, "content-length", "12045") ;
    print_header(&header) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) -1) ;
    add_header(&header, "Cookie", buff) ;
    print_header(&header) ;
    free_http_header(&header) ;
}

void delete_header_test()
{
    http_request_header_t header ;
    init_http_header(&header) ;
    add_header(&header, "content-type1", "application/json ;charset=utf-8"); 
    print_header(&header) ;
    add_header(&header, "content-length", "12045") ;
    print_header(&header) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) -1) ;
    add_header(&header, "Cookie", buff) ;
    print_header(&header) ;
    //free_http_header(&header) ;
    
    delete_header(&header, "Cookie") ;
    print_header(&header) ;
    delete_header(&header, "Content-length") ;
    print_header(&header) ;
    delete_header(&header, "content-length");
    print_header(&header) ;

    delete_header(&header, "content-type") ; 
    print_header(&header) ;
    free_http_header(&header) ;
}

void update_header_test()
{
    http_request_header_t header ;
    init_http_header(&header) ;
    add_header(&header, "content-type", "application/json ;charset=utf-8"); 
    print_header(&header) ;
    add_header(&header, "content-length", "12045") ;
    print_header(&header) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) -1) ;
    add_header(&header, "Cookie", buff) ;
    print_header(&header) ;

    update_header(&header, "content-length", "12345") ;
    print_header(&header) ;
    update_header(&header, "content-type", "plain/text");
    print_header(&header) ;
    update_header(&header, "Cookie", "12ksfjskdjksdjl") ;
    print_header(&header) ;
    free_http_header(&header) ;
}
int httpheader_main(int argc, char *argv[])
{
    add_header_test() ;
    delete_header_test() ;
    update_header_test() ;
    return 0 ;
}

void print_data(data_t *data)
{
    if(data == NULL)
    {
        return  ;
    }
    //int i = 0 ;
    //for(i = 0 ;i < data ->curr_sz ;i++)
    //{
    //    printf("key:%s\nval=%s\n", data->data[i].key, data ->data[i].val) ;
    //}
    FOREACH(key, val, data)
    {
        printf("key:%s\nval=%s\n", key,val) ;
    }
}

void Add_data()
{
    data_t data ;
    init_data(&data) ;
    int i = 0 ;
    for(i = 0 ;i < 20 ;i++)
    {
        add_data(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
        add_data(&data, "test", strlen("test"), "this is a test", strlen("this is a test")) ;
    }
    print_data(&data) ;
    free_data(&data) ;
}
void del_data()
{
    data_t data ;
    init_data(&data) ;
    add_data(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
    delete_data(&data, "测试", strlen("测试")) ;
    add_data(&data, "test", strlen("test"), "this is a test", strlen("this is a test")) ;
    add_data(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
    print_data(&data) ;
    free_data(&data) ;
}
void update()
{
    data_t data ;
    init_data(&data) ;
    add_data(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
    update_data(&data, "测试", strlen("测试"), "this is a test", strlen("this is a test")) ;
    print_data(&data) ;
    free_data(&data) ;

}
int data_main(int argc, char *argv[])
{
    //Add_data() ; 
    //del_data() ;
    update() ;
    return 0 ;
}

int print_url(const http_url_t *p_url)
{
    if(p_url == NULL) 
    {
         return -1 ;
    }
    printf("protocol:%s\nhost:%s\nport:%s\npath:%s\nparams:%s\nquery:%s\nfragment:%s\n", 
            p_url ->protocol, p_url ->host,p_url ->port, p_url ->path, p_url ->param, p_url ->query, p_url ->frag) ;
}

int url_main(int argc, char *argv[])
{
    http_url_t *p_url = parse_url("http://172.100.101.145:8088\0/index.html/;name=zs?key=value#this is in the fragment") ;
    print_url(p_url) ;
    free_url(p_url);
    return 0 ;
}

void print_http_response_header(http_response_header_t *httphdr)
{
    if(!httphdr)
    {
        return ;
    }
    int i = 0;
    printf("version=%s\ncode=%s\nreason=%s\n", httphdr ->version, httphdr ->code, httphdr ->reason);
    for (i = 0 ;i < httphdr ->count; i++)
    {
       printf("%s:%s\n", httphdr ->key_val[i].key, httphdr ->key_val[i].value) ; 
    }
}

int request_main(int argc, char *argv[])
{
    http_request_t request ;
    init_http_request(&request) ;
    //set_http_request_url(&request,"http://172.100.101.106:9393/util/v1/uploadfile") ;
    set_http_request_url(&request,"http://search.jd.com/Search?keyword=http%E5%8D%8F%E8%AE%AE&enc=utf-8&suggest=1") ;
    //add_request_data(&request, "userid", strlen("userid"), "Gcq_ra_net", strlen("Gcq_ra_net")) ;
    //add_request_data(&request, "category", strlen("category"), "1", strlen("1")) ;
    //add_request_data(&request, "source", strlen("source"), "1", strlen("1")) ;
    //add_request_data(&request, "tag", strlen("tag"), "avatar", strlen("avatar")) ;
    //add_post_file(&request, "file", "small.jpg") ;
    //add_post_file(&request, "file1", "/home/zhangshuang/info.log") ;
    http_response_t *rsp = send_request_with_get(&request, -1) ;
    if(rsp == NULL)
    {
        printf("fail to parse the http response \n") ;
        return -1 ;
    }
    char buff[10250] = {0} ;
    //int ret = read_fully(rsp ->rspfd, buff, sizeof(buff)-1) ;
    //printf("%s", buff) ;
    read_response(rsp, buff, sizeof(buff) - 1) ;
    printf("read [%s]\n", buff) ;
    free_http_request(&request) ;
    //print_http_response_header(&rsp ->httprsphdr) ;
    free_http_response(rsp) ;
    return 0 ;
}

void print_buffer(buffer_t *buff)
{
    printf("buff:curr_sz = %u size = %u\n", buff ->curr_sz, buff ->size) ;
}
void append_buffer_test()
{
   buffer_t buff ;
   init_buffer(&buff) ;
   append_buffer(&buff, "hello,world",strlen("hello,world")) ;
   print_buffer(&buff) ;
   append_buffer(&buff, "hello,world",strlen("hello,world")) ;
   print_buffer(&buff) ;
   char bf[1024] = {0} ;
   append_buffer(&buff, bf, sizeof(bf)) ;
   print_buffer(&buff) ;
   free_buffer(&buff) ;
}
int buffer_main(int argc, char *argv[])
{
    append_buffer_test() ; 
    return 0 ; 
}

int util_main(int argc,char *argv[])
{
    int fd = connect_to_server(argv[1], argv[2], atoi(argv[3])) ;
    printf("%d\n", fd)  ;
    char buff[] = {"GET / HTTP/1.1\r\n_content-Type:plain/text\r\n_user-Agent:curlib2.7\r\n\r\n"} ; 
    printf("write = %d\n", write(fd,buff, strlen(buff))) ;
    int len = 0 ;
    int total ;
    char *bff = read_until(fd, &total, &len, "\r\n\r\n") ;
    printf("len = [%d]\ntotal=%d\n", len, total) ;
    printf("%s\n", bff) ;
    close(fd) ;
    FREE(bff) ;
    perror("error msg") ;
	return 0 ;
}

int log_main(int argc, char *argv)
{
    init_logger(NULL) ;
    return 0 ;
}

int main(int argc, char *argv[])
{
    char buff[20] ;
    request_main(argc, argv) ;
    //cookie_main(argc, argv) ;
    return 0 ;
}
