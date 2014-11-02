#ifndef FACE_PSS_H
#define FACE_PSS_H

#include "face_ios.h"
#include "globals.h"

#include "face_ios.h"

#define MAX_CONNECTIONS 32

int main(int argc, char *argv[]);

void setDiscrete(FACE_INTERFACE_HANDLE_TYPE handle, int channel, uint8_t value,
   FACE_RETURN_CODE_TYPE *retCode);

uint8_t readDiscrete(FACE_INTERFACE_HANDLE_TYPE handle, int channel,
   FACE_RETURN_CODE_TYPE *retCode);

#endif
