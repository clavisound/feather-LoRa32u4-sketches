// Added by clavisound, from Jul2019 to Apr2020
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

// TODO: EEPROM https://learn.adafruit.com/memories-of-an-arduino/optimizing-sram

// ChangeLog
// [solved] BUG (EPIC): after one day the days are ++ every TX, hours are ok. Watchdog was resetting the millis.

// RESULTS: with LED on idle (blink every 8 secs) 1 month of uptime with SF9 battery 350mAh.
