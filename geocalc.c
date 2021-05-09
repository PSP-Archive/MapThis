// geocalc.c
//
// This code is freely distributable. Please contact the author with
// bug reports or any questions regarding the navigation calculations.
//
// Written by: Rick Chapman
//             JHU/APL
//             rick.chapman@jhuapl.edu
//			   8 November 2002
//


#include <math.h>
#include "geocalc.h"
#include "main.h"


// ********************************************************************
// Global variables
// ********************************************************************

//double  nan;
double g_ra[] = {6377563.396,6378160,6377397.155,6377483.865,6378206.4,6378249.145,
				 6377298.556,6377276.345,6377301.243,6377309.613,6377304.063,6377295.664,
				 6378137,6378200,6378270,6378160,6378388,
				 6378245,6377340.189,6378155,6378160,6378135,6378137};
double g_invf[] = {299.3249646,298.25,299.1528128,299.1528128,294.9786982,293.465,
				300.8017,300.8017,300.8017,300.8017,300.8017,300.8017,
				298.257222101,298.3,297,298.247,297,
				298.3,299.3249646,298.3,298.25,298.26,298.257223563};

static  double pixelsPerLonDegree[20];
static  double pixelsPerLonRadian[20];
static  double numTiles[20];
static  double bitmapOrigo[20];
static  double bc = 2 * PI;
static  double Wa = PI / 180;


// ********************************************************************
// Geo Calculation Functions
// ********************************************************************


void initLatLon() {
        static int c = 256;
        int d;
        for ( d = 17; d >= -2; --d) {
                 int dd=d;
                 if (dd<0) dd=dd*(-1)+17;
                 pixelsPerLonDegree[dd] = (double)c / 360;
                 pixelsPerLonRadian[dd] = (double)c / bc;
                 bitmapOrigo[dd] = (double)c/2;
                 numTiles[dd] = c / 256;
                 c *= 2;
        }
}

Coord getLatLong(int x, int y, int zoom) {
      double lon      = -180; // x
      double lonWidth = 360; // width 360
      int zooom=zoom+2;
      double lat       = -1;
      double latHeight = 2;

      int tilesAtThisZoom = 1 << (19 - zooom);
      lonWidth  = 360.0 / tilesAtThisZoom;
      lon       = -180 + (x * lonWidth);
      latHeight = -2.0 / tilesAtThisZoom;
      lat       = 1 + (y * latHeight);

      // convert lat and latHeight to degrees in a transverse mercator projection
      // note that in fact the coordinates go from about -85 to +85 not -90 to 90!
      latHeight += lat;
      latHeight = (2 * atan(exp(PI * latHeight))) - (PI / 2);
      latHeight *= (180 / PI);

      lat = (2 * atan(exp(PI * lat))) - (PI / 2);
      lat *= (180 / PI);

      latHeight -= lat;

      if (lonWidth < 0) {
         lon      = lon + lonWidth;
         lonWidth = -lonWidth;
      }

      if (latHeight < 0) {
         lat       = lat + latHeight;
         latHeight = -latHeight;
      }

        Coord c;
        c.lat=lat;
        c.lon=lon;
        c.lonWidth=lonWidth;
        c.latHeight=latHeight;
      return c;
}


//puts x value in Coord.lon and y value in Coord.lat
Coord getTileCoord(double lat, double lon, int zoomLl ) {
        int x,y;
        int zoomLevel=zoomLl;
        if (zoomLevel<0) zoomLevel=zoomLevel*(-1)+17;
         x= (int)(bitmapOrigo[zoomLevel] + lon * pixelsPerLonDegree[zoomLevel]);
         x/=256;
                double e = sin(lat * Wa);

                if (e > 0.9999) {
                        e = 0.9999;
                }

                if (e < -0.9999) {
                        e = -0.9999;
                }

                y = (int)(bitmapOrigo[zoomLevel] + 0.5
                                * log((1 + e) / (1 - e)) * -1
                                * (pixelsPerLonRadian[zoomLevel]));
                y/=256;

                Coord cc;
                cc.lon=x;
                cc.lat=y;
                return cc;

}

//puts x value in Coord.lon and y value in Coord.lat
Coord getXY(double lat, double lon, int zoom) {
        Coord c;
        long x,y;
        int zoomLevel=zoom;
        if (zoomLevel<0) zoomLevel=zoomLevel*(-1)+17;

         x= (long)(bitmapOrigo[zoomLevel] + lon * pixelsPerLonDegree[zoomLevel]);
         //x/=256;
                double e = sin(lat * Wa);

                if (e > 0.9999) {
                        e = 0.9999;
                }

                if (e < -0.9999) {
                        e = -0.9999;
                }

                y = (long)(bitmapOrigo[zoomLevel] + 0.5
                                * log((1 + e) / (1 - e)) * -1
                                * (pixelsPerLonRadian[zoomLevel]));
                //y/=256;
                //c=getLatLong(x,y,basezoom);
                //c.lon=(x-ftx)*TILE_SIZE+(lon-c.lon)*TILE_SIZE/c.lonWidth;
                //c.lat=(y-fty)*TILE_SIZE+(c.lat-lat)*TILE_SIZE/c.latHeight+TILE_SIZE;
                c.lon=x-ftx*TILE_SIZE;
                c.lat=y-fty*TILE_SIZE;
                return c;

}


Coord getGPS (int x, int y) {
        Coord c = getLatLong((int)(x/TILE_SIZE+ftx),(int)(y/TILE_SIZE+fty-1),basezoom);
        c.lon=c.lon+x%TILE_SIZE*c.lonWidth/TILE_SIZE;
        c.lat=c.lat-y%TILE_SIZE*c.latHeight/TILE_SIZE;
        return c;
}




// find geodesic range and initial bearing from loc1 to loc2
// lat and lng are given in degrees, with negative values being west of the prime meridian and south of the equator
// bearing is in degrees true, range is in meters
// uses datum specified in g_ra[22] and g_prefs.invf
void GeoRangeAndBearing(double lat1, double lng1, double lat2, double lng2, double *range, double *bearing) {
	double f = 1.0/g_invf[22];	// Flattening factor.
	double ra = g_ra[22];			// Semimajor axis (m).
	double eps=5.e-14;
	double radeg = 180.0/PI;		// Must use double for mm accuracy.
	
	double r=1.-f;
	double tu1=r*tan(lat1/radeg);
	double tu2=r*tan(lat2/radeg);
	double cu1=1./sqrt(tu1*tu1+1.);
	double su1=cu1*tu1;
	double cu2=1./sqrt(tu2*tu2+1.);
	double s=cu1*cu2;
	double baz=s*tu2;
	double faz=baz*tu1;
	double x=(lng2-lng1)/radeg;
	
	double d,sx,cx,sy,cy,cz,c2a,c,e,y,sa;
	do{
		sx=sin(x);
		cx=cos(x);
		tu1=cu2*sx;
		tu2=baz-su1*cu2*cx;
		sy=sqrt(tu1*tu1+tu2*tu2);
		cy=s*cx+faz;
		y=atan2(sy,cy);
		sa=s*sx/sy;
		c2a=-sa*sa+1.;
		cz=faz+faz;
		if(c2a>0.) cz=-cz/c2a+cy;
		e=cz*cz*2.-1.;
		c=((-3.*c2a+4.)*f+4.)*c2a*f/16.;
		d=x;
		x=((e*cy*c+cz)*sy*c+y)*sa;
		x=(1.-c)*x*f+(lng2-lng1)/radeg;
	}while(fabs(d-x)>eps);
	faz=atan2(tu1,tu2);
	baz=atan2(cu1*sx,baz*cx-su1*cu2)+PI;
	x=sqrt((1/r/r-1.)*c2a+1.)+1.;
	x=(x-2.)/x;
	c=1.-x;
	c=(x*x/4.+1.)/c;
	d=(0.375*x*x-1.)*x;
	x=e*cy;
	s=1.-e*e;
	s=((((sy*sy*4.-3.)*s*cz*d/6.-x)*d/4.+cz)*sy*d+y)*c*ra*r;
	
	*range = s;
	*bearing = faz*radeg;
	if( *bearing > 360.0 ) *bearing -= 360.0;
	if( *bearing < 0.0 ) *bearing += 360.0;
	return;
}

// find end location of geodesic segment
// lat and lng are given in degrees, with negative values being west of the prime meridian and south of the equator
// bearing is in degrees true, range is in meters
// uses datum specified in g_ra[22] and g_invf[22]
void GeoLine(double lat1, double lng1, double bearing, double range, double *lat2, double *lng2) {
	double f = 1.0/g_invf[22];	// Flattening factor.
	double ra = g_ra[22];			// Semimajor axis (m).
	double eps=1.e-13;
	double degrad = PI/180.0;		// Must use double for mm accuracy.
	double r=1.-f;
	double tu=r*tan(lat1*degrad);
	double sf=sin(bearing*degrad);
	double cf=cos(bearing*degrad);
	double baz=atan2(tu,cf)*2.;
	double cu=1./sqrt(tu*tu+1.);
	double su=tu*cu;
	double sa=cu*sf;
	double c2a=1.-sa*sa;
	double x=sqrt((1./r/r-1.)*c2a+1.)+1.;
	double c,d,y,sy,cy,cz,e;
	
	x=(x-2.)/x;
	c=1.-x;
	c=(x*x/4.+1.)/c;
	d=(0.375*x*x-1.)*x;
	tu=range/r/ra/c;
	y=tu;
	do{
		sy=sin(y);
		cy=cos(y);
		cz=cos(baz+y);
		e=cz*cz*2.-1.;
		c=y;
		x=e*cy;
		y=e*e-1.;
		y=(((sy*sy*4.-3.)*y*cz*d/6.+x)*d/4.-cz)*sy*d+tu;
	}while(fabs(y-c)>eps);
	baz=cu*cy*cf-su*sy;
	c=r*sqrt(sa*sa+baz*baz);
	d=su*cy+cu*sy*cf;
	*lat2=atan2(d,c)/degrad;
	c=cu*cy-su*sy*cf;
	x=atan2(sy*sf,c);
	c=((-3.*c2a+4.)*f+4.)*c2a*f/16.;
	d=((e*cy*c+cz)*sy*c+y)*sa;
	*lng2 = (lng1*degrad+x-(1.-c)*d*f)/degrad;
	baz=atan2(sa,baz)+PI;
	
	*lng2 = fmod( *lng2, 360.0 );
	if( *lng2 > 180.0 )
		*lng2 -= 360.0;
	if( *lng2 <= -180.0 )
		*lng2 += 360.0;
	
	return;
}
