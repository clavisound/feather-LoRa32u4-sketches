// Convert Lat / Lon with 5 decimals to 4 bytes
// Example:
//
//latlon4gr 40.627 22.9644 115 196 14
// MSB's 
// -35, -19
// LSB's fit to 20 bits (8+8+4 bits)
/* ./latlon4gr 40.627 22.9644 115 196 14
//location: 40.627 22.9644
//gateway[0][0] = 5 << 4;
//gateway[0][0] = 80;
//gateway[0][0] |= 3;
gateway[0][0] = 83;
gateway[0][1] = 115;
gateway[0][2] = 196;
gateway[0][3] = 14;
*/

#include <stdlib.h>	// atof
#include <stdio.h>	// printf
#include <inttypes.h>	// uint8_t style
#include <math.h>	// failed to round
#include <string.h>	// strcmp

int main(int argc, char *argv[]){

	if ( argc <= 2 ) {
		printf("Usage: latlon3gr 40.61078 22.98785\n");
		printf("Add the last two numbers for reverse calculation. Example latlon3.gr 40.61078 22.98785 153 247\n");
		printf("arguments given: %d\n", argc - 1);
		return 1;
	}

	printf ("//location: %s %s \n", argv[1], argv[2]);

	int MSBlat, MSBlon, LSBlat, LSBlon;
	float lat, lon;

	// high byte0 (lat)
	MSBlat = atoll(argv[1]);
	printf("//gateway[0][0] = %d << 4;\n", MSBlat - 35);
	printf("//gateway[0][0] = %d;\n", ( MSBlat - 35) << 4);

	// low byte0 (lon)
	MSBlon = atoll(argv[2]);
	printf("//gateway[0][0] |= %d;\n", MSBlon - 19);
	printf("gateway[0][0] = %d;\n", ( (MSBlat - 35) << 4 ) | MSBlon - 19);

	// LAT LSB
	LSBlat	= (int) ( ( atof(argv[1]) - MSBlat ) * 1000);
	printf("gateway[0][1] = %d;\n", (uint8_t)(LSBlat & 0xFF)); 				// Keep LSB (8bits) from LSBlat

	// LON LSB
	LSBlon	= (int) ( ( atof(argv[2]) - MSBlon ) * 1000);
	printf("gateway[0][2] = %d;\n", (uint8_t)(LSBlon & 0xFF)); 				// Keep LSB (8bits) from LSBlat
	printf("gateway[0][3] = %d;\n", (uint16_t)( ( ( LSBlon & 0b1100000000 ) >> 6 ) | ( ( LSBlat & 0b1100000000 ) >> 8 ) ));	// Merge LSBlon and LSBlat

	if ( argc == 6 ) { 
		LSBlat	= atoi(argv[3]) + ( ( atoi(argv[5]) & 0b11) << 8 );
		LSBlon	= atoi(argv[4]) + ( ( atoi(argv[5]) & 0b1100 ) << 6 );

		printf("\nValue seen on device as: %d.%d %d.%d", MSBlat, LSBlat, MSBlon, LSBlon);
		printf("\nhttps://www.openstreetmap.org/?mlat=%d.%d&mlon=%d.%d#map=18/%d.%d/%d.%d\n", MSBlat, LSBlat, MSBlon, LSBlon, MSBlat, LSBlat, MSBlon, LSBlon);
	}

	return 0;
}

