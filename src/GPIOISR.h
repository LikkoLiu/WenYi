#ifndef __GPIOISR_H_
#define __GPIOISR_H_

#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <communication.h>

#define GPIO_ISR   GPIO_NUM_25
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_ISR)  // 配置GPIO_IN位寄存器
#define ESP_INTR_FLAG_DEFAULT 0

void IRAM_ATTR gpio_isr_handler(void* arg) ;
void gpio_ISR_task(void* arg) ;
void gpio_ISR_init(void);
void gpio_intr_init(void);

#endif