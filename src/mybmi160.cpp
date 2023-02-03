#include <mybmi160.h>

union float_int
{
    float x;
    uint8_t s[4];
} floattoint;
/*************************** BMI160 ******************************/
DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;

int16_t ax, ay, az;
int16_t gx, gy, gz;
double total_angle = 0;
// #define LED_PIN 13

/***************************************************************/

/* 通过卡尔曼滤波得到的最终角度 */
float Angle = 0.0;

/*由角速度计算的倾斜角度 */
// float Angle_gy = 0.0;

float Q_angle = 0.001;
float Q_gyro = 0.003;
float R_angle = 0.03;
float dt = 0.005; /* dt为kalman滤波器采样时间; */
char C_0 = 1;
float Q_bias, Angle_err;
float PCt_0 = 0.0, PCt_1 = 0.0, E = 0.0;
float K_0 = 0.0, K_1 = 0.0, t_0 = 0.0, t_1 = 0.0;
float Pdot[4] = {0, 0, 0, 0};
float PP[2][2] = {{1, 0}, {0, 1}};

/***************************************************************/
unsigned long mictime = 0;
unsigned long pretime = 0;
double az_angle = 0.0;
double gy_angle = 0.0;
float gyro = 0.0;
double ay_angle = 0.0;
double gx_angle = 0.0;
float gxro = 0.0;

int i = 0;
int rslt;
int16_t accelGyro[6] = {0};
/***************************************************************/

/* 卡尔曼滤波函数 */
float Kalman_Filter(float Accel, float Gyro)
{
    Angle += (Gyro - Q_bias) * dt;

    Pdot[0] = Q_angle - PP[0][1] - PP[1][0];

    Pdot[1] = -PP[1][1];
    Pdot[2] = -PP[1][1];
    Pdot[3] = Q_gyro;

    PP[0][0] += Pdot[0] * dt;
    PP[0][1] += Pdot[1] * dt;
    PP[1][0] += Pdot[2] * dt;
    PP[1][1] += Pdot[3] * dt;

    Angle_err = Accel - Angle;

    PCt_0 = C_0 * PP[0][0];
    PCt_1 = C_0 * PP[1][0];

    E = R_angle + C_0 * PCt_0;

    if (E != 0)
    {
        K_0 = PCt_0 / E;
        K_1 = PCt_1 / E;
    }

    t_0 = PCt_0;
    t_1 = C_0 * PP[0][1];

    PP[0][0] -= K_0 * t_0;
    PP[0][1] -= K_0 * t_1;
    PP[1][0] -= K_1 * t_0;
    PP[1][1] -= K_1 * t_1;

    Angle += K_0 * Angle_err;
    Q_bias += K_1 * Angle_err;

    return Angle;
}

void MPU6050Init()
{
    ESP_LOGI("DEBUG_BMI60", "BMI160 finding ");

    while (bmi160.softReset() != BMI160_OK)
    {
#if serialbmi160_log
        Serial.print(" .");
#endif
        delay(300);
        Serial.printf("%c", 0xEE);
        Serial.printf("%c", ERR_BMI160_Init);
    }

    // set and init the bmi160 i2c address
    ESP_LOGI("DEBUG_BMI60", "BMI160 init ");
    while (bmi160.I2cInit(i2c_addr) != BMI160_OK)
    {
#if serialbmi160_log
        Serial.print(" .");
#endif
        delay(300);
        Serial.printf("%c", 0xEE);
        Serial.printf("%c", ERR_BMI160_I2cInit);
    }

    // #if serialbmi160_log
    //     Serial.println("\r\nBMI160 set setStepPowerMode");
    // #endif
    //     while (bmi160.setStepPowerMode(bmi160.stepLowPowerMode) != BMI160_OK)
    //     {
    // #if serialbmi160_log
    //         Serial.print(" .");
    // #endif
    //         delay(300);
    // }
}

void BMI160_math_display()
{
    unsigned long time = 0;
    // get both accel and gyro data from bmi160
    // parameter accelGyro is the pointer to store the data
    rslt = bmi160.getAccelGyroData(accelGyro);
    if (rslt == 0)
    {
        for (i = 0; i < 6; i++)
        {
            if (i < 3)
            {
                // the first three are gyro data
                // Serial.print(accelGyro[i] * 3.14 / 180.0);
                // Serial.print("\t");
                if (i == 0)
                    gx = accelGyro[i];
                if (i == 1)
                    gy = accelGyro[i];
                if (i == 2)
                    gz = accelGyro[i];
            }
            else
            {
                // the following three data are accel data
                // Serial.print(accelGyro[i] / 16384.0);
                // Serial.print("\t");
                if (i == 3)
                    ax = accelGyro[i];
                if (i == 4)
                    ay = accelGyro[i];
                if (i == 5)
                    az = accelGyro[i];
            }
        }
    }
    else
    {
#if serialbmi160_log
        Serial.print("get BIM160_data Error");
#endif
        Serial.printf("%c", 0xEE);
        Serial.printf("%c", ERR_BMI160_read);
    }

    if (pretime == 0)
    {
        pretime = millis();
        return;
    }
    mictime = millis();
    time = mictime - pretime;
    pretime = mictime;
    /* 加速度量程范围设置2g 16384 LSB/g
     * 计算公式：
     * 前边已经推导过这里再列出来一次
     * x是小车倾斜的角度,y是加速度计读出的值
     * sinx = 0.92*3.14*x/180 = y/16384
     * x=180*y/(0.92*3.14*16384)=
     */
    az -= AZ_ZERO;
    az_angle = az / 262;

    /***************************************************************/
    ay -= AY_ZERO;
    ay_angle = ay / 262;
    /***************************************************************/

    /* 陀螺仪量程范围设置250 131 LSB//s
     * 陀螺仪角度计算公式:
     * 小车倾斜角度是gx_angle,陀螺仪读数是y,时间是dt
     * gx_angle +=(y/(131*1000))*dt
     */

    gy -= GY_ZERO;
    gyro = gy / 131.0;

    gx -= GX_ZERO;
    gxro = gx / 131.0;

    gy_angle = gyro * time;
    gy_angle = gy_angle / 1000.0;

    gx_angle = gxro * time;
    gx_angle = gx_angle / 1000.0;
    /***************************************************************/

    total_angle -= gy_angle;

    //  获取运行时间必须放kalman最后
    dt = time / 1000.0;
    Angle = Kalman_Filter(az_angle, gyro);

// Serial.print(az_angle);
// Serial.print("   ,   ");
// Serial.print(total_angle);
// Serial.print("   ,   ");

    if (HEX_Format_flag)
    {
        if ((COMMNUI_CH_flag == 0) || (COMMNUI_CH_flag == 2))
            float_to_hex_printf(0xaa, Angle);
        if ((COMMNUI_CH_flag == 1) || (COMMNUI_CH_flag == 2))
            Wifi_float_to_hex_printf(0xaa, Angle);
    }
    else
    {
        if ((COMMNUI_CH_flag == 0) || (COMMNUI_CH_flag == 2))
        {
            // Serial.print(",");
            // Serial.print(az_angle);
            // Serial.print(",");
            // Serial.print(gyro);
            // Serial.print(",");
            // Serial.println(Angle);
            Serial.print("Angle: ");
            Serial.println(Angle);
        }
        if ((COMMNUI_CH_flag == 1) || (COMMNUI_CH_flag == 2))
        {
            wifi_printf(0xEE);
            wifi_printf(ERR_Format); // 打印信息
        }
    }
    Flash_Write_float_Data("A", Angle);
    // delay(60);
    /***************************************************************/
    //   Kalman_Filter(ay_angle, gxro);

    // // Serial.print(az_angle);
    // // Serial.print("   ,   ");
    // // Serial.print(total_angle);
    // // Serial.print("   ,   ");
    // #if !AS7341ScanDebug
    //   Serial.print("Angle2:");
    //   Serial.println(Angle);
    //   Serial.print(",");
    // #endif
    /***************************************************************/
}
/*********************************************************/
