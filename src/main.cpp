#include <Arduino.h>
#include <WiFi.h>
#include "WiFiUdp.h"
#include <communication.h>
#include <mybmi160.h>
#include <myuv.h>
#include "DFRobot_AS7341.h"

#define BMI160_Flag 0
#define UV_Flag 0
#define AS7341_Flag 0

void setup()
{
  Serial.begin(115200);
  delay(100);

#if BMI160_Flag
  MPU6050Init();
#endif

#if UV_Flag
  UVInit();
#endif

}

void loop()
{
#if BMI160_Flag
  BMI160_math_display();
#endif

#if UV_Flag
  UVDisplay();
#endif

  serialEvent();

  delay(50);
}
