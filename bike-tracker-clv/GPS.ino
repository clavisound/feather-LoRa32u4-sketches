// Freaky IDEA: don't send zeroed bytes, instead change port accordingly.
// Freaky IDEA #2: with port offset the variables. ex: port 80 for speeds over 90
// TODO: Send fix on heading change or time. Whichever comes first.

#if GPS == 1

#define HDOP_LIMIT  4000
#define NO_FIX_COUNT 120

void checkFix() {

  #if LISDH == 1 | MMA8452 == 1
     // disablePinChange();
     // disableTAP();
  #endif
   
   GPSwake();
  
  while ( 1 ) {                                    // run forever (until return)
    while (gps.available( Serial1 )) {             // Read only one character, so you have to call it FAST.
      fix = gps.read();
      //if (fix.valid.location ) {                     // EVAL better if fix.hdop < 8000?
      if (fix.hdop < HDOP_LIMIT && fix.hdop > 0 ) {    // MAX I have seen 8720 akai 8.7 DOP (NeoGPS multiplies * 1000)
        // fixCount++;                                 // In balcony with cloudy weather 2000 to 830
        // if ( ++fixCount >= 1 ) {
        
        #if DEBUGINO == 1
          Serial.println(F("YES fix"));
        #endif

        speed = (int)fix.speed_kph(); checkSpeed(); // modify GPS rate and TAP according to speed.

          if ( checkDistance() == 1 ) {    // new location. Prepare data and send.
            prepareGPSLoRaData();
          } else {                         // same location, send samelocation.
            FramePort = FRAME_PORT_NO_GPS; // TODO: PORT for same location
            loraSize = LORA_HEARTBEAT;     // don't send the GPS data.
            #if DEBUGINO == 1
              printDebug();
            #endif
            checkTXms();
          }
        // } // end if fixcount
      
      } else {                             // NO FIX

      #if DEBUGINO == 1
        Serial.print(F("Seeking FIX, UP secs: ")); Serial.println(uptime / 1000);
      #endif

        // we have many efforts and bad quality HDOP. Don't waste energy with GPS seeking.
        // Send the message with LOW HDOP
        if ( ++noFixCount >= 20 && fix.hdop >= HDOP_LIMIT ) {
          
          #if DEBUGINO == 1
            Serial.print(F("High HDOP fix: GIVE UP secs: ")); Serial.println(uptime / 1000);
            displayGPS();
          #endif
          
          prepareGPSLoRaData();

          #if MMA8452 == 1 | LISDH == 1                      // we lost gps enable TAP to be safe.
            // setupLIS();
          #endif
        } else if ( noFixCount <= NO_FIX_COUNT ) { // Continue for number of failures (on update 1Hz every second)
          
          #if DEBUGINO == 1
            Serial.print("No fix: "); Serial.println(noFixCount);
            displayGPS();
          #endif
          
        } else if ( noFixCount > NO_FIX_COUNT ) { // Give UP
          #if DEBUGINO == 1
            Serial.print(F("No fix: GIVE UP secs: ")); Serial.println(uptime / 1000);
            displayGPS();
          #endif

          #if MMA8452 == 1 | LISDH == 1
             //setupLIS();
             GPSsleep();
          #endif
            FramePort = FRAME_PORT_NO_GPS;
            loraSize = LORA_HEARTBEAT;

            noFixCount = 0; fixCount = 0;            // reset the counters for gps fixes
            checkTXms();

            #if MMA8452 == 1 | LISDH == 1                      // we lost gps enable TAP to be safe.
             // setupLIS();
            #endif
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
  Serial.print(F("\nGPS uptime (s): ")); Serial.println( fix.dateTime );
  Serial.print(F("HDOP: ")); Serial.print( fix.hdop ); Serial.print(F(", latitude: ")); Serial.print( fix.latitude(), 6 ); Serial.print(F(", longitude: ")); Serial.println( fix.longitude(), 6 );
  //Serial.print(F("lat / lon: "));Serial.print( lat ); Serial.print(F(" "));Serial.println( lon );
  Serial.print(F("alt: ")); Serial.print( fix.alt.whole ); Serial.print(F(", sats: ")); Serial.println( fix.satellites );
  Serial.print(F("speed: ")); Serial.println(speed);
  Serial.print(F("LoRa HDOP: ")); Serial.println(loraData[11]);
}
#endif

// commands
// gps.send_P ( &gpsPort, F("PMTK000") );      // test (to wake up)
// gps.send_P ( &gpsPort, F("PMTK102") );      // Ephemeris delete
// gps.send_P ( &gpsPort, F("PMTK103") );      // Ephemeris, almanac, time, Position, delete
// gps.send_P ( &gpsPort, F("PMTK104") );      // Factory reset
// gps.send_P ( &gpsPort, F("PMTK161,0") );    // Standby (wake with any byte)
// gps.send_P ( &gpsPort, F("PMTK220,3000") ); // update 1time in 10s (value in ms)
// gps.send_P ( &gpsPort, F("PMTK869,1") );    // EASY enabled (guick fix with 1Hz update

// gps.send_P ( &gpsPort, F("PMTK225,2,120000,43200000,240000,4320000") ); // Periodic Mode:
// Parameters
// MODE              : 0 disable auto sleep 1 backup 2 standby 4 perpetual 8 AlwaysLocateTM standby 9 AlwaysLocateTM backup.
// Try for FIX for ms: 0 disable 1000-518400000 (144 hours!) (120000 = two minutes)
// Sleep time        : see try for FIX 43200000 = 12hours
// Second run        : (larger than first time)
// Second sleep      :
// gps.send_P ( &gpsPort, F("PMTK225,0");

void GPSsleep() {
  /* *SOMETIMES* the light stays ON after power OFF.
     The right procedure is here (NOT IMPLEMENTED)
    // a) Poll for led ON (is hardwired to FIX pin)
    // b) Wait some ?? time
    // c) Shutdown GPS
  */

#if DEBUGINO == 1
    Serial.print(F("GPS OFF\n"));
#endif

  delay(2000);                                 // MTK3339 needs some time.
  //digitalWrite(GPS_SLEEP, HIGH);             // Power OFF GPS.
  gps.send_P ( &gpsPort, F("PMTK161,0") );     // sleep (wake with any byte)
  //gps.send_P ( &gpsPort, F("PMTK225,9") );   // Periodic Mode, Always Locate (standby) 8 for standby, 9 for backup.
}

void GPSwake() {
#if DEBUGINO == 1
  Serial.print(F("GPS ON\n"));
#endif

  delay(2000);
  // digitalWrite(GPS_SLEEP, LOW);             // Power ON GPS.
  gps.send_P ( &gpsPort, F("PMTK000") );       // module wakes up with ANY communication
  //gps.send_P ( &gpsPort, F("PMTK225,0") );   // disable Periodic.
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
  return;
  #if MMA8452 == 1 | LISDH == 1
    if ( speed == 0 ) {                // close GPS + TODO sensitive tap
      GPSsleep(); // enablePinChange(); setupLIS();
    } else {
      GPSwake(); //disablePinChange();   // open GPS + TODO in-sensitive tap
    }
  #endif
  
  // EVAL there is benefit?
  //if ( speed > 19 ) GPSfastRate();             // Attention with the order.
  //if ( speed < 5  ) GPSslowRate();             // It creates problems. No GPS position.
}

void prepareGPSLoRaData(){
          FramePort = FRAME_PORT_GPS; // We have GPS data, choose decoder with GPS.
          noFixCount = 0; fixCount = 0;            // reset the counters for gps fixes
          loraSize = LORA_TTNMAPPER;

          // https://github.com/ricaun/esp32-ttnmapper-gps/blob/8d37aa60e96707303ae07ca30366d2982e15b286/esp32-ttnmapper-gps/lmic_Payload.ino#L21
          // accuracy till 5-6 decimal
          lat = ((fix.latitude() + 90) / 180) * 16777215;
          lon = ((fix.longitude() + 180) / 360) * 16777215;

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

          if ( fix.alt.whole < 0 ) {       // somegps may report minus altitude. Handle that.
            loraData[14] = 0;
            loraData[15] = 0;
          } else {
            loraData[14] = fix.alt.whole >> 8;   // MSB altitude 16bits
            loraData[15] = fix.alt.whole;        // TODO, we don't need those two bytes if we have < 255 metres.
          }                                      // ^^ divide by 3 so in 8bits we have max alt of 765m
          loraData[12] = fix.satellites;         // TODO: count bits, or maybe + 1 / divide with 2. Aka for 4 bits (max 16 sats)
          loraData[13] = speed;                  // TODO: divide by 3 to have max speed of 90 with 5 bits
          loraData[16] = fix.heading_cd() / 2;   // TODO: divide by 6 to fit in 6bits

          oldLat = fix.latitude() * 10E5;        // store lat / lon to compare with next fix.
          oldLon = fix.longitude() * 10E5;

          //GPSsleep();                            // Close GPS we are done
          //gps.send_P ( &gpsPort, F("PMTK225,8") ); // Periodic Mode.
          #if DEBUGINO == 1
            displayGPS();
          #endif
          
          checkTXms();                             // Transmit if we are between TTN limits
}

uint8_t checkDistance(){
       #if DEBUGINO == 1
          // EVAL Probably 400meters off in distance of 4000meters. So, for every 10m we have 1meter of mistake?
          // Serial.print( F(" From WT (m): ") );
          // Serial.println( DistanceBetween(40.62635955 * 10E5, 22.948298 * 10E5, fix.latitude() * 10E5, fix.longitude() * 10E5 ));

          Serial.print( F("Distance from Previous send: ") );
          Serial.println( DistanceBetween(oldLat, oldLon, fix.latitude() * 10E5, fix.longitude() * 10E5 ));
          Serial.println( oldLat);Serial.println(oldLon);Serial.println(fix.latitude() * 10E5);Serial.println(fix.longitude() * 10E5 );
       #endif
  
     if ( DistanceBetween(oldLat, oldLon, fix.latitude() * 10E5, fix.longitude() * 10E5 ) > 49 ) { // distance more than 49 meters
      return 1;
     } else {
      return 0;
     }
}

uint32_t GPSuptime() {
  #if DEBUGINO == 1
    Serial.println(F("GPSuptime"));
  #endif
  currentTime = GPStime() - bootTime;
}

uint32_t GPStime() {
  #if DEBUGINO == 1
    Serial.print(F("GPStime\n"));
  #endif
  GPSwake();
  
  // Boot time of GPS is 2080 aka 2.525.283.747 seconds and more... for 2020 May uptime is more than 642.022.998)
  while (1) {                                 // try forever...
    while (gps.available( Serial1 )) {        // Read only one character, so you have to call it FAST.
      fix = gps.read();

      if (fix.valid.location ) {               // ... unless we have location. Then we have time.
        GPSsleep();
        return fix.dateTime;
      } else if ( fix.dateTime < 2432123456 ) {  // ... unless date is before 2080. THIS IS HACK small problem for 2080 and onwards! Device / library specific.
      //} else if ( fix.dateTime > 2432123456 ) {  // DEBUG
        GPSsleep();
        return fix.dateTime;
      }

/*#if DEBUGINO == 1
      Serial.print("time: "); Serial.println(fix.dateTime);
      Serial.print("millis (s): "); Serial.println(millis() / 1000);
#endif
*/

    }
  }
  
#if DEBUGINO == 1
  #if GPS == 1
    Serial.print(F("Uptime GPS (s): ")); Serial.println(currentTime);
  #else
    Serial.print(F("Uptime millis (s): ")); Serial.println(uptime / 1000);
  #endif
#endif
}

#endif // GPS == 1
