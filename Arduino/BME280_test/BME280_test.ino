/*
 * ESP32 or M5StackとBME280をI2C接続し、温度、湿度、気圧を測定しプリントアプトする
 * Boschライブラリーを使う
 */
#ifdef ARDUINO_M5Stack-Core-ESP32
#include <M5Stack.h>
#endif

#include <Wire.h>
#include "bme280_i2c.h"

#ifdef ARDUINO_M5Stack-Core-ESP32
#define SDA 21
#define SCL 22
#else
#define SDA 12
#define SCL 14
#endif

BME280 bme280(BME280_I2C_ADDR_PRIM);

void setup(){
#ifdef ARDUINO_M5Stack-Core-ESP32
    M5.begin();
#endif
    pinMode(SDA, INPUT_PULLUP); // SDAピンのプルアップの指定
    pinMode(SCL, INPUT_PULLUP); // SCLピンのプルアップの指定
    Wire.begin(SDA, SCL);

    Serial.begin(115200);

    Serial.print("BME280 test\r\n");

    bme280.begin(); // BME280の初期化
}

void loop() {
    int8_t rslt;
    struct bme280_data data;

    rslt = bme280.get_sensor_data(&data);
    Serial.printf("%0.2f, %0.2f, %0.2f\r\n", data.temperature, data.humidity, data.pressure / 100);

    delay(3000);
}

