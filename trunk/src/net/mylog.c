
#include "mylog.h"

#include <stdio.h>
#include <stdarg.h>

#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LOGMSG_LEN		1024

static char* logLevelDesc[]	=
								{"LOG_DEBUG",
								"LOG_INFO",
								"LOG_NOTICE",
								"LOG_WARNING"};

typedef enum Color
{
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    GREY,
    YELLOW,
    LRED,
    LGREEN,
    LBLUE,
    LMAGENTA,
    LCYAN,
    WHITE,
    COLORCOUNT
} Color;

static enum Color  logLevelColor[] = {
	    RED,
	    GREEN,
	    BROWN,
	    BLUE};

typedef enum bool {
	true = 0,
	false = 1
} bool;

void SetColor(bool stdout_stream, Color color)
{
    enum ANSITextAttr
    {
        TA_NORMAL=0,
        TA_BOLD=1,
        TA_BLINK=5,
        TA_REVERSE=7
    };

    enum ANSIFgTextAttr
    {
        FG_BLACK=30, FG_RED,  FG_GREEN, FG_BROWN, FG_BLUE,
        FG_MAGENTA,  FG_CYAN, FG_WHITE, FG_YELLOW
    };

    enum ANSIBgTextAttr
    {
        BG_BLACK=40, BG_RED,  BG_GREEN, BG_BROWN, BG_BLUE,
        BG_MAGENTA,  BG_CYAN, BG_WHITE
    };

    static int UnixColorFG[COLORCOUNT] =
    {
        FG_BLACK,                                           // BLACK
        FG_RED,                                             // RED
        FG_GREEN,                                           // GREEN
        FG_BROWN,                                           // BROWN
        FG_BLUE,                                            // BLUE
        FG_MAGENTA,                                         // MAGENTA
        FG_CYAN,                                            // CYAN
        FG_WHITE,                                           // WHITE
        FG_YELLOW,                                          // YELLOW
        FG_RED,                                             // LRED
        FG_GREEN,                                           // LGREEN
        FG_BLUE,                                            // LBLUE
        FG_MAGENTA,                                         // LMAGENTA
        FG_CYAN,                                            // LCYAN
        FG_WHITE                                            // LWHITE
    };

    fprintf((stdout_stream? stdout : stderr), "\x1b[%d%sm",UnixColorFG[color],(color>=YELLOW&&color<COLORCOUNT ?";1":""));
}

void mylog(int level, const char *fmt, ...) {

    char buf[64];
    char msg[MAX_LOGMSG_LEN];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    time_t now = time(NULL);
    strftime(buf,sizeof(buf),"%d %b %H:%M:%S",localtime(&now));
    SetColor(true, logLevelColor[level]);
    printf("[%d] %s %s %s\n",	(int)getpid(),	buf,	logLevelDesc[level], msg);
}

