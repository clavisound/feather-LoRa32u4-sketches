void transmit(){
  setupLora();

  #if DEBUGINO == 1
    uptime = millis();  // Ok only for normal code (not sleepy)
  #else
    uptime += millis(); // Ok only for sleepy code (millis are resetting)
  #endif
  
  if ( uptime - lastTXtime >= DAY ) { // we have a new day
    days++;        // one day more uptime.
    totalTXms = 0; // reset TX counter, we have a new day.
    uptime = 0;    // reset uptime. There is a better solution?
  }

   // prepare data for ttn. Lower bits 1-2, are for batC, low bits 3-7 are for TX seconds. Spare (last) bit for TODO (button)
   loraData[1] = ((totalTXms / 1000 ) << 2) | vbatC; // Make room for 2 bits. Add the bits 1+2 (aka: OR vbatC)
   loraData[2] = days;                               // max 255 days.
   loraData[3] = txPower;                            // TODO -1 to 20dBm, not -80 to 20
   loraData[4] = uptime / 1000 / 60 / 60;            // convert ms to hours (24hours = 5bits) (see blinkLed)

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

  startTXms = millis(); // count transmission duration. This is start. // DONT MOVE
  #if PHONEY == 1
    delay(680); // emulate SF11
  #else
    lora.sendData(loraData, sizeof(loraData), fc, FramePort);
  #endif

  // Store the time of last TX.
  // Bear in mind that when we have DAY>0 we have to calculate the modulo.
  // lastTXtime = uptime % DAY; // valid for delay without sleep.
  lastTXtime = millis(); // valid for sleep (millis are resetting)

  fc++;

  // calculate the total of transmission duration. This is the end.
  currentTXms = lastTXtime - startTXms;
  totalTXms += currentTXms;

  #if LED == 1
    blinkLed(7, 10, 250); // short fast blinks for notify we send a lora data.
  #endif

  #if DEBUGINO == 1
    Serial.print(F("Sent, sleeping for minutes: ")); Serial.println((secondsSleep / 60) + (randMS * 10) / 1000);
    Serial.print("uptime: " );Serial.println(uptime);
    Serial.print("lastTXtime: " );Serial.println(lastTXtime);
    Serial.print("totalTXms: " );Serial.println(totalTXms);
    Serial.print(F("ms transmitted: ")); Serial.println(currentTXms);
  #endif

  #if LED == 2
    blinkLed(blinks, 1, 8000); // blink every 8 seconds
  #endif
  
  #if LED == 0 & DEBUGINO == 1
    delay(secondsSleep * 1000); // delay works with ms, so multiply with 1000
  #endif
  
  // EVAL Sleep without LED
  #if LED <= 1 & DEBUGINO == 0
    times = blinks;
    for ( times > 0; times--; ) {
     sleepMS = Watchdog.sleep(8000);  // Sleep for up to 8 seconds
     uptime += sleepMS;
     // EVAL
     sleepMS = Watchdog.sleep(randMS);  // Sleep for random time
     uptime += sleepMS;
    }
  #endif
}
