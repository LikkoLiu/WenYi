#include <myas7341.h>

Adafruit_AS7341 as7341;
uint16_t readings[12];

void AS7341init()
{
#if serialAS7341_log
    Serial.println("find AS7341");
#endif

    while (!as7341.begin())
    {
        delay(100);
#if serialAS7341_log
        Serial.print(". ");
#endif
    }

    as7341.setATIME(100);
    as7341.setASTEP(999);
    as7341.setGain(AS7341_GAIN_256X);
}

void AS7341Scan()
{

    if (!as7341.readAllChannels(readings))
    {
#if serialAS7341_log
        Serial.println("Error reading all channels!");
#endif
        return;
    }

#if serialAS7341_log
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

    Serial.println();
#endif
    uint16_to_hex_printf(readings[0]);
    uint16_to_hex_printf(readings[1]);
    uint16_to_hex_printf(readings[2]);
    uint16_to_hex_printf(readings[3]);
    uint16_to_hex_printf(readings[6]);
    uint16_to_hex_printf(readings[7]);
    uint16_to_hex_printf(readings[8]);
    uint16_to_hex_printf(readings[9]);
}
