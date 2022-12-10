#ifndef __COMMUNICATION_H_
#define	__COMMUNICATION_H_
#include <Arduino.h>

extern uint8_t table_data[9]; // 这是提前定义一个数组存放接收到的数据
extern uint8_t table_cp[9];   // 这是额外定义一个数组，将接收到的数据复制到这里面
extern uint16_t count;    // 接收数据计数

void serialEvent();

#endif
