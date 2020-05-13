void transmit(){
  // EVAL disable INT1 + INT2. So in case of transmission, we don't have INTerruption.

  #if MMA8452 == 1 | LISDH == 1
    //disablePinChange();
  #endif

  #if GPS == 1
    //GPSuptime(); // Wave time from fix.
    currentTime = fix.dateTime - bootTime;
  #else
    updUptime();
  #endif

  #if GPS == 1
    if ( currentTime - lastTXtime >= DAY ) { // we have a new day // BUG / EVAL I think this wrong!
      days++;                                // one day more uptime. // BUG / EVAL I think this wrong!
      totalTXms = 0;                         // reset TX counter, we have a new day.
    }
  #else
    if ( uptime - endTXtime >= DAY ) {    // we have a new day
      days++;                             // one day more uptime.
      totalTXms = 0;                      // reset TX counter, we have a new day.
      uptime = 0;                         // reset uptime. There is a better solution?
    }
  #endif

   setupLora();
   // prepare data for ttn. Lower bits 1-2, are for batC,
   // low bits 3-7 are for TX seconds. Spare (last) bit for TODO (button)

   // totalTXms needs 5 bits
   loraData[1] = ((totalTXms / 1000 ) << 2) | vbatC; // Make room for 2 bits. Add the bits 1+2 (aka: OR vbatC)
   loraData[2] = days;                               // max 255 days.
   loraData[3] = txPower;                            // TODO -1 to 20dBm, not -80 to 20
   
   #if GPS == 1
     loraData[4] = currentTime / 60 / 60;                 // convert seconds to hours (24hours = 5bits) (see blinkLed)
   #else
     loraData[4] = uptime / 1000 / 60 / 60;          // convert ms to hours (24hours = 5bits) (see blinkLed)
   #endif
   
   // loraData of GPS data are in GPS tab.
   
  // add some chaos. (when to send data / how much to wait. vbat = 500*- (* to verify) to ~680.
  #if CHAOS == 1
    uint8_t timeu = micros(); // grab last numbers only
    randMS = random(0,64) + timeu;
  #else
    uint16_t randMS = 0; // disabling chaos.
  #endif

  #if DEBUGINO == 1 & CHAOS == 1
    Serial.print("\nTimeu: " );Serial.print(timeu);Serial.print(", randMS: " );Serial.println(randMS);
  #endif

  startTXms = millis(); // count transmission duration. This is start. DONT MOVE
  #if PHONEY == 1
    delay(680); // emulate SF11
  #else
    lora.sendData(loraData, loraSize, fc, FramePort);
  #endif

  // Store the time of last TX.
  // Bear in mind that when we have DAY>0 we have to calculate the modulo.
  #if DEBUGINO == 1 & GPS == 0
    //endTXtime = uptime % DAY;        // valid for delay without sleep. Get the remainder to have DAYtime
    endTXtime = millis() % DAY;        // valid for delay without sleep. Get the remainder to have DAYtime
  #elif DEBUGINO == 1 & GPS == 1
    endTXtime = millis();
    lastTXtime = GPStime() - bootTime;  // Get the remainder to have DAYtime
  #elif DEBUGINO == 0 & GPS == 1
    endTXtime = millis();
    lastTXtime = GPStime() - bootTime;  // Get the remainder to have DAYtime
  #else
    endTXtime = millis();              // valid for sleep (millis are resetting)
  #endif

  fc++;

  // calculate the total of transmission duration. This is the end.
  currentTXms = endTXtime - startTXms - 15; // TinyLoRa setup has delay 10 + 5
  totalTXms += currentTXms;

  #if LED == 1
    blinkLed(7, 20, 250); // short fast blinks for notify we send a lora data.
  #endif

  #if DEBUGINO == 1
    if ( speed == 0 ) {
      Serial.print(F("\nSent, sleeping for minutes: "));
      Serial.print((secondsSleep / 60)); 
      } else { Serial.print(F("\nMCU OFF"));
      }
    Serial.print(F(" s: ")); Serial.println(secondsSleep);
    Serial.print("uptime: " );Serial.println(uptime);
    #if GPS == 1
      Serial.print("lastTXtime: " );Serial.println(lastTXtime);
    #endif
    Serial.print("endTXtime: " );Serial.println(endTXtime);
    Serial.print("totalTXms: " );Serial.println(totalTXms);
    Serial.print(F("ms transmitted: ")); Serial.println(currentTXms);
    Serial.print(F("HEX: "));
    for ( uint8_t counter = 0; counter < loraSize; counter++ ) {
        Serial.print(loraData[counter], HEX);Serial.print(F(" "));
      }
      Serial.println();
  #endif

  // TODO MAKE IT BETTER with #if's

  // EVAL enable INT1 + INT2.
  #if ( MMA8452 == 1 | LISDH == 1 ) & DEBUGINO == 1
    
    // Stuck here...
    toBeOrNotToBe();             // decide to sleep or to wait.
  #endif

  #if LED == 2 & DEBUGINO == 0
    blinkLed(blinks, 1, 8000); // blink every 8 seconds
  #endif

  // TODO check this option
  //#if LED == 0 & DEBUGINO == 1
  //  delay(secondsSleep * 1000); // delay works with ms, so multiply with 1000
  //#endif
  
  // EVAL Sleep without LED
  #if LED == 0 & DEBUGINO == 0 & ( LISDH == 0 | MMA8452 == 0 )
    toBeOrNotToBe();             // decide to sleep or to wait.
  #endif

  #if LED == 0 & DEBUGINO == 0 & ( LISDH == 1 | MMA8452 == 1 )
    delay(secondsSleep * 1000); // delay works with ms, so multiply with 1000
    goToSleep();
  #endif 
}
