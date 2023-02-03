#ifndef __MYBMI160_H_
#define	__MYBMI160_H_
#include <Arduino.h>
#include <DFRobot_BMI160.h>
#include <communication.h>
#include "myflash.h"
#define serialbmi160_log 0

/* 把mpu6050放在水平桌面上，分别读取读取2000次，然后求平均值 */
#define AZ_ZERO (747.25) /* 加速度计的0偏修正值 */
#define GY_ZERO (11.52)  /* 陀螺仪的0偏修正值 */
#define AY_ZERO (0) /* 加速度计的0偏修正值 */
#define GX_ZERO (0) /* 陀螺仪的0偏修正值 */

/***************************************************************/


extern int16_t ax, ay, az;
extern int16_t gx, gy, gz;
extern double total_angle;
// #define LED_PIN 13

/***************************************************************/

/* 通过卡尔曼滤波得到的最终角度 */
extern float Angle;

/*由角速度计算的倾斜角度 */
// float Angle_gy = 0.0;

extern float Q_angle;
extern float Q_gyro;
extern float R_angle;
extern float dt; /* dt为kalman滤波器采样时间; */
extern char C_0;
extern float Q_bias, Angle_err;
extern float PCt_0, PCt_1, E;
extern float K_0, K_1, t_0, t_1;
extern float Pdot[4];
extern float PP[2][2];

extern unsigned long mictime;
extern unsigned long pretime;
extern double az_angle;
extern double gy_angle;
extern float gyro;
extern double ay_angle;
extern double gx_angle;
extern float gxro;

extern int i;
extern int rslt;
extern int16_t accelGyro[6];
/***************************************************************/


float Kalman_Filter(float Accel, float Gyro);
void MPU6050Init();
void BMI160_math_display();

#endif
