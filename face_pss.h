#ifndef FACE_PSS_H
#define FACE_PSS_H

#include "face_ios.h"
#include "globals.h"

#define MAX_CONNECTIONS 32

void setDiscrete(FACE_INTERFACE_HANDLE_TYPE handle, uint8_t channel, uint8_t value,
   FACE_RETURN_CODE_TYPE *retCode);

uint8_t readDiscrete(FACE_INTERFACE_HANDLE_TYPE handle, uint8_t channel,
   FACE_RETURN_CODE_TYPE *retCode);

void sendArinc429(FACE_INTERFACE_HANDLE_TYPE handle, uint8_t channel, uint32_t *data, uint32_t numLabels,
		  FACE_RETURN_CODE_TYPE *retCode);

void readArinc429(FACE_INTERFACE_HANDLE_TYPE handle, uint8_t *channel, uint32_t *data, uint32_t *numLabels,
		  FACE_RETURN_CODE_TYPE *retCode);
#endif
