#feather-LoRa32u4-sketches
Here you can find two sketches for [TTNmapper](http://ttnmapper.org). One to use Adafruit Feather LoRa 32u4 with smartphone and one with
FeatherWing GPS.

##You need in both
1. Modify `TinyLoRa.h` according to your region!

##Feather LoRa 32u4 + Smartphone
1. Install libraries: Adafruit SleepyDog, Adafruit TinyLoRa.
 library. (`GPSfix_cfg.h`)
2. You have to modify `AAfeathers.ino` according to your device / application.
3. Make integration with TTNmapper.
4. copy the appropriate decodings to the console.thethingsnetwork.org
5. All neccessary variables are in first tab.
6. Install to your Smartphone ttnmapper and configure it.

##Feather LoRa 32u4 + GPS
1. Install libraries: NeoGPS, Adafruit SleepyDog, Adafruit TinyLoRa.
2. In sketch with GPS you have to uncomment HDOP / SATELLITES from NeoGPS
 library. (`GPSfix_cfg.h`)
3. You have to modify `AAfeathers.ino` according to your device / application.
4. Make integration with TTNmapper.
5. copy the appropriate decodings to the console.thethingsnetwork.org
6. All neccessary variables are in first tab.
7. Connect TX from [Adafruit GPS](https://www.adafruit.com/product/3133) to RX in [Feather 32u4 - LoRa](https://www.adafruit.com/product/3078)
8. Connect RX from Adafruit GPS to TX in feather
9. Connect Power and GND.
10. Connect EN pin from Adafruit GPS to A4 pin (feather 32u4)
