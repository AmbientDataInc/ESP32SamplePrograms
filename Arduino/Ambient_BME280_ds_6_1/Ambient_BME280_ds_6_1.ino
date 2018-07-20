/*
 * ESP32でBME280を読み、Ambientに送り、DeepSleepする
 * 6回に1回Ambientに送信する
 */
#ifdef ARDUINO_M5Stack-Core-ESP32
#include <M5Stack.h>
#endif
#include <Wire.h>
#include <esp_wifi.h>
#include "bme280_i2c.h"
#include "Ambient.h"
#include <time.h>

#define JST     3600* 9
#define UTC     3600* 0

#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  300
#define SEND_CYCLE 6  // 6回分のデータをまとめてAmbientに送信する

#define DELAY_CONNECTION 100

RTC_DATA_ATTR int bootCount = 0;  // RTCスローメモリ領域に割り付ける
struct SensorData {
    float temp;
    float humid;
    float pressure;
};
RTC_DATA_ATTR SensorData sensorData[SEND_CYCLE];

#ifdef ARDUINO_M5Stack-Core-ESP32
#define SDA 21
#define SCL 22
#else
#define SDA 12
#define SCL 14
#endif

BME280 bme280(BME280_I2C_ADDR_PRIM);

WiFiClient client;
Ambient ambient;

const char* ssid = "ssid";
const char* password = "password";

unsigned int channelId = 100; // AmbientのチャネルID
const char* writeKey = "writeKey"; // ライトキー

esp_sleep_wakeup_cause_t wakeup_reason;

time_t getSntpTime() {
    time_t t;
    struct tm *tm;

    configTime(UTC, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp", NULL);

    t = time(NULL);
    tm = localtime(&t);
    while ((tm->tm_year + 1900) < 2000) {
        t = time(NULL);
        tm = localtime(&t);
        delay(100);
    }
    Serial.printf("Now: %04d/%02d/%02d %02d:%02d:%02d\r\n\r\n", tm->tm_year + 1900,
        tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    return t;
}

#define BUFSIZE 500

void sendDataToAmbient(time_t current) {
    char buffer[BUFSIZE];
    int i;

    ambient.begin(channelId, writeKey, &client); // チャネルIDとライトキーを指定してAmbientの初期化

    sprintf(buffer, "{\"writeKey\":\"%s\",\"data\":[", writeKey);
    if (wakeup_reason != 3) { // if wakeup by reset
        i = SEND_CYCLE - 1; // リセット直後なら最後のデーターだけ送信
    } else {
        i = 0; // そうでないなら全部のデーターを送信
    }
    for (; i < SEND_CYCLE; i++) {
        time_t created = current - TIME_TO_SLEEP * (SEND_CYCLE - 1 - i);
        sprintf(&buffer[strlen(buffer)], "{\"created\":%d,\"time\":1,\"d1\":%2.1f,\"d2\":%.1f,\"d3\":%.0f},", created, sensorData[i].temp, sensorData[i].humid, sensorData[i].pressure);
    }
    buffer[strlen(buffer)-1] = '\0';
    sprintf(&buffer[strlen(buffer)], "]}\r\n");
    Serial.printf("%s", buffer);

    int n = ambient.bulk_send(buffer);
    Serial.printf("sent: %d\r\n", n);
}

void setup(){
    time_t t = millis();

#ifdef ARDUINO_M5Stack-Core-ESP32
    M5.begin();
#endif
    pinMode(SDA, INPUT_PULLUP); // SDAピンのプルアップの指定
    pinMode(SCL, INPUT_PULLUP); // SCLピンのプルアップの指定
    Wire.begin(SDA, SCL);

    Serial.begin(115200);

    bme280.begin(); // BME280の初期化

    int8_t rslt;
    struct bme280_data data;

    rslt = bme280.get_sensor_data(&data); // センサデータの取得
    Serial.printf("%0.2f, %0.2f, %0.2f\r\n", data.temperature, data.humidity, data.pressure / 100);

    wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason != 3) { // not wakeup by timer -> reset
        bootCount = SEND_CYCLE - 1;
    }
    Serial.print("Boot count: "); Serial.println(bootCount);

    sensorData[bootCount].temp = data.temperature;
    sensorData[bootCount].humid = data.humidity;
    sensorData[bootCount].pressure = data.pressure / 100;

    if (bootCount == (SEND_CYCLE - 1)) {
        int ret, i;
        while ((ret = WiFi.status()) != WL_CONNECTED) {
            Serial.printf("> stat: %02x", ret);
            ret = WiFi.begin(ssid, password);  //  Wi-Fi APに接続
            esp_wifi_set_max_tx_power(-4); // Wi-Fi出力を下げる
            i = 0;
            while ((ret = WiFi.status()) != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
                delay(DELAY_CONNECTION);
                if ((++i % (1000 / DELAY_CONNECTION)) == 0) {
                    Serial.printf(" >stat: %02x", ret);
                }
                if (i > 10 * 1000 / DELAY_CONNECTION) { // 10秒経過してもDISCONNECTEDのままなら、再度begin()
                    break;
                }
            }
        }
        Serial.print("\r\nWiFi connected\r\nIP address: ");
        Serial.println(WiFi.localIP());

        time_t sntptime = getSntpTime();

        sendDataToAmbient(sntptime);

        bootCount = 0;
    } else {
        bootCount++;
    }

    time_t elapse = (millis() - t) * 1000; // 経過時間(μS)
    time_t time2sleep = TIME_TO_SLEEP * uS_TO_S_FACTOR - elapse; // DeepSleep時間(μS)
    if (time2sleep < 1 * uS_TO_S_FACTOR) { // DeepSleep時間が1秒未満だったら
        time2sleep = 1 * uS_TO_S_FACTOR; // 1秒に設定
    }
    esp_sleep_enable_timer_wakeup(time2sleep);
    esp_deep_sleep_start();
}

void loop(){
}

