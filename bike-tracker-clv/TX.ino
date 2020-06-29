void transmit(){
   noFixCount = 0; // just in case
  
  // EVAL disable INT1 + INT2. So in case of transmission, we don't have INTerruption.
  #if LISDH == 1 | MMA8452 == 1
     //disablePinChange();
     //readIRQ();
  #endif

  #if GPS == 1
    if ( uptimeGPS - lastTXtime >= DAY ) {   // check if uptime in days is different
      // TODO maybe we slept more that one DAY. check GPS for that.
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
     loraData[4] = uptimeGPS / 60 / 60;                 // convert seconds to hours (24hours = 5bits) (see blinkLed)
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
    lastTXtime = uptimeGPS;            // store the lastTXtime
  #elif DEBUGINO == 0 & GPS == 1
    endTXtime = millis();
    lastTXtime = uptimeGPS;            // store the lastTXtime
  #else
    endTXtime = millis();              // valid for sleep (millis are resetting)
  #endif

  #if LED == 3
   ledDEBUG(7,30,100);
  #endif
  
  fc++;

  // calculate the total of transmission duration. This is the end.
  currentTXms = endTXtime - startTXms - 15; // TinyLoRa setup has delay 10 + 5
  totalTXms += currentTXms;

  #if LED == 1
    blinkLed(7, 30, 250); // short fast blinks for notify we send a lora data.
  #endif

  #if DEBUGINO == 1
    Serial.print(F(" s: ")); Serial.println(secondsSleep);
    Serial.print("endTXtime: " );Serial.println(endTXtime);
    Serial.print("totalTXms: " );Serial.println(totalTXms);
    Serial.print(F("ms transmitted: ")); Serial.println(currentTXms);
    Serial.print(F("HEX: "));
    for ( uint8_t counter = 0; counter < loraSize; counter++ ) {
        Serial.print(loraData[counter], HEX);Serial.print(F(" "));
      }
    Serial.println();
      
    printDebug();
  #endif

  // EVAL enable INT1 + INT2.
  
  // Stuck here...
  toBeOrNotToBe();             // decide to sleep or to wait.
}
