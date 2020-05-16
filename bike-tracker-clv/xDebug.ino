void printDebug(){
    #if LED == 2
    // EVAL not tested.
    // USBDevice.attach();delay(2000);
    #endif

    #if DEBUGINO == 1
      // for F() macro check https://learn.adafruit.com/memories-of-an-arduino/optimizing-sram (convert SRAM to PROGMEM)
      Serial.print(F("\nFC: ")); Serial.print(fc);
      Serial.print(F(", TotalTXms: "));Serial.print(totalTXms);
      Serial.print(F(", days: "));Serial.print(days);
      Serial.print(F(", millis: "));Serial.println(millis());
      Serial.print(F("previousTX was on s: "));Serial.println(endTXtime / 1000);
      Serial.print(F("MILLIS uptime in hours: "));Serial.println(millis() / 1000 / 3600);
      Serial.print(F("uptime in mins: "));Serial.println(millis() / 1000 / 60);
      Serial.print(F("uptime in secs: "));Serial.println(millis() / 1000);
      #if GPS == 1
       Serial.print(F("GPS days: "));Serial.println( uptimeGPS / DAY );
       Serial.print(F("bootTime : "));Serial.println(bootTime);
       Serial.print(F("timeGPS  : "));Serial.println(fix.dateTime);
       Serial.print(F("uptimeGPS: "));Serial.println(uptimeGPS);
       Serial.print(F("lastTXtime: "));Serial.println(lastTXtime);
       Serial.print(F("Transmit before (s): "));Serial.println( uptimeGPS - lastTXtime );
       Serial.print(F("speed: "));Serial.println(speed);
     #endif
   #endif
}
