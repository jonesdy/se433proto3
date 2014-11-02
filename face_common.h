// Copied from FACE standard 2.0

#ifndef FACE_COMMON_H
#define FACE_COMMON_H        

#include "globals.h"   // I added this


typedef int64_t FACE_SYSTEM_TIME_TYPE; /* 64-bit signed int with 1 nanosecond LSB */
static const int64_t FACE_INF_TIME_VALUE = -1;

typedef enum {
    /* 0 */ FACE_NO_ERROR, /* request valid and operation performed */
    /* 1 */ FACE_NO_ACTION, /* status of system unaffected by request */
    /* 2 */ FACE_NOT_AVAILABLE, /* resource required by request unavailable */
    /* 3 */ FACE_ADDR_IN_USE, /* address currently in use */
    /* 4 */ FACE_INVALID_PARAM, /* invalid parameter specified in request */
    /* 5 */ FACE_INVALID_CONFIG, /* parameter incompatible with configuration */
    /* 6 */ FACE_PERMISSION_DENIED, /* no send permission or connecting to wrong partition */
    /* 7 */ FACE_INVALID_MODE, /* request incompatible with current mode */
    /* 8 */ FACE_TIMED_OUT, /* time-out tied up with request has expired */
    /* 9 */ FACE_MESSAGE_STALE, /*current timestamp exceeds configured limits*/
    /* 10 */ FACE_CONNECTION_IN_PROGRESS,/* asynchronous connection in progress*/
    /* 11 */ FACE_CONNECTION_CLOSED,
    /* 12 */ DATA_BUFFER_TOO_SMALL /* Data buffer was too small for the message */
} FACE_RETURN_CODE_TYPE; /* POSIX ERRONO mapped to these codes by transport services */

/* This type is used to represent a system address. */
/* Note: This is expected to be a pointer type such as (void *) in C. */
typedef void *SYSTEM_ADDRESS_TYPE;

/* This type has a one nanosecond resolution. */
typedef FACE_SYSTEM_TIME_TYPE FACE_TIMEOUT_TYPE;
/* Connection provides reliable or non-reliable transport */

typedef enum {
    FACE_RELIABLE,
    FACE_NON_RELIABLE
} FACE_RELIABILITY_TYPE;

/* Connection provides queuing or sampling behavior */
typedef enum {
    /* 0 */ FACE_QUEUING,
    /* 1 */ FACE_SAMPLING
} FACE_READ_WRITE_BEHAVIOR_TYPE;

typedef int32_t FACE_MESSAGE_RANGE_TYPE;


#endif

