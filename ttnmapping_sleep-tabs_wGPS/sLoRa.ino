void setupLora()
{
  // Initialize LoRa
  #if DEBUGINO == 1
    Serial.print("Starting LoRa...");
  #endif
  lora.setChannel(MULTI); // define multi-channel sending or CH0, CH1 to CH7

  // TX times as counted by this program. (Yes I also found some results* bizzare)
  // (for 4 bytes) 7: 74ms, 8: 114ms  9: 187ms 10: 352ms  11: 682ms* 12: 1340ms
  // (for 2 bytes) 7: 68ms, 8: 117ms* 9: 187ms 10: 353ms* 11: 680ms  12: 1176ms

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

  #if SF == 12
    lora.setDatarate(SF12BW125);
  #elif SF == 11
   lora.setDatarate(SF11BW125);
  #elif SF == 10
   lora.setDatarate(SF10BW125);
  #elif SF == 9
   lora.setDatarate(SF9W125);
  #elif SF == 8
   lora.setDatarate(SF8BW125);
  #else
    lora.setDatarate(SF7BW125);
  #endif
  
  if(!lora.begin())
  {
    #if DEBUGINO == 1
      Serial.println(F("Failed, Check your radio"));
    #endif
    while(true);
  }
  #if DEBUGINO == 1
    Serial.println("LoRa ok");
  #endif

  // ATTN: it hangs if it's *BEFORE* lora.begin
  lora.setPower(txPower); // EU limit is 14dBm
}
