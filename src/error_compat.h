/*
 * Compatibility header for error() function
 * GNU error() is not available on macOS, so we provide a portable implementation
 */

#ifndef ERROR_COMPAT_H
#define ERROR_COMPAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

/* Portable error() function that works on both Linux and macOS */
static inline void error(int status, int errnum, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    
    /* Print program name and error message */
    fprintf(stderr, "Error: ");
    vfprintf(stderr, format, ap);
    
    /* If errnum is set, print strerror */
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    
    fprintf(stderr, "\n");
    fflush(stderr);
    
    va_end(ap);
    
    /* Exit with status if requested */
    if (status != 0) {
        exit(status);
    }
}

#endif /* ERROR_COMPAT_H */
