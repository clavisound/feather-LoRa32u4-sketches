if (port === 3) {
  
  decoded.vbatRaw = bytes[0]; // raw battery - 450. Range 460 - 700
  decoded.vbatArea = bytes[1] & 3; // AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs = (bytes[1] & 252) >> 2; // AND 11111100 then shift 2 positions for correct value.
  decoded.days = bytes[2]; // uptime in days. Overflow after 255 days.
  decoded.uptime = bytes[4]; // uptime in hours
  // handle negative number
    if ( ( bytes[3] & 128 ) == 128 ) {
      decoded.dBm = (bytes[3] ^ 255) * -1; // XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
  }