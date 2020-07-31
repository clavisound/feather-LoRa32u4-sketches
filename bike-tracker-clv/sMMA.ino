// MMA8452 accelorometer setup and functions. (you need my library)

#if MMA8452 == 1
void setupMMA(){
  if (accel.begin() == false) {
      #if LED == 1
        blinkLed(40, 50, 100); // fast blink for MMA error
      #endif
    goToSleep(); // hang for ever.
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
  accel.setupMotion(1,0,7,1,1);

  accel.setupTap(0x10, 0x10, 0x80);                  // enable taps on X, Y, Z 0x80 to disable
  
  //accel.setupPL(0x05);                               // debounce rate, default 0x50
  accel.setupPL(0x50);                               // debounce rate, default 0x50

  // EVAL
  //accel.lowPowerAN();

  accel.enableEvents(0,0,0,1,0);                     // sleep, transient, orientation, tap, motion
  accel.interruptPin(0,0,0,0,0,1,0);                 // sleep, transient, orientation, tap, motion, polarity, opendrain
                                                     // 1 = pin1 0 = pin2.
  accel.setDataRate(ODR_12);
  
  //accel.lowPowerEVAL();
  
  #endif

  pinMode(INT_PIN_TEN, INPUT);            // Pin for INT #1
  pinMode(INT_PIN_ELEVEN, INPUT);         // Pin for INT #2
  digitalWrite (INT_PIN_TEN, LOW);        // disable pull-up
  digitalWrite (INT_PIN_ELEVEN, LOW);     // disable pull-up
  
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
       ledDEBUG(3,10,10);
     #endif
   }
   #if DEBUGINO == 1
     Serial.print("interruptEvents: ");Serial.print(interruptEvent);
     Serial.print(", Wake or sleep: ");Serial.print(accel.wakeOrSleep());
     Serial.print(", Interrupt Reason: ");Serial.print(evntMMA);
     Serial.print("\nUptime: ");Serial.println(millis() / 1000);
   #endif
   interruptEvent = 0;
}

void readIRQMMA(){
  if ( interruptEvent > 0 ) {
   readMMA();
  }
}

uint8_t checkFall(){
   // first thing to do: check if we don't transmitted according to secondsSleep
   #if GPS == 1
     if ( GPStime() - endTXtime < secondsSleep ) {          // continue to sleep.        
   #else
    if ( uptime - endTXtime < secondsSleep ) {          // continue to sleep.
   #endif
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
    if ( interruptEvent > 0 ) {
      interruptEvent = 0;       // we woke up.
      disablePinChange();       // disable Interrupts
    }

    // TODO if checkFall not UP send message.
    // checkFall();
    //#if DEBUGINO == 1
    //  Serial.print("Orientation: ");Serial.println(orientation);
    //#endif
}

void readIRQ(){
  #if DEBUGINO == 1
    // Serial.print("\n* readIRQ\n");
  #endif
  
  if ( interruptEvent > 0 ) { // nornally > 0 but LIS3DH fires two INTerrupts on setup!
    #if DEBUGINO == 1
      Serial.print("# of events: ");Serial.println(interruptEvent);
    #endif
    
    interruptEvent = 0;
    checkPin();
  }
}

void checkPin(){
  #if DEBUGINO == 1
   Serial.println("* checkPin");
  #endif
  if ( digitalRead(INT_PIN_TEN) == 1 ) { // blink on connection.
   #if DEBUGINO == 1
    Serial.println(F("\npin #10 INT1\n"));
   #endif
   #if LED == 3
    ledDEBUG(1, 1000, 200);
   #endif
   #if LED == 1
    ledDEBUG(1,10,0);
   #endif
  }
  if ( digitalRead(INT_PIN_ELEVEN) == 1 ) { // blink on connection.
   #if DEBUGINO == 1
    Serial.println(F("\npin #11 INT2\n"));
   #endif
   #if LED == 3
    ledDEBUG(1, 5000, 200);
   #endif
   #if LED == 1
    ledDEBUG(2,10,10);
   #endif
  }
  readMMA();
}
#endif
