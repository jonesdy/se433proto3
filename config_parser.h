// Nice comment block goes here

#ifndef __CONFIG_PARSER_H
#define __CONFIG_PARSER_H

#include "globals.h"
#include "face_ios.h"

#ifdef WINDOWS_OS
   #include <winsock2.h>
#endif

#ifdef LINUX_OS
   #include <sys/socket.h>
#endif

#define MAX_IP_LENGTH  15

typedef enum  {
   FACE_UNKNOWN_CONNECTION_TYPE = 0,
   FACE_DIRECT_CONNECTION,
   FACE_UDP_CONNECTION
} FACE_CONNECTION_TYPE;

typedef enum { 
   FACE_NEITHER_DIRECTION = 0, 
   FACE_TRANSMIT,
   FACE_RECEIVE 
}  FACE_DIRECTIONALTY_TYPE;

typedef enum {
   FACE_A429_ODD,
   FACE_A429_EVEN
} FACE_A429_PARITY_TYPE;

typedef enum {
   FACE_A429_LOW,
   FACE_A429_HIGH
} FACE_A429_SPEED_TYPE;

// For simplicity, I parse everything and put in one struct.
// A given component will only use a subset of the information.
// Moreover, not everything applies to any given connection.

typedef struct
{
    FACE_INTERFACE_NAME_TYPE  name;                                  // Name of this connection
    FACE_CONNECTION_TYPE      connectionType;                        // Connection type
    FACE_DIRECTIONALTY_TYPE   direction;                             // Directionality
    uint64_t                  refreshPeriod;                         // How often it needs to be updated
    char                      sourceAddress[MAX_IP_LENGTH + 1];      // Address of PSS machine
    char                      destinationAddress[MAX_IP_LENGTH + 1]; // Address of I/O Service machine
    uint16_t                  sourcePort;                            // Port used on the PSS machine
    uint16_t                  destinationPort;                       // Port used on the I/O Service machine
    FACE_BUS_TYPE             busType;                               // Bus Type
    uint16_t                  channel;                               // Logical channel number, 1-based (will need mapping to hardware in I/O Seg)
    uint8_t                   discreteInitialValue;                  // Initial value for a Discrete output
    FACE_A429_PARITY_TYPE     a429Parity;                            // Parity of the A429 bus
    FACE_A429_SPEED_TYPE      a429Speed;                             // Speed of the A$29 bus
} FACE_CONFIG_DATA_TYPE;


// Parses a config file and returns the config array and numbe of connections.
// The user must pass a pointer to an array of CONNECTION_INFO_TYPE and maximum number of elements in the array.
// The function passes back the connection data in the array and the number of connections.
// It returns true on success, false on failure.
//  Call it as (make constants instead of using magic numbers)
//     CONNECTION_INFO_TYPE configData[20];
//     int numConnections = 20;
//     PasrseConfigFile("config.xml", configData, &numConnections);
_Bool ParseConfigFile( /* in */ const char * filename, /*out */ FACE_CONFIG_DATA_TYPE config[], /*in out */ uint32_t * numConnections);


#endif

