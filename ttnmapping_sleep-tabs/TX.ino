void transmit(){
  setupLora();

  #if DEBUGINO == 1
    uptime = millis(); // Ok only for normal code (not sleepy)
  #else
    uptime += millis(); // Ok only for sleepy code (millis are resetting)
  #endif
  
  if ( uptime - lastTXtime >= DAY ) {
    days++; // We have one day more uptime.
    totalTXms = 0; // reset TX counter, we have a new day.
  }
  
   // lower bits 1-2, are for batC, low bits 3-7 are for TX seconds. Spare (last) bit for TODO (button)
   loraData[1] = ((totalTXms / 1000 ) << 2) | vbatC; // Make room for 2 bits. Add the bits 1+2 (aka: OR vbatC)

   // prepare data for ttn.
   /* case for unsigned int
   // loraData[3] = highByte(days); // MSB
   // loraData[4] = lowByte(days); // LSB
   */
   loraData[2] = days; // for 255 days.
   loraData[3] = txPower; // TODO -1 to 20dBm, not -80 to 
   loraData[4] = uptime / 1000 / 60 / 60; // convert ms to hours (24hours = 5bits) (see blinkLed)

  // add some chaos. (when to send data / how much to wait. vbat = 500*- (* to verify) to ~680.
  #if CHAOS == 1
    uint8_t timeu = micros(); // grab last numbers only
    uint8_t vbatOver = vbat; // add some overflow to vbat, otherwise we have always downfall...
  #endif
  #if DEBUGINO == 1 & CHAOS == 1
    Serial.print("Timeu: " );Serial.println(timeu);
    Serial.print("uptime: " );Serial.println(uptime);
  #endif

  #if CHAOS == 1
    uint16_t randMS = random(0,64) + vbatOver >> 2 + timeu >> 2;
    #else
    uint16_t randMS = 0; // disabling chaos.
  #endif
  
  #if DEBUGINO == 1 // & CHAOS == 1 // prefer to see the emptyness of chaos.
    Serial.print("Chaos ms: ");Serial.println(randMS);
  #endif

  startTXms = millis(); // count transmission duration. This is start. // DONT MOVE
  #if PHONEY == 1
    delay(680); // emulate SF11
  #else
    lora.sendData(loraData, sizeof(loraData), fc, FramePort);
  #endif

  // Store the time of last TX.
  // Bear in mind that when we have DAY>0 we have to calculate the modulo.
  // lastTXtime = uptime % DAY; // valid for delay without sleep.
  
  lastTXtime = millis();

  fc++;

  // calculate the total of transmission duration. This is the end.
  currentTXms = lastTXtime - startTXms;
  totalTXms += currentTXms;

  #if LED == 1
    blinkLed(7, 20, 250); // short fast blinks for notify we send a lora data.
  #endif

  #if DEBUGINO == 1
    Serial.print(F("Sent, sleeping for minutes: ")); Serial.println((secondsSleep / 60) + (randMS * 10) / 1000);
    
    /*
     * CHAOS deviation is 3.84 seconds.
     * Example: secondsSleep = 100 (1 minutes 40 secs), without CHAOS: (100 + 0) * 20 / 1000 = 2secs between TX. 
     * 2nd Example (worst scenario with CHAOS: secondsSleep = 100 (1 minutes 40 secs), with CHAOS: (100 +  192) * 20 / 1000 = 5.84secs between TX.
     */
    // EVAL: integer overflow
    // Serial.println((secondsSleep * 20 / 60, 2); // 20 = blinks, 60 = divide seconds to find minutes.
    // Serial.println(secondsSleep * 0.3, 2); // 20 = blinks, 60 = divide seconds to find minutes = * 0.3
    Serial.print("uptime: " );Serial.println(uptime);
    Serial.print("lastTXtime: " );Serial.println(lastTXtime);
    Serial.print("totalTXms: " );Serial.println(totalTXms);
    Serial.print(F("ms transmitted: ")); Serial.println(currentTXms);
  #endif

  #if LED == 2
    blinkLed(blinks, 30, 5970 + randMS); // blink every 5 seconds
   // blinkLed(10, 30, (((secondsSleep * 1000 + randMS) - 30)) / 10); // blink 10 times, duration blink (ms), pause (ms)
  #else
    delay(secondsSleep * 1000); // delay works with ms, so multiply with 1000
  #endif
}
