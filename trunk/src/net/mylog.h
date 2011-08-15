
#ifndef LOG_H_
#define LOG_H_

#ifdef _cplusplus
extern "C" {
#endif

typedef enum LogLevel {
	LOG_DEBUG			=	0,
	LOG_INFO				=	1,
	LOG_NOTICE			=	2,
	LOG_WARNING	=	3
} LogLevel;

void mylog(int level, const char *fmt, ...);

#ifdef _cplusplus
}
#endif

#endif /* LOG_H_ */
