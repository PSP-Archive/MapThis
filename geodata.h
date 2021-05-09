//////////////////////////////////////////////////////////////////////////////
/**
    geodata.h: functions to manage geo lookup 
*/

//////////////////////////////////////////////////////////////////////////////

#ifndef GEODATA_H
#define GEODATA_H

//////////////////////////////////////////////////////////////////////////////

ADDRESS addrs[MAXZIP];
int zipcounter;


int reverse_lookup(double lat, double lon, char * streetbuf, char * zipfile);
int address_geolookup( char * zipfile, char *addr, char* zipcode, char * city_filter, char * state_filter , double minlat, double minlon, double zmaxlat, double zmaxlon, int tolerance);

#endif // MENU_H

