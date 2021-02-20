void transmit(){

  // FramePort setup from values 0: speedHigh, 1: fall (TODO), 2: batlsb, 3: batmsb, 4: lowqgps, 5: ttnmapper
  // check batt.ino, GPS.ino (twice)

  // hack to select a contradictory port if we have low speed AND no-fall AND low-battery AND high-HDOP AND no-ttnmapper
  // if FramePort is 0 because speedHigh = 0, fall = 0, bat = 0, lowqgps = 0 and ttnmapper = 0
  if ( FramePort == 0 ) { FramePort = 0x30; } // then select lowqgps = 1 and ttnmapper = 1 which means not ttnmapper but also not lowqgps!

  #if DEBUGINO == 1
    Serial.println("\n* TX");
  #endif
  
  #if GPS == 1
    #if INDOOR == 1
      delay(500);            // When debuging with INDOOR, led stays on, wait half second for LED to power off.
    #endif
      GPSsleep();            // Close GPS we are done
  #endif

  checkBatt();

  #if LORA_VERB == 1 & GPS == 1
    fixes += noFixCount / 60;                    // add noFixCount to fixes, convert to minutes.
    if ( loraSize == LORA_HEARTBEAT ) {          // we don't have GPS data
      #if DEBUGINO == 1
        Serial.println(F("\n* LORA_VERB"));
      #endif
      loraData[5] = fixes;
      }
    else {
      loraData[15] = fixes;    // we have GPS data.
      }
  #endif

  #if GPS == 1
    noFixCount = 0;             // reset the counters for gps fixes
  #endif
  
  // EVAL disable INT1 + INT2. So in case of transmission, we don't have INTerruption.
  #if LISDH == 1 | MMA8452 == 1
//     disablePinChange();
//     readIRQ();
   #endif

   #if GPS == 1
      days = uptimeGPS / DAY;             // calculate days.
   #else
    #if DEBUGINO == 1
      uptime = millis();                  // ok with no sleep (millis are not resetting)
    #endif
    #if DEBUGINO == 0
      uptime += millis();                 // valid for sleep (millis are resetting)
    #endif
    if ( uptime - endTXtime >= DAY ) {    // we have a new day
      days++;                             // one day more uptime.
      uptime = 0;                         // reset uptime. There is a better solution?
    }
   #endif

   // We have a new DAY, reset the TX counter
   if ( days != loraData[2] ) {
    #if DEBUGINO == 1
      Serial.println("reset totalTXms");
    #endif
    totalTXms = 0;
    } // reset TX counter, we have a new day.
    
   setupLora();
   
   // prepare data for ttn. Lower bits 1-2, are for batC,
   // low bits 3-7 are for TX seconds. Spare (last) bit for TODO (button)
   // totalTXms needs 5 bits
   #if LORA_VERB == 1
     loraData[1] = ((totalTXms / 1000 ) << 2);         // Make room for 2 bits.
     loraData[2] = days;                               // Days: max 255 days. TODO: better weeks with 63 max (6bits).
     if ( txPower == -80 ) {
        loraData[3] = 31;
      } else {
        loraData[3] = txPower;                        // 5bits 0dBm - 20dBm
      }
   #endif
   
   #if GPS == 1
     #if LORA_VERB == 1
       loraData[4] = ( uptimeGPS % DAY ) / 3600;       // convert seconds to hours [TODO: 24hours = 5bits (see blinkLed)]
     #endif
   #else
     #if LORA_VERB == 1
       loraData[4] = uptime / 1000 / 60 / 60;          // convert ms to hours [TODO: 24hours = 5bits (see blinkLed)]
     #endif
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
    delay(500);   // emulates SF10 with 30 bytes

  #else
    lora.sendData(loraData, loraSize, fc, FramePort);
  #endif

  // Store the time of last TX.
  // Bear in mind that when we have DAY>0 we have to calculate the modulo.
  #if DEBUGINO == 1 & GPS == 0
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

  // calculate the total of transmission duration. This is the end.
  currentTXms = endTXtime - startTXms - 15; // TinyLoRa setup has delays 10 + 5
  totalTXms += currentTXms;                 // Add to the total TX time

  #if LED >= 1
    ledDEBUG(15, 20, 80); // short fast blinks for notify we send a lora data.
  #endif

  #if DEBUGINO == 1
    Serial.print(F(" s: ")); Serial.println(secondsSleep);
    Serial.print("endTXtime: " );Serial.println(endTXtime);
    Serial.print("totalTXms: " );Serial.println(totalTXms);
    Serial.print(F("ms transmitted: ")); Serial.println(currentTXms);
    Serial.print(F("HEX: "));
    for ( uint8_t counter = 0; counter < loraSize; counter++ ) {
      if ( loraData[counter] < 0x11 ) Serial.print("0"); // Add one zero if value is 0-F
        Serial.print(loraData[counter], HEX);Serial.print(F(" "));
      }
    Serial.println();
    Serial.print(F("P: "));Serial.println(FramePort);
      
    printDebug();
  #endif

    fc++; // Increase FrameCounter

  // EVAL enable INT1 + INT2.
  #if LISDH == 1 | MMA8452 == 1
     //enablePinChange();
     //readIRQ();
  #endif
  
  FramePort   = 0;             // Reset FramePort to zero. Life goes on. Or not?
  
  // loraData[0] = 0;             // This is used for several things, erase it. BUG: interferes with TXms
  // TXms.ino: if ( fix.dateTime - GPS_old_time > 30 || fix.hdop == 0 )
  
  toBeOrNotToBe();             // decide to sleep or to wait.
}
