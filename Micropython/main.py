from machine import I2C, Pin
import utime
# from m5stack import lcd
import bme280
import ambient

ssid = 'ssid'
password = 'password'
channelId = 100
writeKey = 'writeKey'

def do_connect(ssid, password):
    import network
    sta_if = network.WLAN(network.STA_IF)
    if not sta_if.isconnected():
        print('connecting to network...')
        sta_if.active(True)
        sta_if.connect(ssid, password)
        while not sta_if.isconnected():
            pass
    print('network config:', sta_if.ifconfig())

i2c = I2C(sda=Pin(12, pull=Pin.PULL_UP), scl=Pin(14, pull=Pin.PULL_UP))
bme = bme280.BME280(i2c=i2c)
am = ambient.Ambient(channelId, writeKey)

do_connect(ssid, password)

while True:
    # lcd.clear()
    # lcd.print(str(bme.values), 0, 0)
    data = bme.read_compensated_data()
    print(bme.values)
    r = am.send({'d1': data[0] / 100.0, 'd2': data[2] / 1024.0, 'd3': data[1] / 25600.0})
    print(r.status_code)
    r.close()

    utime.sleep(5)
