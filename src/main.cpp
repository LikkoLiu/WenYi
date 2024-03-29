#include <Arduino.h>
#include <mywifi.h>
#include <communication.h>
#include <mybmi160.h>
#include <myuv.h>
#include <myas7341.h>
#include <lowpower.h>
#include <myflash.h>
#include "esp32-hal-cpu.h"
#include <RTCclock.h>
#include <GPIOISR.h>

uint8_t _I2C_BMI160INIT_Flag = 0;
uint8_t _I2C_AS7341INIT_Flag = 0;

void setup()
{
  pinMode(25, INPUT_PULLUP);
  pinMode(5, OUTPUT);
  digitalWrite(15,LOW);
  
  if (digitalRead(25))
  {
    esp_log_level_set(ERROR_DEBUG, ESP_LOG_NONE);
    esp_log_level_set(INFO_DEBUG, ESP_LOG_NONE);
  }
  else
  {
    esp_log_level_set(ERROR_DEBUG, ESP_LOG_INFO);
    esp_log_level_set(INFO_DEBUG, ESP_LOG_INFO);
  }
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  Serial.begin(115200);
  if (setCpuFrequencyMhz(80)) // 设置CPU主频为80MHz
    ESP_LOGI(INFO_DEBUG, "CPU clock be set to %u MHz", getCpuFrequencyMhz());

  UVInit();       // UV传感器初始化配置
  myflash_init(); // extr flash 初始化配置

  if (TIME_TO_SLEEP == 0)
  {
    // if (esp_wait_sntp_sync())
    //   ESP_LOGI(INFO_DEBUG, "WIFI disconnected, Skiping system time SET.");
  }
  Wifi_init_succ = 0; // WiFi关闭标志

  // vTaskDelay(500 / portTICK_PERIOD_MS);
  // digitalWrite(15, LOW); // 外围传感器通电控制
  // Serial.println("OK4");

  // Serial.println("OK5");
  // digitalWrite(15, HIGH); // CPU正常启动后亮灯闪烁
  Serial.println("###");
}

void loop()
{
  if (!digitalRead(25))
  {
    Flash_read_flag = 1;
    read_data_in_batches("/extflash/hello.txt"); // 读取Flash
    if (TIME_TO_SLEEP == 0)
      _STOPACAN();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    if (!digitalRead(25))
      Flash_Erase_FATfile("/extflash/hello.txt");
  }

  serialEvent();
  read_data_config("/extflash/config.txt"); // 读取上一次的设置内容

  if (Wifi_init_succ == 1)
    wifiEvent();

  if (((COMMNUI_CH_flag == 1) || (COMMNUI_CH_flag == 2)) && Wifi_init_succ == 0)
    wifi_ap_init();
  if ((COMMNUI_CH_flag == 0) && Wifi_init_succ == 1)
  {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    ESP_LOGI(INFO_DEBUG, "WiFI_OFF is successed");
    Wifi_init_succ = 0;
  }

  if (SINGLE_flag == 0 || SINGLE_flag == 1)
  {
    // Serial.println("OK6");
    Flash_Write_RTC();
    if (AS7341_SCAN_flag)
    {
      // Serial.println("OK7");
      // vTaskDelay(1700 / portTICK_PERIOD_MS);
      digitalWrite(15, LOW);

      if (!_I2C_AS7341INIT_Flag)
      {
        // Serial.println("OK9");
        AS7341init();
        _I2C_BMI160INIT_Flag = 0;
        _I2C_AS7341INIT_Flag = 1;
      }
      // Serial.println("OK10");
      AS7341Scan();
      digitalWrite(15, HIGH);
    }

    if (UV_SCAN_flag)
    {
      digitalWrite(15, LOW);

      //UV_Fake
      if (!_I2C_AS7341INIT_Flag)
      {
        // Serial.println("OK9");
        AS7341init();
        _I2C_BMI160INIT_Flag = 0;
        _I2C_AS7341INIT_Flag = 1;
      }

      UVDisplay_Fake();
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

    if (SINGLE_flag == 1)
      SINGLE_flag = 2;
    // Flash_Writeln(); // flansh存储换行
  }

  if ((TIME_TO_SLEEP > 0) && (SINGLE_flag == 0))
  {
    serialEvent();

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

    serialEvent();

    ESP_LOGI(INFO_DEBUG, "Going to sleep now");

    serialEvent();

    esp_deep_sleep_start();
  }
}
