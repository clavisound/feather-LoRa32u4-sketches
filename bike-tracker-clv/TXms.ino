void checkTXms (){
  // if we transmitted too many seconds (see TXMS), wait for one day.
  if  ( totalTXms > TXMS ) { // too many seconds, retry the next day! Subtract last (currentTXms) value from total.
      #if DEBUGINO == 1
        #if GPS == 0
          Serial.print(F("ABORT. seconds / hours to sleep: ")); Serial.print((DAY - endTXtime) / 1000); // 86400000 = 24 hours in ms.
          Serial.print(" / "); Serial.print((DAY - endTXtime) / 3600000);                               // convert to hours.
          Serial.print(F(", TTN limit: "));Serial.print(TXMS);Serial.print(F(", SecondsSleep: "));Serial.print(secondsSleep);
        #else
          Serial.print(F("ABORT. seconds / hours to sleep: ")); Serial.print( DAY - ( lastTXtime % DAY ) ); // 86400 = 24 hours in seconds.
          Serial.print(" / "); Serial.print((DAY - ( lastTXtime % DAY ) ) / 3600);                          // convert to hours.
          Serial.print(F(", TTN limit: "));Serial.print(TXMS);Serial.print(F(", SecondsSleep: "));Serial.print(secondsSleep);
        #endif
        printDebug();
      #endif

      #if LED == 1
        ledDEBUG(40, 50, 100); // fast blink for ABORT
      #endif

      #if DEBUGINO == 1
        #if GPS == 1 // sleep for X time (calculate for the rest of 24 hours)
          delay( ( DAY - lastTXtime % DAY ) * 1000); // DAY = 86400 (seconds)
        #else
          delay(DAY - endTXtime); // DAY = 86400000 (ms)
        #endif
      #endif

      #if DEBUGINO == 0
        #if GPS == 1 // Even with MMA8452, we have to wake after the DAY limit. Just in case.
          times = ( DAY - lastTXtime ) / 8; // sleep for a day.
          for ( times > 0; times--; ) {
            uptime += Watchdog.sleep(8000);  // we need this because sleep resets millis.
          }
        #else // GPS == 0
          times = ((DAY - endTXtime) / 1000) / 8; // sleep for a day.
          for ( times > 0; times--; ) {
            uptime += Watchdog.sleep(8000);  // we need this because sleep resets millis.
          }
        #endif
      #endif

      totalTXms = 0;   // we have new day, reset the TX counter
      #if GPS == 1
        GPSwake();     // GPS ON
      #endif
      return;
  }


// TODO: better make a function checkTXtime().
  #if DEBUGINO == 1 & GPS == 1 & ( LISDH == 1 | MMA8452 == 1)
    
    Serial.println("* TXms");
    GPStime(); updUptime();                                      // update (GPS) fix values and update uptimeGPS
    if ( uptimeGPS - lastTXtime > secondsSleep ) {               // we trasmitted long time ago: Transmit!
      if ( fix.dateTime - GPS_old_time > 30 || fix.hdop == 0 ) { // We have old fix OR no-fix, re-gain fix.
        Serial.print("\n!Re-fix, ");
        Serial.print("fix before: ");Serial.println(fix.dateTime - GPS_old_time);
        Serial.print("hdop: ");Serial.println(fix.hdop);
        //printDebug();
        //displayGPS();
        checkFix(); return;
        }
      Serial.println("\n!Use old-fix");
      //printDebug();
      //displayGPS();
      checkPin(); transmit(); return;
      /*
    } else if ( uptimeGPS - lastTXtime >= 240 ) { // We have too much time to wait. 4 minutes and more. Inform and sleep without fix.
      // TODO disablePinChange
      // TODO: send with FALL
      printDebug();
      Serial.println(F("\nWAY Too SOON! Wake in : "));Serial.println(secondsSleep - ( uptimeGPS - lastTXtime ) );
      delay( ( secondsSleep - ( uptimeGPS - lastTXtime ) ) * 1000);    // We waited some time, so subtract it.
      // checkFix(); // TODO: eval periodic mode
      */
    } else {                                                             // Wait and send data
      //gps.send_P ( &gpsPort, F("PMTK225,2,3000,300000,0") );             // enable periodic. 1.5mA after fix BUG: LED can stay on!
      gps.send_P ( &gpsPort, F("PMTK225,2,40000,500000,0,0") );         // Periodic 10secs fix, 50 sleep, Turtle track: https://forums.adafruit.com/viewtopic.php?t=54272
      //gps.send_P ( &gpsPort, F("PMTK225,8") );                         // Always Locate Stand-by. 1.5mA after fix.
      //gps.send_P ( &gpsPort, F("PMTK225,9") );                           // Always Locate Back-up. Same as standby?
      printDebug();
      Serial.println(F("\nToo SOON! wake in: "));Serial.println(secondsSleep - ( uptimeGPS - lastTXtime ) );
      delay( ( secondsSleep - ( uptimeGPS - lastTXtime ) ) * 1000 );    // Calculate difference from last transmission
    }
  #endif

    #if DEBUGINO == 0 & GPS == 1 & ( LISDH == 1 | MMA8452 == 1 )
    GPStime(); updUptime();                                         // update (GPS) fix values and update uptimeGPS
      if ( uptimeGPS - lastTXtime > secondsSleep ) {                // we trasmitted long time ago: Transmit!
        if ( fix.dateTime - GPS_old_time > 30 || fix.hdop == 0 ) {  // We have old fix OR no-fix, re-gain fix.
         checkFix(); return;
        }                                                           // use old-fix
        checkPin(); transmit(); return;                             
      } else {                                                           // We have to wait to transmit.
        GPSsleep();
      //gps.send_P ( &gpsPort, F("PMTK225,2,3000,300000,0") );             // enable periodic. 1.5mA after fix BUG: LED can stay on!
      //gps.send_P ( &gpsPort, F("PMTK225,2,40000,500000,0,0") );         // Periodic 10secs fix, 50 sleep, Turtle track: https://forums.adafruit.com/viewtopic.php?t=54272
      //gps.send_P ( &gpsPort, F("PMTK225,8") );                         // Always Locate Stand-by. 1.5mA after fix.
      //gps.send_P ( &gpsPort, F("PMTK225,9") );                           // Always Locate Back-up. Same as standby?
      //gps.send_P ( &gpsPort, F("PMTK161,1") );                        //  stand-by
      //gps.send_P ( &gpsPort, F("PMTK161,") );                        //  Sleep
        #if LED >= 2
          blinks = ( secondsSleep - ( uptimeGPS - lastTXtime ) ) / 8;   // Calculate difference from last transmission, then recalculate blinks. 
          // ledDEBUG(blinks, 15, 8000);
          blinkLed(blinks, 15, 8);
        #else
          sleepForSeconds(secondsSleep);
        #endif // LED == 2
      }
    #endif

    #if GPS == 0 & (LISDH == 0 & MMA8452 == 0)
      updUptime();
      transmit();
      sleepForSeconds(secondsSleep);
    #endif

    #if GPS == 0 & (LISDH == 1 | MMA8452 == 1)
    updUptime();
      if ( uptime - endTXtime > (secondsSleep * 1000 ) ) {               // we trasmitted long time ago: Transmit!
      transmit();
    } else {
      sleepForSeconds(secondsSleep);
    }
    #endif

    #if GPS == 1 & (LISDH == 0 & MMA8452 == 0)
      GPStime(); updUptime();                                         // update (GPS) fix values and update uptimeGPS
      transmit();
      sleepForSeconds(secondsSleep);
    #endif
}
