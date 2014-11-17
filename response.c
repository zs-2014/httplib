#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "response.h"
#include "util.h"
#include "global.h"

#define VERSION(m, n) (((m)-'0')*10 + (n) - '0')
#define MIN_VERSION VERSION('1', '0') 

//初始化http 响应头
int initHttpResponseHeader(HttpResponseHeader *httprsphdr)
{
    if(httprsphdr == NULL)
    {
        return -1 ;
    }
    memset(httprsphdr, 0, sizeof(*httprsphdr)) ;
    httprsphdr ->headbuff = NULL ;
    httprsphdr ->size = 0 ;
    httprsphdr ->key_val = NULL ;
    return 0 ;
}

//释放http响应头
int freeHttpResponseHeader(HttpResponseHeader * httphdr)
{
    if(httphdr == NULL)
    {
        return 0 ;
    }
    if(httphdr ->headbuff)
    {
        FREE(httphdr ->headbuff) ;
        httphdr ->headbuff = NULL ;
    }

    if(httphdr ->key_val)
    {
        FREE(httphdr ->key_val) ;
        httphdr ->key_val = NULL ;
    }
    return 0 ;
}

//检查响应的http版本是否正确
static int checkVersion(HttpResponseHeader *httphdr)
{
    if(httphdr == NULL)
    {
        return -1;
    }
    char *p = httphdr ->version;
    char *p1 = strchr(p, '/') ;
    if(p1 == NULL ||strncasecmp("HTTP", p, p1 - p) != 0)
    {
        return -1;
    }
    if(!isdigit(*++p1) || *++p1 != '.'|| !isdigit(*++p1))
    {
        return -1 ;
    }
    if(VERSION(*(p1 - 2), *p1) < MIN_VERSION)
    {
        return -1;
    }
    return 0 ;
}

//检查http响应状态码是否合法
static int checkCode(HttpResponseHeader * httphdr)
{
    if(httphdr == NULL)
    {
        return -1;
    }
    int code = atoi(httphdr ->code) ;
    if(code < 100 || code > 500)
    {
        return -1;
    }
    return 0;
}

//检查http 返回状态信息
static int checkReason(HttpResponseHeader *httphdr)
{
    if(httphdr == NULL)
    {
        return -1;
    }
    return 0;
}

//检查http响应的第一行
static int checkStatusLine(HttpResponseHeader *httphdr)
{
    if(httphdr == NULL)
    {
        return -1 ; 
    } 
    return !(!checkVersion(httphdr) && !checkCode(httphdr) && !checkReason(httphdr)) ;
}

#define COPY_FIELD(beg, delimit, dst, sz) ({\
    char *end = strstr(beg, delimit) ;\
    int errflag = 0 ;\
    if(end <= beg || end - beg >= sz)\
    {\
        return -1 ;\
    }\
    strncpy(dst, beg, end - beg) ;\
    end - beg + strlen(delimit) ;\
})

//解析http响应的第一行  比如 HTTP/1.1 200 OK
static int parseStatusLine(HttpResponseHeader *httphdr ,const char *hdrbuff)
{
    if(hdrbuff == NULL || httphdr == NULL)
    {
        return -1 ;
    }
    const char *p = hdrbuff ;
    p += COPY_FIELD(p, " ", httphdr ->version, sizeof(httphdr ->version))  ;
    p += COPY_FIELD(p, " ", httphdr ->code,    sizeof(httphdr ->code)) ; 
    p += COPY_FIELD(p, "\r\n", httphdr ->reason, sizeof(httphdr ->reason)) ;
    return p - hdrbuff ;
}

//删除http响应行中续行，并把所有的续行合并成一行
static char *deleteContinueLineFlag(char *hdrbuff)
{
    if(hdrbuff == NULL)
    {
        return NULL ;
    }
    char *tmp = hdrbuff ;
    while(*tmp != '\0') 
    {
        char *p = tmp ;
        if(*tmp++ == '\r' && *tmp == '\n')
        {
            tmp++ ;
          //此行是续行
           if(*tmp == ' ' || *tmp == '\t') 
           {
                tmp++ ;
                strcpy(p, tmp) ;
                tmp = p + 1;
           }
           //\r\n\r\n 意味着http头的结束
           else if(*tmp++ == '\r' && *tmp == '\n')
           {
                *++tmp = '\0' ;
                return hdrbuff;
           }
        }
    }
    return NULL;
}

//解析http响应头中的名称/值对
static int parseKeyValue(HttpResponseHeader *httphdr)
{ 
    if(httphdr == NULL)
    {
        return -1 ;
    }
    //跳过首行
    char *p = strstr(httphdr ->headbuff, "\r\n") ;
    if(p == NULL) 
    {
        return -1 ;
    }
    p += 2 ;
    while(*p != '\0')
    {
        char *key_b = p ;
        //名称与键之间 以 : 隔开
        char *key_e = strchr(key_b, ':');
        if(key_e != NULL)
        {
            char *val_b = key_e + skipChar(key_e + 1, ' ') + 1 ; 
            //此行的结束
            char *val_e = strstr(val_b, "\r\n");
            if(val_e == NULL)
            {
                return -1 ;
            }
            *key_e = '\0' ;
            *val_e = '\0' ;
            p = val_e + 2 ;
            if(httphdr ->count >= httphdr ->size)
            {
               httphdr ->key_val = (NODE *)REALLOC(httphdr ->key_val, sizeof(NODE)*(httphdr ->size + 20));
               if(httphdr ->key_val == NULL)
               {
                    return -1 ;
               }
               httphdr ->size += 20  ;
            }
            httphdr ->key_val[httphdr ->count].key = key_b ;
            httphdr ->key_val[httphdr ->count++].value = val_b ;
        }
        //http 解析完毕  \r\n标志着结束
        else if(key_e == NULL && strcmp(key_b, "\r\n") == 0)
        {
            return 0 ;
        }
        else
        {
            return -1 ;
        }
    }
    return -1 ;
}

//解析httprsp中hdr中的http响应头
int parseHttpResponseHeader(HTTPRESPONSE *httprsp)
{
    if(httprsp == NULL)
    {
        return -1;
    }
    HttpResponseHeader *httphdr = &httprsp ->httprsphdr ; 
    deleteContinueLineFlag(httphdr ->headbuff) ;
    // 第一次分配100个键值对
    httphdr ->key_val = (NODE *)MALLOC(sizeof(NODE)*30) ;
    if(httphdr ->key_val == NULL)
    {
        goto _fails ;
    }
    httphdr ->size = 30;
    httphdr ->count = 0 ;
    //提取开始行
    int i = parseStatusLine(httphdr, httphdr ->headbuff);
    if(i < 0)
    {
       goto _fails ; 
    }
    if(checkStatusLine(httphdr) != 0) 
    {
        goto _fails ;
    }
    if(parseKeyValue(httphdr) != 0)
    {
        printf("fail to parse key and value\n") ;
        goto _fails ;
    }
    return 0 ;

_fails:
    freeHttpResponseHeader(httphdr) ;
    return -1;
}

//初始化一个http响应对象
HTTPRESPONSE *initHttpResponse(HTTPRESPONSE *httprsp)
{
   if(httprsp == NULL) 
   {
        return NULL ;
   }
   httprsp ->rspfd = -1 ;
   httprsp ->nextChunkSize = 0 ;
   memset(httprsp ->buff, 0, sizeof(httprsp ->buff)) ;
   httprsp ->currSz = 0 ;
   httprsp ->currPos = 0 ;
   httprsp ->chunkCount = 0 ;
   initHttpResponseHeader(&httprsp ->httprsphdr) ;
   return httprsp ;
}

int setResponseExtraData(HTTPRESPONSE *httprsp, char *extraData, int sz)
{ 
    if(httprsp == NULL || extraData == NULL)
    {
        return -1 ;
    }
    httprsp ->currSz = MIN(sizeof(httprsp ->buff) - 1, sz) ;
    memcpy(httprsp ->buff, extraData, httprsp ->currSz) ;
    printf("%s", httprsp ->buff) ;
    return 0 ;
}

int freeHttpResponse(HTTPRESPONSE *httprsp)
{
    if(httprsp == NULL)    
    {
        return -1 ;
    }
    if(httprsp ->rspfd >= 0 )
    {
        close(httprsp ->rspfd) ;
    }
    httprsp ->rspfd = -1 ;
    httprsp ->currPos = 0;
    httprsp ->currSz = 0 ;
    return freeHttpResponseHeader(&httprsp ->httprsphdr) ;
}

//设置http响应对象中头缓冲
//当isCopy 为False时 hdrbuff必须是malloc分配最终由http释放
//isCopy == True 从hdrbuff中拷贝一份
int setResponseHeaderBuff(HTTPRESPONSE *httprsp, char *hdrbuff, int isCopy)
{
   if(httprsp == NULL || hdrbuff == NULL) 
   {
        return -1 ;
   }
   if(isCopy == 0)
   {
        httprsp ->httprsphdr.headbuff = hdrbuff ;
   }   
   else
   {
        httprsp ->httprsphdr.headbuff = strdup(hdrbuff) ;
   }
   return httprsp ->httprsphdr.headbuff == NULL ? -1 : 0 ;
}

int setResponseSocket(HTTPRESPONSE *httprsp, int sockfd)
{
    if(httprsp == NULL || sockfd < 0)
    {
        return -1 ;
    }
    httprsp ->rspfd = sockfd ;
    return 0 ;
}

static int searchResponseHeader(HttpResponseHeader *httphdr, const char *key, int num)
{
    if(httphdr == NULL || key == NULL)
    {
        return -1 ;
    } 
    int i = 0 ;
    for(i = 0 ;i < httphdr ->count ; i++)
    {
        //忽略大小写
        if(strcasecmp(httphdr ->key_val[i].key, key) == 0)
        {
            //num代表获取第几个key  因为在http头中，可能有多个相同的key,比如Set-Cookie
            if(--num == 0)
            {
                return i ;
            }
        }
    }
    return -1 ;
}

int copyValueFromResponseHeader(HttpResponseHeader *httphdr, const char *key, void *buff, int sz, int num)
{
    if(httphdr == NULL || key == NULL)
    {
        return -1 ;
    } 
    int idx = searchResponseHeader(httphdr, key, num) ;
    if(idx != -1)
    {
       if(sz == 0) 
       {
            return strlen(httphdr ->key_val[idx].value) ;
       }
       else
       {
            strncpy(buff, httphdr ->key_val[idx].value, sz) ;
            return strlen(httphdr ->key_val[idx].value) ;
       }
    }
    return -1 ;
}

char *getTransferEncoding(HttpResponseHeader *httphdr, void *buff, int sz)
{
    int ret = copyValueFromResponseHeader(httphdr, "Transfer-Encoding", buff, sz, 1) ; 
    if(ret == -1 && ret >= sz)    
    {
        return NULL ; 
    }
    return buff ;
}

//传输编码是否是chunked
//chunked编码的格式如下
/*chunked-len\r\n
  chunked-data\r\n
  .....
  0\r\n
*/
int isChunkedEncoding(HttpResponseHeader *httphdr)
{
    char buff[512] = {0} ;
    char *p = getTransferEncoding(httphdr, buff, sizeof(buff) - 1) ;
    return p == NULL ? 0 :strcasecmp(p, "chunked") == 0 ;
}

int getContentLength(HttpResponseHeader *httphdr)
{
    char buff[40] = {0};
    int ret = copyValueFromResponseHeader(httphdr, "Content-Length", buff, sizeof(buff)-1, 1) ;
    if(ret == -1)
    {
        return -1 ;
    }
    return atoi(buff) ;
}

int copyResponseHeaderValue(HTTPRESPONSE *httprsp, const char *key, void *buff, int sz, int num)
{
    if(httprsp == NULL || key == NULL)
    {
        return -1 ;
    }
    return copyValueFromResponseHeader(&httprsp ->httprsphdr, key, buff, sz, num) ;
}
static int readChunkedLen(HTTPRESPONSE *httprsp)
{
    int flag = 0 ;
    char beg[2] = {0} ;
    //读取chunk头 并获取长度
    if(httprsp ->currSz != 0)
    {
        if(httprsp ->chunkCount != 0)
        {
            httprsp ->currPos += 2 ;
        }
        char *p = strchr(httprsp ->buff + httprsp ->currPos, '\r') ; 
        if(p != NULL)
        {
            if(p[1] == '\n')   
            {
                    httprsp ->nextChunkSize = strtol(httprsp ->buff + httprsp ->currPos, (char **)NULL, 16) ; 
                    httprsp ->currSz -= p - (httprsp ->buff + httprsp ->currPos) + 2 ;
                    httprsp ->currPos = p - httprsp ->buff + 2 ;
                    return 0 ;
            }
            else if(p[1] == '\0')
            {
                *p = '\0' ;
                httprsp ->nextChunkSize = strtol(p, (char **)NULL, 16) ; 
                flag = 1 ;
            }
            else
            {
                return -1 ;
            }
       }
       else
       {
            httprsp ->nextChunkSize = strtol(httprsp ->buff + httprsp ->currPos, (char **)NULL, 16) ;
            httprsp ->currPos = 0 ;
            httprsp ->currSz = 0 ;
       }
    }
    do
    {
        int ret = readFully(httprsp ->rspfd, beg, 1) ;
        if(ret != 1)
        {
            printf("chunk 数据格式错误") ;
            return -1 ;
        }
        if(isxdigit(beg[0]) && flag == 0)
        {
            httprsp ->nextChunkSize = httprsp ->nextChunkSize*16 + (beg[0] >= '0' && beg[0] <= '9' ? beg[0] - '0' : toupper(beg[0] - 'A' + 10)) ;
        }
        else if (beg[0] == '\n' && flag == 1)
        {
            break ;
        }
        else if(flag == 0 && beg[0] == '\r')
        {
            flag = 1 ;
        }
        else
        {
            printf("chunk 数据格式错误\n") ; 
            return -1 ;
        }
    }while(1) ;
    return 0 ;
}

//响应头时chunked数据
int readChunkedResponse(HTTPRESPONSE *httprsp, void *buff, int sz)
{
    if(httprsp == NULL || buff == NULL || sz <= 0 || httprsp ->rspfd < 0)
    {
        return 0 ;
    }
    int nRecv = 0 ;
    while(sz != 0)
    {
        //缓冲区中有数据，且当前chunk还未读完
        if(httprsp ->currSz != 0 && httprsp ->nextChunkSize != 0)
        {
            int min = MIN(sz, httprsp ->currSz) ;
            min = MIN(httprsp ->nextChunkSize, min) ;
            memcpy(((char *)buff) + nRecv, httprsp ->buff + httprsp ->currPos, min) ;
            sz -= min ; 
            httprsp ->currSz -= min ; 
            httprsp ->nextChunkSize -= min ;
            httprsp ->currPos += min ;
            nRecv += min ;
        }
        //需要读取下一个chunk
        else if(httprsp ->currSz == 0) 
        {
            //chunk size为0 的话，开始读取下一个chun块
            if(httprsp ->nextChunkSize == 0)
            {
                //非第一次读chunk数据，需要跳过数据尾部跟着的\r\n
                if(httprsp ->chunkCount != 0)
                {
                    char end[3] = {0} ;
                    //读出数据尾部结束标记的两个字节
                    if(readFully(httprsp ->rspfd, end, 2) != 2 ||(end[0] != '\r' || end[1] != '\n'))
                    {
                        printf("chunk 数据格式错误\n") ;
                        return -1;
                    }
                }
                if(readChunkedLen(httprsp) != 0)
                {
                    return -1 ;
                }
                httprsp ->chunkCount += 1 ;
            }

           //经过上面的处理之后，如果还是为0则表明是最后一个chunk了
           if(httprsp ->nextChunkSize == 0)
           {
               return nRecv ;
           }
           //限制一次读取的数据不能超过chunk size中指定的大小
           int min = MIN(sz, sizeof(httprsp ->buff)-1) ;
           min = MIN(httprsp ->nextChunkSize, min) ;
           int ret = readFully(httprsp ->rspfd, httprsp ->buff, min) ;
           if(ret != min)
           {
               printf("read error\n") ;
               return -1;
           }
           httprsp ->buff[ret] = '\0' ;
           httprsp ->currSz = ret ;
           httprsp ->currPos = 0 ;
        }
        else if(httprsp ->nextChunkSize == 0)
        {
            if(readChunkedLen(httprsp) != 0)
            {
                return -1 ;
            }
            httprsp ->chunkCount += 1 ;
        }
    }
    return nRecv ;
}

int readResponse(HTTPRESPONSE *httprsp, void *buff, int sz)
{
    if(httprsp == NULL || buff == NULL || sz <= 0 || httprsp ->rspfd < 0)
    {
        return 0 ;
    } 
    //采用chunked编码
    if(isChunkedEncoding(&httprsp ->httprsphdr))
    {
        return readChunkedResponse(httprsp, buff, sz) ;
    }
    else
    {
        if(httprsp ->currSz != 0)
        {
            memcpy(buff, httprsp ->buff + httprsp ->currPos, MIN(httprsp ->currSz, sz)) ;
            //如果在读取响应头时 读出的body数据大于要取出的数据
            if(httprsp ->currSz >= sz)
            {
                httprsp ->currSz -= sz ;
                httprsp ->currPos += sz ;
                return sz ;
            }
            else
            {
                httprsp ->currSz = 0 ;
                httprsp ->currPos = 0 ;
                sz -= httprsp ->currSz ;
            }
        }                //响应头设置了Content-Length
        int length = getContentLength(&httprsp ->httprsphdr) ; 
        if(length != -1)
        {
            return readFully(httprsp ->rspfd, buff, MIN(length, sz)) ;
        }
        else
        {
            //没有采用chunked也没有指定Content-Length
            return readFully(httprsp ->rspfd, buff, sz) ;
        }
    }
}
