#include <myuv.h>

/*************************** UV ******************************/
int ReadUVintensityPin = 1; // Output from the sensor

float mapfloat(float x, float in_min /*噪声*/, float in_max, float out_min /*偏移*/, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Takes an average of readings on a given pin
// Returns the average
int averageAnalogRead(int pinToRead)
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
    int uvLevel = averageAnalogRead(ReadUVintensityPin);

    float outputVoltage = 5.0 * uvLevel / 1024;
    float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);

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
        float_to_hex_printf(0xcc, uvIntensity);
    else
    {
        Serial.print("UV Intensity: ");
        Serial.println(uvIntensity);
    }

    // if (cyclesCount == 10)
    //   UV[bootCount - 1] = uvIntensity;
    // sendCallBack(" UV Intensity: ", uvIntensity);
}
/*********************************************************/
