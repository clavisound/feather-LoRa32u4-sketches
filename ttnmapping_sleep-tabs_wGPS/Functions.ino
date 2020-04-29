void blinkLed(int16_t times, uint16_t duration, uint16_t pause){ // x, ms, ms
  for ( times > 0; times--; ) {
    digitalWrite(LED_BUILTIN, HIGH);delay(duration);digitalWrite(LED_BUILTIN, LOW);
    #if DEBUGINO == 1
      Serial.println(F("led"));
      delay(pause);
    #else
      uint16_t sleepMS = Watchdog.sleep(pause);  // Sleep for up to 8 seconds
      uptime += sleepMS; // we need this because Watchdog reset millis.
    #endif
  }
  GPSwake(); // end waiting, GPS ON
}

void setLed(int8_t times){
      blinkLed(3, 30, 500); // short fast blinks for notify we SET a lora data.
      #if DEBUGINO == 0
         uint16_t sleepMS = Watchdog.sleep(1000);  // Sleep for 1 second (max 8)
         uptime += sleepMS;
      #else
         delay(1000);
      #endif
      blinkLed(times, 500, 500); // short fast blinks for notify we SET a lora data.
}

void updUptime(){
  #if DEBUGINO == 1
    uptime = millis();  // Ok only for normal code (not sleepy)
  #else
    uptime += millis(); // Ok only for sleepy code (millis are resetting)
  #endif
}
