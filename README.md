# WARNING: obsolete project
Since TTN V3, this project is considered hostile to the TTN, since TTN *requires* all the nodes to support MAC commands. This project uses TinyLoRa which does not support MAC commands. SlimLoRa have some support, but it's untested. I think it's fairly easy to port this project to LMIC if you have a MCU with more than 64kB of flash RAM.

# feather-LoRa32u4-sketches
Here you can find three sketches. Two for [TTNmapper](http://ttnmapper.org). One to use Adafruit Feather LoRa 32u4 with smartphone and one with
FeatherWing GPS. Extra sketch is a bike tracker.

What is supported:
1. Deep sleep. Map for one month (maybe more) with 350mA battery --- if attached accelerometer
2. GPS sleep.
3. Cycle SF --- optional.
4. Cycle Power --- optional.
5. Respect to TTN limits (30 seconds per day)
6. Randomness to vary transmissions (to avoid colissions with other static nodes) --- optional.
7. Info / DEBUG via LED --- optional
8. Randomnly transmission beetween two predefined SF's --- optional.
9. Transmit only if we are away 50 meteres --- configurable.
10. Hopefully, more to come!

Debug options:
1. Debugging via Serial (I hope you will not need this)
2. PHONEY mode (for debugging - no transmission. Emulates some transmission time)
3. INDOOR mode (more debugging help)
4. TODO: Send DEBUG data via LoRaWAN.

## You need in both
1. Modify `TinyLoRa.h` according to your region!

## Feather LoRa 32u4 + Smartphone
1. Install libraries: Adafruit SleepyDog, Adafruit TinyLoRa.
2. You have to modify `AAfeathers.ino` according to your device / application.
3. Make integration with TTNmapper.
4. copy the appropriate decodings to the console.thethingsnetwork.org
5. All neccessary variables are in first tab.
6. Install to your Smartphone ttnmapper and configure it.

## Feather LoRa 32u4 + GPS
1. Install libraries: NeoGPS, Adafruit SleepyDog, Adafruit TinyLoRa.
2. In sketch with GPS you have to uncomment HDOP / SATELLITES from NeoGPS library. (`GPSfix_cfg.h`)
3. You have to modify `AAfeathers.ino` according to your device / application.
4. Make integration with TTNmapper.
5. copy the appropriate decodings to the console.thethingsnetwork.org
6. All neccessary variables are in first tab.
7. Connect TX from [Adafruit GPS](https://www.adafruit.com/product/3133) to RX in [Feather 32u4 - LoRa](https://www.adafruit.com/product/3078)
8. Connect RX from Adafruit GPS to TX in feather
9. Connect Power and GND.
10. Connect EN pin from Adafruit GPS to A4 pin (feather 32u4)

## bike-tracker-clv (Feather LoRa 32u4 + GPS + Accelerometer)
Here you need extra software *and* hardware. This skecth is mainly tested with MMA8452 (latest testing) but it should work with LIS3DH (early testing).

You can also find one third sketch: bike-tracker-clv. You will *also* need the my custom library [mma8452](https://github.com/clavisound/SparkFun_MMA8452Q_Arduino_Library) *if* you need Low Power (deep sleep) of 0.15mA.

If you don't mind about +.7mA in sleep you can use Adafruit's LIS3DH with LDO and level shifter again with my custom library: [lis3dh](https://github.com/clavisound/lis3dh-motion-detection/) - thanks to [lbad](https://github.com/ldab/lis3dh-motion-detection) for the initial release and the [LIS3DH accelerometer](https://www.adafruit.com/product/2809) from Adafruit.

Extra things supported:
1. Power Down (wake from accelerometer)
2. Send location if the traveled distance is more than 50 meters, otherwise send only "Heartbeat".
3. Connect A4 pin to a transistor's base to completely shutdown GPS. (I know about EN pin of GPS but it [fails to me](https://github.com/clavisound/feather-LoRa32u4-sketches/issues/5) for Deep Sleep for unknown reason [+1.5.ma]. Extra benefit, [no need to think about the LED](https://github.com/clavisound/feather-LoRa32u4-sketches/issues/4)).

### extra connections
1a. Connect INT1 pin from LIS3DH to #10 of feather-32u4-LoRa *OR*

1b. Connect INT1 pin from SparkFun MMA8452Q to #10 and INT2 to #11 of feather.

2. Install a PNP transistor. Collector to 3V3 of feather, Base > 270R (don't forget the resistor, or the feather will die!) > A4 pin of Feather and Emitter on GPS 3V3.

3. Connect 3.3V (Feather 3V3) to BAT pin of GPS to have RTC. *It's important* to have RTC, _otherwise sketch will be unpredictable_.
