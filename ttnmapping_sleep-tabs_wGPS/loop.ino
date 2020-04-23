void loop()
{
    #if CYCLESF == 0
      checkgps();
    #endif

    #if CYCLESF == 1
      checkgps(); // SF already set in setup.
      lora.setDatarate(SF9BW125);setLed(3);checkgps();
      lora.setDatarate(SF8BW125);setLed(2);checkgps();
      lora.setDatarate(SF10BW125);setLed(4);checkgps();
    #endif

    #if CYCLESF == 2
      checkgps(); // SF already set in setup.
      totalTXms = TXMS + 1; // fake TXms to send only once per day.
    #endif

    #if CYCLESF == 3
      lora.setDatarate(SF7BW125);setLed(1);checkgps();
      lora.setDatarate(SF8BW125);setLed(2);checkgps();
      lora.setDatarate(SF9BW125);setLed(3);checkgps();
      lora.setDatarate(SF10BW125);setLed(4);checkgps();
    #endif

    #if CYCLESF == 4
      lora.setDatarate(SF10BW125);setLed(4);checkgps();
      lora.setDatarate(SF11BW125);setLed(5);checkgps();
      lora.setDatarate(SF12BW125);setLed(6);checkgps();
    #endif
}
