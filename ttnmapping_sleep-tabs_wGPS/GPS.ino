// Freaky IDEA: don't send zeroed bytes, instead change port accordingly.

uint8_t checkgps(){
   while (gps.available( Serial1 )) {            // Read only one character, so you have to call it FAST.
    fix = gps.read();
     //if (fix.valid.location ) {               // EVAL better if fix.hdop < 8000?
      if (fix.hdop < 8000 && fix.hdop > 0 ) {   // MAX I have seen 8720 akai 8.7 DOP (NeoGPS multiplies * 1000)
         fixCount++;                            // In balcony with cloudy weather 2000 to 830
         if ( ++fixCount >= 1 ) {

           speed = (int)fix.speed_kph();
          // checkStatus();
          
          // TODO: Need to send fix on heading change or time. Whichever comes first.
          // TODO: Send only once if stopped and restart with starting.
          // TODO: grab best signal? (I need to compare HDOP's)
          
          FramePort = FRAME_PORT_GPS; // We have GPS data, choose decoder with GPS.
          
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
            loraData[14] = 0;              // TODO, we don't need those two bytes!
            loraData[15] = 0;
          } else {
          loraData[14] = fix.alt.whole >> 8;   // MSB altitude 16bits
          loraData[15] = fix.alt.whole;
          }
          loraData[12] = fix.satellites;       // TODO: count bits, or maybe + 1 / divide with 2. Aka for 4 bits (max 16 sats)
          loraData[13] = (int)fix.speed_kph(); // TODO: divide by 3 to have max speed of 90 with 5 bits
          loraData[16] = fix.heading_cd() / 2; // TODO: divide by 6 to fit in 6bits
        
          #if DEBUGINO == 1
            Serial.println(F("YES fix"));
          #endif

          #if GPS == 1 & DEBUGINO == 1
            displayGPS();
          #endif

          noFixCount =0; fixCount = 0;         // reset the counters for gps fixes
          FramePort = FRAME_PORT_GPS;

          //GPSsleep();                            // Close GPS we are done
          gps.send_P ( &gpsPort, F("PMTK225,8") ); // Periodic Mode.
          checkTXms();                             // Transmit if we are between TTN limits
      } // end if fixcount
    } else { // No fix
     
     #if DEBUGINO == 1
        Serial.print(F("Seeking FIX, UP secs: "));Serial.println(uptime / 1000);
     #endif
      
       if ( ++noFixCount <= 120 ) { // Continue for number of failures (on update 1Hz every second)
       #if DEBUGINO == 1
         Serial.println("No fix");
         displayGPS();
        #endif
       } else { // give up
        #if DEBUGINO == 1
         Serial.print(F("No fix: GIVE UP secs: "));Serial.println(uptime / 1000);
         displayGPS();
        #endif
        FramePort = 3;

        GPSsleep(); // Close GPS we are done
        checkTXms();
        
        noFixCount =0;
        fixCount = 0; // reset the counters for gps tracking.
       }
   }
  }
}

void displayGPS(){
    Serial.print(F("Year: "));Serial.print( fix.dateTime.year );
    Serial.print(F(" Month: "));Serial.print( fix.dateTime.month );
    Serial.print(F(" Seconds: "));Serial.println( fix.dateTime.seconds );
    Serial.print(F("HDOP: "));Serial.print( fix.hdop );Serial.print(F(", latitude: "));Serial.print( fix.latitude(), 6 );Serial.print(F(", longitude: "));Serial.println( fix.longitude(), 6 );
    Serial.print(F("lat / lon: "));Serial.print( lat ); Serial.print(F(" "));Serial.println( lon );
    Serial.print(F("alt: "));Serial.print( fix.alt.whole ); Serial.print(F(", sats: "));Serial.println( fix.satellites );
    Serial.print(F("LoRa HDOP: "));Serial.println(loraData[11]);

    /*
     * EVAL
     * 
   * https://github.com/AmedeeBulle/ttn-mapper/blob/master/ttn-mapper-gps/ttn_mapper.cpp
   * data = GPS.latitude_fixed * (GPS.lat == 'N' ? 1 : -1) + 90 * 1E7;
      payload[idx++] = data >> 24;
      payload[idx++] = data >> 16;
      payload[idx++] = data >> 8;
      payload[idx++] = data;
      data = GPS.longitude_fixed * (GPS.lon == 'E' ? 1 : -1) + 180 * 1E7;
      payload[idx++] = data >> 24;
      payload[idx++] = data >> 16;
      payload[idx++] = data >> 8;
      payload[idx++] = data;
   */
}

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

void GPSsleep(){
  /* *SOMETIMES* the light stays ON after power OFF.
   * 
   * The right procedure is here (NOT IMPLEMENTED)
   *
  // a) Poll for led ON (is hardwired to FIX pin)
  // b) Wait some ?? time
  // c) Shutdown GPS
  *
  *
  */
  
  #if DEBUGINO == 1
    Serial.println(F("GPS OFF"));
  #endif
  //digitalWrite(GPS_SLEEP, HIGH);              // Power OFF GPS.
  //gps.send_P ( &gpsPort, F("PMTK161,0") );    // sleep (wake with any byte)
  gps.send_P ( &gpsPort, F("PMTK225,8") );   // Periodic Mode, Always Locate (standby) 9 for backup.
}

void GPSwake(){
  #if DEBUGINO == 1
    Serial.println(F("GPS ON"));
  #endif
  // digitalWrite(GPS_SLEEP, LOW);             // Power ON GPS.
  //gps.send_P ( &gpsPort, F("PMTK000") );       // test command (module wakes up with ANY communication)
  gps.send_P ( &gpsPort, F("PMTK225,0") );   // disable Periodic.
}

void GPSfastRate(){
	// we moving, GPS always ON with less power
  gps.send_P ( &gpsPort, F("PMTK220,1000") );  //update 1time in 10seconds (value in ms)
}

void GPSslowRate(){
  // we are going slow
  // gps.send_P ( &gpsPort, F("PMTK225,8") ); // Periodic Mode.
  gps.send_P ( &gpsPort, F("PMTK220,3000") );  //update 1time in 3seconds (value in ms)
}

void checkStatus(){
  if ( speed > 19 ) GPSfastRate();             // Attention with the order.
  if ( speed < 5  ) GPSslowRate();
}
