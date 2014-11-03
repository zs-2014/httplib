#ifndef __HTTP_URL__H
#define __HTTP_URL__H

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

extern int freeURL(URL *pUrl) ;
extern URL* parseURL(const char *pUrlStr) ;

#ifdef __cplusplus
} 
#endif

#endif
