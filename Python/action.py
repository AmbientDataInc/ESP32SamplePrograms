# -*- coding: utf-8 -*-
#
import requests
import ambient

IFTTT_URL = 'https://maker.ifttt.com/trigger/indoor_temp_humid/with/key/'
IFTTT_KEY = 'IFTTT_KEY'

channelId = 100 #  チャネルID
readKey = 'readKey'

debug = True

# 熱中症の危険度判定
# 0: 注意、1: 警戒、2: 厳重警戒、3: 危険
# 日本生気象学会「日常生活における熱中症予防指針」Ver.3を元に作成
# http://seikishou.jp/pdf/news/shishin.pdf

def WBGTlevel(humid, temp):
    if (temp > (humid * (-12.0 / 70.0) + 40.0 + 36.0 / 7.0)):
        return 3
    if (temp > humid * (-13.0 / 80.0) + 25.0 + 130.0 / 8.0):
        return 2
    if (temp > humid * (-3.0 / 20.0) + 37.0):
        return 1
    else:
        return 0

WBGTmsg = ['注意', '警戒', '厳重警戒', '危険']

def main():
    am = ambient.Ambient(channelId, '', readKey)

    data = am.read(n=1)
    temp = data[0]['d1']
    humid = data[0]['d2']
    created = data[0]['created']
    level = WBGTlevel(humid, temp)
    print({'temp': temp, 'humid': humid, 'level': level, 'created': created})
    if (level > 1 or debug):
        requests.post(IFTTT_URL + IFTTT_KEY, json = {'value1': temp, 'value2': humid, 'value3': WBGTmsg[level]})

if __name__ == "__main__":
    main()
