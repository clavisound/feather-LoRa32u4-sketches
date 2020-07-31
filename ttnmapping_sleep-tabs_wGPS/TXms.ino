void checkTXms (){
  // if we transmitted too many seconds (see TXMS), wait for one day.
  if  ( totalTXms > TXMS ) { // too many seconds, retry the next day!
      #if DEBUGINO == 1
        Serial.print(F("ABORT. seconds / hours to sleep: ")); Serial.print((DAY - lastTXtime) / 1000); // 86400000 = 24 hours in ms.
        Serial.print(" / "); Serial.print((DAY - lastTXtime) / 3600000); // convert to hours.
        Serial.print(F(", TXtotal / ")); Serial.print(TXMS); Serial.print(secondsSleep);
        printDebug();
      #endif

      #if LED == 1
        blinkLed(40, 50, 100); // fast blink for ABORT
      #endif

      #if DEBUGINO == 1
        // sleep for X time (calculate for the rest of 24 hours)
        delay(DAY - lastTXtime); // DAY = 86400000
      #else
        uint16_t times = ((DAY - lastTXtime) / 1000) / 8; // sleep for a day.
        for ( times > 0; times--; ) {
            uint16_t sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 seconds
            uptime += sleepMS; // we need this because sleep resets millis.
        }
      #endif
      
      totalTXms = 0; // we have new day, reset the TX counter
      GPSwake();     // GPS ON
      return;
  }
  #if DEBUGINO == 1
    printDebug();
  #endif
  checkBatt();
  transmit();
}
