#pragma once
#ifdef __ANDROID__
#include <android/log.h>
#else
#include <stdio.h>
#include <stdarg.h>
#define ANDROID_LOG_ERROR 6
static inline int __android_log_print(int priority,const char *tag,const char *format,...){
    (void)priority;fprintf(stderr,"[%s] ",tag);
    va_list args;va_start(args,format);int n=vfprintf(stderr,format,args);va_end(args);
    fputc('\n',stderr);return n;
}
#endif
