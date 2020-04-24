uint8_t checkgps(){
   while (gps.available( Serial1 )) { // Read only one character, so you have to call it FAST.
    fix = gps.read();
    //if (fix.valid.location ) { // EVAL better if fix.hdop < 20?
      if (fix.hdop < 255000 && fix.hdop > 0 ) { // MAX I have seen 8720 akai 8.7 DOP (NeoGPS multiplies * 1000)
         fixCount++;                            // In balcony with cloudy weather 2000 to 830
         if ( ++fixCount >= 1 ) {
          // TODO: Need to send fix on heading change or time. Whichever comes first.
          // TODO: Send only once if stopped and restart with starting.
          // TODO: grab best signal? (I need to compare HDOP's)
          
          FramePort = FRAME_PORT_GPS; // We have GPS data, choose decoder with GPS.
          
          // https://github.com/ricaun/esp32-ttnmapper-gps/blob/8d37aa60e96707303ae07ca30366d2982e15b286/esp32-ttnmapper-gps/lmic_Payload.ino#L21
          //lat = ((fix.latitude() + 90) / 180) * 16777215;
          //lon = ((fix.longitude() + 180) / 360) * 16777215;

          lat = fix.latitude() * 10E5; // convert float to int. EVAL: BUG in JS decoding in TTN?
          lon = fix.longitude() * 10E5;
    
          loraData[5]  = lat >> 24; // LSB
          loraData[6]  = lat >> 16;
          loraData[7]  = lat >> 8;
          loraData[12] = lat;

          loraData[8]  = lon >> 24;
          loraData[9]  = lon >> 16;
          loraData[10] = lon >> 8;
          loraData[13] = lon;

          if ( fix.hdop >= 255000 ) {      // If DOP is more than 255, store 25.5
            loraData[11] = 255;            // I don't want to handle DOP in two bytes. MAX DOP stored is 255000.
          } else {                         // NeoGPS makes float to integer * 1000. Examples: 8720 (NeoGPS) = 8.72 DOP, 1960 (NeoGPS) = 1.96 DOP
            loraData[11] = fix.hdop / 100; // I divide with 100 to make 8720 = 87.20 HDOP stored as 87. Re-divided in TTN with 10,
                                           // So... I can fit a 255000 value (DOP 255) in one byte.
                                           // So... value of 830 (DOP 0.83) TXmitted as 8 and in decoder divided as 0.8
                                           // EVAL: optimization with divide by two and use less bits?
          }
          loraData[14] = fix.alt.whole >> 8;
          loraData[15] = fix.alt.whole;
          loraData[12] = fix.satellites;
        
          #if DEBUGINO == 1
            Serial.println(F("YES fix"));
          #endif

          #if GPS == 1 & DEBUGINO == 1
            displayGPS();
          #endif
          noFixCount =0; fixCount = 0; // reset the counters for gps
          FramePort = FRAME_PORT_GPS;

          GPSsleep(); // Close GPS we are done
          checkTXms();
      } // end if fixcount
    } else { // No fix
     
     #if DEBUGINO == 1
        Serial.print(F("Seeking FIX, UP secs: "));Serial.println(uptime / 1000);
     #endif
      
       if ( ++noFixCount <= 10 ) { // Continue if trying less than 5 minutes. (value in seconds)
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
     updUptime();
   }

  /*
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
}

void displayGPS(){
    Serial.print(F("Year: "));Serial.print( fix.dateTime.year );
    Serial.print(F(" Month: "));Serial.print( fix.dateTime.month );
    Serial.print(F(" Seconds: "));Serial.println( fix.dateTime.seconds );
    Serial.print(F("HDOP: "));Serial.print( fix.hdop );Serial.print(F(", latitude: "));Serial.print( fix.latitude(), 6 );Serial.print(F(", longitude: "));Serial.println( fix.longitude(), 6 );
    Serial.print(F("lat / lon: "));Serial.print( lat ); Serial.print(F(" "));Serial.println( lon );
    Serial.print(F("alt: "));Serial.print( fix.alt.whole ); Serial.print(F(", sats: "));Serial.println( fix.satellites );
    Serial.print(F("LoRa HDOP: "));Serial.println(loraData[11]);
}

// commands
// gps.send_P ( &gpsPort, F("PMTK000") );   // test (to wake up)
// gps.send_P ( &gpsPort, F("PMTK102") );   // Ephemeris delete
// gps.send_P ( &gpsPort, F("PMTK103") );   // Ephemeris, almanac, time, Position, delete
// gps.send_P ( &gpsPort, F("PMTK104") );   // Factory reset
// gps.send_P ( &gpsPort, F("PMTK161,0") ); // sleep (wake with any byte)

void GPSsleep(){
  /* *SOMETIMES* the light stays ON after power OFF.
   * 
   * The right procedure NOT IMPLEMENTED is here
  
  // Poll for led ON (is hardwired to FIX pin)
  // Wait some ?? time
  // Shutdown GPS
  
  *
  */
  
  #if DEBUGINO == 1
    Serial.println(F("GPS OFF"));
  #endif
  digitalWrite(GPS_SLEEP, HIGH); // Power OFF GPS.
  // gps.send_P ( &gpsPort, F("PMTK161,0") ); // sleep (wake with any byte)
}

void GPSwake(){
  #if DEBUGINO == 1
    Serial.println(F("GPS ON"));
  #endif
  digitalWrite(GPS_SLEEP, LOW); // Power ON GPS.
  // gps.send_P ( &gpsPort, F("PMTK000") );   // test (to wake up
}
