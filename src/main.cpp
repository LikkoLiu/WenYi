#include <Arduino.h>
#include <WiFi.h>
#include "WiFiUdp.h"
#include <communication.h>
#include <mybmi160.h>
#include <myuv.h>
#include <myas7341.h>

uint8_t _I2C_BMI160INIT_Flag = 0;
uint8_t _I2C_AS7341INIT_Flag = 0;

void setup()
{
  Serial.begin(115200);
  delay(100);

  UVInit();
}

void loop()
{
  if (SINGLE_flag == 0 || SINGLE_flag == 1)
  {
    if (AS7341_SCAN_flag)
    {
      if (!_I2C_AS7341INIT_Flag)
      {
        AS7341init();
        _I2C_BMI160INIT_Flag = 0;
        _I2C_AS7341INIT_Flag = 1;
      }
      AS7341Scan();
    }

    if (BMI160_SCAN_flag)
    {
      if (!_I2C_BMI160INIT_Flag)
      {
        MPU6050Init();
        _I2C_BMI160INIT_Flag = 1;
        _I2C_AS7341INIT_Flag = 0;
        delay(50);
      }
      for (int j = 0; j < 10; j++)
      {
        BMI160_math_display();
        // delay(50); //must timeout --> mybmi.cpp|BMI160_math_display()
      }
    }

    if (UV_SCAN_flag)
    {
      UVDisplay();
    }

    if (SINGLE_flag == 1)
      SINGLE_flag = 2;
  }
  serialEvent();
}
