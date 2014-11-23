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

int printCookie(COOKIE *cookie)
{
    if(cookie == NULL)
    {
        return -1 ;
    }
    printf("cookiebuff = %s\n", cookie ->cookieBuff) ;
    return 0 ;
}
void updateTest()
{
    COOKIE cookie ;
    initCookie(&cookie) ;
    addKeyValue(&cookie, "path", "/") ;
    addKeyValue(&cookie, "name1", "zs1") ;
    addKeyValue(&cookie, "name", "test1 for update Test") ;
    printCookie(&cookie) ;

    updateKey(&cookie, "name", "test1 for update Test") ;
    printCookie(&cookie) ;
    
    updateKey(&cookie, "name", "test1 for update Test  xxxxxxx") ;
    printCookie(&cookie) ;

    updateKey(&cookie, "name", "") ;
    printCookie(&cookie) ;

    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    updateKey(&cookie, "name", buff);
    printCookie(&cookie) ;
    freeCookie(&cookie) ;
}

void delTest()
{
    COOKIE cookie ;
    initCookie(&cookie) ;
    addKeyValue(&cookie, "path", "/") ;
    addKeyValue(&cookie, "name1", "zs1") ;
    addKeyValue(&cookie, "name", "test1 for update Test") ;
    printCookie(&cookie) ;

    deleteKey(&cookie, "name") ;
    printCookie(&cookie) ;
    
    updateKey(&cookie, "name", "test1 for update Test  xxxxxxx") ;
    printCookie(&cookie) ;

    deleteKey(&cookie, "name") ;
    printCookie(&cookie) ;

    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    deleteKey(&cookie, "name");
    printCookie(&cookie) ;

    freeCookie(&cookie) ;

}

void addTest()
{
    COOKIE cookie ;
    initCookie(&cookie) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) - 1) ;
    addKeyValue(&cookie, "name", "val") ;
    printCookie(&cookie) ;
    addKeyValue(&cookie, "key", "value") ;
    printCookie(&cookie) ;
    addKeyValue(&cookie, "key", buff) ;
    printCookie(&cookie) ;

    freeCookie(&cookie) ;
}

void OptionTest()
{
    COOKIE cookie ;
    initCookie(&cookie) ;
    addHttponlyOption(&cookie) ;
    printCookie(&cookie) ;

    addKeyValue(&cookie, "name", "zs") ;
    addSecureOption(&cookie) ;
    printCookie(&cookie);
    addKeyValue(&cookie, "name1", "zs") ;

    delHttponlyOption(&cookie) ;
    printCookie(&cookie) ;
    delSecureOption(&cookie) ;
    printCookie(&cookie) ;

    freeCookie(&cookie) ;
}
void getValTest()
{
    COOKIE cookie;
    initCookie(&cookie) ;
    addKeyValue(&cookie, "name", "value") ;
    addKeyValue(&cookie, "name1", "value1") ;
    addKeyValue(&cookie, "name2", "value2") ;
    char buff[30] = {0} ;
    char buff1[30] = {0} ;
    char buff2[30] = {0} ;
    char buff3[30] = {0} ;
    printf("name = %s\nname1 = %s\nname2 = %s\nname3=%s\n", copyValue(&cookie, "name", buff), copyValue(&cookie, "name1", buff1), copyValue(&cookie, "name2", buff2), copyValue(&cookie, "name3", buff3)) ;

}

int cookie_main(int argc, char *argv[])
{
    //updateTest() ;
    //delTest() ;
    //addTest() ;
    //getValTest() ;
    OptionTest() ;
    return 0 ;
}

void printHeader(HEADER *httphdr)
{
    printf("[%s]\n", httphdr ->hdrBuff) ;
}
void addHeaderTest()
{
    HEADER header ;
    initHttpHeader(&header) ;
    addHeader(&header, "content-type", "application/json ;charset=utf-8"); 
    printHeader(&header) ;
    addHeader(&header, "content-length", "12045") ;
    printHeader(&header) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) -1) ;
    addHeader(&header, "Cookie", buff) ;
    printHeader(&header) ;
    freeHttpHeader(&header) ;
}

void deleteHeaderTest()
{
    HEADER header ;
    initHttpHeader(&header) ;
    addHeader(&header, "content-type1", "application/json ;charset=utf-8"); 
    printHeader(&header) ;
    addHeader(&header, "content-length", "12045") ;
    printHeader(&header) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) -1) ;
    addHeader(&header, "Cookie", buff) ;
    printHeader(&header) ;
    //freeHttpHeader(&header) ;
    
    deleteHeader(&header, "Cookie") ;
    printHeader(&header) ;
    deleteHeader(&header, "Content-length") ;
    printHeader(&header) ;
    deleteHeader(&header, "content-length");
    printHeader(&header) ;

    deleteHeader(&header, "content-type") ; 
    printHeader(&header) ;
    freeHttpHeader(&header) ;
}

void updateHeaderTest()
{
    HEADER header ;
    initHttpHeader(&header) ;
    addHeader(&header, "content-type", "application/json ;charset=utf-8"); 
    printHeader(&header) ;
    addHeader(&header, "content-length", "12045") ;
    printHeader(&header) ;
    char buff[1024] = {0} ;
    memset(buff, 'a', sizeof(buff) -1) ;
    addHeader(&header, "Cookie", buff) ;
    printHeader(&header) ;

    updateHeader(&header, "content-length", "12345") ;
    printHeader(&header) ;
    updateHeader(&header, "content-type", "plain/text");
    printHeader(&header) ;
    updateHeader(&header, "Cookie", "12ksfjskdjksdjl") ;
    printHeader(&header) ;
    freeHttpHeader(&header) ;
}
int httpheader_main(int argc, char *argv[])
{
    addHeaderTest() ;
    deleteHeaderTest() ;
    updateHeaderTest() ;
    return 0 ;
}

void printData(DATA *data)
{
    if(data == NULL)
    {
        return  ;
    }
    //int i = 0 ;
    //for(i = 0 ;i < data ->currSz ;i++)
    //{
    //    printf("key:%s\nval=%s\n", data->data[i].key, data ->data[i].val) ;
    //}
    FOREACH(key, val, data)
    {
        printf("key:%s\nval=%s\n", key,val) ;
    }
}

void AddData()
{
    DATA data ;
    initData(&data) ;
    int i = 0 ;
    for(i = 0 ;i < 20 ;i++)
    {
        addData(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
        addData(&data, "test", strlen("test"), "this is a test", strlen("this is a test")) ;
    }
    printData(&data) ;
    freeData(&data) ;
}
void delData()
{
    DATA data ;
    initData(&data) ;
    addData(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
    deleteData(&data, "测试", strlen("测试")) ;
    deleteData(&data, "测试", strlen("测试")) ;
    addData(&data, "test", strlen("test"), "this is a test", strlen("this is a test")) ;
    addData(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
    printData(&data) ;
    freeData(&data) ;
}
void update()
{
    DATA data ;
    initData(&data) ;
    addData(&data, "测试", strlen("测试"), "这是个测试", strlen("这是个测试")) ;
    updateData(&data, "测试", strlen("测试"), "this is a test", strlen("this is a test")) ;
    printData(&data) ;
    freeData(&data) ;

}
int data_main(int argc, char *argv[])
{
    //AddData() ; 
    //delData() ;
    update() ;
    return 0 ;
}

int printURL(const URL *pUrl)
{
    if(pUrl == NULL) 
    {
         return -1 ;
    }
    printf("protocol:%s\nhost:%s\nport:%s\npath:%s\nparams:%s\nquery:%s\nfragment:%s\n", 
            pUrl ->protocol, pUrl ->host,pUrl ->port, pUrl ->path, pUrl ->param, pUrl ->query, pUrl ->frag) ;
}


int url_main(int argc, char *argv[])
{
    URL *pUrl = parseURL("http://172.100.101.145:8088\0/index.html/;name=zs?key=value#this is in the fragment") ;
    printURL(pUrl) ;
    freeURL(pUrl);
    return 0 ;
}

void printHttpResponseHeader(HttpResponseHeader *httphdr)
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
    HTTPREQUEST request ;
    initHttpRequest(&request) ;
    //setHttpRequestUrl(&request,"http://172.100.102.153:9393/util/v1/uploadfile") ;
    setHttpRequestUrl(&request,"http://192.168.182.131:8080/util/v1/uploadfile") ;
    addRequestData(&request, "userid", strlen("userid"), "GcqRaNEt", strlen("GcqRaNEt")) ;
    addRequestData(&request, "category", strlen("category"), "1", strlen("1")) ;
    addRequestData(&request, "source", strlen("source"), "1", strlen("1")) ;
    addRequestData(&request, "tag", strlen("tag"), "avatar", strlen("avatar")) ;
    addPostFile(&request, "file", "/root/main.cpp") ;
    HTTPRESPONSE *rsp = sendRequestWithPOST(&request, -1) ;
    if(rsp == NULL)
    {
        printf("fail to parse the http response \n") ;
        return -1 ;
    }
    char buff[10250] = {0} ;
    //int ret = readFully(rsp ->rspfd, buff, sizeof(buff)-1) ;
    //printf("%s", buff) ;
    readResponse(rsp, buff, sizeof(buff) - 1) ;
    printf("read [%s]\n", buff) ;
    freeHttpRequest(&request) ;
    //printHttpResponseHeader(&rsp ->httprsphdr) ;
    freeHttpResponse(rsp) ;
    return 0 ;
}

void printBuffer(BUFFER *buff)
{
    printf("buff:currSz = %u size = %u\n", buff ->currSz, buff ->size) ;
}
void appendBufferTest()
{
   BUFFER buff ;
   initBuffer(&buff) ;
   appendBuffer(&buff, "hello,world",strlen("hello,world")) ;
   printBuffer(&buff) ;
   appendBuffer(&buff, "hello,world",strlen("hello,world")) ;
   printBuffer(&buff) ;
   char bf[1024] = {0} ;
   appendBuffer(&buff, bf, sizeof(bf)) ;
   printBuffer(&buff) ;
   freeBuffer(&buff) ;
}
int buffer_main(int argc, char *argv[])
{
    appendBufferTest() ; 
    return 0 ; 
}

int util_main(int argc,char *argv[])
{
    int fd = connectToServer(argv[1], argv[2], atoi(argv[3])) ;
    printf("%d\n", fd)  ;
    char buff[] = {"GET / HTTP/1.1\r\nContent-Type:plain/text\r\nUser-Agent:curlib2.7\r\n\r\n"} ; 
    printf("write = %d\n", write(fd,buff, strlen(buff))) ;
    int len = 0 ;
    int total ;
    char *bff = readUntil(fd, &total, &len, "\r\n\r\n") ;
    printf("len = [%d]\ntotal=%d\n", len, total) ;
    printf("%s\n", bff) ;
    close(fd) ;
    FREE(bff) ;
    perror("error msg") ;
	return 0 ;
}

int main(int argc, char *argv[])
{
    char buff[20] ;
    request_main(argc, argv) ;
    //cookie_main(argc, argv) ;
    return 0 ;
}
