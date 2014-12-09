#ifndef __HTTP_URL__H
#define __HTTP_URL__H

typedef struct http_url_t
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
}http_url_t;

#ifdef __cplusplus
extern "C" {
#endif

extern int free_url(http_url_t *p_url) ;
extern http_url_t* parse_url(const char *p_url_str) ;

#ifdef __cplusplus
} 
#endif

#endif
