#ifndef __COMMUNICATION_H_
#define __COMMUNICATION_H_
#include <Arduino.h>
#include <mywifi.h>
#include <lowpower.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define _BMI160()         \
    BMI160_SCAN_flag = 1; \
    AS7341_SCAN_flag = 0; \
    UV_SCAN_flag = 0;
#define _AS7341()         \
    BMI160_SCAN_flag = 0; \
    AS7341_SCAN_flag = 1; \
    UV_SCAN_flag = 0;
#define _UV()             \
    BMI160_SCAN_flag = 0; \
    AS7341_SCAN_flag = 0; \
    UV_SCAN_flag = 1;
#define _STOPACAN()       \
    BMI160_SCAN_flag = 0; \
    AS7341_SCAN_flag = 0; \
    UV_SCAN_flag = 0;
#define _ALLCAN()         \
    BMI160_SCAN_flag = 1; \
    AS7341_SCAN_flag = 1; \
    UV_SCAN_flag = 1;

RTC_DATA_ATTR extern uint8_t HEX_Format_flag;
RTC_DATA_ATTR extern uint8_t LOW_POWER_flag;
RTC_DATA_ATTR extern uint8_t BMI160_SCAN_flag;
RTC_DATA_ATTR extern uint8_t AS7341_SCAN_flag;
RTC_DATA_ATTR extern uint8_t UV_SCAN_flag;
RTC_DATA_ATTR extern uint8_t SINGLE_flag;
RTC_DATA_ATTR extern uint8_t COMMNUI_CH_flag;
extern uint8_t table_data[9]; // 这是提前定义一个数组存放接收到的数据
extern uint8_t table_cp[9];   // 这是额外定义一个数组，将接收到的数据复制到这里面
extern uint16_t count;        // 接收数据计数
extern uint8_t table_sum;
RTC_DATA_ATTR extern uint8_t gainval;

void serialEvent();
void getEventFlag();
void float_to_hex_printf(uint8_t signnum,float num);
void uint16_to_hex_printf(uint16_t val, uint8_t chval , uint8_t gainval);

#endif
