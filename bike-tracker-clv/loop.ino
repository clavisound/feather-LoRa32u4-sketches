#if GPS == 1 & (LISDH == 1 | MMA8452 == 1)
void loop()
{
    #if CYCLESF == 0
     readIRQ();                       // clear events
     #if DEBUGINO == 1
       Serial.println("\n** Loop");
     #endif
     checkFix();
     readIRQ();                       // Read events from accelerometer TODO: MASK events to variable.
     GPStime();updUptime();           // Get new time and store it to uptime.
     checkTXms();
    #endif

    #if CYCLESF == 1
      checkFix(); // SF already set in setup.
      lora.setDatarate(SF9BW125);setLed(3);checkFix();
      lora.setDatarate(SF8BW125);setLed(2);checkFix();
      lora.setDatarate(SF10BW125);setLed(4);checkFix();
    #endif

    #if CYCLESF == 2
      checkFix(); // SF already set in setup.
      totalTXms = TXMS + 1; // fake TXms to send only once per day.
    #endif

    #if CYCLESF == 3
      lora.setDatarate(SF7BW125);setLed(1);checkFix();
      lora.setDatarate(SF8BW125);setLed(2);checkFix();
      lora.setDatarate(SF9BW125);setLed(3);checkFix();
      lora.setDatarate(SF10BW125);setLed(4);checkFix();
    #endif

    #if CYCLESF == 4
      lora.setDatarate(SF10BW125);setLed(4);checkFix();
      lora.setDatarate(SF11BW125);setLed(5);checkFix();
      lora.setDatarate(SF12BW125);setLed(6);checkFix();
    #endif
}
#endif

#if GPS == 0 & (LISDH == 1 | MMA8452 == 1)
void loop()
{
    #if CYCLESF == 0
      readIRQ();
      checkTXms();
    #endif

    #if CYCLESF == 1
      checkTXms(); // SF already set in setup.
      lora.setDatarate(SF9BW125);setLed(3);checkTXms();
      lora.setDatarate(SF8BW125);setLed(2);checkTXms();
      lora.setDatarate(SF10BW125);setLed(4);checkTXms();
    #endif

    #if CYCLESF == 2
      checkTXms();// SF already set in setup.
      totalTXms = TXMS + 1; // fake TXms to send only once per day.
    #endif

    #if CYCLESF == 3
      lora.setDatarate(SF7BW125);setLed(1);checkTXms();
      lora.setDatarate(SF8BW125);setLed(2);checkTXms();
      lora.setDatarate(SF9BW125);setLed(3);checkTXms();
      lora.setDatarate(SF10BW125);setLed(4);checkTXms();
    #endif

    #if CYCLESF == 4
      lora.setDatarate(SF10BW125);setLed(4);checkTXms();
      lora.setDatarate(SF11BW125);setLed(5);checkTXms();
      lora.setDatarate(SF12BW125);setLed(6);checkTXms();
    #endif
}
#endif

#if GPS == 1 & (LISDH == 0 & MMA8452 == 0)
void loop()
{
    #if CYCLESF == 0
      checkFix();
    #endif

    #if CYCLESF == 1
      checkFix(); // SF already set in setup.
      lora.setDatarate(SF9BW125);setLed(3);checkFix();
      lora.setDatarate(SF8BW125);setLed(2);checkFix();
      lora.setDatarate(SF10BW125);setLed(4);checkFix();
    #endif

    #if CYCLESF == 2
      checkFix(); // SF already set in setup.
      totalTXms = TXMS + 1; // fake TXms to send only once per day.
    #endif

    #if CYCLESF == 3
      lora.setDatarate(SF7BW125);setLed(1);checkFix();
      lora.setDatarate(SF8BW125);setLed(2);checkFix();
      lora.setDatarate(SF9BW125);setLed(3);checkFix();
      lora.setDatarate(SF10BW125);setLed(4);checkFix();
    #endif

    #if CYCLESF == 4
      lora.setDatarate(SF10BW125);setLed(4);checkFix();
      lora.setDatarate(SF11BW125);setLed(5);checkFix();
      lora.setDatarate(SF12BW125);setLed(6);checkFix();
    #endif
}
#endif

#if GPS == 0 & LISDH == 0 & MMA8452 == 0
void loop()
{
    #if CYCLESF == 0
      checkTXms();
    #endif

    #if CYCLESF == 1
      checkTXms(); // SF already set in setup.
      lora.setDatarate(SF9BW125);setLed(3);checkTXms();
      lora.setDatarate(SF8BW125);setLed(2);checkTXms();
      lora.setDatarate(SF10BW125);setLed(4);checkTXms();
    #endif

    #if CYCLESF == 2
      checkTXms();// SF already set in setup.
      totalTXms = TXMS + 1; // fake TXms to send only once per day.
    #endif

    #if CYCLESF == 3
      lora.setDatarate(SF7BW125);setLed(1);checkTXms();
      lora.setDatarate(SF8BW125);setLed(2);checkTXms();
      lora.setDatarate(SF9BW125);setLed(3);checkTXms();
      lora.setDatarate(SF10BW125);setLed(4);checkTXms();
    #endif

    #if CYCLESF == 4
      lora.setDatarate(SF10BW125);setLed(4);checkTXms();
      lora.setDatarate(SF11BW125);setLed(5);checkTXms();
      lora.setDatarate(SF12BW125);setLed(6);checkTXms();
    #endif
}
#endif
