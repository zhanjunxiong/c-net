/*
 * err.h
 *
 *  Created on: 2011-8-6
 *      Author:
 */

#ifndef ERR_H_
#define ERR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>

//  This macro works in exactly the same way as the normal assert. It is used
//  in its stead because standard assert on Win32 in broken - it prints nothing
//  when used within the scope of JNI library.
#define myAssert(x) \
    do {\
        if ((!(x))) {\
            fprintf (stderr, "Assertion failed: %s (%s:%d)\n", #x, \
                __FILE__, __LINE__);\
            abort ();\
        }\
    } while (0)

#define gettid()	syscall(__NR_gettid)

#endif /* ERR_H_ */
