#include <Arduino.h>
#include <WiFi.h>
#include "WiFiUdp.h"
#include <communication.h>
#include <mybmi160.h>
#include <myuv.h>
#include <myas7341.h>
#include <lowpower.h>

uint8_t _I2C_BMI160INIT_Flag = 0;
uint8_t _I2C_AS7341INIT_Flag = 0;

void setup()
{
  Serial.begin(115200);
  delay(100);

  UVInit();

  /*************************** POWER ******************************/
  // Print the wakeup reason for ESP32
  print_wakeup_reason();
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  // if (TIME_TO_SLEEP > 0)
  //   esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

#if serialpower_log
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
                 " Seconds");
  Serial.println();
#endif
  /****************************************************************/
}

void loop()
{
  serialEvent();
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

  // Serial.printf("TIME_TO_SLEEP is : %d\r\n",TIME_TO_SLEEP);
  if ((TIME_TO_SLEEP > 0)&&(SINGLE_flag == 0))
  {
    serialEvent();
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    serialEvent();
    Serial.println("Going to sleep now");
    serialEvent();
    esp_deep_sleep_start();
  }
}
