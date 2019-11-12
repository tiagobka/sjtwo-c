//
// Created by tiago on 11/11/2019.
//
#include "queue.h"
#include "semphr.h"

#ifndef SJTWO_C_GLOBALS_H
#define SJTWO_C_GLOBALS_H

const int LEN_OF_QUEUE = 32;
static QueueHandle_t music_name;
bool MN_initialized = false;
static QueueHandle_t data_queue;

#endif // SJTWO_C_GLOBALS_H
