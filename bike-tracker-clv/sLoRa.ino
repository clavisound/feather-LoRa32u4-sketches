void setupLora()
{   
  // Initialize LoRa
  #if DEBUGINO == 1
    Serial.print("\n* LoRa...");
  #endif
  lora.setChannel(MULTI); // define multi-channel sending or CH0, CH1 to CH7

  // TX times as counted by this program. (Yes I also found some results* bizzare)
  // (for 4 bytes) 7:  58ms, 8:  99ms  9: 172ms 10: 337ms  11: 667ms* 12: 1325ms
  // (for 2 bytes) 7:  53ms, 8: 102ms* 9: 172ms 10: 338ms* 11: 665ms  12: 1161ms
  // (for 5 bytes) 7:  85ms,

  /* calculate TX's per day for 4 bytes payload.
   *  (30000ms / 353ms) = how many times per day?
   *  calculate the distance in minutes
   *  1440 minutes / (30000ms / 353ms) = 17 minutes for SF10
   *  rough numbers.
   *  SF7  every  3.5 minutes or 375 messages per day. 
   *  SF8  every  5.5 minutes or 260 messages per day.
   *  SF9  every  9   minutes or 150 messages per day.
   *  SF10 every 17   minutes or  83 messages per day.
   *  SF11 every 32   minutes or  43 messages per day.
   *  SF12 every 64   minutes or  22 messages per day. 
   */

   #if TWOSF == 1
     // #if GPS == 1
       if ( totalTXms & 0x01 ) { // if we have odd second select SFB
        #if DEBUGINO == 1
          Serial.print("\n* SFB: ");Serial.println(totalTXms);
        #endif
        #if SFB == 12
          lora.setDatarate(SF12BW125);
        #elif SFB == 11
          lora.setDatarate(SF11BW125);
        #elif SFB == 10
          lora.setDatarate(SF10BW125);
        #elif SFB == 9
          lora.setDatarate(SF9BW125);
        #elif SFB == 8
          lora.setDatarate(SF8BW125);
        #else
          lora.setDatarate(SF7BW125);
        #endif // SFB
        } else { // we have even second, select SF
          #if DEBUGINO == 1
            Serial.println("\n* SF(a)\n");
          #endif
        #if SF == 12
          lora.setDatarate(SF12BW125);
        #elif SF == 11
          lora.setDatarate(SF11BW125);
        #elif SF == 10
          lora.setDatarate(SF10BW125);
        #elif SF == 9
          lora.setDatarate(SF9BW125);
        #elif SF == 8
          lora.setDatarate(SF8BW125);
        #else
          lora.setDatarate(SF7BW125);
        #endif // SF
       }
     // #endif // GPS
     #else // MULTISF != 1
      #if SF == 12
        lora.setDatarate(SF12BW125);
      #elif SF == 11
        lora.setDatarate(SF11BW125);
      #elif SF == 10
        lora.setDatarate(SF10BW125);
      #elif SF == 9
        lora.setDatarate(SF9BW125);
      #elif SF == 8
        lora.setDatarate(SF8BW125);
      #else
        lora.setDatarate(SF7BW125);
      #endif // SF
      
   #endif // MUTLTISF
  
  if(!lora.begin())
  {
    #if DEBUGINO == 1
      Serial.println(F("LoRa Failed"));
    #endif
    while(true);
  }
  #if DEBUGINO == 1
    Serial.println(" ok");
  #endif

  // ATTN: it hangs if it's *BEFORE* lora.begin
  lora.setPower(txPower); // EU limit is 14dBm
}
