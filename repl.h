#ifndef REPL_H
#define REPL_H

#include "face_messages.h"
#include "config_parser.h"

void repl(FACE_INTERFACE_HANDLE_TYPE handles[], FACE_CONFIG_DATA_TYPE config[]);

void *readConnection(void *handlePtr);

#endif
