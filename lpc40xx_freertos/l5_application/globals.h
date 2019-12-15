//
// Created by tiago on 11/11/2019.
//
#include "queue.h"
#include "semphr.h"

#ifndef SJTWO_C_GLOBALS_H
#define SJTWO_C_GLOBALS_H

const int LEN_OF_NAME = 32;
static QueueHandle_t MUSIC_NAME_QUEUE;
const int LEN_OF_DATA = 32;
static QueueHandle_t DATA_QUEUE;
char TBFILENAME[32];
#endif // SJTWO_C_GLOBALS_H
