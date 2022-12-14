#ifndef __MYAS7341_H_
#define __MYAS7341_H_
#include <Arduino.h>
#include <Adafruit_AS7341.h>
#include <communication.h>
#define serialAS7341_log 0


extern uint16_t readings[12];

void AS7341init();
void AS7341Scan();

#endif
