#ifndef __MYWIFI_H_
#define __MYWIFI_H_
#include <WiFi.h>
#include "WiFiUdp.h"
#include <communication.h>
#include <ERR.h>
#define serialwifi_log 1
#define CONSOLE_IP "192.168.1.2"
#define CONSOLE_PORT 4210

extern uint8_t Wifi_init_succ;
extern const char *ssid;
extern const char *password;
extern unsigned int localUdpPort; // 自定义本地监听端口
extern uint8_t incomingPacket[256]; // 保存Udp工具发过来的消息

void wifi_ap_init();
void wifi_printf(char buffer /*, char *Val*/);
void wifiEvent();
void Wifi_getEventFlag();
void Wifi_float_to_hex_printf(uint8_t signnum, float num);
void Wifi_uint16_to_hex_printf(uint16_t val, uint8_t chval, uint8_t gainval);
void wifi_sta_connect();

#endif
