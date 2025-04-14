#ifndef	__DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>


//#define DEBUG


#ifdef DEBUG
#define DBG(fmt,...) printf( "%s:%s:%d: " fmt, __FILE__,  __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define DBG(fmt,...)
#endif


#endif //__DEBUG_H__
