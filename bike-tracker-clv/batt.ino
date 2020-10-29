void checkBatt(){
  Serial.print(F("\n*BAT"));

    vbat = analogRead(VBATPIN) - 450; // store to 8bit
    
    #if LORA_VERB == 1
      loraData[0] = vbat;
    #endif

    if ( loraSize == LORA_HEARTBEAT ) {
      if ( vbat < 90 ) {
        loraData[0] = 0;
      } else if ( vbat > 200 ) {  // overflow measurement, we have full battery or charging.
        loraData[0] = 10;         // max: 5 bits
      } else {
        loraData[0] = map(vbat, 80, 200, 0, 10);  // map 30 -- 150 to 0 -- 10. must fit to 4 bits.
      }
      if ( ! fix.valid.location ) { loraData[0] |= 0x80; } // mark that we don't have gps
    }
    
    /*
    vbat *= 2;    // we divided by 2, so multiply back
    vbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    vbat /= 1024; // convert to voltage
    all in one: vbat *=0.0064453125
    */
    
    // vbat: Connected to USB 4.29Volts, vbat operating: 688MAX
    // vbat charging: 3.9, 4.1, 4.5 (699)

    // map to 0, 40, 60, 80% baterry charging levels, EVAL: check for lowest level and highest level for fauny.
    // IMPORTANT: 465.5 = 3.0Volt. < DON'T GO THERE! Those are safe: 480 (30) = 3.1Volts, 496 (46) = 3.2 volts
    // Capacity measured with voltage is not linear! https://learn.adafruit.com/assets/979
    // 4.24V (208) is 100%, 3.95V is 80%, 3.8V = 60%, 3.75 = 40%, 3.7Volt = 20%

    if ( vbat < 80 ) {
      vbatC = 0;
    }
    else {
      vbatC = map(vbat, 80, 200, 0, 3);
    }

    if ( vbatC > 3 ) { vbatC = 3; }  // sometimes vbat is > 635 (aka: 185 after 8bit conversion) and we have overflow.

    FramePort |= vbatC << 2;         // vbatC to FramePort. 00001100 = 3
    
    #if DEBUGINO == 1
      Serial.print(F("* VBat (8bit): ")); Serial.print(vbat);Serial.print(F(", VBatB (volt): ")); Serial.print((vbat + 450) * 0.0064453125);
      Serial.print(F(", VBatC (range): ")); Serial.println(vbatC);
      Serial.print(F("\n*Bat HEART lora: "));Serial.print( loraData[0] );
      Serial.print(F("value, "));Serial.println( map(vbat, 80, 200, 0, 10) );
    #endif
}
