#include <pspdebug.h>
#include <psppower.h>
#include <pspsdk.h>
#include <stdio.h>
#include <string.h>
#include "graphics.h"
#include <zlib.h>
#include <stdlib.h>
#include <pspctrl.h>
#include <dirent.h>
#include <math.h>
#include <psprtc.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <psputility.h>
#include <errno.h>
#include <sys/stat.h>
#include <pspsyscon.h>
#include "nmeap.h"

#include "version.h"
#include "attractions.h"
#include "line.h"


#define PSP_WIDTH       480
#define PSP_HEIGHT      272
#define RADIUS          0 
#define PATHMAX         4000 
#define TILE_SIZE       256

#define MAXLISTSIZE 200
#define MAXZIP   80



#define MAXZIP   80
typedef struct  {
        char zipcode[6];
        char street[63];
        char city[31];
        char state[3];
        double lat;
        double lon;
	double dist;
	double bearing;	
	int num;	
} ADDRESS;



//////////////////////////////////////////////////////////////////////////////

int gAllowBeep;
FILE *gpsout;
char currentpath[128];
char buffer[1024];
int togglefeed;

int isconnected;
 char streetbuf[128];
 u64 displaycounter;
 int Clock;
 int showpanels;
 u64 triptime;


Image  *attraction[MAXIMAGE];
 SceUID thid_wifi,thid_cachemngr,thid_garbagecoll,thid_usermain;
#ifndef GENERIC
 SceUID thid_gpsmngr;
#endif

int TILE_NUM;
int zoom ;
int zm;
int gpsOn;
int basezoom;
int mapsize;
char dirname[128];
int maxzoom;
int oldzoom;
long mapx ;
long mapy ;
double mx,my;
double lat;
double lon;
double bearing;
char zipfile[100];
char filetype[8];
char messages [16][60];
int messagecount;
int tlock;
char options[MAXLISTSIZE][60];

int pathx[PATHMAX];
int pathy[PATHMAX];
int pathcounter;
char attr_file[64];
char attrbuf1[64];
char attrbuf2[64];
int generic;

char heading[8][3];
int optcount;
int datatype;         // 0=dir; 1=zipfile

double lat1, lon1;
double curlat, curlon, pathdist;
int mapx1, mapy1;
int marker;
double ftx, fty;
int idle;  //@ZZ: might overlap with gMapLoaded

int attr_flag;
int waypoint;
char attr_ifile[MAXIMAGE][128];
int attr_width;
int attr_height;
FILE * fp;

double        altitude;
unsigned long ltime;
int           satellites;
int           satellites_in_view;
int           quality;
double        hdop;
double        geoid;
char          warn[4];

double        latitude;
double        longitude;
double        prevlat;
double        prevlon;
double        speed;
double        avgspeed;
double        distance;
double        course;
double        course1;
unsigned long date;
double        magvar;
int duration;

int running;
int gMapLoaded;

float fixvar, anglefix, zmfix, transitiontimer;
LINE l;
int atimer;

//////////////////////////////////////////////////////////////////////////////

void checkforspeed(double speed, double speedlimit); 
void play_alert(int which);
void readPath();
int isOnLine(LINE l, int x, int y);
void turn_off_attractions();
void turn_on_attractions ();
void initLatLon(); 
int powerOf(int p);
void beep();
Image* ldImage(const char* filename);
Image* ldImage1(const char* filename, int negate);
#include "pspusbgps.h"
u32 gpsState;
gpsdata gpsd;
satdata  satd;
long getLocation;
#ifndef GENERIC
#include <pspuser.h>
#include <pspusb.h>

#else
#include <pspkernel.h>
#include <pspnet_resolver.h>
#endif

#include <pspnet_resolver.h>
#include <pspwlan.h>
