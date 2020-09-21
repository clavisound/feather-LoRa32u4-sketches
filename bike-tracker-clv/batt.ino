void checkBatt(){

    vbat = analogRead(VBATPIN) - 450; // convert to 8bit
    //#if LORA_VERB == 1
      loraData[0] = vbat; // loraData[0] = highByte(vbat);loraData[1] = lowByte(vbat);
    //#endif
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
    // 3.95V is 80%, 3.8V = 60%, 3.75 = 40%, 3.7Volt = 20%

    // 3.4 feather-clv raw: 77 (only LoRa) is off!
    if ( vbat < 29 ) {
      vbatC = 0;
    }
    else {
    vbatC = map(vbat, 30, 150, 0, 3);
    }
      
    if ( vbatC > 3 ){ vbatC = 3; } // sometimes vbat is > 635 (aka: 185 after 8bit conversion) and we have overflow.
    
    #if DEBUGINO == 1
      Serial.print(F("* VBat (8bit): ")); Serial.print(vbat);Serial.print(F(", VBatB (volt): ")); Serial.print((vbat + 450) * 0.0064453125);
      Serial.print(F(", VBatC (range): ")); Serial.println(vbatC);
    #endif
}
