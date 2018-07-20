# ESP32 Arduino/Micropython サンプルプログラム

Interface誌2018年9月号に掲載したプログラムの全体です。

ESP32のArduinoとMicropythonでセンサデーターを読み、[IoTデーター可視化サービスAmbient](https://ambidata.io)に
送信し、可視化するプログラムです。

* Arduino/BME280_test: ArduinoでのBME280動作確認プログラム
* Arduino/Ambient_BME280: ArduinoでBME280のデーターをAmbientに送信するプログラム
* Arduino/Ambient_BME280_ds: ArduinoでBME280のデーターをAmbientに送信するプログラムで、送信と送信の間をDeepSleepするバージョン
* Arduino/Ambient_BME280_lowWiFi: ArduinoでBME280のデーターをAmbientに送信するプログラムで、Wi-Fi出力を下げたバージョン
* Arduino/Ambient_BME280_ds_6_1: ArduinoでBME280のデーターをAmbientに送信するプログラムで、6回に1回まとめて送信するバージョン
* Micropython: MicropythonでBME280のデーターをAmbientに送信するプログラム
* Python: Raspberry Pi上でAmbientから最新の温度、湿度を読み出し、熱中症の危険度を判定し、IFTTT経由でslackに通知するPythonプログラム
