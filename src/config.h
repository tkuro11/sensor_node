#ifndef __MCONFIG__
#define __MCONFIG__
#include <Arduino.h>

typedef struct {
  uint16_t valid;
  uint16_t temp;
  uint16_t lux;
  uint16_t sound;
  uint16_t hop;
  int rssi;
} Packet;

typedef struct {
  uint16_t checker;
  char id;
  char left, right;
} Config;

const int MAGIC = 0xcafe;
const int sensorPin = GPIO_NUM_34;    // select the input pin for the potentiometer
const int luxPin = GPIO_NUM_39;
const int sndPin = GPIO_NUM_36;

#define VER_MAJOR "0"
#define VER_MINOR "4"
#define VER_SUB_MINOR "1"
static const char* VERSION = VER_MAJOR "." VER_MINOR "." VER_SUB_MINOR;

#endif
