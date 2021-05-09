// geocalc.h 
//
// header file for geocalc.c 
//
//
// Copyright (c) 2005 Rick Chapman.
// All rights reserved.



// ********************************************************************
// Global variables
// ********************************************************************

#define kMaxMantissaDigits	8		// max digits in display value, NOT including decimal
#define PI 3.14159265358979326433
#define FOURTHPI  (PI/4.0)
#define deg2rad  (PI/180.0)
#define rad2deg  (180.0/PI)


typedef struct
{
        double lon;
        double lat;
        double latHeight;
        double lonWidth;
} Coord;


Coord c;

// ********************************************************************
// Calculation Functions
// ********************************************************************

void initLatLon();
Coord getLatLong(int x, int y, int zoom);
Coord getTileCoord(double lat, double lon, int zoomLl );
Coord getXY(double lat, double lon, int zoom);
Coord getGPS (int x, int y);

void GeoRangeAndBearing(double lat1, double lng1, double lat2, double lng2, double *range, double *bearing) ;
void GeoLine(double lat1, double lng1, double bearing, double range, double *lat2, double *lng2) ;

