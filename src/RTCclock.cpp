#include <RTCclock.h>

RTC_DATA_ATTR uint8_t RESTART = 0;

static uint8_t esp_initialize_sntp(void)
{
    if (wifi_sta_connect())
    {
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        ESP_LOGI(INFO_DEBUG, "WiFI_OFF is successed");
        return 1;
    }

    ESP_LOGI(INFO_DEBUG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "cn.pool.ntp.org");
    sntp_setservername(1, "210.72.145.44"); // 国家授时中心服务器 IP 地址
    sntp_setservername(2, "ntp1.aliyun.com");
    sntp_setservername(3, "1.cn.pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
    return 0;
}

uint8_t esp_wait_sntp_sync(void)
{
    char strftime_buf[64];
    if (esp_initialize_sntp())
        return 1;

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;

    while (timeinfo.tm_year < (2019 - 1900))
    {
        ESP_LOGE(ERROR_DEBUG, "Please waiting for system time to be set... (%d)", ++retry);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    // vTaskDelay(50 / portTICK_PERIOD_MS);
    // set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGE(INFO_DEBUG, "The current date/time in HANGZHOU is: %s", strftime_buf);
    // Serial.println("OK");
    sntp_stop();
    WiFi.disconnect();
    // Serial.println("OK");
    WiFi.mode(WIFI_OFF);
    // Serial.println("OK");
    ESP_LOGI(INFO_DEBUG, "system time is build, WiFI_OFF is successed!");

    return 0;
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(INFO_DEBUG, "Notification of a time synchronization event");
}
