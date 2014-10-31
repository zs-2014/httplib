#ifndef __HTTP_ULR__H
#define __HTTP_URL__H

#define quotebuff(urlstr, sz) quote(urlstr, sz)
#define quotestr(urlstr) quote(urlstr, strlen(urlstr))

#define unquotebuff(urlstr, sz) unquote(urlstr, sz)
#define unquotestr(urlstr) unquote(urlstr, strlen(urlstr))

typedef struct URL
{
    char *urlbuff ;   
    char *protocol ;
    char *username ;
    char *passwd ;
    char *host ;
    char *port ;
    char *path ;
    char *param ;
    char *query ;
    char *frag ;

//private:
    int next ;
}URL;

#ifdef __cplusplus
extern "C" {
#endif

extern int init() ;
extern char* quote(const uchar* urlstr, uint sz) ;
extern uchar* unquote(const char* urlstr, uint sz) ; 
extern URL* parseURL(const char *pUrlStr) ;

#ifdef __cplusplus
} 
#endif

#endif
