#ifndef __RTCCLOCK_H_
#define __RTCCLOCK_H_

#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/apps/sntp.h"
#include "esp_log.h"
#include <mywifi.h>
#include <sys/time.h>
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_sntp.h"

extern RTC_DATA_ATTR uint8_t RESTART;

void esp_wait_sntp_sync(void);

void time_sync_notification_cb(struct timeval *tv);

#endif