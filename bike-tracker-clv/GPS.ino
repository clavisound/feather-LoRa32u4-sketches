// IDEA: don't send zeroed bytes, instead change port accordingly.
// IDEA #2: with port offset the variables. ex: port 80 for speeds over 90
// TODO: Send fix on heading change or time. Whichever comes first.
// TO test: Periodic (ex. if speed > 10)

#if GPS == 1

#define HDOP_LIMIT  4000                   // good quality at 4000, faster fix for greater values.

#if INDOOR == 1
  #define NO_FIX_COUNT 8                   // debug indoors with '8' led stays on with GPSsleep();
#else
  #define NO_FIX_COUNT 245                 // over 2 minutes. Some days 500 seconds are not enough!
#endif

void checkFix() {

  GPSwake();
  gps.send_P ( &gpsPort, F("PMTK225,0") );         // disable periodic. (just in-case)
  
  while ( 1 ) {                                    // run forever (until return)
    while (gps.available( Serial1 )) {             // Read only one character, so you have to call it FAST.

      fix = gps.read();

      if ( fix.satellites < 3 && fix.hdop == 0 ) {
        blinkLed(1,15,16);                                 // close MCU to save energy
        // sleepForSeconds(16,8);
          noFixCount += 15;
        } 
      if ( fix.satellites >= 3 && fix.hdop == 0 ) {
        blinkLed(1,15,3);                                 // close MCU to save energy
        // sleepForSeconds(3,1);
          noFixCount += 2;
        }

      if ( fix.hdop < HDOP_LIMIT && fix.hdop > 0 ) {    // MAX I have seen 8720 aka 8.7 DOP (NeoGPS multiplies * 1000)
                                                        // In balcony with cloudy weather 2000 to 830
  
        #if DEBUGINO == 1
          Serial.print(F("\nYES fix\n"));
        #endif

        speed = (int)fix.speed_kph();
        if ( speed == 31 ) { speed = 30; }        //
        if ( speed > 31  ) { FramePort |= 0x01; } // Set HighSpeed port.
        // checkSpeed();                          // EVAL: modify GPS rate and TAP according to speed.

          if ( checkDistance() == 1 ) {    // new location. Prepare data and send.
            #if DEBUGINO == 1
              Serial.println("* Will send GPS");
            #endif
            
            prepareGPSLoRaData();
            return;
            
          } else {                         // same location, send samelocation.
            #if LORA_VERB == 1
              loraSize = LORA_HEARTBEAT;     // don't send the GPS data, but be verbose
            #else
              loraSize = LORA_HEARTBEAT;     // don't send the GPS data. Only battery with 5% steps
            #endif

            #if DEBUGINO == 1
              printDebug();
            #endif

            return;
          }
      } else {                             // NO FIX

      #if DEBUGINO == 1
        Serial.print(F("Seeking FIX, UP secs: ")); Serial.println(uptime / 1000);
      #endif

        // Send High HDOP fix.
        // we have many efforts and bad quality HDOP. Don't waste energy with GPS seeking.
        if ( ++noFixCount >= NO_FIX_COUNT / 2 && fix.hdop >= HDOP_LIMIT ) {

          FramePort |= 0x10; // decimal: 16 - 5th bit
           
          #if DEBUGINO == 1
            Serial.print(F("High HDOP fix: GIVE UP secs: ")); Serial.println(uptime / 1000);
            displayGPS();
          #endif

         #if LED >= 2
           //ledDEBUG(5,10,100);
         #endif

          updUptime();
          prepareGPSLoRaData();

          #if LISDH == 1 | MMA8452 == 1
            checkPin();
          #endif
          transmit();
          return;

        } else if ( noFixCount <= NO_FIX_COUNT ) {           // Continue for number of failures (on update 1Hz every second)
          
          #if DEBUGINO == 1
            Serial.print("No fix: "); Serial.println(noFixCount);
            displayGPS();
          #endif

         #if LED >= 2
//           ledDEBUG(1,1,0);
         #endif

          // checkPin(); // yes pin, works
          
        } else if ( noFixCount > NO_FIX_COUNT ) {            // Give UP
          
          #if DEBUGINO == 1
            Serial.print(F("No fix: GIVE UP secs: ")); Serial.println(uptime / 1000);
            displayGPS();
          #endif

            updUptime();

            #if LORA_VERB == 1
              loraSize = LORA_HEARTBEAT;
            #else
              loraSize = LORA_HEARTBEAT; // send only battery, but with 5bits (5% steps)
            #endif

            #if LISDH == 1 | MMA8452 == 1
              checkPin();
            #endif
            transmit();                        // Small BUG: normally we should call checkTXms to check for new day or TTN limits.
            return;
        }
      } // no fix
    } // while gps.avail
  } // while 1
} // checkFix

// DEGREE, Diff, CosFis DistanceBetween from http://www.technoblogy.com/show?10WT
const long DEGREE = 600000;

long Diff (long deg1, long deg2) {
  long result = deg2 - deg1;
  if (result > 180 * DEGREE) return result - 360 * DEGREE;
  else if (result < -180 * DEGREE) return result + 360 * DEGREE;
  else return result;
}

unsigned int CosFix (long angle) {
  unsigned long u = labs(angle) >> 16;
  u = (u * u * 6086) >> 24;
  return 246 - u;
}

unsigned int DistanceBetween (long lat1, long long1, long lat2, long long2) {
  long dx = (Diff(long2, long1) * CosFix((lat1 + lat2) / 2)) / 256;
  long dy = Diff(lat2, lat1);
  unsigned long adx = labs(dx);
  unsigned long ady = labs(dy);
  unsigned long b = max(adx, ady);
  unsigned long a = min(adx, ady);
  if (b == 0) return 0;
  return 95 * (b + (110 * a / b * a + 128) / 256) / 512;
}

#if DEBUGINO == 1
void displayGPS() {
  Serial.print(F("Year: ")); Serial.print( fix.dateTime.year );
  Serial.print(F(" Month: ")); Serial.print( fix.dateTime.month );
  Serial.print(F(" Hour: ")); Serial.print( fix.dateTime.hours );
  Serial.print(F(" minutes: ")); Serial.print( fix.dateTime.minutes );
  Serial.print(F(" Seconds: ")); Serial.print( fix.dateTime.seconds );
  Serial.print(F("\nGPS time (s): ")); Serial.print( fix.dateTime );
  Serial.print(F("\nHDOP: ")); Serial.print( fix.hdop ); Serial.print(F(", latitude: ")); Serial.print( fix.latitude(), 6 ); Serial.print(F(", longitude: ")); Serial.println( fix.longitude(), 6 );
  //Serial.print(F("lat / lon: "));Serial.print( lat ); Serial.print(F(" "));Serial.println( lon );
  Serial.print(F("alt: ")); Serial.print( fix.alt.whole ); Serial.print(F(", sats: ")); Serial.print( fix.satellites );Serial.print(F(", head: ")); Serial.print( fix.heading_cd() / 100 );
  //Serial.print(F(", speed: ")); Serial.print(speed);
  Serial.print(F(", LoRa HDOP: ")); Serial.println(loraData[11]);
}
#endif

// commands
// gps.send_P ( &gpsPort, F("PMTK000") );      // test (to wake up)
// gps.send_P ( &gpsPort, F("PMTK102") );      // Ephemeris delete
// gps.send_P ( &gpsPort, F("PMTK103") );      // Ephemeris, almanac, time, Position, delete
// gps.send_P ( &gpsPort, F("PMTK104") );      // Factory reset
// gps.send_P ( &gpsPort, F("PMTK161,0") );    // Standby (wake with any byte)
//                          $PMTK161,1         // "Sleep" (7mA)
// gps.send_P ( &gpsPort, F("PMTK220,3000") ); // update 1time in 3s (value in ms)
// gps.send_P ( &gpsPort, F("PMTK869,1") );    // EASY enabled (guick fix with 1Hz update

// Periodic Mode:
// gps.send_P ( &gpsPort, F("PMTK225,2,120000,43200000,240000,4320000") );
// gps.send_P ( &gpsPort, F("PMTK225,2,3000,30000,6000,60000") ); // EVAL;
// gps.send_P ( &gpsPort, F("PMTK225,4") ); // EVAL Perpetual
// PMTK225,1,3000,12000,18000,72000 = periodic mode with 3s in tracking mode and 12s in backup mode based on GPS&GLONASS.
// The average current consumption is calculated as below:I periodic = (I tracking×T1+I backup×T2)/ (T1+T2)=(26mA×3s + 0.007mA×12s)/(3s+12s)≈5.2(mA)
// Parameters
// MODE              : 0 disable auto sleep 1 backup 2 standby 4 perpetual 8 AlwaysLocateTM standby 9 AlwaysLocateTM backup.
// Try for FIX for ms: 0 disable 1000-518400000 (144 hours!) (120000 = two minutes)
// Sleep time        : see try for FIX 43200000 = 12hours
// Second run        : (larger than first time)
// Second sleep      :
// gps.send_P ( &gpsPort, F("PMTK225,0");

void tristateSerialB(){ // FAILED!
    pinMode(0, INPUT);        // RX pin (2.5mA)
    digitalWrite (0, LOW);    //

    pinMode(1, INPUT);        // TX pin. Go to INPUT_PULLUP
    digitalWrite (1, LOW);    //
    
}

// ATTENTION don't call this twice, or the GPS will wake! GPS wakes with *ANY* command
void GPSsleep() {
  /* *SOMETIMES* the light stays ON after power OFF.
     The right procedure is here (NOT IMPLEMENTED)
    // a) Poll for led ON (is hardwired to FIX pin)
    // b) Wait some 600ms??
    // c) Shutdown GPS
  */

  #if DEBUGINO == 1
    Serial.print(F("\n* GPS OFF\n"));
  #endif

  #if GPS_SLEEP_PIN_EN == 1
    digitalWrite(GPS_SLEEP_PIN, HIGH);         // Power OFF GPS.
    //tristateSerialB();
  #else

  #if GPS_TRANSISTOR_PIN == 0
    delay(900);                                  // Hack for the light for first run.
  #endif
    
  gps.send_P ( &gpsPort, F("PMTK161,0") );       // sleep (wake with any byte)
  // +1.5mA??
  // gps.send_P ( &gpsPort, F("PMTK161,1") );    // standby(wake with any byte) (HANG on wake up)

  // We are in #else
  #if GPS_TRANSISTOR_PIN == 1             // First thing to do, power up GPS
    pinMode(PNP_GPS_PIN, OUTPUT);          // Initialize pin LED_BUILTIN as an output
    digitalWrite(PNP_GPS_PIN, HIGH);       // Power off GPS
    #if DEBUGINO == 1
      Serial.println("\n PNP off");
    #endif
  #endif


   #if TRISTATE == 1
     //* FAILED (?) effort to Tristate
     // delay(1000);
     gpsPort.end();             // Close UART
     delay(1000);
   //tristateSerialB();
   #endif // TRISTATE
  //gps.send_P ( &gpsPort, F("PMTK225,9") );     // Periodic Mode, Always Locate (standby) 8 for standby, 9 for backup.
  #endif // GPS_SLEEP_PIN_EN (else)
  
  #if LED == 3
    // ledDEBUG(1, 1900, 100);
  #else
    sleepForSeconds(1,2);                       // MTK3339 needs some time. (2000 was fine, 100, 300 hangup)
  #endif
}

void GPSwake() {
  #if DEBUGINO == 1
   Serial.print(F("\n* GPS ON\n"));
   delay(200);
  #endif

  #if GPS_TRANSISTOR_PIN == 1             // First thing to do, power up GPS
   pinMode(PNP_GPS_PIN, OUTPUT);    // Initialize pin LED_BUILTIN as an output
   digitalWrite(PNP_GPS_PIN, LOW);  // Power on GPS
   #if DEBUGINO == 1
     Serial.println("\n PNP on");
   #endif
   delay(200);
 #endif

  #if GPS_SLEEP_PIN_EN == 1
    digitalWrite(GPS_SLEEP_PIN, LOW);             // Power ON GPS.
    #if TRISTATE == 1
      gpsPort.begin(9600);                          // Open UART
    #endif // TRISTATE
  #else // GPS_SLEEP_PIN_EN != 0
    #if TRISTATE == 1
      gpsPort.begin(9600);                          // Open UART
      delay(1000);                                  // Wait UART to open
  #endif // GPS_SLEEP_PIN_EN
    gps.send_P ( &gpsPort, F("PMTK000") );       // module wakes up with ANY communication
    delay(200);
  //gps.send_P ( &gpsPort, F("PMTK225,0") );     // disable Periodic.
  delay(200);
  #endif // GPS_SLEEP_PIN_EN
}

void GPSfastRate() {
  // we moving, GPS always ON with less power
  gps.send_P ( &gpsPort, F("PMTK220,1000") );  //update 1time in 10seconds (value in ms)
}

void GPSslowRate() {
  // we are going slow
  // gps.send_P ( &gpsPort, F("PMTK225,8") ); // Periodic Mode.
  gps.send_P ( &gpsPort, F("PMTK220,3000") );  //update 1time in 3seconds (value in ms)
}

void checkSpeed() {
  #if MMA8452 == 1 | LISDH == 1
    if ( speed < 5 ) {                // close GPS + TODO sensitive tap
      #if DEBUGINO == 1
       Serial.println(F("* checkSpeed0"));
      #endif
      //GPSsleep(); // enablePinChange(); setupLIS();
    } else {
      GPSwake(); //disablePinChange();   // open GPS + TODO in-sensitive tap
    }
  #endif
  
  // EVAL there is benefit?
  //if ( speed > 19 ) GPSfastRate();             // Attention with the order.
  //if ( speed < 5  ) GPSslowRate();             // It creates problems. No GPS position.
}

void prepareGPSLoRaData(){
  #if DEBUGINO == 1
    Serial.println("* prepareGPSLoRa");
  #endif

          // https://github.com/ricaun/esp32-ttnmapper-gps/blob/8d37aa60e96707303ae07ca30366d2982e15b286/esp32-ttnmapper-gps/lmic_Payload.ino#L21
          // accuracy till 5-6 decimal
          lat = ((fix.latitude() + 90) / 180) * 16777215;
          lon = ((fix.longitude() + 180) / 360) * 16777215;

          #if LORA_VERB == 1
            loraSize = 17;
            
            loraData[5]  = lat >> 16;      // MSB
            loraData[6]  = lat >> 8;
            loraData[7]  = lat;            // LSB

            loraData[8]  = lon >> 16;
            loraData[9]  = lon >> 8;
            loraData[10] = lon;

          if ( fix.hdop >= 255000 ) {      // If DOP is more than 255, store 25.5
            loraData[11] = 255;            // I don't want to handle DOP in two bytes. MAX DOP stored is 255000.
          } else {                         // NeoGPS makes float to integer * 1000. Examples: 8720 (NeoGPS) = 8.72 DOP, 1960 (NeoGPS) = 1.96 DOP
            loraData[11] = fix.hdop / 100; // I divide with 100 to make 8720 = 87.20 HDOP stored as 87. Re-divided in TTN with 10,
            // So... I can fit a 255000 value (DOP 255) in one byte.
            // So... value of 830 (DOP 0.83) TXmitted as 8 and in decoder divided as 0.8
            // EVAL: optimization with divide by two and use less bits?
          }
            loraData[12] = (fix.satellites - 3) & 0x00FF;   // TODOTTN: min sats are 3. 0000 = 3, 0010 = 5 e.t.c. [we need 4 bits for 18 sats]
            loraData[12] |= (fix.heading_cd() / 2400) << 4; // TODOTTN: NeoGPS multiplies by 100. We divide by 24 to fit in one nibble.
     // OLD loraData[16] = fix.heading_cd() / 200;          // NeoGPS multiplies by 100. We divide by two to fit in one byte (divide by 6 fits in 6 bits)
            loraData[13] = speed;
            
           if ( fix.alt.whole < 0 ) {       // somegps may report minus altitude. Handle that.
              loraData[14] = 0;
            } else {
              loraData[14] = fix.alt.whole / 25;           // TODOTTN: divide by 25. Alternative: divide by 50. Everest is 8846m / 50 = 178, Olympus: 2900m = 58 [6bits] 750m [4bits]
            }
          #else // LORA_VERB == 0
          if ( ttncounter < 10 || fix.hdop >= TTNMAPPING ) {    // wait 10 times to send to TTNmapper. Or if bad HDOP, again don't send to TTNmapper
            ttncounter++;
            loraSize = 7;
            
            loraData[0] = lat >> 16;      // MSB
            loraData[1] = lat >> 8;
            loraData[2] = lat;            // LSB

            loraData[3] = lon >> 16;
            loraData[4] = lon >> 8;
            loraData[5] = lon;
            
            loraData[6] = fix.heading_cd() / 2400;       // TODOTTN: NeoGPS multiplies by 100. We divide by 24 to fit in one nibble.
            loraData[6] |= ( speed / 2 ) << 4;           // TODOTTN: divide by 2 to have max speed of 30 with 4 bits with 2km/h steps.

            oldLat = fix.latitude()  * 10E5;             // store lat / lon to compare with next fix.
            oldLon = fix.longitude() * 10E5;
          }
          #if TTNMAPPING > 0
          else if ( ttncounter > 9 && fix.hdop < TTNMAPPING ) { // ttnmapping!
            loraSize = 9;
            ttncounter = 0;
            loraData[7] = fix.hdop / 200;    // TODOTTN: I divide with 200 to make 2999 = 14.995 HDOP stored as 14. Re-divided in TTN with 5 = 2.8. Fits in nibble.
            if ( fix.alt.whole < 0 ) {       // somegps may report minus altitude. Handle that.
              loraData[8] = 0;               // erase left nibble.
            } else {
              loraData[8] = fix.alt.whole / 25;           // TODOTTN: divide by 25. Alternative: divide by 50. Everest is 8846m / 50 = 178, Olympus: 2900m = 58 [6bits] 750m [4bits]
            }
              loraData[7] |= (fix.satellites - 3) << 4;  // TODOTTN: min sats are 3. 0000 = 3, 0010 = 5 e.t.c. [we need 4 bits for 18 sats]            
            }
            #endif
          #endif
          
          #if DEBUGINO == 1
            displayGPS();
          #endif
          
          // return; // EVAL comment
}

uint8_t checkDistance(){
    GPS_old_time = fix.dateTime;
       #if DEBUGINO == 1
          // EVAL Probably 400meters off in distance of 4000meters. So, for every 10m we have 1meter of mistake?
          // Serial.print( F(" From WT (m): ") ); // WT == White Tower of Thessalonikiw
          // Serial.println( DistanceBetween(40.62635955 * 10E5, 22.948298 * 10E5, fix.latitude() * 10E5, fix.longitude() * 10E5 ));

          Serial.print( F("* Distance from Previous send: ") );
          Serial.println( DistanceBetween(oldLat, oldLon, fix.latitude() * 10E5, fix.longitude() * 10E5 ));
          Serial.println( oldLat);Serial.println(oldLon);Serial.println(fix.latitude() * 10E5);Serial.println(fix.longitude() * 10E5 );
       #endif

     if ( DistanceBetween(oldLat, oldLon, fix.latitude() * 10E5, fix.longitude() * 10E5 ) >= SAMEDISTANCE ) { // distance more than 49 meters
      return 1;
     } else {
      return 0;
     }
}

uint32_t GPStime() {
  #if DEBUGINO == 1
    Serial.print(F("\n* GPStime\n"));
  #endif
  
  GPSwake();
  
  // Boot time of GPS is 2080 aka 2.525.283.747 seconds and more... for 2020 May uptime is more than 642.022.998)
  while (1) {                                 // try forever...
    while (gps.available( Serial1 )) {        // Read only one character, so you have to call it FAST.
      fix = gps.read();
      noFixCount++;
      
      if ( fix.valid.location ) {               // ... unless we have location. Then we have time.
        
        noFixCount++;
        return fix.dateTime;
        
      } else if ( fix.dateTime < 2432123456 ) {  // ... unless date is before 2080. THIS IS HACK. Problem for 2080 and onwards! Device / library specific.
            
            #if DEBUGINO == 1
              Serial.print("time: "); Serial.println(fix.dateTime);
              Serial.print("millis (s): "); Serial.println(millis() / 1000);
            #endif

        return fix.dateTime;
        
        #if INDOOR == 1
          } else if ( fix.dateTime > 2432123456 ) {  // DEBUG Indoors
            
            #if DEBUGINO == 1
              Serial.print("time: "); Serial.println(fix.dateTime);
              Serial.print("millis (s): "); Serial.println(millis() / 1000);
            #endif

          return fix.dateTime;
        #endif     
      }
    }
  }
}

#endif // GPS == 1
