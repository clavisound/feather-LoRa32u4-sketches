// LIS3DH accelorometer setup and functions. (you need my library)
#if LISDH == 1

#define LOW_POWER

void setupLIS(){
  
  if( lis.begin(LIS_RATE, 1, 1, 1, LIS_RANGE) != 0 )
  {
    #if DEBUGINO == 1
      Serial.print(F("LIS3DH failed.\n"));
    #endif
  }
  else
  {
    #if DEBUGINO == 1
      Serial.print(F("LIS3DH on.\n"));
    #endif
  }

  // Confirm configuration:
  //lis.readRegister(&temp, LIS3DH_INT1_CFG);
  //lis.readRegister(&temp, LIS3DH_INT2_CFG);

  // Get the ID:
  #if DEBUGINO == 1
    lis.readRegister(&temp, LIS3DH_WHO_AM_I);
    Serial.print("Who am I? 0x");
    Serial.println(temp, HEX);
  #endif

  //lis.wakeUpInertialAN();
  
  readLIS(); // clear INT
}

void readIRQ(){
  #if DEBUGINO == 1
    Serial.print("\n* readIRQ\n");
  #endif
  
  if ( interruptEvent > 0 ) { // nornally > 0 but LIS3DH fires two INTerrupts on setup!
    #if DEBUGINO == 1
      Serial.print("# of events: ");Serial.println(interruptEvent);
    #endif
    
    interruptEvent = 0;
    checkPin();
  }
}

void readLIS(){
  evntLis = lis.readEvent();       // clear IRQ with read on INT1_SRC
  
  if ( evntLis & 0b01000000 ) {    // latched event
  #if DEBUGINO == 1
    Serial.print("Event: ");Serial.print(evntLis);
  #endif
  
  #if DEBUGINO == 1
    Serial.print("\t");
    if ( ( XHIGH & evntLis ) == XHIGH  ) Serial.print("XH ");
    if ( ( XLOW  & evntLis ) == XLOW   ) Serial.print("XL ");
    if ( ( YHIGH & evntLis ) == YHIGH  ) Serial.print("YH ");
    if ( ( YLOW  & evntLis ) == YLOW   ) Serial.print("YL ");
    if ( ( ZHIGH & evntLis ) == ZHIGH  ) Serial.print("ZH ");
    if ( ( ZLOW  & evntLis ) == ZLOW   ) Serial.print("ZL ");
  #endif

    int16_t dataHighres = 0;

    if( lis.readRegisterInt16( &dataHighres, LIS3DH_OUT_X_L ) != 0 )
    {
      errorsAndWarnings++;
    }
    #if DEBUGINO == 1
     Serial.print(F("RAW: "));
     Serial.print(dataHighres);Serial.print(F(", "));
    #endif

    if( lis.readRegisterInt16( &dataHighres, LIS3DH_OUT_Y_L ) != 0 )
     {
      errorsAndWarnings++;
    }
    #if DEBUGINO == 1
      Serial.print(dataHighres);Serial.print(F(", "));
    #endif

    if( lis.readRegisterInt16( &dataHighres, LIS3DH_OUT_Z_L ) != 0 )
    {
      errorsAndWarnings++;
    }
   
   #if DEBUGINO == 1
    Serial.print(dataHighres);
    Serial.print(F(" millis: "));Serial.println(millis() / 1000);
   #endif

   #if LED == 3
    ledDEBUG(5, 3, 997);
   #endif
   
    }
    
    // check tap
    evntLis = lis.readTap(); // CLICK_SRC
    
    #if DEBUGINO == 1 // TODO: go there only if eventLis > 0
      if ( ( TAPINT  & evntLis ) == TAPINT   ) Serial.print(F("Tap Interrupt. ")); 
      if ( ( XTAP    & evntLis ) == XTAP     ) Serial.print(F("Xtap "));
      if ( ( YTAP    & evntLis ) == YTAP     ) Serial.print(F("Ytap "));
      if ( ( ZTAP    & evntLis ) == ZTAP     ) Serial.print(F("Ztap "));
      if ( ( SNGTAP  & evntLis ) == SNGTAP   ) Serial.print(F("Single tap "));
      if ( ( DBLTAP  & evntLis ) == DBLTAP   ) Serial.println(F("Double tap "));
      if ( ( TAPSIGN & evntLis ) == TAPSIGN  ) Serial.print(F("- "));
      Serial.print(F("* LisTap: "));Serial.println(evntLis);
    #endif

   Serial.println();

/*
   if ( evntLis > 0 ) {
    #if DEBUGINO == 1
      Serial.print(F("Go to GPS: "));
    #endif
      checkFix();
    }
   
   evntBuf++;

   #if DEBUGINO == 1
      Serial.print("evntBuf: ");Serial.println(evntBuf);
   #endif

   if ( evntBuf > 3 ) {
    evntBuf = 0; // This is a hack for the LIS3DH
    checkFix();
   }
   */
}

//uint8_t checkFall(){}

void enableTAP(){
  // RESULTS
  // EVAL with bike 32 for ride and fall detection
  // EVAL with bike  8 for park.
  lis.configTap(1,1,1,1,1,1,8); // Enable Z2,Z1,Y2,Y1,X2,X1,threshold [default 10, 7bit]

  lis.autoSleep(16, 64);        // theshold 3 [7bit], time-to-sleep-and-to-wake 120 [8bit]

  // RESULTS
  // Rate 50 Hz
  // lis.intConf(INT_1, BIKE_FALL, 40, 1, 100);
  // lis.autoSleep(16, 64); 
  // INT 100 = fire every 10 secs.  With motion fires for 10 secs every 2 secs.
  // INT 127 = fire every 13 secs. With motion fires for 10 secs every 2 secs.

  // Rate 25 Hz
  // lis.intConf(INT_1, BIKE_FALL, 40, 1, 127);
  // lis.autoSleep(32, 64);
  // INT 127 = fire every 14 secs. Wakes and fires for 10 secs every 2 secs.

  #if DEBUGINO == 1
    Serial.println(F("EN-able TAP"));
    lis.readRegister(&temp, LIS3DH_CLICK_CFG);
    Serial.print("CLICK_CFG: ");Serial.print(temp, HEX);Serial.print(", ");Serial.println(temp, BIN);
  #endif
}

void disableTAP(){
  #if DEBUGINO == 1
    Serial.println(F("* DIS-able TAP"));
  #endif
  lis.configTap(0,0,0,0,0,0,8); // Enable Z2,Z1,Y2,Y1,X2,X1,threshold [default 10, 7bit]
}

void setupXYZevents(){ // EVAL setupXYZevents(INT_0); for disable
  //Detection threshold can be from 1 to 127 and depends on the Range
  //chosen above, change it and test accordingly to your application
  //Duration [7bit] = timeDur x Seconds / sampleRate.
  // 80 with 10Hz = 8 secs, 127 MAX = 13secs
  //  8 with 1Hz = 10 secs,  32 = 35 secs, 64 = ~64secs, 127 MAX = 130secs (MAX)
  
  // WAS OK
  //lis.intConf(INT_1, BIKE_FALL, 40, 32, 1); // default (INT_1, DET_MOVE, 13, 0, 2) 2 = seconds, 1 = polarity

  // it works even
   lis.intConf(INT_1, BIKE_FALL, 40, 1, 32); // default (INT_1, DET_MOVE, 13, 0, 2)  1 = polarity, 2 = seconds,
  
  //lis.intConf(INT_2, DET_STOP, 13, 10); // adafruit has only one INT :(

  #if DEBUGINO == 1
    Serial.println(F("* Enable XYZ"));
    lis.readRegister(&temp, LIS3DH_INT1_CFG);
    Serial.print("INT_CFG: ");Serial.print(temp, HEX);Serial.print(", ");Serial.println(temp, BIN);
  #endif
}

void checkPin(){
  #if DEBUGINO == 1
   Serial.println("* checkPin");
  #endif
   readLIS();
  
  if ( digitalRead(INT_PIN_ELEVEN) == 0 ) { // blink on connection.
   #if DEBUGINO == 1
    Serial.println(F("\nGND pin #11\n"));
   #endif
   #if LED == 3
    ledDEBUG(1, 6000, 200);
   #endif
  }
}

#endif
