// Added by clavisound, from Jul2019 to Jun2020
// 1. SF cycle for experiments with ttnmapper.org
// 2. led notifications
// 3. Battery monitor / TXms (duration) monitor
// 4. Respect the EU/TTN policy: don't transmit more than 30 seconds (TODO: breaks with sleep watchdog)
// 5. Reset after 24 hours (EU/TTN policy)
// 6. Random delay between TX's.
// 7. compile options DEBUGINO, LED, PHONEY, CHAOS, CYCLESF

// instructions for use
// a. setup account in TTN, integrate with ttnmapper.
// b. install in your mobile phone ttnmapper and setup as experimental
// unless you use only SF7. ttnmapper wants only SF7.
//
// What this program do with the feather-32u4-lora
// 1. After power up we have 6 blinks with NO activity. (in case you want to abort example
// 2. Sends data with SF that *YOU* define.
// 3. Feather will transmit after the delay *YOU* define to check with
// your ttnmapper app if the data are received.
// 6. if the 30 TX seconds limit is reached, feather will sleep for the
// next duty cycle (approx. 24 hours) and you will see fast blinks before that.
// 7. Feather sends battery charging levels (20, 40, 60, 80%) expressed as 0, 1, 2, 3 and
// 7b. TX time in seconds.
// 7c. uptime in days.

// TODOs
// EEPROM https://learn.adafruit.com/memories-of-an-arduino/optimizing-sram Wear after ?? writes
// ACCEL       - Byte [DONE] + code [semiDONE]
// Button      - Code [DONE] + resistor [DONE with internal pull-up] (interrupt code)
// Buzzer      - Choose resistor (25ma max. EVAL 330Ohm)
// GPS         - DONE
// Reed switch - Test with magnet [pre-testing]
// BLE         - Installation + code

// ChangeLog
// [solved] SYSTEM BUG #1 (EPIC): after one day the days are ++ every TX. Hours where ok. Watchdog was resetting the millis.

// notes
// GPS [Done] 22-apr-2020: first boot of Adafruit featherwing 0,2080-01-06 00:01:21.790,,,0,0,,0,327,0,12477,
// GPS [Done] 23-apr-2020: Polling GPS with NeoGPS
// GPS [Done] 24-apr-2020: TTNmapper payload, 23-apr-2020
// GPS [Done] 24-apr-2020: Send some data to TTNmapper, 23-apr-2020
// GPS [Done]: get best fix? (with HDOP) - I think no, better with heading, 23-apr-2020.
// GPS TODO: If heading changes send message, 23-apr-2020
// GPS TODO [Discard]: After stop send only once and restart with km/h >= 3, 23-apr-2020
// GPS TODO: When sleep if LED, is on, it stays on 23-apr-2020.
// GPS TODO: Compare 24bits loc with 16bits, 23-apr-2020
// GPS TODO: bike payload, 23-apr-2020
// GPS [Done] 29-May-2020: distance? 23-apr-2020
// GPS [Done] 24-apr-2020: Power off GPS with two methods. Pin and Command.
// GPS [Done] 28,apr,2020: latitude payloads testing 10E5 vs 100000 e.t.c.,

// GPS WIP: EVAL power consumption with pin, sleep, auto-sleep, 23-apr-2020 
// GPS ON                              = 400 mA in 14 hours TX every 30m. 23-apr-2020
// GPS OFF with pin > seek two minutes =  40 mA in 17 hours TX every 30m. 24-apr-2020
// GPS OFF with TX  > seek two minutes = ??  mA in ?? hours TX every 30m.
// GPS AlwaysLocate > seek two minutes = ??  mA in ?? hours TX every 30m.

// NiMH LSD = 40mA in 24hours.

// MMA see GPS.ino. Stuck to MMA if enabled. [6-may-2020]
// MMA check if uptime more than ?? hours reset MMA just in case.
// LIS [13-May-2020]: Detect fall, EVAL reference register.

// SYSTEM BUG #2: hour uptime is wrong, it's counting every half hour. (check 1 may)
// SYSTEM QOS #1 [Done] 13-may-2020: Not an issue, just a counter. When Transmitting, disable INT1 + INT2. So in case of transmission, we don't have INTerruption.

// SYSTEM regression [Done] 6-may-2020: Wrong TX calculation [6-may-2020]

// SYSTEM rename: lastTXtime endTXtime to GPSendTXdayTime endTXdayTime
// SYSTEM eval DAY reset (ex. every 3 hours) with GPS: [7-may-2020]
// SYSTEM I think I don't need lastTXtime but only uptimeGPS
// GPS if after 12 hours wake up, we are in different location, send LoRa

// SYSTEM BUG #3 [Done] check BUG #8: GPS OFF via UART hangs something. (not the MCU, or Serial BUS, maybe the Interrupt BUS is constant ON?).

// BUG #4 [Done] 29-Jun-2020 (Solution: temporarily disable watchdog ): Sends every 3-4 minutes unconditionally
// BUG #5 [Done]: Wrong uptime after fix.
// BUG #6: Wrong Fixes 2008,8,featherb,-, 0s, 2d200h, 3.44(84), 1, 14, 2020-06-20 08:44:29, -26.320642, -177.068324, 0m, 239km/h240, 0sats, 0.1, 867.5, SF9BW125, eui-b827ebfffe7eed02, -, 2#, -104, 0.5
// BUG #7 [Done] 28-Jun-2020: Does not wake up if GPSsleep via UART! a) try to disable GPS with cable, then sleep. Solution: Exit checkFix() with return to free Serial1
// BUG #8 [Done]: GPS ON when Feather goToSleep(!) if DEBUGINO == 0 (!!) Watchdog was waking the MCU.
// BUG #9: Slow operation with Battery (3xAA eneloop style) vs USB (!)

// Senario #1: GPS   fix and speed 0    > accel sensitive > power down > wait accel for ever.
// Senario #2: GPS noFix                > accel sensitive > power down > wait accel for ever.
// Senario #3: GPS fix and speed over 0 > accel insensitive > scan every three minutes
// Senario #4: GPS same location        > accel insensitive > scan every three minutes
// Senario #5: Bike Fall                > send message unconditionally.

// SYSTEM: FEATHER 32u4 Deep Sleep = 0.2mA - 0.3mA, 0.7mA LIS3DH, GPS 1.5mA, RFM95 adds 2ma if not sleeping.
// Sleep without LoRa Sleeping 4.5mA (PHONEY RFM95 not sleeping)
// Sleep with all systems down: 2.5mA
