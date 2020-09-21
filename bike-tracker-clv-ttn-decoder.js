function Decoder(bytes, port) {
  var decoded = {};
  
      if (port === 7) {
  
  decoded.vbatRaw  =  bytes[0]; 	     	// raw battery - 450. Range 460 - 700
  decoded.vbatArea =  bytes[1] & 3; 		// AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs   = (bytes[1] & 252) >> 2; 	// AND 11111100 then shift 2 positions for correct value.
  decoded.days     =  bytes[2]; 	   	// uptime in days. Overflow after 255 days.
  decoded.uptime   =  bytes[4]; 		// uptime in hours
    if ( ( bytes[3] & 128 ) == 128 ) {		// handle negative number
      decoded.dBm = (bytes[3] ^ 255) * -1; 	// XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
     // MSB
     // it gives: 40.626354850909394, 22.94828432490135 vs 40.62635955, 22.948298
     //                   ^error (6)         ^error (5)
     // https://www.thethingsnetwork.org/forum/t/how-to-send-float-value-and-convert-it-into-bytes/9646/8
    decoded.latitude  = +((bytes[5] << 16 | bytes[6] << 8 | bytes[7]) / 16777215.0 * 180.0 - 90).toFixed(6);
    decoded.longitude = +((bytes[8] << 16 | bytes[9] << 8 | bytes[10]) / 16777215.0 * 360.0 - 180).toFixed(6);
    decoded.hdop      = bytes[11] / 10;
    decoded.altitude  = bytes[14] << 8 | bytes[15];
    decoded.sats      = bytes[12];
    decoded.speed     = bytes[13];
    decoded.heading   = bytes[16] * 2;
  }
  
   if (port === 11) { // LORA_VERB heartbeat
  
  decoded.vbatRaw  =  bytes[0];                 // raw battery - 450. Range 460 - 700
  decoded.vbatArea =  bytes[1] & 3;             // AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs   = (bytes[1] & 252) >> 2;     // AND 11111100 then shift 2 positions for correct value.
  decoded.days     =  bytes[2];                 // uptime in days. Overflow after 255 days.
  decoded.uptime   =  bytes[4];                 // uptime in hours
  decoded.noFix    =  (bytes[5] << 8 | bytes[6]) * 5; // failed Fixes
  // handle negative number
    if ( ( bytes[3] & 128 ) == 128 ) {
      decoded.dBm = (bytes[3] ^ 255) * -1; // XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
  }

  if (port === 3 || port === 1) {
  
  decoded.vbatRaw  =  bytes[0]; // raw battery - 450. Range 460 - 700
  decoded.vbatArea =  bytes[1] & 3; // AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs   = (bytes[1] & 252) >> 2; // AND 11111100 then shift 2 positions for correct value.
  decoded.days     =  bytes[2]; // uptime in days. Overflow after 255 days.
  decoded.uptime   =  bytes[4]; // uptime in hours
  // handle negative number
    if ( ( bytes[3] & 128 ) == 128 ) {
      decoded.dBm = (bytes[3] ^ 255) * -1; // XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
  }

// ttnmapper format
   if (port === 8) {
  //decoded.user_agent = "bike-tracker-clv";
  decoded.vbatRaw  =  bytes[0]; 		// raw battery - 450. Range 460 - 700
  decoded.vbatArea =  bytes[1] & 3; 		// AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs   = (bytes[1] & 252) >> 2; 	// AND 11111100 then shift 2 positions for correct value.
  decoded.days     =  bytes[2]; 		// uptime in days. Overflow after 255 days.
  decoded.uptime   =  bytes[4]; 		// uptime in hours
    if ( ( bytes[3] & 128 ) == 128 ) {		// handle negative number
      decoded.dBm = (bytes[3] ^ 255) * -1; 	// XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
     // MSB
     // it gives: 40.626354850909394, 22.94828432490135 vs 40.62635955, 22.948298
     //                   ^error (6)         ^error (5)
     // https://www.thethingsnetwork.org/forum/t/how-to-send-float-value-and-convert-it-into-bytes/9646/8
    decoded.latitude  = +((bytes[5] << 16 | bytes[6] << 8 | bytes[7]) / 16777215.0 * 180.0 - 90).toFixed(6);
    decoded.longitude = +((bytes[8] << 16 | bytes[9] << 8 | bytes[10]) / 16777215.0 * 360.0 - 180).toFixed(6);
    decoded.hdop      = bytes[11] / 10;
    decoded.altitude  = bytes[14] << 8 | bytes[15];
    decoded.sats      = bytes[12];
    decoded.speed     = bytes[13];
    decoded.heading   = bytes[16] * 2;
    
    /*
    Payload HEX 67 01 00 0E 00 B9 C7 9A 90 51 9B 1C 04 01 00 6C 14 in port8 gives:
  "TXsecs": 0,
  "altitude": 108,
  "dBm": 14,
  "days": 0,
  "hdop": 2.8,
  "heading": 40,
  "latitude": 40.626355,
  "longitude": 22.948284,
  "sats": 4,
  "speed": 1,
  "uptime": 0,
  "vbatArea": 1,
  "vbatRaw": 10
    */
  }
  
  // ttnmapper format
   if (port === 18) {


     
  //decoded.user_agent = "bike-tracker-clv";
  decoded.vbatRaw  =  bytes[0]; 		// raw battery - 450. Range 460 - 700
  decoded.vbatArea =  bytes[1] & 3; 		// AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs   = (bytes[1] & 252) >> 2; 	// AND 11111100 then shift 2 positions for correct value.
  decoded.days     =  bytes[2]; 		// uptime in days. Overflow after 255 days.
  decoded.uptime   =  bytes[4]; 		// uptime in hours
    if ( ( bytes[3] & 128 ) == 128 ) {		// handle negative number
      decoded.dBm = (bytes[3] ^ 255) * -1; 	// XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
     // MSB
     // it gives: 40.626354850909394, 22.94828432490135 vs 40.62635955, 22.948298
     //                   ^error (6)         ^error (5)
     // https://www.thethingsnetwork.org/forum/t/how-to-send-float-value-and-convert-it-into-bytes/9646/8
    decoded.latitude  = +((bytes[5] << 16 | bytes[6] << 8 | bytes[7]) / 16777215.0 * 180.0 - 90).toFixed(6);
    decoded.longitude = +((bytes[8] << 16 | bytes[9] << 8 | bytes[10]) / 16777215.0 * 360.0 - 180).toFixed(6);
    decoded.hdop      = bytes[11] / 10;
    decoded.altitude  = bytes[14] << 8 | bytes[15];
    decoded.sats      = bytes[12];
    decoded.speed     = bytes[13];
    decoded.heading   = bytes[16] * 2;
    decoded.noFix     = (bytes[17] << 8 | bytes[18]) * 5;
   }
  
  return decoded;
}