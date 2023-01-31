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
  pinMode(15, OUTPUT); // 外围传感器通电控制
  digitalWrite(15, LOW);
  Serial.begin(115200);

  setCpuFrequencyMhz(80); // 设置CPU主频为80MHz
  ESP_LOGI("CPU: ", "CPU clock be set to %u MHz", getCpuFrequencyMhz());

  vTaskDelay(300 / portTICK_PERIOD_MS);
  digitalWrite(15, HIGH); // CPU正常启动后亮灯闪烁

  UVInit(); // UV传感器初始化配置
  myflash_init(); // extr flash 初始化配置

  Wifi_init_succ = 0; // WiFi关闭标志
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
