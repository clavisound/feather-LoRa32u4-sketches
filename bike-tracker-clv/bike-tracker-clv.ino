// Bike tracker. Made by clavisound.
// #define EU863 // BUG: TinyLoRa.h ignores this. Modify TinyLoRa.h else default is: US902. Other options: EU863, AU915, AS920

#define SF        9     // [default 7] SF7 to SF12. Use 12 only for testing, if you are away from gateway and stationery
#define CYCLESF   0     // [default 0] 0 = don't cycleSF, 1 = cycle SF10 to SF8, 2 = send only once per day [default 0 or 3] 3 = from SF7 to SF10, 4 = from SF10 to SF12
#define LED       3     // [default 0] 0 = no led. 1=led for BOOT, TX, ABORT (not IDLE) [+94 bytes program] 2=led for BOOT, (not TX), ABORT, IDLE [+50 bytes program] 3 = ledDEBUG [default: 2]
#define CHAOS     1     // [default 1] 1 = use some 'random' numbers to generate 'chaos' in delay between TX's. +392 program bytes, +3 bytes RAM; [default 1]
#define GPS       1     // [default 1] 0 to use with your smartphone + ttnmapper app. 1 = For adafruit GPS ultimate featherwing
#define LISDH     1     // [default 1] 0 for LIS3DH  accelerator, 1 for yes.
//#define BUZZER    1   // TODO [default 0] 1 to hear some beeps!

int8_t txPower = 3;     // valid values -80, 0-20. For EU limit is 14dBm

#define MMA8452   0     // [NOT SUPPORTED - FAILED EFFORT. After abuse it locks. Maybe with I2C it's reliable - also maybe my building enviroment was faulty.]
                        // 0 to disable code for MMA8452 accelerator, 1 to enable.
// DEBUG options
#define DEBUGINO  0     // [default 0] 1 = for debugging via serial. Sleep is OFF! 0 to save some ram and to enable sleep. +3904 bytes of program, +200 bytes of RAM. [default 0]
#define INDOOR    0     // [default 0] For DEBUG INDOORs
#define PHONEY    0     // [default 0] 1 = don't TX via Radio LoRa (aka RF) but calculates some phoney TX time. (useful for debugging) [default 0]

// Data Packet to Send to TTN
#define FRAME_PORT_NO_GPS 3
#define LORA_HEARTBEAT    5

// USER SETTINGS //
uint16_t fc = 3000;                     // framecounter. We need this if we sleep. In that case lora module forgets everything. TODO store in EEPROM

// Send every secs / mins: 7200''/ 120', 4200''/ 90', 3600''/ 60', 1800''/ 30', 1200''/ 20', 600''/ 10', 300''/ 5', 180''/3'
// ** BE CAREFUL TTN SUGGESTS MINUTES BETWEEN TRANSMISSIONS! **
uint32_t secondsSleep = 1800;
// ** THIS IS THE LIMIT OF THIS PROGRAM **, DO NOT USE LESS THAN 10 SECONDS!
// uint32_t const secondsSleep = 10; // sleep for 10 seconds.

#include <TinyLoRa.h>
#include <SPI.h>
#include <Adafruit_SleepyDog.h>

#if GPS == 1
  #define DAY  86400    // = day in seconds. Use to comply with TTN policy (7200 for 2 hours)
#else
  #define DAY  86400000 // = day in ms. (you are using millis instead of GPS time)
#endif
#define TXMS 30000      // = TTN limit 30.000ms (30 seconds) per day.
//#define TXMS 1000      // DEBUG

#define VBATPIN A9 // battery pin to measure voltage

#if GPS == 1
  #include <NMEAGPS.h>
  #include <GPSport.h>
  #define  GPS_SLEEP_PIN A4 // Pin connected to EN pin of GPS to SLEEP / WAKE
  NMEAGPS  gps;
  gps_fix  fix;

  uint32_t lat, lon;        // 32 bits
  uint32_t oldLat, oldLon;  // 32 bits
  uint16_t altitude;        // 16 bits. Everest is >8.000meters
  uint8_t  speed;       
  //uint8_t  oldSpeed;     
  uint8_t  heading;     
  //uint8_t  oldHeading;  
  uint8_t  hdop;                                      // > 20 = 20 x 5 meters = 100m
  uint8_t  sats;                                      // satellites
  uint8_t noFixCount;                                 // in cloudy balcony fix after 70 seconds.

  #define FRAME_PORT_GPS 7                            // TTN mapper to 8. 7th port for 5 floats precision.
  #define LORA_TTNMAPPER 17                      
  uint32_t bootTime;
  uint32_t uptimeGPS;
  uint32_t lastTXtime = 86400;                        // have to set this, otherwise first TX fails;
  uint8_t FramePort = FRAME_PORT_GPS;                 // port with GPS data
  uint8_t loraData[LORA_TTNMAPPER] = {};              // bytes to send
  uint8_t loraSize = LORA_TTNMAPPER;
#else
  uint8_t FramePort = FRAME_PORT_NO_GPS;              // port without GPS
  uint8_t loraData[LORA_HEARTBEAT] = {};              // bytes to send. 5 = 5bytes
  uint8_t loraSize = LORA_HEARTBEAT;
#endif

#if MMA8452 == 1
  #include <avr/sleep.h>
  #include <avr/power.h>
  #include <Wire.h>                 // Must include Wire library for I2C
  #include "SparkFun_MMA8452Q.h"    // Click here to get the library: http://librarymanager/All#SparkFun_MMA8452Q

  #define INT_PIN_TEN    10
  #define INT_PIN_ELEVEN 11

  // MASKS
  #define MSKMTN   0x04             //Motion
  #define MSKTAP   0x08             //Tap
  #define MSKPL    0x10             //Orientation
  #define MSKTRS   0x20             //Transient

  uint8_t evntMMA;                           // event type
  volatile uint8_t interruptEventMMA = 0;    // 0 for no interruptEvent
  uint8_t orientation;                       // 0 (right fall), 1 (left fall), 2 (normal), 3 (upside down), 64 (endo or whellie)
  uint8_t tap;                               // 16 tap on X, 24 double? tap X, 17 tap Y, 25 double? tap Y,
                                             // 64 tap from behind?
                                             // 68 ?
                                             // 32 tap

  MMA8452Q accel;                   // create instance of the MMA8452 class
#endif

#if LISDH == 1

  #include "lis3dh-motion-detection.h"

  #include <avr/sleep.h>
  #include <avr/power.h>

  #define INT_PIN_TEN    10
  #define INT_PIN_ELEVEN    11

  #define LIS_RATE     50    // HZ - Samples per second - 1, 10, 25, 50, 100, 200, 400, 1600, 5000
  #define LIS_RANGE     2    // 2G, 4G, 8G, 16H

  uint16_t errorsAndWarnings = 0;

  uint8_t evntLis;                  // event type
  uint8_t evntBuf;                  // event buffer
  volatile uint8_t interruptEvent = 0;       // 0 for no interruptEvent

  LIS3DH lis(0x18);                 //Default address for adafruit is 0x18.

// MASKS

// XYZ
  #define INTR   0b01000000
  #define ZHIGH  0b00100000
  #define ZLOW   0b00010000
  #define YHIGH  0b00001000
  #define YLOW   0b00000100
  #define XHIGH  0b00000010
  #define XLOW   0b00000001

// TAPS
  #define XTAP    0b00000001
  #define YTAP    0b00000010
  #define ZTAP    0b00000100
  #define TAPSIGN 0b00001000
  #define SNGTAP  0b00010000   // Single Tap.
  #define DBLTAP  0b00100000   // Double Tap. Never fires, WHY?
  #define TAPINT  0b01000000
  
#endif

#if DEBUGINO == 0
  uint16_t wtimes;	    // watchdog times
  bool     overflow;    // wtimes uint16_t MAX 65535 seconds aka 18.2 hours) 43200 = 12 hours. Handle that with overflow.
  uint16_t sleepMS;     // normally 8000, maybe we can scrap this.
#endif

// Pinout for Adafruit Feather 32u4 LoRa
TinyLoRa lora = TinyLoRa(7, 8, 4);

uint32_t uptime;   // uptime in ms
uint8_t temp = 0;  // just a temp value

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

uint16_t randMS;      // add randomness in wakeup / transmission time.

/* To have every 5 sec blink we divide the secondsSleep with 5 secs and we find the number of blinks
*  Example: 100 seconds / 5 = 20 blinks, another example: 600 seconds / 5 = 120 blinks.
*/
uint16_t blinks = secondsSleep / 8; // blink every 8 seconds (maximum of watchdog).

// after 24 hours (86.400.000 millis), we can re-send messages if we hit the wall (TTN rule)
// millis are rollover after 49 days and 17 hours
// rollover check: https://www.baldengineer.com/arduino-how-do-you-reset-millis.html
uint32_t endTXtime;

void setup(){
  Watchdog.reset();             // BUG #8 Solution. Without this strange behaviour QST: Why this is needed? // QST #2: why watchdog kicks in only on DEBUGINO == 0?

  #if DEBUGINO == 1
    Serial.begin(9600);
    while (! Serial); // don't start unless we have serial connection
  #endif
  
  #if LED > 0
    pinMode(LED_BUILTIN, OUTPUT); // Initialize pin LED_BUILTIN as an output
  #endif

  #define STARTDELAY 2 // seconds
  #if LED == 0
    delay(STARTDELAY * 1000); // wait (value in ms) before sending 1st message
  #else
    blinkLed(STARTDELAY, 250, 1000); // STARTDELAY times for Y duration and Z pause. (example is: 12, 250, 1000) aka 12 times, blink for 250 ms puse for 1 second.  
  #endif

  #if LISDH == 1
   enablePinChange();
  
   pinMode(INT_PIN_TEN, INPUT_PULLUP);            // Setup the button pin
   pinMode(INT_PIN_ELEVEN, INPUT_PULLUP);         // Setup the button pin
  
   setupLIS();
   
   // This order is important!
   setupXYZevents();  // INT_0 disable XYZ events. INT_1 enable
   enableTAP();
   readLIS();
  #endif
  
  #if GPS == 1
    pinMode(GPS_SLEEP_PIN, OUTPUT); // use GPS_SLEEP_PIN pin as output. HIGH to sleep.
    gpsPort.begin(9600);

    // ATTENTION This setting is changing the wait times for fix in GPS.ino
    // If you change here the value, you may want to change the value NO_FIX_COUNT
    // in GPS.ino 
    gps.send_P ( &gpsPort, F("PMTK220,1000") );   // update 1time in 10seconds (value in ms) EVAL problem with 3000 and periodic

    //gps.send_P ( &gpsPort, F("PMTK104") );      // Factory reset DOES NOT WORK?
    
    gps.send_P ( &gpsPort, F("PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0") ); // RMC and GCA

    bootTime = GPStime();  // Don't continue unless we have GPStime!
  #endif

  // started
  ledDEBUG(2, 500, 1000);

  // QUESTION: Why I can't use the variable from second .cpp or .ino?
  //  Serial.print("Feather");Serial.println(feather);
}
