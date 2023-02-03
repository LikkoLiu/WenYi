#include <mywifi.h>

/*************************** WIFI ******************************/
uint8_t Wifi_init_succ = 0;
// the IP of the machine to which you send msgs - this should be the correct IP in most cases (see note in python code)
const char *ssid = "ESP32";
const char *password = "12345678";
WiFiUDP Udp;
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
unsigned int localUdpPort = 1210; // 自定义本地监听端口
uint8_t incomingPacket[256];      // 保存Udp工具发过来的消息

const char *WIFIssid = "liu"; // The SSID (name) of the Wi-Fi network you want to connect to
const char *WIFIpassword = "33333333";

// 向udp工具发送消息
void wifi_printf(char buffer /*, char *Val*/)
{
    Udp.beginPacket(CONSOLE_IP, CONSOLE_PORT); // 配置远端ip地址和端口
    Udp.printf("%c", buffer);                  // 把数据写入发送缓冲区  Udp.beginPacket(Udp.remoteIP(), remoteUdpPort);//配置远端ip地址和端口
    //   Udp.print(Val);                              //把数据写入发送缓冲区
    Udp.endPacket(); // 发送数据
}

void wifi_ap_init()
{
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);

    if (Udp.begin(localUdpPort))
    { // 启动Udp监听服务
        ESP_LOGI(DEBUG_WIFI, "WiFI_begin is builded");
        Wifi_init_succ = 1;
    }
    else
    {
#if serialwifi_log
        Serial.printf("%c", 0xEE);
        Serial.printf("%c", ERR_InitErr); // 向串口打印信息
        ESP_LOGI(DEBUG_WIFI, "WiFI_begin is defeated");
#endif
    }
}

void wifi_sta_connect()
{
    WiFi.begin(WIFIssid, WIFIpassword); // Connect to the network
    ESP_LOGI(DEBUG_WIFI, "WIFI Connecting ");

    while (WiFi.status() != WL_CONNECTED)
    { // Wait for the Wi-Fi to connect
        delay(500);
        Serial.print('.');
    }
    ESP_LOGI(DEBUG_WIFI, " Connection established!");
    ESP_LOGI(DEBUG_WIFI, "IP address:%d", WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
}

void wifiEvent()
{
    // 解析Udp数据包
    if (Udp.parsePacket()) // 解析包不为空
    {
// 收到Udp数据包
//  Udp.remoteIP().toString().c_str()用于将获取的远端IP地址转化为字符串
#if serialwifi_log
        // Serial.printf("receive IP: %s , remote_Port: %d , Packet Bytes: %d\r\n", Udp.remoteIP().toString().c_str(), Udp.remotePort(), packetSize);
#endif
        // 读取Udp数据包并存放在incomingPacket
        int len = Udp.read(incomingPacket, 255); // 返回数据包字节数
        if (len == 9)
        {
            // incomingPacket[len - 1] = 0; // 标记字符串尾部
            uint8_t sum_res = 0;
            for (int i = 0; i < 8; i++)
            {
                // USART_SendData(USART1, incomingPacket[i]);
                sum_res += incomingPacket[i];
            }
            if (sum_res == incomingPacket[8])
            {
#if serialwifi_log
                // Serial.printf("UDP receive:  ");
                for (int k = 0; k < 9; k++)
                {
                    Serial.printf("%c", incomingPacket[k]); // 向串口打印信息
                }
#endif
                for (int k = 0; k < 9; k++)
                {
                    wifi_printf(incomingPacket[k]);
                    // delay(10);
                }

                Wifi_getEventFlag();
            }
            else
            {
#if serialwifi_log
                Serial.printf("%c", 0xEE);
                Serial.printf("%c", ERR_InputSumErr); // 向串口打印信息
#endif
            }
        }
        else
        {
#if serialwifi_log
            Serial.printf("%c", 0xEE);
            Serial.printf("%c", ERR_InputLess); // 向串口打印信息
#endif
        }
    }
}

void Wifi_getEventFlag()
{
    HEX_Format_flag = incomingPacket[6];
    TIME_TO_SLEEP = incomingPacket[7];
    COMMNUI_CH_flag = incomingPacket[5];
    if (incomingPacket[2] == 0x30)
    {
        _STOPACAN();
    }

    else if (incomingPacket[2] == 0x31)
    {
        _BMI160();
        SINGLE_flag = incomingPacket[3];
        // switch (incomingPacket[3])
        // {
        // case 0x31:

        //   break;
        // case 0x32:

        //   break;
        // case 0x33:

        //   break;
        // case 0x34:

        //   break;
        // default:
        //   break;
        // }
    }

    else if (incomingPacket[2] == 0x32)
    {
        _AS7341();
        SINGLE_flag = incomingPacket[3];
        gainval = incomingPacket[4];
        // switch (incomingPacket[5])
        // {
        // case 0x31:

        //   break;
        // case 0x32:

        //   break;
        // case 0x33:

        //   break;
        // case 0x34:

        //   break;
        // default:
        //   break;
        // }
    }
    else if (incomingPacket[2] == 0x33)
    {
        _UV();
        SINGLE_flag = incomingPacket[3];
        // switch (incomingPacket[3])
        // {
        // case 0x00:

        //   break;
        // case 0x01:

        //   break;
        // case 0x03:

        //   break;
        // default:
        //   break;
        // }
    }
    else if (incomingPacket[2] == 0x34)
    {
        _ALLCAN();
        SINGLE_flag = incomingPacket[3];
        gainval = incomingPacket[4];
        // switch (incomingPacket[3])
        // {
        // case 0x00:

        //   break;
        // case 0x01:

        //   break;
        // case 0x03:

        //   break;
        // default:
        //   break;
        // }
    }
}

void Wifi_float_to_hex_printf(uint8_t signnum, float num)
{
    union float_int
    {
        float x;
        uint8_t s[4];
    } floattoint;

    floattoint.x = num;
    wifi_printf(signnum);
    wifi_printf(floattoint.s[0]);
    wifi_printf(floattoint.s[1]);
    wifi_printf(floattoint.s[2]);
    wifi_printf(floattoint.s[3]);
}

void Wifi_uint16_to_hex_printf(uint16_t val, uint8_t chval, uint8_t gainval)
{
    uint8_t hexSendBuff[4];
    uint8_t i;
    hexSendBuff[0] = (0xff00 & val) >> 8;
    hexSendBuff[1] = (0xff & val);
    // hexSendBuff[2] = (0xff & chval);
    hexSendBuff[2] = chval << 4;
    hexSendBuff[2] += gainval;
    for (i = 0; i < 3; i++)
    {
        hexSendBuff[3] += hexSendBuff[i];
    }
    wifi_printf(0xbb);
    for (i = 0; i < 3; i++)
    {
        // USART_SendData(USART1, table_cp[i]);
        wifi_printf(hexSendBuff[i]);
    }
}
