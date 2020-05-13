void blinkTen(){
   digitalWrite (LED_BUILTIN, HIGH);
   delay (10);
   digitalWrite (LED_BUILTIN, LOW);
}

void blinkLed(int16_t times, uint16_t duration, uint16_t pause){ // x, ms, ms
  if ( times == 0 ) times = 1; // make sure we have one loop
  for ( times > 0; times--; ) {
    digitalWrite(LED_BUILTIN, HIGH);delay(duration);digitalWrite(LED_BUILTIN, LOW);
    #if DEBUGINO == 1
      Serial.println(F("led"));
      delay(pause);
    #else
      uint16_t sleepMS = Watchdog.sleep(pause);  // Sleep for up to 8 seconds
      uptime += sleepMS;                         // we need this because Watchdog reset millis.
      //uptime += 8000;                          // EVAL BUG #2
    #endif
  }
}

void sleepForSeconds(int16_t secs){             // sleep for seconds  (maximum is 65535 seconds = 18.2 hours) 43200 = 12 hours.
  uint16_t wtimes = secs / 8;                   // watchdog times.
  if ( wtimes == 0 ) wtimes = 1;                // wait at least 8 seconds
  for ( wtimes > 0; wtimes--; ) {
      uint16_t sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 seconds
      uptime += sleepMS;                        // we need this because Watchdog reset millis.
      //uptime += 8000;                         // EVAL BUG #2
  }
}

void setLed(int8_t times){
      blinkLed(3, 30, 500);                        // short fast blinks for notify we SET a lora data.
      #if DEBUGINO == 0
         uint16_t sleepMS = Watchdog.sleep(1000);  // Sleep for 1 second (max 8)
         uptime += sleepMS;
      #else
         delay(1000);
      #endif
      blinkLed(times, 500, 500);                   // short fast blinks for notify we SET a lora data.
}

void updUptime(){
  
  #if DEBUGINO == 1 & ( MMA8452 == 0 | LISDH == 0 )
    uptime = millis();                             // Ok only for normal code (not sleepy)
  #endif
  
  #if ( MMA8452 == 0 & LISDH == 0 ) & DEBUGINO == 0
    uptime += millis() / 2;                        // Ok only for sleepy code (millis are resetting) EVAL (see BUG #2)
  #endif
}

#if MMA8452 == 1 | LISDH == 1
ISR (PCINT0_vect){
  interruptEvent++;
}

void goToSleep(){
  #if DEBUGINO == 1
    Serial.print(F("\nSleep"));
  #endif
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //noInterrupts ();
  ADCSRA = 0;                                  // turn off ADC
  power_all_disable ();                        // power off ADC, Timer 0 and 1, serial interface
  sleep_enable();
  //interrupts ();
  sleep_cpu();                             
  sleep_disable();   
  power_all_enable();                          // power everything back on
}  // end of goToSleep 

void toBeOrNotToBe(){
  #if DEBUGINO == 1 & GPS == 1 & LISDH == 0
    Serial.println(F("ToBeA"));
    delay(secondsSleep * 1000);                // delay works with ms, so multiply with 1000
    GPSsleep();                                // Hang here but wakes with LIS(?!)
  #endif
                                               
   #if DEBUGINO == 1 & GPS == 1 & LISDH == 1
      GPSsleep();                                // Hang here but wakes with LIS(?!)
      //enablePinChange();                       // TODO enable only for orientation.
      if ( speed == 0 ) {
        goToSleep();                             // sleep forever (wake with accel)  
      } else {
        delay(secondsSleep * 1000);              // delay works with ms, so multiply with 1000
      }
   #endif

   #if DEBUGINO == 0 & GPS == 1 & LISDH == 1
      GPSsleep();                                // Hang here but wakes with LIS(?!)
      //enablePinChange();                       // TODO enable only for orientation.
      if ( speed == 0 ) {
        goToSleep();             // sleep forever (wake with accel)  
      } else {
        #if LED == 2
          blinkLed(blinks, 1, 8000); // blink every 8 seconds
        #else
          uint16_t times = blinks;
          for ( times > 0; times--; ) {
            uint16_t sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 seconds
              uptime += sleepMS;                        // EVAL BUG #2 uptime += 8000;
              sleepMS = Watchdog.sleep(randMS);         // Sleep for random time
              uptime += sleepMS;
          }
        #endif
      }
   #endif
   
}

void enablePinChange(){

  delay(500);
  
  #if DEBUGINO == 1
    Serial.println(F("EN-able PinChange"));
  #endif
    // pin change interrupt
  PCICR  |= 0b00000001;      // turn on port b https://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts/
  //PCICR  |= bit (PCIE0);     // enable pin change interrupts for D8 to D13
  
  PCMSK0 |= 0b01000000;      // Enable PCINT7 BUG: I can't enable both? (0b11000000)
                             // with 0b11000000 LED always on WHY?
                             // with 0b10000000 pin #11 (PCINT7) blinks
                             // with 0b01000000 pin #10 (PCINT6) blinks
  //PCMSK0 |= bit (PCINT6);  // Pin D10 http://gammon.com.au/interrupts variants/leonardo/pins_arduino.h
  //PCMSK0 |= bit (PCINT7);  // Pin D11.
  
  PCIFR  |= bit (PCIF0);     // clear any outstanding interrupts
}

void disablePinChange(){
  delay(500);
  
  #if DEBUGINO == 1
    Serial.println(F("DIS-able PinChange"));
  #endif
  
    // pin change interrupt
  PCICR  |= 0b00000001;      // turn on port b https://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts/
  //PCICR  |= bit (PCIE0);     // enable pin change interrupts for D8 to D13
  
  PCMSK0 |= 0b00000000;      // Enable PCINT7 BUG: I can't enable both? (0b11000000)
                             // with 0b11000000 LED always on WHY?
                             // with 0b10000000 pin #11 (PCINT7) blinks
                             // with 0b01000000 pin #10 (PCINT6) blinks
  //PCMSK0 |= bit (PCINT6);  // Pin D10 http://gammon.com.au/interrupts variants/leonardo/pins_arduino.h
  //PCMSK0 |= bit (PCINT7);  // Pin D11.
  
  PCIFR  |= bit (PCIF0);     // clear any outstanding interrupts
}
#endif
