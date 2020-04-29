function Decoder(bytes, port) {
  var decoded = {};
  
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

      if (port === 7) {
  
  decoded.vbatRaw  =  bytes[0]; 		// raw battery - 450. Range 460 - 700
  decoded.vbatArea =  bytes[1] & 3; 		// AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs   = (bytes[1] & 252) >> 2; 	// AND 11111100 then shift 2 positions for correct value.
  decoded.days     =  bytes[2]; 		// uptime in days. Overflow after 255 days.
  decoded.uptime   =  bytes[4]; 		// uptime in hours
    if ( ( bytes[3] & 128 ) == 128 ) {		// handle negative number
      decoded.dBm = (bytes[3] ^ 255) * -1; 	// XOR with 11111111
    } else { decoded.dBm = bytes[3]; }

    decoded.latitude  = (bytes[5] << 24 | bytes[6] << 16 | bytes[7] << 8) / 10e5;
    decoded.longitude = (bytes[8] << 24 | bytes[9] << 16 | bytes[10] << 8) / 10e5;
    decoded.hdop      =  bytes[11] / 10;
    decoded.altitude  =  bytes[14] << 8 | bytes[15];
    decoded.sats      =  bytes[12];
    decoded.speed     =  bytes[16];
    decoded.heading   =  bytes[17] * 2;
  }

     if (port === 8) {
  
  decoded.vbatRaw = bytes[0]; 			// raw battery - 450. Range 460 - 700
  decoded.vbatArea = bytes[1] & 3; 		// AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs = (bytes[1] & 252) >> 2; 	// AND 11111100 then shift 2 positions for correct value.
  decoded.days = bytes[2]; 			// uptime in days. Overflow after 255 days.
  decoded.uptime = bytes[4]; 			// uptime in hours
    if ( ( bytes[3] & 128 ) == 128 ) {		// handle negative number
      decoded.dBm = (bytes[3] ^ 255) * -1; 	// XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
    decoded.latitude  = (bytes[5] << 24 | bytes[6] << 16 | bytes[7] << 8 | bytes[12]) / 10e5;
    decoded.longitude = (bytes[8] << 24 | bytes[9] << 16 | bytes[10] << 8 | bytes[13]) / 10e5;
    decoded.hdop      = bytes[11] / 10;
    decoded.altitude  = bytes[14] << 8 | bytes[15];
    decoded.sats      = bytes[12];
    decoded.speed     =  bytes[16];
    decoded.heading   =  bytes[17] * 2;
  }
  
  return decoded;
}
