void printDebug(){
  
    #if DEBUGINO == 2
      // for F() macro check https://learn.adafruit.com/memories-of-an-arduino/optimizing-sram (convert SRAM to PROGMEM)
      Serial.print(F("\nFC: ")); Serial.print(fc);
      Serial.print(F(", TotalTXms: "));Serial.print(totalTXms);
      Serial.print(F(", days: "));Serial.print(days);
      //Serial.print(F(", millis: "));Serial.println(millis());
      Serial.print(F(", endTXtime was on s: "));Serial.println(endTXtime / 1000);
      //Serial.print(F("MILLIS uptime in hours: "));Serial.println(millis() / 1000 / 3600);
      //Serial.print(F("uptime in mins: "));Serial.println(uptime / 1000 / 60);
      //Serial.print(F("uptime in secs: "));Serial.println(uptime / 1000);
      //Serial.print(F("uptime in secs (millis): "));Serial.println(millis() / 1000);
      #if GPS == 1
       Serial.print(F("GPS days: "));Serial.println( uptimeGPS / DAY );
       Serial.print(F("bootTime : "));Serial.println(bootTime);
       Serial.print(F("timeGPS  : "));Serial.println(fix.dateTime);
       Serial.print(F("Transmit before (s): "));Serial.print( uptimeGPS - lastTXtime );
       Serial.print(F(", uptimeGPS: "));Serial.print(uptimeGPS);
       Serial.print(F(", lastTXtime: "));Serial.println(lastTXtime);
       Serial.print(F("\nFixCount: "));Serial.print(fixes);
       //Serial.print(F("GPSnow   : "));Serial.println(GPSnow);
       Serial.print(F("\nspeed: "));Serial.println(speed);
     #endif
   #endif
}
