void checkTXms (){
  // if we transmitted too many seconds (see TXMS), wait for one day.
  // TODO if  ( totalTXms > (TXMS - currentTXms) ) { // too many seconds, retry the next day! Subtract last (currentTXms) value from total.
  if  ( totalTXms > TXMS ) { // too many seconds, retry the next day! Subtract last (currentTXms) value from total.
      #if DEBUGINO == 1 & GPS == 0
        Serial.print(F("ABORT. seconds / hours to sleep: ")); Serial.print((DAY - endTXtime) / 1000); // 86400000 = 24 hours in ms.
        Serial.print(" / "); Serial.print((DAY - endTXtime) / 3600000);                               // convert to hours.
        Serial.print(F(", TTN limit: "));Serial.print(TXMS);Serial.print(F(", SecondsSleep: "));Serial.print(secondsSleep);
        printDebug();
      #endif

      #if DEBUGINO == 1 & GPS == 1
        Serial.print(F("ABORT. seconds / hours to sleep: ")); Serial.print( DAY - ( lastTXtime % DAY ) ); // 86400 = 24 hours in seconds.
        Serial.print(" / "); Serial.print((DAY - ( lastTXtime % DAY ) ) / 3600);                          // convert to hours.
        Serial.print(F(", TTN limit: "));Serial.print(TXMS);Serial.print(F(", SecondsSleep: "));Serial.print(secondsSleep);
        printDebug();
      #endif

      #if LED == 1
        blinkLed(40, 50, 100); // fast blink for ABORT
      #endif

      #if DEBUGINO == 1 & GPS == 0
        // sleep for X time (calculate for the rest of 24 hours)
        delay(DAY - endTXtime); // DAY = 86400000 (ms)
      #endif

      #if DEBUGINO == 1 & GPS == 1
        GPSsleep();                                 // GPS OFF
        // sleep for X time (calculate for the rest of 24 hours)
        delay( ( DAY - lastTXtime % DAY ) * 1000); // DAY = 86400 (seconds)
      #endif

      #if DEBUGINO == 0 & GPS == 0
        uint16_t times = ((DAY - endTXtime) / 1000) / 8; // sleep for a day.
        for ( times > 0; times--; ) {
            uint16_t sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 seconds
            uptime += sleepMS; // we need this because sleep resets millis.
        }
      #endif

      #if DEBUGINO == 0 & GPS == 1 // Even with MMA8452, we have to wake after the DAY limit. Just in case.
        GPSsleep();                // GPS OFF
        uint16_t times = ( DAY - lastTXtime ) / 8; // sleep for a day.
        for ( times > 0; times--; ) {
            uint16_t sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 seconds
            uptime += sleepMS; // we need this because sleep resets millis.
        }
      #endif
      
      totalTXms = 0; // we have new day, reset the TX counter
      #if GPS == 1
        GPSwake();     // GPS ON
      #endif
      return;
  }

  // TODO check if uptime more than 12 hours reset MMA

  // TODO check if already send message in last secondsSleep

  #if DEBUGINO == 1 & GPS == 1 & LISDH == 1
    GPSuptime();
    if ( currentTime - lastTXtime > secondsSleep ) { // we have reached the time. Transmit!
      printDebug();
      checkBatt();
      readLIS();
      transmit();
    } else {
      // TODO disablePinChange
      Serial.println(F("\nToo SOON! will wait: "));Serial.println(secondsSleep - ( currentTime - lastTXtime ) );
      delay( ( secondsSleep - ( currentTime - lastTXtime ) ) * 1000);    // We waited some time, so subtract it.
      checkFix();
    }
  #endif

    #if DEBUGINO == 0 & GPS == 1 & LISDH == 1
      GPSuptime();
      if ( currentTime - lastTXtime > secondsSleep ) { // we have reached the time. Transmit!
        checkBatt();
        readLIS();
        transmit();
      } else {
        // TODO disablePinChange
        #if LED == 2
        // recalculate blinks.
          blinks = ( secondsSleep - ( currentTime - lastTXtime ) ) / 8;
          blinkLed(blinks, 1, 8000); // blink every 8 seconds
        #else
          blinks = ( secondsSleep - ( currentTime - lastTXtime ) ) / 8;
          if ( blinks == 0 ) blinks = 1;
          uint16_t times = blinks;
          for ( times > 0; times--; ) {
            uint16_t sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 seconds
              uptime += sleepMS;                        // EVAL BUG #2 uptime += 8000;
          }
        #endif
        checkFix();
      }
    #endif
}
