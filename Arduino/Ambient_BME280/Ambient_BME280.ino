/*
 * ESP32 or M5StackとBME280をI2C接続し、温度、湿度、気圧を測定し
 * Ambientに送信する
 * Boschライブラリーを使う
 */
#ifdef ARDUINO_M5Stack_Core_ESP32
#include <M5Stack.h>
#endif

#include <Wire.h>
#include <esp_wifi.h>
#include "bme280_i2c.h"
#include "Ambient.h"

#define PERIOD 300

#define DELAY_CONNECTION 100

#ifdef ARDUINO_M5Stack_Core_ESP32
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

void setup(){
#ifdef ARDUINO_M5Stack_Core_ESP32
    M5.begin();
#endif
    pinMode(SDA, INPUT_PULLUP); // SDAピンのプルアップの指定
    pinMode(SCL, INPUT_PULLUP); // SCLピンのプルアップの指定
    Wire.begin(SDA, SCL);

    Serial.begin(115200);

    Serial.print("BME280 -> Ambient\r\n");

    int ret, i;
    while ((ret = WiFi.status()) != WL_CONNECTED) {
        Serial.printf("> stat: %02x", ret);
        ret = WiFi.begin(ssid, password);  //  Wi-Fi APに接続
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
    Serial.print("WiFi connected\r\nIP address: ");
    Serial.println(WiFi.localIP());

    bme280.begin(); // BME280の初期化

    ambient.begin(channelId, writeKey, &client); // チャネルIDとライトキーを指定してAmbientの初期化
}

void loop() {
    int t = millis();
    int8_t rslt;
    struct bme280_data data;

    rslt = bme280.get_sensor_data(&data);
    Serial.printf("%0.2f, %0.2f, %0.2f\r\n", data.temperature, data.humidity, data.pressure / 100);

    // 温度、湿度、気圧、CO2、TVOCの値をAmbientに送信する
    ambient.set(1, String(data.temperature).c_str());
    ambient.set(2, String(data.humidity).c_str());
    ambient.set(3, String(data.pressure / 100).c_str());

    ambient.send();

    t = millis() - t;
    t = (t < PERIOD * 1000) ? (PERIOD * 1000 - t) : 1;
    delay(t);
}
