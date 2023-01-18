#include <myuv.h>

/*************************** UV ******************************/
int ReadUVintensityPin = 34; // Output from the sensor

float mapfloat(float x, float in_min /*噪声*/, float in_max, float out_min /*偏移*/, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Takes an average of readings on a given pin
// Returns the average
unsigned int averageAnalogRead(int pinToRead)
{
    byte numberOfReadings = 8; // Averg_times
    unsigned int runningValue = 0;

    for (int x = 0; x < numberOfReadings; x++)
        runningValue += analogRead(pinToRead);
    runningValue /= numberOfReadings;

    return (runningValue);
}

void UVInit()
{
    pinMode(ReadUVintensityPin, INPUT);
}

void UVDisplay()
{
    unsigned int uvLevel = averageAnalogRead(ReadUVintensityPin);

    float outputVoltage = 5.0 * uvLevel / 1024;
    float uvIntensity = mapfloat(outputVoltage, 0.09, 2.9, 0.0, 15.0);

    // Serial.print("UVAnalogOutput: ");
    // Serial.println(uvLevel);

    // Serial.print(" OutputVoltage: ");
    // Serial.print(outputVoltage);

#if 0
    Serial.print("UV Intensity: ");
    Serial.println(uvIntensity);
    // Serial.print(" mW/cm^2");
    // Serial.print(",");
#endif

    if (HEX_Format_flag)
    {
        if ((COMMNUI_CH_flag == 0) || (COMMNUI_CH_flag == 2))
            float_to_hex_printf(0xcc, uvIntensity);
        if ((COMMNUI_CH_flag == 1) || (COMMNUI_CH_flag == 2))
            Wifi_float_to_hex_printf(0xcc, uvIntensity);
    }
    else
    {
        if ((COMMNUI_CH_flag == 0) || (COMMNUI_CH_flag == 2))
        {
            Serial.print("UV Intensity: ");
            Serial.println(uvIntensity);
        }
        if ((COMMNUI_CH_flag == 1) || (COMMNUI_CH_flag == 2))
        {
            wifi_printf(0xEE);
            wifi_printf(ERR_Format); // 打印信息
        }
    }

    // if (cyclesCount == 10)
    //   UV[bootCount - 1] = uvIntensity;
    // wifi_printf(" UV Intensity: ", uvIntensity);
}
/*********************************************************/
