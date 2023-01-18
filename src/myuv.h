#ifndef __MYUV_H_
#define __MYUV_H_
#include <Arduino.h>
#include <communication.h>
#include <mywifi.h>
#define serialAS7341S_log 0

extern int ReadUVintensityPin; // Output from the sensor

float mapfloat(float x, float in_min /*噪声*/, float in_max, float out_min /*偏移*/, float out_max);
unsigned int averageAnalogRead(int pinToRead);
void UVInit();
void UVDisplay();

#endif
