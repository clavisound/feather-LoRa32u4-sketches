# feather-LoRa32u4-sketches
Here you can find two sketches for [TTNmapper](http://ttnmapper.org). One to use Adafruit Feather LoRa 32u4 with smartphone and one with
FeatherWing GPS.

What is supported:
1. Deep sleep. Map for one month (maybe more) with 350mA battery.
2. GPS sleep.
3. Cycle SF
4. Cycle Power
5. Respect to TTN limits (30 seconds per day)
6. Randomness to vary transmissions (to avoid colissions with other static nodes)
7. Info via LED
8. Debugging via Serial (I hope you will not need this  - it also disables Deep Sleep)
9. PHONEY mode. (for debugging - no transmission)

## You need in both
1. Modify `TinyLoRa.h` according to your region!

## Feather LoRa 32u4 + Smartphone
1. Install libraries: Adafruit SleepyDog, Adafruit TinyLoRa.
 library. (`GPSfix_cfg.h`)
2. You have to modify `AAfeathers.ino` according to your device / application.
3. Make integration with TTNmapper.
4. copy the appropriate decodings to the console.thethingsnetwork.org
5. All neccessary variables are in first tab.
6. Install to your Smartphone ttnmapper and configure it.

## Feather LoRa 32u4 + GPS
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

## bike-tracker-clv
You can also find one third sketch: bike-tracker-clv. You will *also* need the library [lis3dh](https://github.com/clavisound/lis3dh-motion-detection/) - thanks to [lbad](https://github.com/ldab/lis3dh-motion-detection) for the initial release and the [LIS3DH accelerometer](https://www.adafruit.com/product/2809) from Adafruit.

Extra things supported:
1. Power Down (wake from accelerometer)
2. Send location if the traveled distance is more than 50 meters.
3. Connect INT1 pin fron LIS3DH to #10 of feather-32u4-LoRa
