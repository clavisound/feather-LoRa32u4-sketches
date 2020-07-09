// MMA7452 accelorometer setup and functions. (you need my library)

#if MMA8452 == 1
void setupMMA(){
  if (accel.begin() == false) {
      #if LED == 1
        blinkLed(40, 50, 100); // fast blink for MMA error
      #endif
    while (1);
  }

  accel.reset();

  #if OLDINIT == 1
    accel.initOld();
  #endif

  #if OLDINIT == 0
  // BUG setupSleep hangs everything?
  // accel.setupSleep(); // default parameters: accel.setupSleep(1,0,1,0,0,1)
  // (1:enable, 0:firstPin interrupt [default second pin], oversampling [0-3], sleep sample rate [0-3], wake sample rate [0-7],
                        // timeout for sleep [0-81])
  //accel.setupSleep(0,1,0,0,0,20);
  
  //accel.setupMotion();      // defaults (latch, freefall, axes [1 = X, 3 = X+Y, 7 = X+Y+Z], threshold, debounce)
                              // accel.setupMotion(1,0,3,3,5)
  //accel.setupMotion(1,0,7,1,1);

  accel.setupTap(0x10, 0x80, 0x80);                  // enable taps on X, Y, Z 0x80 to disable

  accel.setupPL(0x05);                               // debounce rate, default 0x50

  // EVAL
  //accel.lowPowerAN();

  accel.enableEvents(1,0,1,1,0);                     // sleep, transient, orientation, tap, motion
  accel.interruptPin(0,0,0,0,0,1,0);                 // sleep, transient, orientation, tap, motion, polarity, opendrain
                                                     // 1 = pin1 0 = pin2.
  accel.setDataRate(ODR_12);
  
  //accel.lowPowerEVAL();
  
  #endif

  pinMode(INT_PIN_TEN, INPUT);            // Setup the button pin
  pinMode(INT_PIN_ELEVEN, INPUT);         // Setup the button pin
  digitalWrite (INT_PIN_TEN, LOW);        // disable pull-up on button
  digitalWrite (INT_PIN_ELEVEN, LOW);     // disable pull-up on button
  
}

void readMMA(){
   evntMMA = accel.readIRQEvent();
   uint8_t t;
   
   if ( evntMMA > 0 ) {
    if ( (evntMMA & MSKPL) == MSKPL ) {
      t = accel.readPL();
      
      #if DEBUGINO == 1
        Serial.print("Orientation: ");Serial.println(t);
      #endif
      }
      if ( (evntMMA & MSKTAP) == MSKTAP ) {
        t = accel.readTap();
        #if DEBUGINO == 1
          Serial.print("Tap        : ");Serial.println(t);
        #endif
      }

    if ( (evntMMA & MSKMTN) == MSKMTN ) {
      t = accel.readMotionType();
      #if DEBUGINO == 1
        Serial.print("Motion     : ");Serial.print(t);
      #endif
      if ( t > 0 ) {
        #if DEBUGINO == 1
         Serial.print("\tX: ");Serial.print(accel.getX());
         Serial.print(", Y: ");Serial.print(accel.getY());
         Serial.print(", Z: ");Serial.print(accel.getZ());
       #endif
      }
     }
     
     if ( (evntMMA & MSKTRS) == MSKTRS ) {
      #if DEBUGINO == 1
        Serial.print("Transient: ");
      #endif
      }
     #if DEBUGINO == 1
       Serial.print("\n");
     #endif
     #if LED == 1
       digitalWrite (LED_BUILTIN, HIGH);
       delay (10);
       digitalWrite (LED_BUILTIN, LOW);
     #endif
   }
   #if DEBUGINO == 1
     Serial.print("interruptEventMMAs: ");Serial.print(interruptEventMMA);
     Serial.print(", Wake or sleep: ");Serial.print(accel.wakeOrSleep());
     Serial.print(", Interrupt Reason: ");Serial.print(evntMMA);
     Serial.print("\nUptime: ");Serial.println(millis() / 1000);
   #endif
   interruptEventMMA = 0;
}

void readIRQMMA(){
  if ( interruptEventMMA > 0 ) {
   readMMA();
  }
}

uint8_t checkFall(){
   // first thing to do: check if we don't transmitted according to secondsSleep
   if ( GPStime() - endTXtime < secondsSleep ) {          // continue to sleep.        
      // EVAL transmit only on Orientation change.
      //uint8_t evntMMA = accel.readIRQEvent();
   
     if ( accel.readIRQEvent() == MSKPL ) { // check for orientation event
       orientation = accel.readPL();
       #if DEBUGINO == 1
         Serial.print("Orientation: ");Serial.println(orientation);
       #endif
       // return orientation;
       }
     }
      // goToSleep();
}

void enableTAPmma(){
  accel.enableEvents(1,0,1,1,0);                     // sleep, transient, orientation, tap, motion 
}

void disableTAPmma(){
  accel.enableEvents(1,0,1,0,0);                     // sleep, transient, orientation, tap, motion 
}

void wakeFromMMA(){
    if ( interruptEventMMA > 0 ) {
      interruptEventMMA = 0;       // we woke up.
      disablePinChange();       // disable Interrupts
    }

    // TODO if checkFall not UP send message.
    // checkFall();
    //#if DEBUGINO == 1
    //  Serial.print("Orientation: ");Serial.println(orientation);
    //#endif
}
#endif
