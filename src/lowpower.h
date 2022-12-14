#ifndef __LOWPOWER_H_
#define __LOWPOWER_H_
#include <Arduino.h>

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
// #define TIME_TO_SLEEP 7           /* Time ESP32 will go to sleep (in seconds) */
#define serialpower_log 0

RTC_DATA_ATTR extern uint8_t TIME_TO_SLEEP;
void print_wakeup_reason();

#endif
