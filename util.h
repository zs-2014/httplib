#ifndef __UTIL__H
#define __UTIL__H

#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif
extern const char *casefind(const char *src, const char *needle) ;
extern int create_server_socket(const char *ip, const char *port, int lsncnt) ;
extern int connect_to_server(const char *server, const char *port, int timeout) ;
extern int send_data(int fd, const void *buff, int sz) ;
extern int is_start_with(const char *src, const char *needle) ;
extern int write_all(int fd, void *buff, int sz) ;
extern int read_fully(int fd, void *buff, int sz) ;
extern char *itoa(int64_t num, char *buff) ;
extern char *read_until(int fd, int *total_len, int *len, const char *flagstr) ;
#ifdef __cplusplus
};
#endif

#endif
