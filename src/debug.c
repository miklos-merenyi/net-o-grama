#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ncurses.h>
#include <time.h>
#include <error.h>
#include <errno.h>

extern int debugLevel;

FILE* logstream;

void vfechof(FILE* stream, char* beg, char* fmt, va_list ap)
{
    //fprintf(stream,"%ld",time(NULL));
    fprintf(stream,"%s",beg);
    vfprintf(stream,fmt,ap);
    fflush(stream);
}


void debug(int level, char* msg, ...)
{
    //	FILE *stream;
    if (level>debugLevel) return;
    if (logstream==NULL) return;
    va_list ap;
    va_start(ap,msg);
    //vdebug(level,msg, ap);
    vfechof(logstream,"debug :",msg,ap);
    va_end(ap);

}


void echof(int level, char* msg, ...)
{
    if (level>debugLevel) return;
    va_list ap;
    va_start(ap,msg);
    vfechof(stdout,"",msg,ap);
    if (logstream) vfechof(logstream,"debug :",msg,ap);

    va_end(ap);
}


void initLogfile(char *logfile)
{
    logstream=fopen(logfile,"w");
    if (logstream==NULL) error(1,errno,"Error opening the logfile \"%s\"",logfile);

}


void closeLogfile()
{
    fclose(logstream);
}
