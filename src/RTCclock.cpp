#include <RTCclock.h>

RTC_DATA_ATTR uint8_t RESTART = 0;

static const char *TAG = "sntp";

static void esp_initialize_sntp(void)
{
    wifi_sta_connect();
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp1.aliyun.com");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

void esp_wait_sntp_sync(void)
{
    char strftime_buf[64];
    esp_initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;

    while (timeinfo.tm_year < (2019 - 1900))
    {
        ESP_LOGD(TAG, "Waiting for system time to be set... (%d)", ++retry);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    
    // set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in HANGZHOU is: %s", strftime_buf);
    sntp_stop();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    ESP_LOGI(DEBUG_WIFI, "WiFI_OFF is successed");
}


void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}
