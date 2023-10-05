// Convert Lat / Lon with 5 decimals to 3 bytes
// Example:
//
// 40.61078, 22.98785
// MSB's 
// -35, -19
// LSB's divide by / 4
/*
gateway[0][0] = 5 << 4;                    // move LAT MSB to high byte
gateway[0][0] |= 3;                        // add  LON MSB to the low  byte
gateway[0][1] = 153;                       // LAT LSB
gateway[0][2] = 246;                       // LAT LSB
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

	printf ("//location: %s, %s \n", argv[1], argv[2]);

	int MSBlat, MSBlon;
	float lat, lon, LSBlat, LSBlon;

	// high byte0 (lat)
	MSBlat = atoll(argv[1]);
	printf("gateway[0][0] = %d << 4;\n", MSBlat - 35);

	// low byte0 (lon)
	MSBlon = atoll(argv[2]);
	printf("gateway[0][0] |= %d;\n", MSBlon - 19);

	// LAT LSB
	LSBlat = ( atof(argv[1]) - MSBlat ) * 1000 / 4;
	// less accuracy?
	printf("gateway[0][1] = %.2f;\n", ( ( ( atof(argv[1]) - MSBlat ) * 1000 ) / 4 ) );
	// TODO: round does not work
//	printf("gateway[0][1] = %f;\n", roundf( ( ( atof(argv[1]) - MSBlat ) * 1000 ) / 4 ) );

	// LON LSB
	LSBlon = ( atof(argv[2]) - MSBlon ) * 1000 / 4;
	// less accuracy?
	printf("gateway[0][2] = %.2f;\n", ( ( ( atof(argv[2]) - MSBlon ) * 1000 ) / 4 ) );

	if ( argc == 5 ) { 
		lat = (float)MSBlat + ( atof(argv[3]) / 1000 ) * 4;
		lon = (float)MSBlon + ( atof(argv[4]) / 1000 ) * 4;
//		lon = (float)MSBlon + (float)( userLSBlon / 1000 ) * 4;
		printf("\nValue seen on device as: %f, %f", lat, lon);
		printf("\nhttps://www.openstreetmap.org/?mlat=%f&mlon=%f#map=18/%f/%f\n", lat, lon, lat, lon);
	}

	return 0;
}

