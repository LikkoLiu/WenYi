#include <myas7341.h>

Adafruit_AS7341 as7341;
uint16_t readings[12];
RTC_DATA_ATTR uint8_t gainval;

void AS7341init()
{
    ESP_LOGI("DEBUG_AS7341", "AS7341 finding ......");

    while (!as7341.begin())
    {
        vTaskDelay(200 / portTICK_PERIOD_MS);
        Serial.printf("%c", 0xEE);
        Serial.printf("%c", ERR_AS7341_Init);
        ESP_LOGI("DEBUG_AS7341", "AS7341 couldn't find");
    }

    as7341.setATIME(100);
    as7341.setASTEP(999);
    // gainval = AS7341_GAIN_256X;
    as7341.setGain((as7341_gain_t)gainval);

    // Serial.printf("%c", 0xFF);
    // Serial.printf("%c", Succ_AS7341_Init);
}

void AS7341Scan()
{
    as7341.setGain((as7341_gain_t)gainval);
    if (!as7341.readAllChannels(readings))
    {
#if serialAS7341_log
        Serial.println("Error reading all channels!");
#endif
        Serial.printf("%c", 0xEE);
        Serial.printf("%c", ERR_AS7341_read);
        ESP_LOGI("DEBUG_AS7341", "AS7341 couldn't read");
        return;
    }

    if (HEX_Format_flag)
    {
        if ((COMMNUI_CH_flag == 0) || (COMMNUI_CH_flag == 2))
        {
            uint16_to_hex_printf(readings[0], 1, gainval);
            uint16_to_hex_printf(readings[1], 2, gainval);
            uint16_to_hex_printf(readings[2], 3, gainval);
            uint16_to_hex_printf(readings[3], 4, gainval);
            uint16_to_hex_printf(readings[6], 5, gainval);
            uint16_to_hex_printf(readings[7], 6, gainval);
            uint16_to_hex_printf(readings[8], 7, gainval);
            uint16_to_hex_printf(readings[9], 8, gainval);
        }
        if ((COMMNUI_CH_flag == 1) || (COMMNUI_CH_flag == 2))
        {
            Wifi_uint16_to_hex_printf(readings[0], 1, gainval);
            Wifi_uint16_to_hex_printf(readings[1], 2, gainval);
            Wifi_uint16_to_hex_printf(readings[2], 3, gainval);
            Wifi_uint16_to_hex_printf(readings[3], 4, gainval);
            Wifi_uint16_to_hex_printf(readings[6], 5, gainval);
            Wifi_uint16_to_hex_printf(readings[7], 6, gainval);
            Wifi_uint16_to_hex_printf(readings[8], 7, gainval);
            Wifi_uint16_to_hex_printf(readings[9], 8, gainval);
        }
    }
    else
    {
        if ((COMMNUI_CH_flag == 0) || (COMMNUI_CH_flag == 2))
        {
            Serial.print("GAIN grade : ");
            Serial.println(gainval);
            Serial.print("F1 415nm : ");
            Serial.println(readings[0]);
            Serial.print("F2 445nm : ");
            Serial.println(readings[1]);
            Serial.print("F3 480nm : ");
            Serial.println(readings[2]);
            Serial.print("F4 515nm : ");
            Serial.println(readings[3]);
            Serial.print("F5 555nm : ");

            /*
            // we skip the first set of duplicate clear/NIR readings
            Serial.print("ADC4/Clear-");
            Serial.println(readings[4]);
            Serial.print("ADC5/NIR-");
            Serial.println(readings[5]);
            */

            Serial.println(readings[6]);
            Serial.print("F6 590nm : ");
            Serial.println(readings[7]);
            Serial.print("F7 630nm : ");
            Serial.println(readings[8]);
            Serial.print("F8 680nm : ");
            Serial.println(readings[9]);

            // Serial.print("ADC4/Clear    : ");
            // Serial.println(readings[10]);
            // Serial.print("ADC5/NIR      : ");
            // Serial.println(readings[11]);
        }
        if ((COMMNUI_CH_flag == 1) || (COMMNUI_CH_flag == 2))
        {
            wifi_printf(0xEE);
            wifi_printf(ERR_Format); // 打印信息
        }
    }
    Flash_Write_Data("B", readings, 10, gainval);
    
}
