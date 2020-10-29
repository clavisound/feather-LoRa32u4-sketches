function Decoder(bytes, port) {
  var decoded = {};
  var length  = bytes.length;
  var speed   = 0;
  
  if ( length === 1 ) { // HEARTBEAT with battery 10% step
    decoded.battery = (bytes[0] & 0x0F) * 10; // discard 4 last bits. 0x0F
    decoded.mov     = (bytes[0] & 0x20) >> 5; // read 6th bit
    decoded.nogps   = (bytes[0] & 0x80) >> 7; // read 8th bit
  }
  
  if ( length === 7 ) { // we have gps data.
     // https://www.thethingsnetwork.org/forum/t/how-to-send-float-value-and-convert-it-into-bytes/9646/8
    decoded.latitude  = +((bytes[0] << 16 | bytes[1] << 8 | bytes[2]) / 16777215.0 * 180.0 - 90).toFixed(6);
    decoded.longitude = +((bytes[3] << 16 | bytes[4] << 8 | bytes[5]) / 16777215.0 * 360.0 - 180).toFixed(6);
    if ( ( port & 0x01 ) === 1 ) { speed = 30; }                       // add 30 km/h if highspeed port
    decoded.heading   =  ( bytes[6] & 0x0F ) * 24;
    decoded.speed     = ( ( ( bytes[6] & 0xF0 ) >> 4 ) * 2 ) + speed;   // read last 4 bits. Multiply by two
    decoded.battery   = ( port & 0x0c ) >> 2;                           // read bits 2-3. Shift two times.
    decoded.fall      = ( port & 0x02 ) >> 1;                           // read 2nd bit.
    decoded.highHDOP  = ( port & 0x10 ) >> 4;                           // read 5th bit.
     /* Payload: B9 C7 9A 90 51 9B 41 on port 55
  gives:
  "battery": 1,
  "fall": 1,
  "heading": 24,
  "highHDOP": 1,
  "latitude": 40.626355,
  "longitude": 22.948284,
  "speed": 38
     */
  }

  return decoded;
}