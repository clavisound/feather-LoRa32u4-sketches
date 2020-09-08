void blinkTen(){
   digitalWrite (LED_BUILTIN, HIGH);
   delay (10);
   digitalWrite (LED_BUILTIN, LOW);
}

void blinkLed(uint16_t times, uint16_t duration, uint8_t pause){ // x, ms, seconds
  if ( times == 0 ) times = 1; // make sure we have one loop
  for ( times > 0; times--; ) {
    digitalWrite(LED_BUILTIN, HIGH);
    #if DEBUGINO == 1
      delay(duration);
      digitalWrite(LED_BUILTIN, LOW);
      Serial.print(F("led "));
      delay(pause * 1000);
    #else
      uptime += Watchdog.sleep(duration);               // Sleep for up to 8 seconds (8000ms, 4000, 2000, 1000, 500, 250, 120, 60, 30, 15ms)
      digitalWrite(LED_BUILTIN, LOW);
      uptime += Watchdog.sleep(pause * 1000);           // Sleep for up to 8 seconds (8000ms, 4000, 2000, 1000, 500, 250, 120, 60, 30, 15ms)
    #endif
  }
}

void ledDEBUG(int16_t times, uint16_t duration, uint16_t pause){ // x, ms, ms
  for ( times > 0; times--; ) {
    digitalWrite(LED_BUILTIN, HIGH);delay(duration);digitalWrite(LED_BUILTIN, LOW);
    delay(pause);
  }
}

void sleepForSeconds(uint16_t secs, uint8_t wdtogSecs = 8){ // sleep for seconds  (maximum is 65535 seconds = 18.2 hours) 43200 = 12 hours. wdtogMS = 8, 2, 4, 1
  #if DEBUGINO == 1
  /*
      wtimes = secs / wdtogSecs;                       // watchdog times.
    if ( wtimes == 0 ) wtimes = 1;                 // wait at least 8 seconds
      for ( wtimes > 0; wtimes--; ) {
        uptime += Watchdog.sleep(wdtogSecs * 1000);    // Sleep and uptdate uptime because Watchdog reset millis.
        */
    delay(secs * 1000);
  #else

    wtimes = secs / wdtogSecs;                       // watchdog times.
    if ( wtimes == 0 ) wtimes = 1;                 // wait at least 8 seconds
      for ( wtimes > 0; wtimes--; ) {
        uptime += Watchdog.sleep(wdtogSecs * 1000);    // Sleep and uptdate uptime because Watchdog reset millis.
  }
  #endif
}

void setLed(int8_t times){
      ledDEBUG(3, 30, 500);                        // short fast blinks for notify we SET a lora data.
      #if DEBUGINO == 0
         uptime += Watchdog.sleep(1000);  // Sleep for 1 second (max 8)
      #else
         delay(1000);
      #endif
      ledDEBUG(times, 500, 500);                   // short fast blinks for notify we SET a lora data.
}

void updUptime(){
  
  #if GPS == 1
    #if DEBUGINO == 1
      Serial.print("\n* updUptime\n");
    #endif
    uptimeGPS = fix.dateTime - bootTime;
  #endif
  
  #if DEBUGINO == 1 & ( MMA8452 == 0 | LISDH == 0 ) & GPS == 0
    uptime = millis();                             // Ok only for normal code (not sleepy)
  #endif
  
  #if DEBUGINO == 0  & ( MMA8452 == 0 & LISDH == 0 ) & GPS == 0
    uptime += millis() / 2;                        // Ok only for sleepy code (millis are resetting) EVAL (see BUG #2)
  #endif
}

#if MMA8452 == 1 | LISDH == 1
ISR(PCINT0_vect){
  interruptEvent++;
}

void goToSleep(){

  /*
  #if GPS == 1
    GPSsleep();                    // Close GPS we are done
  #endif
  */
  
  #if DEBUGINO == 1
    Serial.print(F("\n* Sleep"));
    delay(100);                                 // we need this to give time to print all the characters.
  #endif

  #if LED == 3
    //ledDEBUG(3, 2000, 2000);
    blinkLed(1, 4000, 0);
    checkPin();                                // EVAL: BUG #12 (when LED ON INT from MMA is useless)
  #endif

  Watchdog.disable();                          // BUG #8 Solution. We need this otherwiser MCU wakes. QST: WHY?
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //noInterrupts();
  ADCSRA = 0;                                  // turn off ADC
  power_all_disable ();                        // power off ADC, Timer 0 and 1, serial interface
  sleep_enable();
  //interrupts();
  sleep_cpu();                             
  sleep_disable();
  power_all_enable();                          // power everything back on
}  // end of goToSleep 

void enablePinChange(){
  
  #if DEBUGINO == 1
    Serial.println(F("* Enable Pin"));
  #endif
  
  // pin change interrupt
  PCICR  |= 0b00000001;      // turn on port b https://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts/
  PCMSK0 |= 0b11000000;       // Enable PCINT7 pin #11
                             // with 0b11000000 LED always on WHY?
                             // with 0b10000000 pin #11 (PCINT7) blinks
                             // with 0b01000000 pin #10 (PCINT6) blinks
  //PCMSK0 |= bit (PCINT6);  // Pin D10 http://gammon.com.au/interrupts variants/leonardo/pins_arduino.h
  //PCMSK0 |= bit (PCINT7);  // Pin D11.
  
  PCIFR  |= bit (PCIF0);     // clear any outstanding interrupts
}

void disablePinChange(){
  
  #if DEBUGINO == 1
    Serial.println(F("* DISable Pin"));
  #endif
  
    // pin change interrupt
  PCICR = 0b00000000;      // turn on port b https://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts/
  //PCICR  |= bit (PCIE0);     // enable pin change interrupts for D8 to D13
  
  PCMSK0 = 0b00000000;      // Enable PCINT7 BUG: I can't enable both? (0b11000000)
                             // with 0b11000000 LED always on WHY?
                             // with 0b10000000 pin #11 (PCINT7) blinks
                             // with 0b01000000 pin #10 (PCINT6) blinks
  //PCMSK0 |= bit (PCINT6);  // Pin D10 http://gammon.com.au/interrupts variants/leonardo/pins_arduino.h
  //PCMSK0 |= bit (PCINT7);  // Pin D11.
  
  PCIFR  |= bit (PCIF0);     // clear any outstanding interrupts
}
#endif // #if MMA8452 == 1 | LISDH == 1

void toBeOrNotToBe(){
  
  #if DEBUGINO == 1 & GPS == 1 & ( LISDH == 0 & MMA8452 == 0 )
    Serial.println(F("\n* ToBeA"));
    delay(secondsSleep * 1000);                // delay works with ms, so multiply with 1000
  #endif
                                               
   #if DEBUGINO == 1 & GPS == 1 & ( LISDH == 1 | MMA8452 == 1 )
     Serial.println(F("* toBeOrNotB"));
      if ( speed < 5 ) {
        Serial.print(F("* Stationary ON (km/h): "));Serial.println(speed);
        goToSleep();                             // sleep forever (wake with accel)  
      } else {
        ledDEBUG(15,100,100);
        Serial.print(F("Sleeping for minutes: "));
        Serial.print((secondsSleep / 60)); 
        delay(secondsSleep * 1000);              // delay works with ms, so multiply with 1000
      }
   #endif

   #if DEBUGINO == 0 & GPS == 1 & ( LISDH == 1 | MMA8452 == 1)
      if ( speed < 5 ) {
       goToSleep();                              // sleep forever (wake with accel)
       return;
      } else {
        ledDEBUG(15,100,100);
        #if LED >= 2
          blinks = ( secondsSleep - ( uptimeGPS - lastTXtime ) ) / 8;
          blinkLed(blinks, 1, 8); // blink every 8 seconds
          return;
        #endif // LED >= 2
        
        #if LED <= 1
          sleepForSeconds(secondsSleep);
          return;
        #endif
      }
   #endif

  #if DEBUGINO == 0 & GPS == 1 & ( LISDH == 0 & MMA8452 == 0 )
    delay(secondsSleep * 1000);                // delay works with ms, so multiply with 1000
  #endif

  #if DEBUGINO == 1 & GPS == 0 & ( LISDH == 1 | MMA8452 == 1 )
      Serial.print("Sleeping for: ");Serial.println(secondsSleep);
      #if LED >= 2
        blinks = ( secondsSleep - ( uptimeGPS - lastTXtime ) ) / 8;
        ledDEBUG(blinks, 1, 8000); // blink every 8 seconds
      #endif // LED >= 2
        
      #if LED <= 1
        delay(secondsSleep * 1000);
      #endif
  #endif

  #if DEBUGINO == 0 & GPS == 0 & ( LISDH == 1 | MMA8452 == 1 )
      #if LED >= 2
        blinks = ( secondsSleep - ( uptimeGPS - lastTXtime ) ) / 8;
        ledDEBUG(blinks, 1, 8000); // blink every 8 seconds
      #endif // LED >= 2
        
      #if LED <= 1
        sleepForSeconds(secondsSleep);
      #endif
  #endif 
}
