#include <DFRobot_BMI160.h>
#include <Arduino.h>
#include <WiFi.h>
#include "WiFiUdp.h"
#include "DFRobot_AS7341.h"

#define serialbmi160_log 1
#define BMI160_Flag 0
#define UV_Flag 0
#define AS7341_Flag 0

/*************************** BMI160 ******************************/
DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;

int16_t ax, ay, az;
int16_t gx, gy, gz;
double total_angle = 0;
// #define LED_PIN 13

/* 把mpu6050放在水平桌面上，分别读取读取2000次，然后求平均值 */
#define AZ_ZERO (747.25) /* 加速度计的0偏修正值 */
#define GY_ZERO (11.52)  /* 陀螺仪的0偏修正值 */

/***************************************************************/
#define AY_ZERO (0) /* 加速度计的0偏修正值 */
#define GX_ZERO (0) /* 陀螺仪的0偏修正值 */
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
static unsigned long pretime = 0;
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
void Kalman_Filter(float Accel, float Gyro)
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
}

void MPU6050Init()
{
#if serialbmi160_log
  Serial.print("BMI160 reset");
#endif
  while (bmi160.softReset() != BMI160_OK)
  {
#if serialbmi160_log
    Serial.print(" .");
#endif
    delay(300);
  }

  // set and init the bmi160 i2c address
#if serialbmi160_log
  Serial.print("BMI160 init");
#endif
  while (bmi160.I2cInit(i2c_addr) != BMI160_OK)
  {
#if serialbmi160_log
    Serial.print(" .");
#endif
    delay(300);
  }
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
  Kalman_Filter(az_angle, gyro);

// Serial.print(az_angle);
// Serial.print("   ,   ");
// Serial.print(total_angle);
// Serial.print("   ,   ");
#if serialbmi160_log
  Serial.print("Angle:");
  Serial.println(Angle);
#endif
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

#if !AS7341ScanDebug
  // Serial.print("UV Intensity: ");
  Serial.println(uvIntensity);
  // Serial.print(" mW/cm^2");
  // Serial.print(",");
#endif

  // if (cyclesCount == 10)
  //   UV[bootCount - 1] = uvIntensity;
  // sendCallBack(" UV Intensity: ", uvIntensity);
}
/*********************************************************/

void setup()
{
  Serial.begin(115200);
  delay(100);

  MPU6050Init();
  // UVInit();
}

void loop()
{
  BMI160_math_display();
}

void serialEvent() //关键的来了。串口中断部分来了。多注意，多百度。
{
  String intchars = ""; //局部变量
  String chars = "";
  int temp;
  while (Serial.available() > 0) //一直等待数据接收完成 用if的话loop函数执行一次接受1个字符
  {
    char inchar = Serial.read(); //如果串口有数据就进到这一部分了，将串口中的数据读到变量里。
    if (isDigit(inchar))         //是数字就执行
    {
      intchars += inchar; //数字字符串                     //我的命令是"a10"代表左移10mm，所以要分开命令和移动距离。
    }                     //这附近一坨呢就是在将字符串和数字分离开。
    else
      chars += inchar; //否则就是字符串
  }
  temp = intchars.toInt(); //将数字字符串转换成整数
                           // zhengshu=temp;//赋值给全局变量，每次发送都覆盖原来的数据
                           // zifu=chars;//赋值给全局变量，每次发送都覆盖原来的数据
                           //--------------------------//
  //    Serial.print("mingling:");
  //    Serial.println(zifu);
  //   Serial.print("shuzi:");                //这一坨就是你调试的时候用来看看命令传的对不对，有没有将方向与距离分开。正式用就注释掉。
  //    Serial.println(zhengshu);
  //--------------------------//
}
