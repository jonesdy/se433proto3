#ifndef __DIRECT_CALL_H
#define __DIRECT_CALL_H

#include "globals.h"
#include "face_common.h"
#include "face_ios.h"


// For when PSS and I/O Seg are in the same program and I/O Lib calls I/O Lib.
typedef void (*IO_Seg_Initialize_PtrType)
     ( /* in */ const FACE_CONGIGURATION_FILE_NAME configuration_file,
       /* out */ FACE_RETURN_CODE_TYPE *return_code);

// For Direct Read when PSS and I/O Seg are in the same program
typedef void (*IO_Seg_Read_PtrType)
   ( /* inout */ FACE_MESSAGE_LENGTH_TYPE *message_length,
     /* in */ FACE_MESSAGE_ADDR_TYPE data_buffer_address,
     /* out */ FACE_RETURN_CODE_TYPE *return_code);

// For Direct Write when PSS and I/O Seg are in the same program
typedef void (*IO_Seg_Write_PtrType)
   (  /* in */ FACE_MESSAGE_LENGTH_TYPE message_length,
      /* in */ FACE_MESSAGE_ADDR_TYPE data_buffer_address,
      /* out */ FACE_RETURN_CODE_TYPE *return_code);


// Set these in I/O if PSS and I/O Seg are in the same program
extern IO_Seg_Initialize_PtrType IO_Seg_Initialize_Ptr;
extern IO_Seg_Read_PtrType IO_Seg_Read_Ptr;
extern IO_Seg_Write_PtrType IO_Seg_Write_Ptr;


#endif