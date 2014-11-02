// Could use a comment bloack

// In Visual, to set struct alignment to 1 byte, do:
//   Project -> Properties -> Configuration Properties -> C/C++ -> Code Generation -> Struct Memory Alignment -> 1 Byte
// Need to check for gcc


#ifndef __FACE_MESSAGES_H_
#define __FACE_MESSAGES_H_

#include "face_ios.h"

/**************************************************************************************
  All Data is stored in Network Order - Big Endian.
  This will simplify when sending out datagrams - all data is array will be right order.
  This does mean that ntohl, ntohs, htonl, htons, will need to be used
  every time data is put in from the host or taken out to use in the host.
  It also will make debugging harder if you look at the struct overlays.
 **************************************************************************************/


#define FACE_MSG_HEADER_SIZE    20


/*************************************************************************************
   Some Macros for Setting/Getting between Machine Endian and Network Endian
   Rest of the fields can be done using ntohl, ntohs, htonl, htons
   The macros are only for those fields that aren't 2 or 4 bytes long.
   Also, don't need them when one byte long (know why, right?)
   Note: Can't use C bit-fields since the endian could still be wrong.
 **************************************************************************************/

#define FaceSetPayLoadLength(faceMsgPtr,length) \
            {  faceMsgPtr->payloadLengthHighByte = (length) >> 16;  \
               faceMsgPtr->payloadLengthLowWord = htons((uint16_t)((length) & 0xFFFF)); }

#define FacePayLoadLength(faceMsgPtr) ( faceMsgPtr->payloadLengthHighByte << 16 | \
		                                ntohs(faceMsgPtr->payloadLengthLowWord) )

// Only 32 bits for Discrete Payload.  Make macros to access the bit fields.
#define FaceSetDiscreteState(faceMsgPtr,state) { faceMsgPtr->data[0] = (faceMsgPtr->data[0] & 0x3F) | ((state) << 6); }
#define FaceDiscreteState(faceMsgPtr) ( faceMsgPtr->data[0] >> 6 )

#define FaceSetDiscreteChannelNumber(faceMsgPtr,chNum) \
            { faceMsgPtr->data[0] = (uint8_t)((faceMsgPtr->data[0] & 0x3F) | ((chNum) << 16)); \
              faceMsgPtr->data[1] = (uint8_t)((chNum) << 8); faceMsgPtr->data[2] = (uint8_t)(chNum); }
#define FaceDiscreteChannelNumber(faceMsgPtr) ( \
		    ((faceMsgPtr->data[0] & 0x3F) << 16) | \
		    (faceMsgPtr->data[1] << 8) | faceMsgPtr->data[2] )

/**************************************************************************************
  All Data is stored in Network Order - Big Endian
  When you use these, "overlay" (via casting) them on an array of char.
 **************************************************************************************/

typedef struct
{
    int32_t   guid;                      // Unique identifier
    uint8_t   busType;                   // Bus Type - use FACE_BUS_TYPE enums
    uint8_t   payloadLengthHighByte;     // High 8 bits of 3-byte Payload Length
    uint16_t  payloadLengthLowWord;      // Low 16 bits of 3-byte Payload Length
    uint16_t  message_type;              // Message Type
    uint16_t  cm_st_usrdef_tsRequired;   // High 15 bits are command/status/user defined, low bit is "Is Time Stamp Required"
    uint32_t  timestamp_MSW;             // High 32 bits of timestamp:  System uptime in nanoseconds
    uint32_t  timestamp_LSW;             // Low 32 bits of timestamp:  System uptime in nanoseconds
    uint8_t   data[1];                   // Message Data - use a dummy index
} FACE_IO_MESSAGE_TYPE;


typedef struct 
{
    uint8_t   channel;                   // ARINC 429 Channel number
    uint8_t   status;                    // Status enum when message type is Data
    uint16_t  num_labels;                // Number of ARINC 429 words
    uint32_t  data[1];                   // num_labels number of 32-bit ARINC 429 words.  Use a dummy index
} FACE_A429_MESSAGE_TYPE;



#endif
