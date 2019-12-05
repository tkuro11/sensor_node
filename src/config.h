#ifndef __CONFIG__
#define __CONFIG__
#include <Arduino.h>

typedef struct {
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
  Packet packet;
} Config;

const int MAGIC = 0xcafe;
const int sensorPin = GPIO_NUM_34;    // select the input pin for the potentiometer
const int luxPin = GPIO_NUM_39;
const int sndPin = GPIO_NUM_26;

#endif
