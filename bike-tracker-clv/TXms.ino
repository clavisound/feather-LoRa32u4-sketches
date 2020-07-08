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
        blinkLed(40, 50, 100); // fast blink for ABORT
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
          uint16_t times = ( DAY - lastTXtime ) / 8; // sleep for a day.
          for ( times > 0; times--; ) {
            uint16_t sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 seconds
            uptime += sleepMS; // we need this because sleep resets millis.
          }
        #else // GPS == 0
          uint16_t times = ((DAY - endTXtime) / 1000) / 8; // sleep for a day.
          for ( times > 0; times--; ) {
            uint16_t sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 seconds
            uptime += sleepMS; // we need this because sleep resets millis.
          }
        #endif
      #endif
      
      totalTXms = 0; // we have new day, reset the TX counter
      #if GPS == 1
        GPSwake();     // GPS ON
      #endif
      return;
  }

  #if DEBUGINO == 1 & GPS == 1 & LISDH == 1
    if ( uptimeGPS - lastTXtime > secondsSleep ) { // we trasmitted long time ago: Transmit!
      printDebug();
      Serial.println("* TXms");
      checkBatt();
      checkPin();
      transmit();
    } else {
      // TODO disablePinChange
      // TODO: send with FALL
      printDebug();
      Serial.println(F("\nToo SOON! wake in: "));Serial.println(secondsSleep - ( uptimeGPS - lastTXtime ) );
      delay( ( secondsSleep - ( uptimeGPS - lastTXtime ) ) * 1000);    // We waited some time, so subtract it.
      checkPin();
      checkFix();
    }
  #endif

    #if DEBUGINO == 0 & GPS == 1 & LISDH == 1
      if ( uptimeGPS - lastTXtime > secondsSleep ) { // we trasmitted long time ago: Transmit!
        checkBatt();
        checkPin();
        transmit();
      } else {
        // TODO disablePinChange
        #if LED == 2
        // recalculate blinks.
          blinks = ( secondsSleep - ( uptimeGPS - lastTXtime ) ) / 8;
          #if LED == 3
            ledDEBUG(blinks, 20, 300);
            ledDEBUG(10, 2, 100);
          #endif  // LED == 3
        #endif // LED == 2
        checkPin();
        checkFix();
      }
    #endif

    #if GPS == 0 & (LISDH == 0 & MMA8452 == 0)
      checkBatt();
      transmit();
    #endif
}
