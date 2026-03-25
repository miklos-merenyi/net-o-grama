#define DEBUG
#include <ncurses.h>
#include <error.h>
#include <errno.h>
void initLogfile();
void closeLogfile();
void debug(int level,char* msg, ...);
//void vdebug(int level,char* fmt, va_list ap);
void echof(int level,char* msg, ...);
//void vechof(int level,char* fmt, va_list ap);
