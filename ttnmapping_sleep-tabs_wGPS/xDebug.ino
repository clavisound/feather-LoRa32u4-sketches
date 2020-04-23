void printDebug(){
    #if LED == 2
    // EVAL not tested.
    // USBDevice.attach();delay(2000);
    #endif
    // for F() macro check https://learn.adafruit.com/memories-of-an-arduino/optimizing-sram (convert SRAM to PROGMEM)
    Serial.print(F("\nFC: ")); Serial.print(fc);
    Serial.print(F(", TotalTXms: "));Serial.print(totalTXms);
    Serial.print(F(", days: "));Serial.print(days);
    Serial.print(F(", millis: "));Serial.print(millis());
    Serial.print(F(", uptime: "));Serial.println(uptime);
    //Serial.print(F(", millis - uptime: "));Serial.println(millis() - uptime);
    Serial.print(F("previousTX was on s: "));Serial.println(lastTXtime / 1000);
    Serial.print(F("uptime in hours: "));Serial.println(millis() / 1000 / 3600);
    Serial.print(F("uptime in mins: "));Serial.println(millis() / 1000 / 60);
    Serial.print(F("uptime in secs: "));Serial.println(millis() / 1000);
}
