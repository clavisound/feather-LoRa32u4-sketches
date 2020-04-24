function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.
  var decoded = {};
  
  if (port === 1) {
    // BUG: Does not delete the null character.
    decoded.string = String.fromCharCode.apply(null, bytes);
  }

  // TTNMAPPER TODO
     if (port === 10) {
  // failed.
  decoded.vbatRaw = bytes[0]; // raw battery - 450. Range 460 - 700
  decoded.vbatArea = bytes[1] & 3; // AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs = (bytes[1] & 252) >> 2; // AND 11111100 then shift 2 positions for correct value.
  decoded.days = bytes[2]; // uptime in days. Overflow after 255 days.
  decoded.uptime = bytes[4]; // uptime in hours
  // handle negative number
    if ( ( bytes[3] & 128 ) == 128 ) {
      decoded.dBm = (bytes[3] ^ 255) * -1; // XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
     // LSB
    latitude = [ bytes[5], bytes[6], bytes[7], bytes[12] ];
    longitude = [ bytes[8], bytes[9], bytes[10], bytes[13] ];
    //lat = [ bytes[12], bytes[7], bytes[6], bytes[5] ];
    //lon = [ bytes[13], bytes[10], bytes[9], bytes[8] ];
    decoded.hdop = bytes[11];
    
    decoded.lat = bytesToFloat(lat);
    decoded.lon = bytesToFloat(lon);
    //decoded.lat = lat;
    //decoded.lon = lon;
    // Test with 0082d241 for 26.3134765625
    loc = [ 0x00, 0x82, 0xd2, 0x41];
    decoded.test = bytesToFloat(loc);
  }
  
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
  
  decoded.vbatRaw = bytes[0]; // raw battery - 450. Range 460 - 700
  decoded.vbatArea = bytes[1] & 3; // AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs = (bytes[1] & 252) >> 2; // AND 11111100 then shift 2 positions for correct value.
  decoded.days = bytes[2]; // uptime in days. Overflow after 255 days.
  decoded.uptime = bytes[4]; // uptime in hours
  // handle negative number
    if ( ( bytes[3] & 128 ) == 128 ) {
      decoded.dBm = (bytes[3] ^ 255) * -1; // XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
     // LSB
    decoded.lat = bytes[5] << 24 | bytes[6] << 16 | bytes[7] << 8 // | bytes[12];
    decoded.lon = bytes[8] << 24 | bytes[9] << 16 | bytes[10] << 8 // | bytes[13];
    decoded.hdop = bytes[11];
  }
  
    if (port === 7) {
  
  decoded.vbatRaw = bytes[0]; // raw battery - 450. Range 460 - 700
  decoded.vbatArea = bytes[1] & 3; // AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs = (bytes[1] & 252) >> 2; // AND 11111100 then shift 2 positions for correct value.
  decoded.days = bytes[2]; // uptime in days. Overflow after 255 days.
  decoded.uptime = bytes[4]; // uptime in hours
  // handle negative number
    if ( ( bytes[3] & 128 ) == 128 ) {
      decoded.dBm = (bytes[3] ^ 255) * -1; // XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
     // LSB
    decoded.lat = bytes[5] << 16 | bytes[6] << 8 | bytes[7]
    decoded.lon = bytes[8] << 8 | bytes[9] << 8 | bytes[10]
    decoded.hdop = bytes[11] / 10;
    decoded.altitude = bytes[14] << 8 | bytes[15];
    decoded.sats = bytes[12];
  }
  
     if (port === 8) {
  
  decoded.vbatRaw = bytes[0]; // raw battery - 450. Range 460 - 700
  decoded.vbatArea = bytes[1] & 3; // AND 00000011 (gives values betwwen 0-3, aka: 0, 30, 60, 90%)
  decoded.TXsecs = (bytes[1] & 252) >> 2; // AND 11111100 then shift 2 positions for correct value.
  decoded.days = bytes[2]; // uptime in days. Overflow after 255 days.
  decoded.uptime = bytes[4]; // uptime in hours
  // handle negative number
    if ( ( bytes[3] & 128 ) == 128 ) {
      decoded.dBm = (bytes[3] ^ 255) * -1; // XOR with 11111111
    } else { decoded.dBm = bytes[3]; }
     // LSB
    decoded.latitude = (bytes[5] << 24 | bytes[6] << 16 | bytes[7] << 8 | bytes[12]) / 10e5;
    decoded.longitude = (bytes[8] << 24 | bytes[9] << 16 | bytes[10] << 8 | bytes[13]) / 10e5;
    decoded.hdop = bytes[11] / 10;
    decoded.altitude = bytes[14] << 8 | bytes[15];
    decoded.sats = bytes[12];
  }
  
  // Based on https://stackoverflow.com/a/37471538 by Ilya Bursov
  function bytesToFloat(bytes) {
    // JavaScript bitwise operators yield a 32 bits integer, not a float.
    // Assume LSB (least significant byte first).
    var bits = bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
    // var bits = 0x00 | 0x82 << 8 | 0xd2 << 16 | 0x41 << 24;
    var sign = (bits>>>31 === 0) ? 1.0 : -1.0;
    var e = bits>>>23 & 0xff;
    var m = (e === 0) ? (bits & 0x7fffff)<<1 : (bits & 0x7fffff) | 0x800000;
    var f = sign * m * Math.pow(2, e - 150);
    return f;
  }
  
  return decoded;
}