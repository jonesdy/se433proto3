#ifndef __GLOBALS_H
#define __GLOBALS_H

// Comment out exactly one of these, uncomment the other
//#define WINDOWS_OS
//#define LINUX_OS

#ifdef WINDOWS_OS
   #define _CRT_SECURE_NO_WARNINGS    // remove depreacated warnings such as on fopen
#endif

#include <stdint.h>


#ifdef LINUX_OS
   #include <stdbool.h>
#endif

#ifdef WINDOWS_OS
   typedef uint8_t _Bool;
   #define true 1
   #define false 0
#endif


#endif
