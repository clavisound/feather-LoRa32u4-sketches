// ttnmapping with static SF or variabLEle SF. Made by clavisound started from: https://learn.adafruit.com/the-things-network-for-feather/using-a-feather-32u4
// #define EU863 // BUG: TinyLoRa.h ignores this. Modify TinyLoRa.h else default is: US902. Other options: EU863, AU915, AS920

#define SF        7     // SF7 to SF12
#define DEBUGINO  1     // 1 = for debugging via serial. Sleep is OFF! 0 to save some ram and to enable sleep. +2404 bytes of program, +80 bytes of RAM. [default 0]
#define PHONEY    1     // 1 = don't TX via Radio LoRa (aka RF) but calculates some phoney TX time. (useful for debugging) [default 0]
#define CYCLESF   0     // 0 = don't cycleSF, 1 = cycle SF10 to SF8, 2 = send only once per day [default 0 or 3] 3 = from SF7 to SF10, 4 = from SF10 to SF12
#define CHAOS     1     // 1 = use some 'random' numbers to generate 'chaos' in delay between TX's. +212 program bytes, +33 bytes RAM; [default 1]
#define LED       2     // 0 = no led. 1=led for BOOT, TX, ABORT (not IDLE) [+94 bytes program] 2=led for BOOT, (not TX), ABORT, IDLE [+50 bytes program] [default: 2]
#define GPS       1     // 0 to use with your smartphone + ttnmapper app. 1 = For adafruit GPS ultimate featherwing
#define USBSERIAL 0     // 1 = to enable serial, 0 to save battery.

#include <TinyLoRa.h>
#include <SPI.h>
#include <Adafruit_SleepyDog.h>

// Data Packet to Send to TTNN
int8_t txPower = 0;                  // valid values -80, 0-20. For EU max limit is 14dBm

#if GPS == 1
#define FRAME_PORT_GPS 8;                // TTN mapper to 8. 7th port for 5 floats precision.
  uint8_t FramePort = FRAME_PORT_GPS;    // port with GPS data
  uint8_t loraData[13] = {};             // bytes to send
#else
  uint8_t FramePort = 3;                 // port without GPS
  uint8_t loraData[5] = {};              // bytes to send. 5 = 5bytes
#endif

uint16_t fc = 10000;                     // framecounter. We need this if we sleep. In that case lora module forgets everything. TODO store in EEPROM.

// Send every secs / mins: 7200''/ 120', 4200''/ 90', 3600''/ 60', 1800''/ 30', 1200''/ 20', 600''/ 10', 300''/ 5'
// ** BE CAREFUL TTN SUGGESTS MINUTES BETWEEN TRANSMISSIONS! **
uint32_t const secondsSleep = 1800;
// ** THIS IS THE LIMIT OF THIS PROGRAM **, DO NOT USE LESS THAN 10 SECONDS!
// uint32_t const secondsSleep = 10; // sleep for 10 seconds.

#define DAY  86400000 // = day in ms. Use to comply with TTN policy
#define TXMS 29000    // = TTN limit 30.000ms (30 seconds) per day.

#define VBATPIN A9 // battery pin to measure voltage

// ERASE THIS
#if GPS == 1 & DEBUGINO == 0
  #error Still testing. Enable DEBUGINO, or Serial dies from sleep.
#endif

#if GPS == 1
  #include <NMEAGPS.h>
  #include <GPSport.h>
  #define GPSSLEEP A4 // To put gps to sleep.
  NMEAGPS  gps;
  gps_fix  fix;

  uint32_t lat, lon;    // 32 bits
  uint8_t speed;        // TODO: divide by 3 to have max speed of 90 with 5 bits
  uint8_t heading;      // TODO: divide by 6 to fit in 6bits
  uint8_t hdop;         // > 20 = 20 x 5 meters = 100m
  uint8_t fixCount;     //
  uint16_t noFixCount;  // in cloudy balcony fix after 70 seconds.
#endif

// we need USBSERIAL if we want debugging.
#if DEBUGINO == 1
  #define USBSERIAL 1
#endif
       
// Pinout for Adafruit Feather 32u4 LoRa
TinyLoRa lora = TinyLoRa(7, 8, 4);

uint32_t uptime = 0;   // uptime in ms

/* don't transmit more than 30 seconds per day. */
// variables to calculate that.
uint32_t startTXms;   // start of TX ms
uint16_t currentTXms; // duration of TX ms
uint16_t totalTXms;   // max is 30 seconds (30000ms) for TTN
uint8_t  days;        // byte is for 255 days, unsigned 16bits are ok for 184 years (65535 / 350 = 184)
                      // Spend two more bytes (uint32_t) for 11 million years :)

/* battery values */
uint8_t vbat;         // raw voltage for battery range from 480 - 680 aka ~200
uint8_t vbatC;        // battery level 0-3 (0%, 30%, 60%, 90%)

uint16_t randMS;      // used to add some randomness in sleep

#if LED > 0
   /* To have every 5 sec blink we divide the secondsSleep with 5 secs and we find the number of blinks
   *  Example: 100 seconds / 5 = 20 blinks, another example: 600 seconds / 5 = 120 blinks.
   */
  uint16_t const blinks = secondsSleep / 8; // 8 = seconds maximum of watchdog
#endif

// after 24 hours (86.400.000 millis), we can re-send messages if we hit the wall (TTN rule)
// millis are rollover after 49 days and 17 hours
// rollover check: https://www.baldengineer.com/arduino-how-do-you-reset-millis.html
uint32_t lastTXtime;

void setup(){
//  delay(2000);

  #if USBSERIAL == 1 & DEBUGINO == 1
    Serial.begin(9600);
    while (! Serial); // don't start unless we have serial connection
  #endif
  
  #if LED > 0
    pinMode(LED_BUILTIN, OUTPUT); // Initialize pin LED_BUILTIN as an output
  #endif
  
  #if GPS == 1
    pinMode(GPSSLEEP, OUTPUT); // use GPSSLEEP pin as output. HIGH to sleep.
    gpsPort.begin(9600);

    Serial.println("command");
    // gps.send_P ( &gpsPort, F("PMTK000*32")); // EVAL commands are ignored?
    // gps.send_P ( &gpsPort, F("PMTK220,30000") ); //update 1time in 30s
    // gps.send_P ( &gpsPort, F("PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0") ); // RMC and GCA
  #endif

#define STARTDELAY 2 // seconds
  #if LED == 0
    delay(STARTDELAY * 1000); // wait (value in ms) before sending 1st message
  #else
    blinkLed(STARTDELAY, 500, 1000); // STARTDELAY times for Y duration and Z pause. (example is: 12, 250, 1000) aka 12 times, blink for 250 ms puse for 1 second.  
  #endif

  // QUESTION: Why I can't use the variable from second .cpp or .ino?
  //  Serial.print("Feather");Serial.println(feather);
}
