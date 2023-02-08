#ifndef __ERR_H_
#define __ERR_H_

#define ERR_InputLess 0x11
#define ERR_InputSumErr 0x12
#define ERR_InitErr 0x13
#define ERR_Format 0x14
#define ERR_disconnect 0x15
#define ERR_AS7341_Init 0x31
#define ERR_AS7341_read 0x32
#define ERR_BMI160_Init 0x21
#define ERR_BMI160_I2cInit 0x22
#define ERR_BMI160_read 0x23

#define Succ_Init 0x13
#define Succ_disconnect 0x15
#define Succ_AS7341_Init 0x31

extern float UVdata_Fake;

const char INFO_DEBUG[] = "INFO_DEBUG";
const char ERROR_DEBUG[] = "ERROR_DEBUG";

#endif
