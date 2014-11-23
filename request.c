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

int initHttpRequest(HTTPREQUEST *httpreq)
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
    httpreq ->currFileCount = 0 ;
    memset(httpreq ->boundary, 0, sizeof(httpreq ->boundary)) ;
    return initData(&httpreq ->data)||initHttpHeader(&httpreq ->header) || initHttpHeader(&httpreq ->header) ;
}

int freeHttpRequest(HTTPREQUEST *httpreq)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    freeURL(httpreq ->url) ; 
    freeHttpHeader(&httpreq ->header) ;
    int i = 0 ;
    for(i = 0 ; i < httpreq ->currFileCount ;i++)
    {
        FREE(httpreq ->filename[i]) ;
        httpreq ->filename[i] = NULL ;
    }
    httpreq ->currFileCount = 0 ;
    return 0 ;
}

int setRequestMethod(HTTPREQUEST *httpreq, int method)
{
    if(httpreq == NULL)
    {
        return -1 ;
    }
    httpreq ->method = (method == GET ?GET :POST) ;
}

int setHttpRequestUrl(HTTPREQUEST *httpreq, const char *urlstr)
{
    if(httpreq == NULL || urlstr == NULL)
    {
        return -1 ;
    }
    httpreq ->url = parseURL(urlstr) ;
    if(httpreq ->url == NULL)
    {
        return -1 ;
    }
    return 0 ; 
}

int setCookie(HTTPREQUEST *httpreq, COOKIE *cookie)
{
    return addRequestHeader(httpreq, "Cookie", cookie2String(cookie)) ;
}

int setContentType(HTTPREQUEST *httpreq, const char *contentType)
{
    return addRequestHeader(httpreq, "Content-Type", contentType);
}
int setUserAgent(HTTPREQUEST *httpreq, const char *userAgent)
{
    return addRequestHeader(httpreq, "User-Agent", userAgent) ;
}

int setHttpVersion(HTTPREQUEST *httpreq, const char *version)
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

int addRequestHeader(HTTPREQUEST *httpreq, const char *key, const char *value)
{

    if(httpreq == NULL) 
    {
        return -1 ;
    }
    if(!hasHeader(&httpreq ->header, key))
    {
        return addHeader(&httpreq ->header, key, value); 
    }
    else
    {
        return 0 ;
    }

}

int addRequestData(HTTPREQUEST *httpreq, const uchar *key, int keySz, const uchar *val, int valSz)
{
    if(httpreq == NULL || key == NULL || val == NULL)
    {
        return -1 ;
    }
    return addData(&httpreq ->data, key, keySz, val, valSz) ;
}

int addPostFile(HTTPREQUEST *httpreq, const char *name, const char *filename)
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
   if(httpreq ->currFileCount >= MAX_FILE_COUNT)
   {
        printf("has beyond the max file count\n") ;
        return -1 ;
   }
   if((httpreq ->filename[httpreq ->currFileCount] = strdup(filename)) == NULL)
   {
        printf("fail to copy the filename:[%s]\n", filename) ;
        return -1 ;
   }
   if((httpreq ->name[httpreq ->currFileCount] = strdup(name)) == NULL)
   {
        printf("fail to copy the name:[%s]\n", name);
        return -1 ;
   }
   httpreq ->currFileCount += 1 ;
   return 0 ;
}

static int addDefaultRequestHeader(HTTPREQUEST *httpreq)
{
    if(!hasHeader(&httpreq ->header, "Content-Type"))
    {
        addRequestHeader(httpreq, "Content-type", "plain/text") ;
    }
    if(!hasHeader(&httpreq ->header, "Host"))
    {
        addRequestHeader(httpreq, "Host", httpreq ->url ->host) ;
    }
    if(!hasHeader(&httpreq ->header, "User-Agent"))
    {
        addRequestHeader(httpreq, "User-Agent", "httplib/1.0") ;
    } 
    return 0 ;
}

static int calcDataLen(DATA *data)
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

static HTTPRESPONSE *getResponse(int fd)
{
    int totalLen = 0 ;
    int hdrLen = 0 ;
    char *hdrbuff = readUntil(fd, &totalLen, &hdrLen, "\r\n\r\n") ;
    if(hdrbuff == NULL)
    {
        return NULL ;
    }
    HTTPRESPONSE *httprsp = initHttpResponse(MALLOC(sizeof(HTTPRESPONSE))) ;
    if(httprsp == NULL)
    {
        printf("fail to create response object:%s\n", strerror(errno)) ;
        goto __fails ; 
    }
    setResponseSocket(httprsp, fd) ;
    setResponseHeaderBuff(httprsp, hdrbuff, 0) ; 
    setResponseExtraData(httprsp, hdrbuff + hdrLen, totalLen - hdrLen) ;
    if(parseHttpResponseHeader(httprsp) != 0)
    {
        printf("fail to parse http response header\n") ;
        goto __fails ;
    }
    return httprsp ;

__fails:
   freeHttpResponse(httprsp) ; 
   FREE(httprsp) ;
   return NULL ;
}

//设置边界
int setBoundary(HTTPREQUEST *httpreq, char *boundary)
{
   if(httpreq == NULL || boundary == NULL) 
   {
        return -1 ;
   }
   memcpy(httpreq ->boundary, boundary, MIN(strlen(boundary), sizeof(httpreq ->boundary)-1)) ;
   return 0 ;
}

static char *produceBoundary(HTTPREQUEST *httpreq)
{
    if(httpreq == NULL || strlen(httpreq ->boundary) != 0) 
    {
        return NULL ;
    }
    char chs[] = {"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZYZ"} ;
    int chsLen = sizeof(chs) - 1;
    int i = 0 ;
    int count = MIN(sizeof(httpreq ->boundary)-1, MAX_BOUNDRY_BYTE) ;
    srand(time(NULL)) ;
    for(i=0; i< count ;i++)
    {
       httpreq ->boundary[i] = chs[rand()%chsLen] ;
    }
    return httpreq ->boundary ;
}

HTTPRESPONSE *sendRequest(HTTPREQUEST *httpreq, int timeout, int method)
{
    if(httpreq == NULL)
    {
        return NULL ;
    }
    httpreq ->method = method ;
    if(httpreq ->method == GET)
    {
        return sendRequestWithGET(httpreq, timeout); 
    }
    else
    {
       return sendRequestWithPOST(httpreq, timeout)  ;
    }
}

HTTPRESPONSE *sendRequestWithGET(HTTPREQUEST *httpreq, int timeout)
{
    if(httpreq == NULL)
    {
        return  NULL ; 
    }
    URL *url = httpreq ->url ;
    httpreq ->method = GET ;
    BUFFER buff ;
    initBuffer(&buff) ;
     //开始拼凑请求头
    appendBuffer(&buff, STR_GET, STR_GET_LEN) ;
    appendBuffer(&buff, " /", strlen(" /")) ;
    if(url ->path != NULL && strlen(url ->path) != 0)
    {
        appendBuffer(&buff, url ->path, strlen(url ->path)) ;
    }
    appendBuffer(&buff, "?", 1) ;
    if(url ->query != NULL && strlen(url ->query) != 0)
    {
        appendBuffer(&buff, url ->query, strlen(url ->query)) ;
        appendBuffer(&buff, "&", 1) ;
    }
    //如果是GET方法的话，data部分应该放到url中
    if(!isEmpty(&httpreq ->data))
    {
        FOREACH(key, val, &httpreq ->data)
        {
            appendBuffer(&buff, key, strlen(key)) ;
            appendBuffer(&buff, "=", 1) ;
            if(val != NULL)
            {
                appendBuffer(&buff, val, strlen(val)) ;
            }
            appendBuffer(&buff, "&", 1) ;
        }
        lstripBuffer(&buff, '&') ;
    }
    lstripBuffer(&buff, '&') ;
    lstripBuffer(&buff, '?') ;
    appendBuffer(&buff, " ", 1) ;
    appendBuffer(&buff, "HTTP/", 5) ;
    appendBuffer(&buff, httpreq ->version, strlen(httpreq ->version)) ;
    appendBuffer(&buff, "\r\n", 2) ; 
    addDefaultRequestHeader(httpreq) ;    
    appendBuffer(&buff, header2String(&httpreq ->header), headerLen(&httpreq ->header)) ;
    appendBuffer(&buff, "\r\n", 2) ;
    int fd = connectToServer(url->host, url ->port == NULL ?DEFAULT_PORT:url ->port, timeout) ;
    if(fd < 0)
    {
       perror("fd < 0") ;
       return NULL; 
    }
    if(sendData(fd, getBufferData(&buff), getBufferSize(&buff)) != getBufferSize(&buff))
    {
        printf("send [%s] fail\n", getBufferData(&buff)) ;
    }
    freeBuffer(&buff) ;
    return getResponse(fd) ;
}

#define CONTENT_DISPOSITION  "Content-Disposition: form-data; name=\""
#define CONTENT_TYPE         "Content-Type: text/plain; charset=utf-8\r\n"
#define CONTENT_ENCODING     "Content-Transfer-Encoding:8bit\r\n"

static int buildFormData(HTTPREQUEST *httpreq, int sockfd, BUFFER *buff, int issend)
{
    if(httpreq == NULL || buff == NULL || sockfd < 0)
    {
        return -1 ;
    }
    int currSz = getBufferSize(buff) ;
    produceBoundary(httpreq) ;
    int boundaryLen = strlen(httpreq ->boundary) ;
    FOREACH(key, val, &httpreq ->data)
    {
        //add boundary
        appendBuffer(buff, "--", 2) ;
        appendBuffer(buff, httpreq ->boundary, boundaryLen) ;
        appendBuffer(buff, "\r\n", 2) ;
        
        //add content-type disposition encoding
        appendBuffer(buff, CONTENT_DISPOSITION, strlen(CONTENT_DISPOSITION)) ;
        appendBuffer(buff, key, strlen(key)) ;
        appendBuffer(buff, "\"\r\n", 3);
        appendBuffer(buff, CONTENT_TYPE, strlen(CONTENT_TYPE)) ;
        appendBuffer(buff, CONTENT_ENCODING, strlen(CONTENT_ENCODING)) ;
        appendBuffer(buff, "\r\n", 2) ;

        appendBuffer(buff, val, strlen(val)) ;
        appendBuffer(buff, "\r\n", 2) ;
    }
    int len = getBufferSize(buff) - currSz ;
    if(issend == 0)
    {
        dropData(buff, len) ;
        return len - currSz ;
    }
    if(writeAll(sockfd, getBufferData(buff), getBufferSize(buff)) != getBufferSize(buff))
    {
        perror("fail to send data\n") ;
        return -1 ;
    }
    return 0 ;
}

static int64_t buildBodyFromFile(HTTPREQUEST *httpreq, int sockfd, int issend)
{
    
    if(httpreq == NULL || sockfd < 0)
    {
        return -1 ;
    }
    produceBoundary(httpreq) ;
    int boundaryLen = strlen(httpreq ->boundary) ;
    int64_t fileLen = 0 ;
    int i = 0 ;
    char buff[16*1024] = {0};
    int offset = 0 ;
    for(i=0; i < httpreq ->currFileCount ; i++)
    {
        /*
         * --boundary\r\n
         * content-disposition:form-data; name="file";filename="test.txt"\r\n
         * content-type:xxxx\r\n
         * Content-Transfer-Encoding:8bit\r\n
         * \r\n
         * ........\r\n
         */
        strcpy(buff + offset, "--") ;
        offset += 2 ;
        strcpy(buff + offset, httpreq ->boundary) ;        
        offset += strlen(httpreq ->boundary) ; 
        strcpy(buff + offset, "\r\n") ;
        offset += 2 ;

        strcpy(buff + offset, CONTENT_DISPOSITION) ;
        offset += strlen(CONTENT_DISPOSITION) ;
        strcpy(buff + offset, httpreq ->name[i]) ;
        offset += strlen(httpreq ->name[i]) ;
        strcpy(buff + offset, "\"") ;
        offset += 1 ;
        strcpy(buff + offset, ";filename=\"") ;
        offset += strlen(";filename=\"") ;
        strcpy(buff + offset, httpreq ->filename[i]) ;
        offset += strlen(httpreq ->filename[i]) ;
        strcpy(buff + offset, "\"\r\n") ;
        offset += strlen("\"\r\n");
        strcpy(buff + offset, "Content-Type:application/octet-stream\r\n") ;
        offset += strlen("Content-Type:application/octet-stream\r\n") ;
        strcpy(buff + offset, "Content-Transfer-Encoding:8bit\r\n\r\n") ;
        offset += strlen("Content-Transfer-Encoding:8bit\r\n\r\n") ; 
        if(issend == 0)
        {
            struct stat st ;
            if(stat(httpreq ->filename[i], &st) == -1)
            {
               return -1 ; 
            }
            fileLen += offset ;
            //文件内容后面需要跟上 \r\n，所以需要+2
            fileLen += st.st_size + 2;
            continue ;
        }
        int ret = 0 ;
        int fd = open(httpreq ->filename[i], O_RDONLY);
        if(fd < 0)
        {
            return -1 ;
        }
        while((ret = readFully(fd, buff + offset, sizeof(buff)-offset)) != 0)
        {
            if(ret < 0)
            {
                close(fd);
                return -1 ;
            }
            ret += offset ;
            offset += 0 ;
            if(writeAll(sockfd, buff, ret) != ret)
            {
               perror("error\n") ; 
               close(fd);
               return -1 ;
            }
        }
        close(fd); 
        strcpy(buff + offset, "\r\n") ;
        offset += 2 ;
    }
    if(issend == 0)
    {
        return fileLen ; 
    }
    if(writeAll(sockfd, buff, offset) != offset)
    {
       perror("error\n") ; 
       return -1 ;
    }
    return 0 ;
}

static int64_t buildBody(HTTPREQUEST *httpreq, int sockfd, BUFFER *buff, int issend)
{
    if(httpreq == NULL || buff == NULL || sockfd < 0)
    {
        return -1;
    }
    int formdataLen = buildFormData(httpreq, sockfd, buff, issend) ;
    int boundaryLen = strlen(httpreq ->boundary) ;
    int64_t fileLen = buildBodyFromFile(httpreq, sockfd, issend) ;
    if(fileLen == -1)
    {
        return -1 ;
    }
    if(issend == 0)
    {
       //--boundary--\r\n 
       return fileLen + formdataLen + 2 + boundaryLen + 2 + 2 ;
    } 
    char endData[1024] = {0} ;
    strcpy(endData, "--") ;
    strcat(endData, httpreq ->boundary) ;
    strcat(endData, "--\r\n") ;
    if(writeAll(sockfd, endData, boundaryLen + 6) != boundaryLen + 6)
    {
        return -1 ;
    }
    return 0 ;
}

HTTPRESPONSE *sendRequestWithPOST(HTTPREQUEST *httpreq, int timeout)
{
    if(httpreq == NULL || httpreq ->url == NULL) 
    {
        return NULL;
    }
    BUFFER buff ;
    initBuffer(&buff) ;
    httpreq ->method = POST ;
    URL *url = httpreq ->url ;
     //开始拼凑请求头
    appendBuffer(&buff, STR_POST, STR_POST_LEN) ;
    appendBuffer(&buff, " /", strlen(" /")) ;
    if(url ->path != NULL && strlen(url ->path) != 0)
    {
        appendBuffer(&buff, url ->path, strlen(url ->path)) ;
    }
    appendBuffer(&buff, "?", 1) ;
    if(url ->query != NULL && strlen(url ->query) != 0)
    {
        appendBuffer(&buff, url ->query, strlen(url ->query)) ;
        appendBuffer(&buff, "&", 1) ;
    }
    //如果是GET方法的话，data部分应该放到url中
    lstripBuffer(&buff, '&') ;
    lstripBuffer(&buff, '?') ;
    appendBuffer(&buff, " ", 1) ;
    appendBuffer(&buff, "HTTP/", 5) ;
    appendBuffer(&buff, httpreq ->version, strlen(httpreq ->version)) ;
    appendBuffer(&buff, "\r\n", 2) ; 
    produceBoundary(httpreq) ;
    char tmpbuff[1024] = {0} ;
    strcpy(tmpbuff, "multipart/form-data; boundary=") ;
    strcat(tmpbuff, httpreq ->boundary) ;
    addRequestHeader(httpreq, "Content-Type", tmpbuff) ;
    //上传文件一定得加上Content-Length，否则无法解析出来
    char lenBuff[50] = {0} ;
    int64_t totalLen = buildBody(httpreq, 0, &buff, 0) ;
    addRequestHeader(httpreq, "Content-Length", itoa(totalLen,lenBuff)) ;
    addDefaultRequestHeader(httpreq) ;    
    appendBuffer(&buff, header2String(&httpreq ->header), headerLen(&httpreq ->header)) ;
    appendBuffer(&buff, "\r\n", 2) ;
    int fd = connectToServer(url->host, url ->port == NULL ?DEFAULT_PORT:url ->port, timeout) ;
    if(fd < 0)
    {
       perror("fd < 0") ;
       return NULL; 
    }
    buildBody(httpreq, fd, &buff, 1) ;
    freeBuffer(&buff) ;
    return getResponse(fd) ;
}
