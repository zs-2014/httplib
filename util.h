#ifndef __UTIL__H
#define __UTIL__H

#ifdef __cplusplus
extern "C" {
#endif


extern char *replacestr(char *src,const char *patternStr,const char *replaceStr) ;


extern char *upper(char *pstr) ;
extern char *lower(char *pstr) ;
extern int  startwith(const char *pstr1, const char *pstr2);
extern int  endwith(const char *pstr1, const char *pstr2) ;
extern char *headstrip(char *pstr,char ch) ;
extern char *tailstrip(char *pstr,char ch) ;
extern char     *strip(char *pstr,char ch) ;

#ifdef __cplusplus
};
#endif

#endif
