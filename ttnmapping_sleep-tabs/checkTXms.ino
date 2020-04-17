void checkTXms (){
  // if we transmitted too many seconds (see TXMS), wait for one day.
  if  ( totalTXms > TXMS ) { // too many seconds, retry the next day!
      #if DEBUGINO == 1
        Serial.print(F("ABORT. seconds / hours to sleep: ")); Serial.print((DAY - lastTXtime) / 1000); // 86400000 = 24 hours in ms.
        Serial.print(" / "); Serial.print((DAY - lastTXtime) / 3600000); // convert to hours.
        Serial.print(F("TXtotal / ")); Serial.print(TXMS); Serial.print(secondsSleep);
        printDebug();
      #endif

      #if LED == 1
        blinkLed(40, 50, 100); // fast blink for ABORT
      #endif
        // sleep for X time (calculate for the rest of 24 hours)
        delay(DAY - lastTXtime); // DAY = 86400000
      return;
  }
  #if DEBUGINO == 1
    printDebug();
  #endif
  checkBatt();
  transmit();
}
