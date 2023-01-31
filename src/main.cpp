#include <Arduino.h>
#include <mywifi.h>
#include <communication.h>
#include <mybmi160.h>
#include <myuv.h>
#include <myas7341.h>
#include <lowpower.h>
#include <myflash.h>
#include "esp32-hal-cpu.h"

uint8_t _I2C_BMI160INIT_Flag = 0;
uint8_t _I2C_AS7341INIT_Flag = 0;

void setup()
{
  pinMode(15, OUTPUT);		//外围传感器通电控制 
  digitalWrite(15, LOW);
  Serial.begin(115200);

  setCpuFrequencyMhz(80);	//设置CPU主频为80MHz
  // Serial.println(getCpuFrequencyMhz());

  delay(200);
  digitalWrite(15, HIGH);	//CPU正常启动后亮灯 

  UVInit();

  /*************************** POWER ******************************/
  // Print the wakeup reason for ESP32
  // print_wakeup_reason();
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

  // norflash_spi_init();

#ifdef FLASH_TEST_ENABLE
  /* readwrite test */
  int g = 0;
  uint8_t str[1280];
  memset(str, 0, sizeof(str));
  unsigned int j = 0;
  for (int k = 0; k < 5; k++)
  {
    for (int i = 0; i < 256; i++)
    {
      str[j] = i;
      j++;
    }
  }
  Serial.println("");
  Serial.println("-----write data-------");
  sector_erase(0x00);
  write_one_sector_data(0x10, str, 256);
  memset(str, 0, sizeof(str));
  read_data(0x00, str, 512);
  Serial.println("str:");
  for (int k = 0; k < 512; k++)
  {
    if (g == 16)
    {
      Serial.println("|");
      if (k % 256 == 0)
        Serial.println("---------------");
      {
        g = 1;
      }
    }
    else
    {
      g++;
    }
    Serial.printf("%02X ", str[k]);
  }
#endif

  Wifi_init_succ = 0;

  // vTaskDelay(5000 / portTICK_PERIOD_MS);();/
}

void loop()
{
  serialEvent();
  if (Wifi_init_succ == 1)
    wifiEvent();

  if (((COMMNUI_CH_flag == 1) || (COMMNUI_CH_flag == 2)) && Wifi_init_succ == 0)
    wifi_ap_init();
  if ((COMMNUI_CH_flag == 0) && Wifi_init_succ == 1)
  {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    Serial.printf("%c", 0xFF);
    Serial.printf("%c", Succ_disconnect);
    Wifi_init_succ = 0;
  }

  if (SINGLE_flag == 0 || SINGLE_flag == 1)
  {
    if (AS7341_SCAN_flag)
    {
      digitalWrite(15, LOW);

      if (!_I2C_AS7341INIT_Flag)
      {
        AS7341init();
        _I2C_BMI160INIT_Flag = 0;
        _I2C_AS7341INIT_Flag = 1;
      }
      AS7341Scan();
      digitalWrite(15, HIGH);
    }

    if (BMI160_SCAN_flag)
    {
      digitalWrite(15, LOW);
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
      digitalWrite(15, HIGH);
    }

    if (UV_SCAN_flag)
    {
      digitalWrite(15, LOW);
      UVDisplay();
      digitalWrite(15, HIGH);
    }

    if (SINGLE_flag == 1)
      SINGLE_flag = 2;
  }

  // Serial.printf("TIME_TO_SLEEP is : %d\r\n",TIME_TO_SLEEP);
  if ((TIME_TO_SLEEP > 0) && (SINGLE_flag == 0))
  {
    serialEvent();
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    serialEvent();
    Serial.println("Going to sleep now");
    serialEvent();
    esp_deep_sleep_start();
  }
}
