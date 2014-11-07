#ifndef __UTIL__H
#define __UTIL__H

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif
extern const char *casefind(const char *src, const char *needle) ;
extern int connectToServer(const char *server, const char *port, int timeout) ;
extern int sendData(int fd, const void *buff, int sz) ;
extern int isStartWith(const char *src, const char *needle) ;
extern int readFully(int fd, void *buff, int sz) ;
#ifdef __cplusplus
};
#endif

#endif
