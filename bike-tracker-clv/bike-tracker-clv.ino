// Bike tracker. Made by clavisound. 2020
// #define EU863 // BUG: TinyLoRa.h ignores this. Modify TinyLoRa.h else default is: US902. Other options: EU863, AU915, AS920

// LoRa and LoRaWAN options
// ** BE CAREFUL TTN SUGGESTS MINUTES BETWEEN TRANSMISSIONS! **
#define SECONDS_SLEEP 32  // Send every secs / mins: MAX 65535''/18hours, 7200''/ 2hours', 4200''/ 90', 3600''/ 1hour', 1800''/ 30', 1200''/ 20', 600''/ 10', 300''/ 5', 180''/3'
// Take care with SF11-SF12! https://lora-alliance.org/sites/default/files/2018-11/Oct122018_NetID_Alloc_Policy_Application_V3.pdf
// "network providers (such as TTN) are required to actively block devices that always send on SF11 or SF12, to keep their LoRa Alliance NetID."
#define SF            7     // [default 10] SF7BW125 to SF10BW125. Use 11-12 only for testing, if you are away from gateway. They are forbidden from TTN.
#define SFB           8      // [default 9] 7 to 10. Use 11-12 only for testing, if you are away from gateway. They are forbitten from TTN.
#define POWER         14     // valid values -80, 0-20. For EU limit is 14dBm, for US +20, but PAY ATTENTION TO THE ANTENNA if +20dBm: You need 1% duty cycle and VWSR ??
#define FRAMECOUNTER  0      // framecounter. We need this variable if we sleep LoRa module forgets everything. TODO store in EEPROM
#define TWOSF         1      // [default 1]. 0 to send only in defined SF, 1 to send also in SFB when time is odd (semi-random).
#define TTNMAPPING    0   // [default 0, proposed 3000 (HDOP: 3) if you want ttn mapping every 10 messages and IF we have good GPS signal. Additional data to sent: HDOP, sats, altitude

// FEATHER behaviour
#define LED           1      // [default 0] 0 = no led. 1=led for BOOT, TX, ABORT (not IDLE) [+94 bytes program] 2=led for BOOT, (not TX), ABORT, IDLE [+50 bytes program] 3 = ledDEBUG [default: 2]
#define CHAOS         1      // [default 1] 1 = use some 'random' numbers to generate 'chaos' in delay between TX's. +356 program bytes, +3 bytes RAM; [default 1]
#define CYCLESF       0      // [default 0] 0 = don't cycleSF, 1 = cycle SF10 to SF8, 2 = send only once per day [default 0 or 3] 3 = from SF7 to SF10, 4 = from SF10 to SF12
#define STARTDELAY    2      // [default 2] Boot delay seconds.

// DEVICE SELECTION
#define GPS                1    // [default 1] 0 to use with your smartphone + ttnmapper app. 1 = For adafruit GPS ultimate featherwing
#define MMA8452            1    // [default 1] 0 to disable code for MMA8452 accelerator, 1 to enable.
#define LISDH              0    // [default 0] 1 for LIS3DH  accelerator, 0 for no. Adafruit is not suitable for low power unless you de-solder some stuff. +0.7mA in sleeping. But it's not a bad choice if you don't care about the battery life.
#define GPS_SLEEP_PIN_EN   0    // [default 0] If `1' connect A4 (feather) to EN pin (Ultimate GPS)
#define GPS_TRANSISTOR_PIN 1    // [default 0] 1 to enable 'transistor' code to bypass EN pin. Using 270R resistor (base) with a PNP transistor. Collector connected to feather 3V3.
                            
//#define BUZZER      1     // TODO [default 0] 1 to hear some beeps!

// DEBUG options
#define DEBUGINO      0     // [default 0] 1 = for debugging via serial. Sleep is OFF! 0 to save some ram and to enable sleep. +3904 bytes of program, +200 bytes of RAM. [default 0]
#define INDOOR        0     // [default 0] For DEBUG INDOORs (disables slow seeking)
#define PHONEY        0     // [default 0] 1 = don't TX via Radio LoRa (aka RF) but calculates some phoney TX time. (useful for debugging) [default 0]
#define LORA_VERB     0     // [default 0] 1 to send verbose (DEBUG) messages via LoRa.
#define SAMEDISTANCE  50     // [default 50] Don't send GPS data if the distance is less than 50meters
//#define TRISTATE  0       // Ignore. Failed experiment for tristate. More Info: https://forums.adafruit.com/viewtopic.php?p=497713#p497708

// Data Packet to Send to TTN
#if LORA_VERB == 1
  #define LORA_HEARTBEAT    6             // Data bytes (LoRa bytes = 20)
#else
  #define LORA_HEARTBEAT    1             // Data bytes (LoRa bytes = 18)
#endif

uint32_t secondsSleep = SECONDS_SLEEP;

#include <TinyLoRa.h>
int8_t  txPower = POWER;
uint16_t fc = FRAMECOUNTER;

#include <SPI.h>
#include <Adafruit_SleepyDog.h>

#if GPS == 1
  #define DAY  86400     // = day in seconds. Use to comply with TTN policy [DEBUG: 7200 for 2 hours]
#else
  #define DAY  86400000  // = day in ms. (you are using millis instead of GPS time)
#endif
#define TXMS 30000       // = TTN limit 30.000ms (30 seconds) per day.
//#define TXMS 1000      // DEBUG

#define VBATPIN A9       // pin to measure battery voltage

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
  uint8_t heading;     
  //uint8_t  oldHeading;  
  uint8_t  hdop;                                      // > 20 = 20 x 5 meters = 100m
  uint8_t  sats;                                      // satellites
  uint8_t  noFixCount;                                // in cloudy balcony fix after 70 seconds. But another day 500 seconds and counting!

  // #if TTNMAPPING > 0
      uint8_t ttncounter;
  // #endif

  #if LORA_VERB == 1
    uint16_t fixes;
  #endif
  
  uint32_t bootTime;
  uint32_t uptimeGPS;
  uint32_t GPSnow, GPS_old_time;
  uint32_t lastTXtime = 86400;                        // have to set this, otherwise first TX fails
  uint8_t FramePort = 0;                              // port with GPS data
  uint8_t loraData[17] = {};                          // make room for bytes to send
  uint8_t loraSize = 7;                               // but we only need 7
#else
  uint8_t FramePort = 0;
  uint8_t loraData[LORA_HEARTBEAT] = {};              // bytes to send.
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
  volatile uint8_t interruptEvent = 0;       // 0 for no interruptEvent
  uint8_t orientation;                       // 0 (right fall), 1 (left fall), 2 (normal), 3 (upside down), 64 (endo or whellie)
  uint8_t tap;                               // 16 tap on X, 24 double? tap X, 17 tap Y, 25 double? tap Y,
                                             // 64 tap from behind?
                                             // 68 ?
                                             // 32 tap
 uint8_t intWake;

  MMA8452Q accel;                   // create instance of the MMA8452 class
#endif

#if LISDH == 1

  #include "lis3dh-motion-detection.h"

  #include <avr/sleep.h>
  #include <avr/power.h>

  #define INT_PIN_TEN       10
  #define INT_PIN_ELEVEN    11

  #define LIS_RATE     10    // HZ - Samples per second - 1, 10, 25, 50, 100, 200, 400, 1600, 5000
  #define LIS_RANGE     2    // 2G, 4G, 8G, 16H

  uint16_t errorsAndWarnings = 0;

  uint8_t evntLis;                           // event type
  uint8_t evntBuf;                           // event buffer
  volatile uint8_t interruptEvent = 0;       // 0 for no interruptEvent

  LIS3DH lis(0x18);                          //Default address for adafruit is 0x18.

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
  uint16_t wtimes, times;	    // watchdog times
  bool     overflow;          // TODO wtimes uint16_t MAX 65535 seconds aka 18.2 hours 43200 = 12 hours. Handle that with overflow.
  uint16_t sleepMS;           // normally 8000, maybe we can scrap this.
#endif

// Pinout for Adafruit Feather 32u4 LoRa
TinyLoRa lora = TinyLoRa(7, 8, 4);

uint32_t uptime;   // uptime in ms
uint8_t temp = 0;  // just a temp variable

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
uint32_t endTXtime =  86400;        // to send first message.

void setup(){

  // to gain -2mA ONLY during first fix. Comment both lines if you don't bother to install my custom TinyLoRa library.
  setupLora();
  lora.sleep();
  
  Watchdog.reset();             // BUG #8 Solution. Without this strange behaviour.
                                // QST #1: Why this is needed?

  #if DEBUGINO == 1
    Serial.begin(9600);
    while (! Serial); // don't start unless we have serial connection
  #endif

  #if GPS_TRANSISTOR_PIN == 1             // First thing to do, power up GPS
    #define PNP_GPS_PIN A4
    pinMode(PNP_GPS_PIN, OUTPUT);    // Initialize pin LED_BUILTIN as an output
    #if GPS == 1
      digitalWrite(PNP_GPS_PIN, LOW);     // Power on GPS
      #else
      digitalWrite(PNP_GPS_PIN, HIGH);    // Power off GPS
    #endif
    #if DEBUGINO == 1
      Serial.println("\n PNP on");
    #endif
  #endif
  
  #if LED > 0
    pinMode(LED_BUILTIN, OUTPUT); // Initialize pin LED_BUILTIN as an output
  #endif

  #if LED == 0
    delay(STARTDELAY * 1000);        // wait (value in ms)
  #else
    ledDEBUG(STARTDELAY, 250, 1000);
  #endif

  #if LISDH == 1
   enablePinChange();
   pinMode(INT_PIN_ELEVEN, INPUT);                  // Setup the LIS INT
   setupLIS();
   
   // This order is important!
   setupXYZevents();  // INT_0 disable XYZ events. INT_1 enable
   enableTAP();
   readLIS();
  #endif

  #if MMA8452 == 1
    enablePinChange();
    setupMMA();
    readMMA();
  #endif
  
  #if GPS == 1
    pinMode(GPS_SLEEP_PIN, OUTPUT); // use GPS_SLEEP_PIN pin as output. HIGH to sleep.
    // digitalWrite(GPS_SLEEP_PIN, LOW);
    gpsPort.begin(9600);

    // EVAL for 4800
    #if TRISTATE == 0 // If trying with tristate, modify there the baudrate.
    // gps.send_P ( &gpsPort, F("$PMTK251,4800")); 4800 bps
    // gpsPort.begin(4800); //
    #endif
    
    // ATTENTION This setting (PMTK220) is changing the wait times for fix in GPS.ino
    // If you change here the value, you may want to change the value NO_FIX_COUNT
    // in GPS.ino 
    gps.send_P ( &gpsPort, F("PMTK220,1000") );   // update 1time in 10seconds (value in ms) EVAL problem with 3000 and periodic
    //gps.send_P ( &gpsPort, F("PMTK104") );       // Factory reset. Forgets almanac
    gps.send_P ( &gpsPort, F("PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0") ); // RMC and GGA

    #if TRISTATE == 1
      Serial1.end();          // close UART, because GPSwake hangs when UART is already open.
      delay(3000);
    #endif
 
    bootTime = GPStime();  // Don't continue unless we have GPStime!
  #endif

  // Booted
  // ledDEBUG(2, 500, 2000);

  // QUESTION: Why I can't use the variable from second .cpp or .ino?
  //  Serial.print("Feather");Serial.println(feather);
}
