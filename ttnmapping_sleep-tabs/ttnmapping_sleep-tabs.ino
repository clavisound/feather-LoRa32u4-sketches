// ttnupmapping with static SF or variable SF. Made by clavisound started from: https://learn.adafruit.com/the-things-network-for-feather/using-a-feather-32u4
// #define EU863 // BUG: TinyLoRa.h ignores this. If not defined default is: US902. Other options: EU863, AU915, AS920

#define DEBUGINO 0  // 1 = for debugging via serial, 0 to save some ram. +1606 bytes of program, +22 bytes of RAM. [default 0]
#define PHONEY 0    // 1 = don't TX via Radio LoRa (aka RF) but calculates some phoney TX time. (useful for debugging) [default 0]
#define CYCLESF 0   // 0 = don't cycleSF, 1 = cycle SF10 to SF8, 2 = send only once per day [default 0 or 3] 3 = from SF7 to SF10, 4 = from SF10 to SF12
#define CHAOS 1     // 1 = use some 'random' numbers to generate 'chaos' in delay between TX's. +212 program bytes, +4 bytes RAM; [default 1]
#define LED 2       // 0 = no led. 1=led for BOOT, TX, ABORT (not IDLE) [+94 bytes program] 2=led for BOOT, (not TX), ABORT, IDLE [+50 bytes program] [default: 2]
#define USBSERIAL 0 // 1 = to enable serial, 0 to save battery.

#include <TinyLoRa.h>
#include <SPI.h>
#include <Adafruit_SleepyDog.h>

// Data Packet to Send to TTNN
uint8_t loraData[5] = {}; // init data to send.
uint16_t fc = 0;          // framecounter. We need this if we sleep. In that case lora module forgets everything. TODO store in EEPROM.
uint8_t FramePort = 3;    // latest is 3
int8_t txPower = -80;     // valid values -80, 0-20. For EU max limit is 14dBm

// Send every secs / mins: 7200''/ 120', 4200''/ 90', 3600''/ 60', 1800''/ 30', 1200''/ 20', 600''/ 10', 300''/ 5'
// ** BE CAREFUL TTN SUGGESTS MINUTES BETWEEN TRANSMISSIONS! **
uint32_t const secondsSleep = 1800;
// ** THIS IS THE LIMIT OF THIS PROGRAM **, DO NOT USE LESS THAN 10 SECONDS!
// uint32_t const secondsSleep = 10; // sleep for 10 seconds.

#define DAY 86400000 // = day in ms. Use to comply with TTN policy
// EVAL test with 1000 and check if TX stops.
#define TXMS 1500 // = TTN limit 30.000ms (30 seconds) per day.
//#define TXMS 15000 // = TTN limit 30.000ms (30 seconds) per day.

// battery pin
#define VBATPIN A9

// we need USBSERIAL if we want debugging.
#if DEBUGINO == 1
  #define USBSERIAL 1
#endif
       
// Pinout for Adafruit Feather 32u4 LoRa
TinyLoRa lora = TinyLoRa(7, 8, 4);

uint32_t uptime = 0; // uptime in ms

// don't transmit more than 30 seconds per day.
// variables to calculate that.
uint32_t startTXms; // start of TX ms
uint16_t currentTXms; // duration of TX ms
uint16_t totalTXms; // max is 30 seconds (30000ms) for TTN
uint8_t days; // byte is for 255 days, usigned int is ok for 184 years (65535 / 350 = 184 change it to long, Spend two more bytes (unsigned long) for 11 million years :)

// battery values
uint8_t vbat; // raw voltage for battery range from 480 - 680 aka ~200
uint8_t vbatC; // battery level 0-3 (0%, 30%, 60%, 90%)

#if LED > 0
  // blink times, duration blink (ms), pause (ms)
  /* To have every 5 sec blink we divide the secondsSleep with 6 secs and we find the number of blinks
   * 
   *  Example: 100 seconds / 5 = 20 blinks
   *  Example: 600 seconds / 5 = 120 blinks.
   *  
   */
  uint32_t const blinks = secondsSleep / 5; // 5 = seconds
#endif

// after 24 hours (86.400.000 millis), we can re-send messages if we hit the wall (TTN rule)
// millis are rollover after 49 days and 17 hours
// rollover check: https://www.baldengineer.com/arduino-how-do-you-reset-millis.html
uint32_t lastTXtime;

void setup(){
  delay(2000);

  #if USBSERIAL == 1 & DEBUGINO == 1
    Serial.begin(9600);
  #endif
  
  // while (! Serial); // don't start unless we have serial connection

  #if LED > 0
    // Initialize pin LED_BUILTIN as an output
    pinMode(LED_BUILTIN, OUTPUT);
  #endif

  #if LED == 0
    delay(6000); // wait 6 seconds (value in ms) before sending 1st message
  #else
    blinkLed(6, 500, 1000); // X times for Y duration and Z pause. (example is: 12, 250, 1000) aka 12 times, blink for 250 ms puse for 1 second.  
  #endif

  // QUESTION: Why I can't use the variable from second .cpp or .ino?
//  Serial.print("Feather");Serial.println(feather);
}
