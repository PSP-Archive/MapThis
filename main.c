#include "main.h"
#include <stdarg.h>
#include "basic.h"
#include "geocalc.h"
#include "utils.h"
#include "font.h"
#include "attractions.h"
#include "line.h"
#include "danzeff.h"
#include "display.h"
#include "menu.h"
#include "geodata.h"
#include "mp3player.h"

extern Coord getDistanceToRoad(int mapx,int mapy,int zm,int TILE_NUM,int size);
void sioInit(int baud);
int sioReadChar();


int gpslock;
int running=1;
int stick=0;
int newzm=0, zoomtimer;

u64 frameTimer;
float zvalue = Z;
double deltax=0;
double deltay=0;
double oldx=0;
double oldy=0;

int framecounter=1;
int gotGpsData=0;
int speedlimitflag=0;
u64 spTick;

int switchcolor=0;
int Clock=333;
int showpanels=1;
int ffreadspeed=1;
u64 triptime=0;
SceUID wifithid=0;

SceCtrlData cpad;
Image *menuwindow=NULL ;
SceUID thid_wifi=0,thid_cachemngr=0,thid_garbagecoll=0,thid_usermain=0;
int killwifi=0;
char heading[8][3] = { " N", "NE", " E", "SE", " S", "SW", " W", "NW" };
int firsttime=1;
int gpsdone=0;
u64 currTick;
u64 routeTick;
u64 deltatime;
u64 frameTick;
u64 frameDelta;

void updateStatusScreen(Image *background, Image *menuwindow, char *label1, char *label2); 
int avgframes=1;

void mysleep(long howlong) {
	u64 cTick;
	sceRtcGetCurrentTick(&cTick);
	cTick = howlong -cTick + frameTimer;
	if (cTick>0 && cTick<howlong)
                        sceKernelDelayThread(cTick);
}

static int             user_data;
nmeap_gga_t g_gga;
static nmeap_context_t nmea;
static nmeap_gga_t     gga;
static nmeap_rmc_t     rmc;
nmeap_gsv_t     gsv;


static void print_gga(nmeap_gga_t *gga)
{
            latitude=gga->latitude; 
            longitude=gga->longitude;
            altitude=gga->altitude; 
            ltime=gga->time; 
            satellites=gga->satellites;
            quality=gga->quality; 
            hdop=gga->hdop;
            geoid=gga->geoid;  
}


static void print_gsv(nmeap_gsv_t *gsv) {
	satellites_in_view= gsv->satellites_in_view;
}

static void print_rmc(nmeap_rmc_t *rmc)
{
	
            ltime=rmc->time;
	    if (rmc->warn=='A')
            	strcpy(warn, "OK ");
	    else
            	strcpy(warn, "BAD");
            latitude=rmc->latitude;
            longitude=rmc->longitude;
            speed=rmc->speed;
	    course1=rmc->course;
            date=rmc->date;
            magvar=rmc->magvar;
  	    gotGpsData=1;
		if (prevlat!=0.00 && prevlon!=0.00) {
                                double rng=0;
                                double bearing=0;
                                GeoRangeAndBearing(latitude, longitude, prevlat,prevlon, &rng, &bearing) ;
                                if (rng<10000 && rng >-10000)
                                        distance+=rng;
                 }
                 prevlat=latitude;
                 prevlon=longitude;
}

static void gpgsv_callout(nmeap_context_t *context,void *data,void *user_data)
{
    nmeap_gsv_t *gsv = (nmeap_gsv_t *)data;
   
    print_gsv(gsv);
}


/** called when a gprmc message is received and parsed */
static void gprmc_callout(nmeap_context_t *context,void *data,void *user_data)
{
    nmeap_rmc_t *rmc = (nmeap_rmc_t *)data;
    print_rmc(rmc);
}

/** called when a gpgga message is received and parsed */
static void gpgga_callout(nmeap_context_t *context,void *data,void *user_data)
{
    nmeap_gga_t *gga = (nmeap_gga_t *)data;
    print_gga(gga);
}

void initNmeaParser() {
	int status = nmeap_init(&nmea,(void *)&user_data);
        if (status != 0) {
                ms_write_log("nmeap_init %d\n",status);
                exit(1);
        }
   
        status = nmeap_addParser(&nmea,"GPGGA",nmeap_gpgga,gpgga_callout,&gga);
        if (status != 0) {
                ms_write_log("nmeap_add %d\n",status);
                exit(1);
        }

        status = nmeap_addParser(&nmea,"GPRMC",nmeap_gprmc,gprmc_callout,&rmc);
        if (status != 0) {
                ms_write_log("nmeap_add %d\n",status);
                exit(1);
        }

        status = nmeap_addParser(&nmea,"GPGSV",custom_gpgsv,gpgsv_callout,&gsv);
        if (status != 0) {
                ms_write_log("nmeap_add %d\n",status);
                exit(1);
        }
}

void loadSioPrx() {
	SceUID mod = pspSdkLoadStartModule("sioprx.prx", PSP_MEMORY_PARTITION_KERNEL);
    	if (mod < 0) {
        	pspDebugScreenPrintf(" Error 0x%08X loading/starting pspsio.prx.\n", mod);
        	sceKernelDelayThread(2000000);
        	sceKernelExitGame();
	}
}

void holdMainFromExiting() {
	while(running)
		sceKernelDelayThread(10000);
}

void getSioData() {
	initNmeaParser();
	int status=0;
	sioInit(config.baud);
	int     ch;
        char buffer1[1024];
        int counter=0;
	int timeout=0;
	while (running) {
	if (config.fakefeed) {
	fp=fopen("gps.txt","r");
	char * s;
	while(running) {
		memset(buffer,0,sizeof(buffer));
		int i=0;
		while (i<2) {
                s=fgets(buffer1,1023,fp);
                while (strncmp(buffer1,"$GPGGA",6)!=0 && strncmp(buffer1,"$GPRMC",6)!=0 && strncmp(buffer1,"$GPGSV",6)!=0 && s!=NULL) {
                        s=fgets(buffer1,1023,fp);
			strcat(buffer,buffer1);
		}
			
                 buffer1[strlen(buffer1)+1]=0;

                //fprintf(stdout,"[%s]\n",buffer1);
		if (strncmp(buffer1,"$GPGSV",6)!=0)
			i++;
                char *pvec =buffer1;
                ch=0;
                while (ch>=0) {
                        if (*pvec == 0)
                                ch = -1;
                        else
                                ch = *pvec++;
                        //printf("%c",ch);
                        status = nmeap_parse(&nmea,ch);
                }
                }
		if (togglefeed==1) {
			togglefeed=0;
			break;
		}
		sceKernelDelayThread(499990);

        }
	fclose(fp); 
	} else {
	while(running) {
                ch = sioReadChar();
		if (ch==-1) {
			if (counter>0 && buffer1[counter-1]=='\n') {
				strcpy(buffer,buffer1);
				nmea_filter(buffer1);
				char *pvec =buffer1;
				char tempch=*pvec++;
				timeout=0;
				while (tempch>0) {
					status = nmeap_parse(&nmea,tempch);
        				tempch = *pvec++;
					sceKernelDelayThread(1);
				}
				isconnected=1;
				if (warn[0]!='O') {
					altitude=0;
					speed=0;
					prevlat=0;
					prevlon=0;
				}
				if (gpsout!=NULL)
					fprintf(gpsout, "%s", buffer1);
				memset(buffer1,0,sizeof(buffer1));
				counter=0;

			} else {
				timeout++;
				if (timeout>15) {
					isconnected=0;
					memset((void*) &gsv, 0, sizeof(gsv));
					satellites=0;
					ltime=0;
					date=0;
					course1=0;
					prevlat=0;
					prevlon=0;
				}
				if (timeout>150)
					timeout=16;
			}
		}
                if(ch >= 0 && counter <1024) 
                        sprintf(&buffer1[counter++],"%c", ch);
		if (togglefeed==1) {
			togglefeed=0;
			break;
		}
        } 
	}
	}
}






int sceNetInetInetAton(const char* host, struct in_addr* addr);
static void cleanup_output(void);

char pspIPAddr[32];
static char resolverBuffer[1024];
static int resolverId;
int status=0;
double        altitude;
unsigned long ltime;
int           satellites;
int           satellites_in_view;
int           quality;
double        hdop;
double        geoid;
char          warn[4]="---";


#ifndef GENERIC

PSP_MODULE_INFO("MapThis", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(20000); 
//PSP_HEAP_SIZE_KB(21000); 

#define printf  pspDebugScreenPrintf
SceUID thid_gpsmngr=0; 
int usbthread=0;
u32 usbState = 0;

int gpsmngr (SceSize args, void *argp) {
        ms_write_log("gpsmngr started\n");
        memset(&gpsd,0x0,sizeof(gpsd));
        memset(&satd,0,sizeof(satd));
        sceUsbActivate(PSP_USBGPS_PID);
        double prevlat=0, prevlon=0;
	short	      prevsecond=0;
        double curlat=0, curlon=0;
        sceUsbGpsSetInitDataLocation(config.initLocation);

       while (running) {
	

        ms_write_log("+\n");
	if (config.fakefeed) {
		int bytes=1;
		fp=fopen("gps.txt","r");
		while (bytes>0) {
			if (fp!=NULL) {
                	bytes=fread (&gpsd,48,1,fp);
                	curlat=gpsd.latitude;
                	curlon=gpsd.longitude;

                	if (curlat!=0.00 && curlon!=0.0) {
                        	if (prevlat!=0.00 && prevlon!=0.00) {
                                	double rng=0;
                                	double bearing=0;
                                	GeoRangeAndBearing(curlat, curlon, prevlat,prevlon, &rng, &bearing) ;
                                	if (rng<10000 && rng >-10000)
                                        	distance+=rng;
                        	}
                        	prevlat=curlat;
                        	prevlon=curlon;
                	}
			}

			if (togglefeed==1) {
                 		togglefeed=0;
                      		break;
                	}
        		if (usbthread>0)
                		break;
			gotGpsData=1;
			sceKernelDelayThread(1000000);
		}
		if (fp!=NULL)
			fclose(fp);
	}else{
		int count=0;
		while (running) {
        	usbState = sceUsbGetState();
        	// Get state of GPS
        	if (usbState & PSP_USB_ACTIVATED)
                	sceUsbGpsGetState(&gpsState);
        	else
                	gpsState = 0;
		sceKernelWaitSema(gpslock,1,0);
        	getLocation=0;
        	memset(&gpsd,0x0,sizeof(gpsd));
        	memset(&satd,0,sizeof(satd));
        	// Get data of GPS
        	if (gpsState == 0x3) {
                	sceUsbGpsGetData(&gpsd,&satd);
                	sceUsbGpsGetInitDataLocation(&getLocation);
                	curlat=gpsd.latitude;
                	curlon=gpsd.longitude;

                	if (curlat!=0.00 && curlon!=0.00) {
                        	if (prevlat!=0.00 && prevlon!=0.00) {
                                	double bearing=0;
                               		double range=0;
                                	GeoRangeAndBearing(curlat, curlon, prevlat,prevlon, &range, &bearing) ;
                                	if (range<10000 && range >-10000)
                                        	distance+=range;
                        		}
					if (gpsd.second!=prevsecond)
						gotGpsData=1;
                        		prevlat=curlat;
                        		prevlon=curlon;
					prevsecond=gpsd.second;
                	}
		}
		count++;
		if (gpsout!=NULL && count>999) {
        		fwrite(&gpsd,48,1,gpsout);
			count=0;
		}
		sceKernelSignalSema(gpslock, 1);
        	if (usbthread>0)
                	break;
	
		if (togglefeed==1) {
                        togglefeed=0;
                        break;
                }
        	sceKernelDelayThread(1000);

        	}
	}

       	if (usbthread>0)
               	break;
   }

        return 1;
}

#else

PSP_MODULE_INFO("MapThis!", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

#endif

void getGPSMessage290() {
        Coord xy_coord;
        framecounter++;

	sceKernelWaitSema(gpslock,1,0);
        latitude=gpsd.latitude;
        longitude=gpsd.longitude;
        altitude=gpsd.altitude;
        speed=gpsd.speed/1.8523;
        hdop=gpsd.hdop;

        if (speed>config.turnspeed) {
                if (abs(course-gpsd.bearing)>180) {
                        if (course > 180)
                                course=course-360;
                        if (gpsd.bearing>180)
                               gpsd.bearing=gpsd.bearing-360;
                }
                course=(course+gpsd.bearing)/2;
                if (course <0)
                        course=course+360;
       }
        ltime=gpsd.hour*10000+gpsd.minute*100+gpsd.second;
        int j;
        int fix=0;
        for (j=0; j<satd.satellites_in_view; j++) {
                if ( satd.satinf[j].good==1)
                        fix++;
        }
        satellites=fix;
	sceKernelSignalSema(gpslock, 1);
        u64 newTick;
        sceRtcGetCurrentTick(&newTick);
        if (gotGpsData==0)
                return;

        currTick=newTick;

        avgframes=framecounter;
        framecounter=1;
	gotGpsData=0;
        if (latitude==0.0 || longitude==0.0) {
                strcpy(warn,"BAD");
                return;
        }
        strcpy(warn,"OK");
        xy_coord=getXY(latitude, longitude, basezoom);
                mapx=(int)xy_coord.lon;
                mapy=(int)xy_coord.lat;
		if (stick) {
			Coord cc=getDistanceToRoad(mapx, mapy, zm, TILE_NUM,100);
			mapx=mapx+cc.lat*zm;
			mapy=mapy+cc.lon*zm;
		}
                mx=mapx;
                my=mapy;
                deltax=mapx-oldx;
                deltay=mapy-oldy;
                oldx=mapx;
                oldy=mapy;
		u64 curTick;
                sceRtcGetCurrentTick(&curTick);
                curTick=(curTick-triptime)/1000000;
                avgspeed=(double)(distance/curTick)*3.6*config.speedfix*0.53986935161690870809264158073746;
        if (xy_coord.lon>0 && xy_coord.lon<TILE_SIZE*TILE_NUM && xy_coord.lat>0 && xy_coord.lat<TILE_SIZE*TILE_NUM) {

           } else {
                char locstr[32];
                sprintf(locstr,"%.5f::%.5f", latitude, longitude);
        }

}


void getGPSMessageGeneric() {

        Coord xy_coord;
	framecounter++;
        if (speed>config.turnspeed) {
                if (fabs(course-course1)>180) {
                        if (course > 180)
                                course=course-360;
                        if (course1>180)
                               course1=course1-360;
                }
                course=(course+course1)/2;
                if (course <0)
                        course=course+360;
       }

        u64 newTick;
        sceRtcGetCurrentTick(&newTick);
        if (gotGpsData==0)
                return;
	avgframes=framecounter;
	framecounter=1;
        currTick=newTick;
	gotGpsData=0;

	if (warn[0]!='O')
                return;

        xy_coord=getXY(latitude, longitude, basezoom);
                mapx=(int)xy_coord.lon;
                mapy=(int)xy_coord.lat;
		if (stick) {
			Coord cc=getDistanceToRoad(mapx, mapy, zm, TILE_NUM,100);
			mapx=mapx+(int)cc.lat*zm;
			mapy=mapy+(int)cc.lon*zm;
		}
                mx=mapx;
                my=mapy;
                deltax=((double)mapx-oldx);
                deltay=((double)mapy-oldy);
                oldx=mapx;
                oldy=mapy;
		u64 curTick;
		sceRtcGetCurrentTick(&curTick);
		curTick=(curTick-triptime)/1000000;
		avgspeed=(double)(distance/curTick)*3.6*config.speedfix*0.53986935161690870809264158073746;
        if (xy_coord.lon>0 && xy_coord.lon<TILE_SIZE*TILE_NUM && xy_coord.lat>0 && xy_coord.lat<TILE_SIZE*TILE_NUM) {
           } else {
                char locstr[32];
                sprintf(locstr,"%.5f::%.5f", latitude, longitude);
        }

}


void record_thread_info(SceUID thid) 
{
SceKernelThreadInfo     tinf;
char                    ts[2];

        if(thid<1) return;
        tinf.size = sizeof(SceKernelThreadInfo);
        if(sceKernelReferThreadStatus(thid, &tinf) == 0)
        {
                if(tinf.status==PSP_THREAD_RUNNING) {ts[0]='R';ts[1]='u';}
                else
                if(tinf.status==PSP_THREAD_READY) {ts[0]='R';ts[1]='d';}
                else
                if(tinf.status==PSP_THREAD_WAITING) {ts[0]='W';ts[1]='a';}
                else
                if(tinf.status==PSP_THREAD_SUSPEND) {ts[0]='S';ts[1]='u';}
                else
                if(tinf.status==PSP_THREAD_STOPPED) {ts[0]='S';ts[1]='t';}
                else
                if(tinf.status==PSP_THREAD_KILLED) {ts[0]='K';ts[1]='i';}
                else
                {ts[0]='U';ts[1]='n';}
                ms_write_log("ThreadInfo : '%s' = %c%c  stack=%p[%d]  ExitStat=%d  IntPCnt=%d ThrPCnt=%d  RelsCnt=%d\n",tinf.name,ts[0],ts[1],tinf.stack,tinf.stackSize,tinf.exitStatus,tinf.intrPreemptCount,tinf.threadPreemptCount,tinf.releaseCount);
        }
        else
                ms_write_log("Can not get thread info on %d\n",thid);
}





void initPath() {
	pathcounter=1;
	pathdist=0.00000;
	curlat=0;
	curlon=0;
	int i;
	for (i=0; i<PATHMAX; i++) {
		pathx[i]=-1;
		pathy[i]=-1;
	}
}




void turn_on_attractions () {
			int attr_counter=load_attractions("_MY_POIS", zipfile, datatype,  mapx,  mapy, zoom, "");
			attr_counter=load_attractions(attr_file, zipfile, datatype,  mapx,  mapy, zoom, "");
		int k;	
		for ( k=0;k<=get_image_count(); k++) {
		if (strlen(attr_ifile[k])==0)
                        attraction[k] = ldImage("system/icons/attraction.png");
                else {
                                char filename1[128];
                                sprintf(filename1,"%s/%s", zipfile,attr_ifile[k]);
				if (my_access(filename1, F_OK) != 0)
                               		sprintf(filename1,"system/icons/%s", attr_ifile[k]);
				if (my_access(filename1, F_OK) != 0)
                               		sprintf(filename1,"system/icons/attraction.png");
                                attraction[k]=ldImage(filename1);
                }
		}
		attr_flag=1;
}

void turn_off_attractions() {
	int i;
	attr_flag=0;
		for ( i=1; i<MAXIMAGE; i++) {
			if (attraction[i]!=NULL)
				freeImage(attraction[i]);	
			attraction[i]=NULL;
		}
	free_attractions();
}

void freeup100points() {
	int i;
	for( i=100; i<PATHMAX; i++) {
		pathx[i-100]=pathx[i];
		pathy[i-100]=pathy[i];
	}

	for (i=PATHMAX-100; i<PATHMAX; i++){
		pathx[i]=-1;
		pathy[i]=-1;
	}
	pathcounter-=100;

}

void addPathPoint(long x, long y) {
	marker=1;
	Coord c=getGPS(x,y);
	//printf("%ld:%ld,%ld      %f:%f\n",x+ftx,y+fty,basezoom,c.lat,c.lon);
	lat1=c.lat;
	lon1=c.lon;
	mapx1=x;
	mapy1=y;
	if (attraction[0]==NULL)
		attraction[0] = ldImage("system/icons/attraction.png");
	if (pathcounter>=PATHMAX-1)
		freeup100points();
		pathx[pathcounter]=x;
		pathy[pathcounter]=y;
		pathcounter++;
}

void readPath() {
	if ((cpad.Buttons &  PSP_CTRL_CIRCLE) || pathcounter==1 ) {
	if (pathcounter>=PATHMAX-1)
		freeup100points();
	if (pathx[pathcounter-1]!=mapx || pathy[pathcounter-1]!=mapy) {
		double deltapath, bearing;
		GeoRangeAndBearing(c.lat, c.lon, curlat,curlon, &deltapath, &bearing) ;
		pathx[pathcounter]=mapx;
		pathy[pathcounter]=mapy;
		if (curlon==0 && curlat==0)
			deltapath=0;
		curlat=c.lat;
		curlon=c.lon;
		pathdist+=deltapath;
		
		pathcounter++;
	}
	
}
}

void myprint(char *fmt,...) {
va_list args;
	//fprintf(stdout,fmt);
	//fprintf(stdout,"\n");
	memset(messages[messagecount],0,60);
	//strcpy(messages[messagecount++], str); 
	 va_start(args, fmt);
	vsprintf(messages[messagecount++], fmt, args); 
	if (messagecount>14) {
		int i;
		for (i=0; i<15; i++)
			strcpy(messages[i], messages[i+1]);
		messagecount--;
	}
	 va_end(args);
}

int wget(SceSize argc, void* argv) { 
	killwifi=0;
	int tilenumber= calc_mapsize(mapsize);
	ms_write_log("IN wget\n");
	myprint("##### PREPAIRING TO GET %d TILES", tilenumber);
    	myprint("##### CONNECTING.... ");
 int err=0;
 FILE * fp;
	int i;

   // init wlan
    err = pspSdkInetInit();
   if (err != 0) {
      myprint("!!!InetInit failed: %d", err);
      //return -1;
   }
   // print available connections
   myprint("##### AVAILABLE CONNECTIONS:\n");
   for (i = 1; i < 100; i++) // skip the 0th connection
   {
      if (sceUtilityCheckNetParam(i) != 0) break;  // no more
      char name[60];
      sceUtilityGetNetParam(i, 0, (netData*) name);
      myprint("%i: %s\n", i, name);
	if (killwifi) return -1;
   }


   int k=0;
   int j=1;
   
   // WLAN is initialized when PSP IP address is available
	myprint("##### GETTING IP ADDRESS...");
   for (j=1;j<=i;j++) {
   	myprint("TRYING CONNECTION # %d....",j);
   	err = sceNetApctlConnect(j);
   	if (err != 0) {
      		myprint("sceNetApctlConnect failed: %d", err);
      		return -1;
   	}

   k=0;
   while (k<20) {
      if (sceNetApctlGetInfo(8, pspIPAddr) == 0) break;
      myprint("attempt %d out of 20",k++);
	if (killwifi) return -1;
      sceKernelDelayThread(1000 * 1000);  // wait a second
   }
  if (k<20)
	break;
  }

	if (k==20) {
      		myprint("Failed to get IP address: %d", err);
      		return -1;
	}


   myprint("##### PSP IP ADDRESS: %s", pspIPAddr);

   // start DNS resolver
   err = sceNetResolverCreate(&resolverId, resolverBuffer, sizeof(resolverBuffer)); 
 if (err != 0) {
      myprint("sceNetResolverCreate failed: %i", err);
      return -1;
   }




   // resolve host
   myprint("##### RESOLVING HOST...\n");
   const char *host = "mt1.google.com";
   int port = 80;
   struct sockaddr_in addrTo;
   addrTo.sin_family = AF_INET;
   addrTo.sin_port = htons(port);

   err = sceNetInetInetAton(host, &addrTo.sin_addr);
    if (err != 0) {
         myprint("INETATON FAILED...");
         return -1;
    }
   if (err == 0) {
      err = sceNetResolverStartNtoA(resolverId, host, &addrTo.sin_addr, 2, 3);
      if (err != 0) {
         myprint("RESOLUTION FAILED...");
         return -1;
      }
   }

    	Coord tileco=getTileCoord(latitude,longitude,basezoom);
	int ms_original = mapsize;
   	mkdir(dirname, S_IREAD| S_IWRITE);

	int tilex=( (int)(tileco.lon/mapsize)) *mapsize;
	int tiley=( (int)(tileco.lat/mapsize)) *mapsize;

	char curdir[256];
	sprintf(curdir,"%s/coords.txt",dirname);
	FILE * cf=fopen(curdir,"w");
	if (cf!=NULL) {
		fprintf(cf,"%d %d %d\n", tilex, tiley, basezoom);
		fclose(cf);
	}
        int counter=0;;

	while(mapsize > 1) {	
		if (killwifi)
			return -1;
		int mapzoom= ms_original/mapsize;
		sprintf(curdir,"%s/%dx",dirname,mapzoom);
		mkdir (curdir, S_IREAD| S_IWRITE);
		int yc=0;
		while (yc < mapsize) {
			sprintf(curdir,"%s/%dx/%03d",dirname,mapzoom,yc);	
			mkdir (curdir, S_IREAD| S_IWRITE);
			int xc=0;
			
			while (xc < mapsize) {	

   // create socket (in blocking mode)
   int sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0) {
      myprint("SOCKET FAILED");
      return -1;
   }

   // connect (this may block some time)
   err = connect(sock, (struct sockaddr*) &addrTo, sizeof(addrTo));
   if (err != 0) {
      myprint("CONNECT FAILED");
      return -1;
   }
 // send HTTP request
   myprint("LOADING %d OUT OF %d", counter++,tilenumber);
   char request[512];
   sprintf(request,"GET /mt?v=w2.71&x=%d&y=%d&zoom=%d HTTP/1.0\r\nhost: mt1.google.com\r\n\r\n",tilex+xc,tiley+yc,basezoom);
   err = send(sock, request, strlen(request), 0);
   if (err < 0) {
      myprint("send failed");
      return -1;
   }

   // read all
   int capacity = 256;
   int len = 0;
   u8* buffer = (u8*) malloc(capacity);
   while (1) {

      // read data
      int count = recv(sock, (u8*) &buffer[len], capacity - len, 0);

      // in blocking mode it has to return something, otherwise it is closed
      // (which is the default for HTTP/1.0 connections)
      if (count == 0) break;

      if (count < 0) {
         myprint("READ ERROR");
         break;
      }

      // adjust buffer, if needed
      len += count;
      if (len + 256 > capacity) {
         capacity *= 2;
         buffer = (u8*) realloc(buffer, capacity);
         if (!buffer) break;
      }
   }
  // now "len" bytes data are in buffer, add a 0 at end for printing and search for header end
   if (buffer) {
      buffer[len] = 0;
        int j;

        for (j=4; j<len ; j++) {
                if (buffer[j]=='\n' && buffer[j-1]=='\r' &&buffer[j-2]=='\n' &&buffer[j-3]=='\r')
                        break;

        }
        j++;
	sprintf(curdir,"%s/%dx/%03d/%dx%03d%03d.png",dirname,mapzoom,yc,mapzoom,yc,xc);	
         fp=fopen(curdir, "w");
        if (fp!=NULL) {
                for (i=j; i<len ; i++)
                        fputc(buffer[i],fp);
                fclose(fp);
         	//myprint("DONE!");
        }
         //printf("\n\n%s", page);
      free(buffer);
   }

        close(sock);
	xc++;
}
	yc++;
}
	tilex/=2;
	tiley/=2;
	basezoom+=1;
	mapsize/=2;
}
   ///////////// PSP specific part ///////////////

   // term, TODO: this doesn't work
   sceNetApctlDisconnect();
   pspSdkInetTerm();
  myprint("DOWNLOAD COMPLETED!!!");
  sceKernelExitDeleteThread(0);
  return 0;
}




void display_message_no_block(char* l1, char* l2, char* l3) {
	char line1[64];
	char line2[64];
	char line3[64];
	strcpy(line1,l1);
	strcpy(line2,l2);
	strcpy(line3,l3);
	translate(line1);
	translate(line2);
	translate(line3);
	int len=30;

	if (len<=(signed int)strlen(line1)) len=strlen(line1)+4;
	if (len<=(signed int)strlen(line2)) len=strlen(line2)+4;
	if (len<=(signed int)strlen(line3)) len=strlen(line3)+4;

	char * messageline = (char*) malloc(len*sizeof(char));

	int x= PSP_WIDTH/2 - len*5;

	if (menuwindow!=NULL) {
		blitAlphaImageToScreen(0,0,100,128,menuwindow,x-10,70);
		blitAlphaImageToScreen(545-len*10,0,len*10-90,128,menuwindow,x+90,70);
	} else
		fillScreenRect(0x000000cc,x-10,700,len*10+20,128);

	memset(messageline,' ', sizeof(char)*len-1);
	messageline[len-1]=0;
	strncpy(&messageline[(len -strlen(line1))/2],line1, strlen(line1));
	draw_big_string (messageline,x,110);

	memset(messageline,' ', sizeof(char)*len-1);
	messageline[len-1]=0;
	strncpy(&messageline[(len -strlen(line2))/2],line2, strlen(line2));
	draw_big_string (messageline,x,130);

	memset(messageline,' ', sizeof(char)*len-1);
	messageline[len-1]=0;
	strncpy(&messageline[(len -strlen(line3))/2],line3, strlen(line3));
	draw_big_string (messageline,x,150);


	free (messageline);

}


void checkforspeed(double speed, double speedlimit) {
	u64 tick;	
	if (speed>speedlimit && speedlimitflag>0) {
		if (speedlimitflag==1) {
			menuwindow=ldImage("system/warning.png");
			playfile("speedlimit.mp3");
			sceRtcGetCurrentTick(&spTick);
		}
		speedlimitflag=2;
		sceRtcGetCurrentTick(&tick);
		if (tick-spTick<3000000)
       			display_message_no_block("","SPEED WARNING!","");
		else {
			speedlimitflag=0;
			if (menuwindow!=NULL) {
				freeImage(menuwindow);
				menuwindow=NULL;
			}
		}
	}
	if (speed<speedlimit)
		speedlimitflag=1;
}


void readMapListings(void) {
  DIR *dp;
  struct dirent *dir;
 optcount=0;
  if ( (dp = opendir("./maps")) == NULL ) {
	display_message("CRITICAL ERROR","","NO MAPS DIRECTORY FOUND!",0);
	display_message("CRITICAL ERROR","","NO MAPS DIRECTORY FOUND!",0);
	display_message("CRITICAL ERROR","","NO MAPS DIRECTORY FOUND!",0);
	display_message("CRITICAL ERROR","","NO MAPS DIRECTORY FOUND!",0);
	display_message("CRITICAL ERROR","","NO MAPS DIRECTORY FOUND!",1000);
	if (gpsout!=NULL) {
		fclose(gpsout);
		gpsout=NULL;
	}
	unload_group(4096,zipfile,datatype);
        imglist_garbagecollect();
	sceKernelDelayThread(500000);
	if (datatype==1)
		gpsfsClose();
	if (attr_flag==1) 
              turn_off_attractions();
	cleanup_output();
        sceKernelExitGame();
	exit(1);
  }
  while ((dir = readdir(dp)) != NULL) {
	if (dir->d_name[0]!='_') continue;
    //fprintf(stdout,"%s\n", dir->d_name);
    strcpy(options[optcount],dir->d_name);
    optcount++;
  }
  if (config.loadwifi)
 	strcpy(options[optcount++],"===_WIFI_MAP_UPLOAD_===");
  closedir(dp);
}







int locate_address() {
	char label1[64]="::: ADDRESS LOCATOR FORM :::"; translate(label1);
	char label2[64]="POPULATE THE FIELDS BELOW AND PRESS [START] BUTTON"; translate(label2);
	char path[128];
	sprintf(path,"%s/geodata.dat",zipfile);
	FILE * test = fopen( path,"r");
	if (test==NULL)
		test = fopen( "geodata.dat","r");
	if (test==NULL) {
		display_message("ERROR!","GEODATA FILE","IS MISSING!",1000);
		return -1;

	} else
		fclose(test);
	int wait=21;
	int current=0;
	char headers[4][16] = { "STREET:" , "CITY:", "STATE:", "ZIPCODE:"  };
	translate(headers[0]);
	translate(headers[1]);
	translate(headers[2]);
	translate(headers[3]);
	char notes[4][64] = { "ENTER STREET NAME AND NUMBER" ,"ENTER NAME OF THE CITY" , "ENTER 2 LETTER STATE CODE", "ENTER 5 DIGIT ZIP CODE" };
	translate(notes[0]);
	translate(notes[1]);
	translate(notes[2]);
	translate(notes[3]);
	int lens[4]= { 37,20,3,6};
	char values[5][64];
    	//Coord co=getGPS(mapx, mapy);
	memset (values,0,sizeof (values));
	int edit_pos=0;
	char error[32];
	strcpy( error,"");
	sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	Image * menuwindow=ldImage("system/menuwindow.png");
	Image * hilite=ldImage("system/hilite.png");

	 Color * bg= (Color *) malloc ( FRAMEBUFFER_SIZE);
        memcpy(bg, getVramDrawBuffer(), FRAMEBUFFER_SIZE);

	danzeff_load(); 
	if (!(danzeff_isinitialized())) { return 0; } 
	danzeff_moveTo(300,96);
	int blinkcounter=0;
	while(1) {
        while (1) {
		sceCtrlPeekBufferPositive(&cpad, 1); 
                sceKernelDelayThread(1);
		if (bg!=NULL)
                        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
		if (menuwindow!=NULL) {
			blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
			blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
		} else
			fillScreenRect(0x000000cc,12,28,455,232);
                draw_string( label1, PSP_WIDTH/2-26*4, 34);
               	draw_string( label2, 42, 48);
		int i,j;
		for (i=0; i<4; i++) {
               		draw_string( headers[i], 20, 73+15*i);
			if (i==current) {
				for (j=0; j<(signed int)strlen(values[i])+2; j++) {
					drawHiliteBar(hilite, 102+j*8, 69+15*i,1 ); 
					drawHiliteBar(hilite, 102+j*8, 77+15*i,1 ); 
				}
				if (blinkcounter++>10) 
               				draw_string( "-", 110+edit_pos*8, 79+15*i);
				if (blinkcounter++>18)
					blinkcounter=0;
			}
               		draw_string( values[i], 110, 73+15*i);
		}
               	draw_string( error, 35,200 );
	
               	draw_string( notes[current], 15,248 );
    		danzeff_render(); 
		char key= danzeff_readInput(cpad);

		if (key==DANZEFF_START) {
			beep();
			if (strlen(values[0])>2 ) 
				break;
			else
				strcpy(error,"INCORRECT VALUES ENTERED!!!"); translate (error);
		}
		if (wait>100) wait=21;
		wait++;
		if (wait>20) {
		switch (key) 
               { 
          case 0: 
          	break; 
         case 9: if (current>0) {
			current--;
			edit_pos=0;
		}
		beep();
		strcpy( error,"");
		break; 
         case '\n': if (current<3) {
			current++; 
			edit_pos=0;
		}
		strcpy( error,"");
		beep();
		break;
         case DANZEFF_LEFT: if (edit_pos>0) edit_pos--;
		strcpy( error,"");
		beep();
		break; 
         case DANZEFF_RIGHT: if (edit_pos<lens[current]) edit_pos++;
		strcpy( error,"");
		beep();
		break;
         case DANZEFF_SELECT: 
		beep();
		if (menuwindow!=NULL)
			freeImage(menuwindow);
		if (hilite!=NULL)
			freeImage(hilite);
		 if (bg!=NULL)
                	free(bg);
		danzeff_free();
		return 0;
         case DANZEFF_START: beep(); break; 
	case 8: //delete
		beep();
		strcpy( error,"");
		deleteChar(values[current],edit_pos);
		break;
	case 7: //backspace
		beep();
		strcpy( error,"");
		if(edit_pos>0)
		{
			--edit_pos;
			deleteChar(values[current],edit_pos);
		}
		break;

         default: 
		beep();
		strcpy( error,"");
		if ((signed int)strlen(values[current])<lens[current]) {
			insertChar(values[current],edit_pos,key);
			edit_pos++;
		}
		toUpperCase(values[current]);
             break; 
       }
	}
		flipScreen();

	}

	zipcounter=0;
	
	display_message("","SEARCHING...","",0);
	display_message("","SEARCHING...","",0);
	display_message("","SEARCHING...","",0);
	char address[256];
	sprintf(address,"%s,%s,%s %s",values[0],values[1],values[2],values[3]);
	Coord min=getGPS(0,0);
	Coord max=getGPS(TILE_SIZE*TILE_NUM,TILE_SIZE*TILE_NUM);

	double minlat=min.lat, maxlat=max.lat, minlon=min.lon,maxlon=max.lon;
	if ( min.lat > max.lat) {
		maxlat=min.lat;
		minlat=max.lat;
	} 
	if ( min.lon > max.lon) {
		maxlon=min.lon;
		minlon=min.lon;
	}

	address_geolookup(zipfile,values[0], values[3], values[1],values[2],minlat-0.1,minlon-0.1,maxlat+0.1,maxlon+0.1,0);
	if (zipcounter<1)
	{
		flipScreen();
		display_message("NO EXACT MATCH FOUND !","","CONDUCTING RELAXED SEARCH...",0);
		display_message("NO EXACT MATCH FOUND !","","CONDUCTING RELAXED SEARCH...",0);
		display_message("NO EXACT MATCH FOUND !","","CONDUCTING RELAXED SEARCH...",0);
		display_message("NO EXACT MATCH FOUND !","","CONDUCTING RELAXED SEARCH...",0);
		display_message("NO EXACT MATCH FOUND !","","CONDUCTING RELAXED SEARCH...",0);
		display_message("NO EXACT MATCH FOUND !","","CONDUCTING RELAXED SEARCH...",0);
		display_message("NO EXACT MATCH FOUND !","","CONDUCTING RELAXED SEARCH...",300);
		address_geolookup(zipfile,values[0], values[3], values[1],values[2],minlat-0.1,minlon-0.1,maxlat+0.1,maxlon+0.1,1); 
		if (zipcounter<1)
		{
			flipScreen();
			display_message("NO MATCH FOUND !","","CONDUCTING WIDE SEARCH...",0);
			display_message("NO MATCH FOUND !","","CONDUCTING WIDE SEARCH...",0);
			display_message("NO MATCH FOUND !","","CONDUCTING WIDE SEARCH...",0);
			display_message("NO MATCH FOUND !","","CONDUCTING WIDE SEARCH...",0);
			display_message("NO MATCH FOUND !","","CONDUCTING WIDE SEARCH...",0);
			display_message("NO MATCH FOUND !","","CONDUCTING WIDE SEARCH...",0);
			display_message("NO MATCH FOUND !","","CONDUCTING WIDE SEARCH...",300);
			address_geolookup(zipfile,values[0], values[3], values[1],values[2],minlat-0.1,minlon-0.1,maxlat+0.1,maxlon+0.1,2); 
			if (zipcounter<1)
			{
				flipScreen();
				display_message("NO MATCHES FOUND !","","PRESS [X] TO CONTINUE",0);
				display_message("NO MATCHES FOUND !","","PRESS [X] TO CONTINUE",0);
				display_message("NO MATCHES FOUND !","","PRESS [X] TO CONTINUE",0);
				display_message("NO MATCHES FOUND !","","PRESS [X] TO CONTINUE",0);
				display_message("NO MATCHES FOUND !","","PRESS [X] TO CONTINUE",30000);
				wait=0;
			}
		}
	    }
	if (zipcounter>0) break;
	}

	danzeff_free();
	int cur=0;
	int timer=0;
	int topline=0;
	int i;

	while (!(cpad.Buttons & PSP_CTRL_CROSS)) {
                sceCtrlPeekBufferPositive(&cpad, 1);
                sceKernelDelayThread(1);

                if ((cpad.Buttons & PSP_CTRL_CIRCLE) && timer>7/(333/Clock)) {
                        timer=0;
                        break;
                }
                if (cpad.Buttons & PSP_CTRL_SELECT) {

			if (menuwindow!=NULL)
				freeImage(menuwindow);
	 		if (bg!=NULL)
               			free(bg);
                        return -100000;
                } 

                if ((cpad.Buttons & PSP_CTRL_DOWN || cpad.Ly >= 0xD0) && timer>2/(333/Clock)) {
                        if (cur<zipcounter-1) {
                                cur++;
                                if (cur-topline>12)
                                        topline++;
                                beep();
                        }       
                        timer=0;
                }

                if ((cpad.Buttons &  PSP_CTRL_UP || cpad.Ly <= 0x30) && timer>2/(333/Clock)) {
                        timer=0;
                        if (cur>0) {
                                cur--;
                                if (cur<topline && topline>0)
                                        topline--;
                                beep();
                        }
                }


                timer++;
                if (timer>100) timer=11;
                if (bg!=NULL)
                        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
                if (menuwindow!=NULL) {
			blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
			blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
                }else
                        fillScreenRect(0x000000cc,12,28,455,232);
		strcpy(label1,"::: ADDRESS SEARCH RESULTS"); translate(label1);
		sprintf(values[0],"%s [%d] :::",label1,zipcounter);
        	draw_string( values[0], PSP_WIDTH/2-26*4, 34);
		drawHiliteBar(hilite,20,(cur-topline)*15+57,55 ); 
		drawHiliteBar(hilite,20,(cur-topline)*15+65,55 ); 


		for (i=0; i<13 && i<zipcounter; i++) {
		     sprintf(values[0],"%s, %s, %s %s",addrs[i+topline].street,addrs[i+topline].city,addrs[i+topline].state ,addrs[i+topline].zipcode);
                        toUpperCase(values[0]);
                        draw_string( values[0], 30, i*15+61);

                }


                flipScreen();
                sceKernelDelayThread(5);
        }

	Coord  c=getXY(addrs[cur].lat,addrs[cur].lon,basezoom);
	//if (c.lon>0 && c.lon<TILE_SIZE*TILE_NUM && c.lat>0 && c.lat<TILE_SIZE*TILE_NUM){
		mapx=(long int)c.lon;
		mapy=(long int)c.lat;
		marker=1;
		lat1=addrs[cur].lat;
		lon1=addrs[cur].lon;
		mapx1=mapx;
		mapy1=mapy;
		if (attraction[0]==NULL)
			attraction[0] = ldImage("system/icons/attraction.png");
	//} else {
	//	display_message("THIS LOCATION CANNOT","BE DISPLAYED ON THIS MAP","PRESS [X] TO CONTINUE",2000);
//	}


	if (menuwindow!=NULL)
		freeImage(menuwindow);
	if (hilite!=NULL)
		freeImage(hilite);
	 if (bg!=NULL)
               	free(bg);
	return 0;
}


void updateStatusScreen(Image *background, Image *menuwindow, char *label1, char *label2) {
	if (label1==NULL)
		return;
        if (background!=NULL)
                        blitAlphaImageToScreen(0,0,PSP_WIDTH,PSP_HEIGHT,background,0,0);
                if (menuwindow!=NULL) {
                        blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
                        blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
                }else
                        fillScreenRect(0x000000cc,12,28,455,232);
                draw_string( label1, PSP_WIDTH/2-26*4, 34);
                int i;
                for ( i=0; i<15; i++)
                        draw_string_no_ctrl( messages[i], 30, 50+i*12);

                draw_string( label2, 15,248 );

                flipScreen();

}


int map_upload_menu() {

	char label1[64]="::: WIFI MAP UPLOAD FORM :::"; translate(label1);
	char label2[64]="EDIT THE FIELDS BELOW AND PRESS [START] BUTTON"; translate(label2);
	int current=0;
	char headers[5][20] = { "MAP NAME" , "LATITUDE:", "LONGITUDE:", "BASE ZOOM", "MAP SIZE" };
	translate(headers[0]);
	translate(headers[1]);
	translate(headers[2]);
	translate(headers[3]);
	translate(headers[4]);
	char notes[5][64] = { "NOTE: SOME CHARACTERS ARE NOT SUPPORTED" , "NOTE: MUST BE IN DECIMAL FORMAT", "NOTE: MUST BE IN DECIMAL FORMAT", "RANGE: 0-14 (0 IS MOST DETAILED)", "MUST BE 4,8,16,32,64,128,256" };
	translate(notes[0]);
	translate(notes[1]);
	translate(notes[2]);
	translate(notes[3]);
	translate(notes[4]);
	int lens[5]= { 40,10,10,2,3};
	char values[5][64];
    	Coord co=getGPS(mapx, mapy);
	sprintf(values[0],"MAP_FOR_LAT_%10.6f__LON_%10.6f",co.lat, co.lon);
	sprintf(values[1],"%f",co.lat);
	sprintf(values[2],"%f", co.lon);
	sprintf(values[3],"2");
	sprintf(values[4],"16");
	int edit_pos=0;
	char error[32];
	strcpy( error,"");
	sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	Image * menuwindow=ldImage("system/menuwindow.png");
	Image * background=ldImage("system/background.png");
	Image * hilite=ldImage("system/hilite.png");
	danzeff_load(); 
	if (!(danzeff_isinitialized())) { return 0; } 
	danzeff_moveTo(300,96);
	int blinkcounter=0;
        while (1) {
		sceCtrlPeekBufferPositive(&cpad, 1); 
                sceKernelDelayThread(1);
		if (background!=NULL)
			blitAlphaImageToScreen(0,0,PSP_WIDTH,PSP_HEIGHT,background,0,0);
		if (menuwindow!=NULL) {
			blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
			blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
		} else
			fillScreenRect(0x000000cc,12,28,455,232);
                draw_string( label1, PSP_WIDTH/2-26*4, 34);
               	draw_string( label2, 58, 58);
		int i,j;
		for (i=0; i<5; i++) {
               		draw_string( headers[i], 20, 83+15*i);
			if (i==current) {
				for (j=0; j<(signed int)strlen(values[i])+2; j++) {
					drawHiliteBar(hilite,112+j*8, 79+15*i,1 ); 
					drawHiliteBar(hilite,112+j*8, 87+15*i,1 ); 
				}
				if (blinkcounter++>10) 
               				draw_string( "-", 120+edit_pos*8, 89+15*i);
				if (blinkcounter++>18)
					blinkcounter=0;
			}
               		draw_string( values[i], 120, 83+15*i);
		}
               	draw_string( error, 35,200 );
	
               	draw_string( notes[current], 15,248 );
    		danzeff_render(); 
		char key= danzeff_readInput(cpad);

		if (key==DANZEFF_START) {
			beep();
			if (check_fields(values[0], atof(values[1]), atof(values[2]), atoi(values[3]), atoi(values[4]))==1) 
				break;
			else
				strcpy(error,"INCORRECT VALUES ENTERED!!!"); translate(error);


		}

		switch (key) 
               { 
          case 0: 
          	break; 
         case 9: if (current>0) {
			current--;
			edit_pos=0;
		}
		beep();
		strcpy( error,"");
		break; 
         case '\n': if (current<4) {
			current++; 
			edit_pos=0;
		}
		beep();
		strcpy( error,"");
		break;
         case DANZEFF_LEFT: if (edit_pos>0) edit_pos--;
		strcpy( error,"");
		beep();
		break; 
         case DANZEFF_RIGHT: if (edit_pos<lens[current]) edit_pos++;
		strcpy( error,"");
		beep();
		break;
         case DANZEFF_SELECT: 
		beep();
		if (menuwindow!=NULL)
			freeImage(menuwindow);
		if (background!=NULL)
			freeImage(background);
		if (hilite!=NULL)
			freeImage(hilite);
		danzeff_free();
		return 0;
         case DANZEFF_START: beep();break; 
	case 8: //delete
		beep();
		strcpy( error,"");
		deleteChar(values[current],edit_pos);
		break;
	case 7: //backspace
		beep();
		strcpy( error,"");
		if(edit_pos>0)
		{
			--edit_pos;
			deleteChar(values[current],edit_pos);
		}
		break;

         default: 
		beep();
		strcpy( error,"");
		if ((signed int)strlen(values[current])<lens[current]) {
			insertChar(values[current],edit_pos,key);
			edit_pos++;
		}
		toUpperCase(values[current]);
             break; 
       }
		flipScreen();

	}

	danzeff_free();

	sprintf (dirname,"%s/maps/_%s", currentpath,values[0]);
	latitude=atof(values[1]);
	longitude=atof(values[2]);
	basezoom=atoi(values[3]);
	mapsize=atoi(values[4]);
	memset(messages,0,sizeof(messages));

	strcpy(label1, "::: WIFI MAP UPLOAD STATUS :::"); translate(label1);
	strcpy(label2, "PRESS [SELECT] TO CANCEL"); translate(label2);

// start upload thread 
	char tname[32];
	u64 tick;	
	sceRtcGetCurrentTick(&tick);
	sprintf(tname,"WIFI#%lo",(long unsigned int)tick);
	if (thid_wifi>0)
                sceKernelTerminateDeleteThread(thid_wifi);
        //thid_wifi = sceKernelCreateThread(tname, wget, 0x11, 512 * 1024, PSP_THREAD_ATTR_USER, NULL);
        thid_wifi = sceKernelCreateThread(tname, wget, 0x12,  8*1024, PSP_THREAD_ATTR_USER, 0);
	if (wifithid>=0){
   		int ret=sceKernelStartThread(thid_wifi, 0, NULL);
		if (ret!=0) ms_write_log("failed to start wifi\n",wifithid);
        } else {
		ms_write_log("Could not start wifi update thread\n");
	}

        while (!(cpad.Buttons & PSP_CTRL_SELECT)) {
	 	sceCtrlPeekBufferPositive(&cpad, 1); 
                sceKernelDelayThread(1);
		updateStatusScreen(background,menuwindow, label1, label2);
	}

	killwifi=1;


	if (menuwindow!=NULL)
		freeImage(menuwindow);
	if (background!=NULL)
		freeImage(background);
	if (hilite!=NULL)
		freeImage(hilite);

	return 0;
}



int display_menu(char * t, char options[][60] , int size, int cur ) {
        int i, timer=0;
	int topline=0;
	char title[64];
	char filename[128];
	sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(1);
	int circle=0;
	Image *icons[18];
	short imgindx[size];
	strcpy(title,t);
	//let's display icons for this menu...
	if (strcmp(title,"::: SELECT ATTRACTION TYPE :::")==0) {
	translate(title);
	char icon_names[19][10] = { "DEFAULT", "AIRPORT", "TRAFFIC", "FIX", "HOTEL", "ATTRACT", "BANK", "FRIEND", "SHOP", "BAR", "CAMERA", "GAS", "PARK", "CINEMA", "CLUB", "HOSPITAL", "ROUTE", "FOOD" };
	char icon_hints[19][10] = { "DEFAULT", "AIRPORT", "CONGESTI", "REPAIR", "MOTEL", "INTEREST", "MONEY", "POI", "MARKET", "PUB", "SPEED", "FUEL", "PARK", "MOVIE", "DANCE", "HOSPITAL", "WAY", "RESTAUR" };
	int j;
	for (i=0; i<size; i++) {
		imgindx[i]=0;
		for (j=0; j<18; j++) {
			strcpy(filename,options[i]);
			toUpperCase(filename);
			if (strstr(filename,icon_names[j])!=NULL || strstr(filename,icon_hints[j])!=NULL)
				imgindx[i]=j;	
		}
	}
	for (i=0; i<18; i++) {
		sprintf(filename,"system/icons/%s.png",icon_names[i]);
		icons[i]=ldImage(filename);
	}
	} else {
		for (i=0; i<18; i++)
			icons[i]=NULL;
		for (i=0; i<size; i++)
			imgindx[i]=0;
	}

	Color * bg= (Color *) malloc ( FRAMEBUFFER_SIZE);
	memcpy(bg, getVramDrawBuffer(), FRAMEBUFFER_SIZE);

	Image * menuwindow=ldImage("system/menuwindow.png");
	Image * hilite=ldImage("system/hilite.png");

        while (!((cpad.Buttons & PSP_CTRL_CROSS )&& timer>10/(333/Clock))) {
	 	sceCtrlPeekBufferPositive(&cpad, 1); 
                sceKernelDelayThread(1);


		if ((cpad.Buttons & PSP_CTRL_CIRCLE) && timer>7/(333/Clock)) {
			timer=0;
			circle=1;	
			break;
		}
		if ((cpad.Buttons & PSP_CTRL_SELECT) && timer>10/(333/Clock)) {
			if (bg!=NULL)
				free(bg);
			if (menuwindow!=NULL)
				freeImage(menuwindow);
			if (hilite!=NULL)
				freeImage(hilite);
			for (i=0; i<18; i++) {
				if(icons[i]!=NULL)
					freeImage(icons[i]);
				}
			return -100000;

		}

		if ((cpad.Buttons & PSP_CTRL_DOWN || cpad.Ly >= 0xD0) && timer>4/(333/Clock)) {
                        if (cur<size-1) {
                                cur++;
				if (cur-topline>12)
					topline++;
				beep();
			}	
                        timer=0;
                }

        	if ((cpad.Buttons &  PSP_CTRL_UP || cpad.Ly <= 0x30) && timer>4/(333/Clock)) {
                        timer=0;
                        if (cur>0) {
                                cur--;
				if (cur<topline && topline>0)
					topline--;
				beep();
			}
                }


                timer++;
                if (timer>100) timer=11;
		if (bg!=NULL)
			memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
		sprintf(filename,"VER: %s",VERSION);
               	draw_string( filename, 376, 251);
		if (menuwindow!=NULL) {
			blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
			blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
		}else
			fillScreenRect(0x000000cc,12,28,455,232);
                draw_string( title, PSP_WIDTH/2-strlen(title)*4, 34);
		drawHiliteBar(hilite,20,(cur-topline)*15+54,55 ); 
		drawHiliteBar(hilite,20,(cur-topline)*15+62,55 ); 
                for (i=0; i<13 && i<size; i++) {
			strcpy(filename,options[i+topline]);
			//toUpperCase(filename);
                	draw_string_no_und( filename, 30, i*15+58);
			if (icons[imgindx[i+topline]]!=NULL) 
				blitAlphaImageToScreen(0,0,14,14,icons[imgindx[i+topline]],18,i*15+54);

                }

			
		flipScreen();
                sceKernelDelayThread(5);


        }
	memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
	memcpy( getVramDisplayBuffer(),bg, FRAMEBUFFER_SIZE);
	flipScreen();
        sceKernelDelayThread(1);

	if (bg!=NULL)
		free(bg);
	if (hilite!=NULL)
		freeImage(hilite);
	if (menuwindow!=NULL)
		freeImage(menuwindow);
	for (i=0; i<18; i++) {
	if(icons[i]!=NULL)
		freeImage(icons[i]);
	}
	if (circle==1) {
		cur++;
		return -cur;
	} else
		return cur;
}


void load_coords(char* zipfile) {
        FILE *fp;
        char line[60];
        char xstr[30], ystr[30];
	Coord c;
        int rd=0;
	ftx=fty=0;
	basezoom=2;
	if (datatype==1) {
		ftx=(double)gpsfsGetX();
		fty=(double)gpsfsGetY();
		basezoom=gpsfsGetBaseZoom();
               	c=getLatLong((int)ftx,(int)fty,basezoom);
		return;
	} else {
		sprintf(line, "%s/coords.txt",zipfile);
		 fp = fopen(line, "r");
	}
        if (fp==NULL) {
                ms_write_log("load_coords:::couldn't open file: %s\n", "coords.txt");
		lat=lon=lat1=lon1=0;
                return;

        }

        if(fgets(line,60,fp) != NULL) {
                rd=sscanf(line,"%s %s %d",  xstr, ystr , &basezoom);
                if (rd<3) {
                        ms_write_log("load_coords:::couldn't read all values %d\n", rd);
			lat=lon=0;
                } else {
			ftx=atof(xstr);
			fty=atof(ystr);

                	c=getLatLong(atoi(xstr),atoi(ystr),basezoom);
			ms_write_log("lat=%f lon=%f lh=%f lw=%f\n", c.lat, c.lon,c.latHeight, c.lonWidth);
		}

        } else
		lat=lon=0;


        fclose(fp);
}




int detect_size(char *zipfile) {
	int maxsize=2048;
	char filename[128];
	Image * last = NULL;
	Image * first = NULL;
	//check png vs jpg
	if (datatype==1) {
		return gpsfsGetMapDimension();
	} else {
		sprintf(filename, "%s/1x/000/1x000000.jpg",zipfile);
		first=loadfromdir(filename);
	}

	if (first==NULL) {
		strcpy(filetype	, "png");
	} else {
		strcpy(filetype	, "jpg");
		unloadmap(filename);
	}

	while (last == NULL && maxsize>1) {
		maxsize=maxsize/2;
			sprintf(filename, "%s/1x/%03d/1x%03d%03d.%s",zipfile,(maxsize-1),(maxsize-1),(maxsize-1),filetype);
			last=loadfromdir(filename);
	}
	unloadmap(filename);
	return maxsize;
}


static void cleanup_output(void)
{
	running=0;
}


int garbage_collector (SceSize args, void *argp) {
	int oldzoom=zm;
	
	while (running) {

		if (!idle && newzm==zm) {
			
        		imglist_garbagecollect();
			cleanup(mapx,mapy,zm,zipfile,datatype);
			if (zm!=oldzoom) {
				unload_group(zm,zipfile,datatype);
				oldzoom=zm;
			} 

        		imglist_garbagecollect();
		}

		sceKernelDelayThread(100000);
	}
	return 0;
}


int cachemngr (SceSize args, void *argp) {
	int tx;
	int ty;
	int tyfix;
	int topx, topy; 
	int oldzoom=newzm;
	char filename[128]; 

	while (running) {
		if (!idle) {

		tx=  (mapx/newzm - PSP_WIDTH/2 - TILE_SIZE-RADIUS)/TILE_SIZE*TILE_SIZE;
		ty=  (mapy/newzm - PSP_WIDTH/2 - TILE_SIZE-RADIUS)/TILE_SIZE*TILE_SIZE;
		if (tx<0) tx=0;
		if (ty<0) ty=0;
		topx=tx+TILE_SIZE*5;
		if (topx>TILE_NUM*TILE_SIZE/newzm)
			topx=TILE_NUM*TILE_SIZE/newzm;
		topy=ty+TILE_SIZE*5;
		if (topy>TILE_NUM*TILE_SIZE/newzm)
			topy=TILE_NUM*TILE_SIZE/newzm;

		tyfix=ty;



	while (tx < topx ) {
		ty=tyfix;
		while (ty < topy ) {
			if (newzm!=oldzoom || idle) {
				//fprintf(stdout,"breaking..\n");
				break;
			}
			//sceKernelDelayThread(1000);
			if (datatype==1)
				sprintf(filename,"%dx%04d%04d.GPS", newzm, (int)ty/TILE_SIZE, (int) tx/TILE_SIZE);
			else
				sprintf(filename,"%s/%dx/%03d/%dx%03d%03d.%s",zipfile, newzm, (int)ty/TILE_SIZE, newzm, (int)ty/TILE_SIZE, (int) tx/TILE_SIZE,filetype);
			if (tx > mapx/newzm - PSP_WIDTH/2 - TILE_SIZE-RADIUS &&  tx < mapx/newzm + PSP_WIDTH/2+RADIUS && ty > mapy/newzm - PSP_WIDTH - TILE_SIZE-RADIUS && ty < mapy/newzm + PSP_WIDTH+RADIUS)  {
				if (datatype==1)
					loadfromgpsfs((int) tx/TILE_SIZE,(int)ty/TILE_SIZE,newzm,TILE_NUM);
				else
					loadfromdir_check4blank(filename,config.nightmode);
			} else 
				unloadmap(filename);


			ty+=TILE_SIZE;
			//sceKernelDelayThread(10000);
		}
		if (newzm!=oldzoom || idle) {
			oldzoom=newzm;
			//fprintf(stdout,"breaking2..\n");
			break;
		}
		
		tx+=TILE_SIZE;
	}

	}
/*

	if (abs(centerx-mapx)/newzm>50 || abs(centery-mapy)/newzm>50 || oldzoom!=zm) {
		centerx=mapx;
		centery=mapy;
		oldzoom=newzm;
		load_lines("vectordata.dat", zipfile, datatype,  centerx,  centery, oldzoom);
	}
	sceKernelDelayThread(200);
*/

	sceKernelDelayThread(20);
	}
	return 0;
}


 /* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
	cleanup_output();
        sceKernelExitGame();
        return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
        int cbid;

        cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
        sceKernelRegisterExitCallback(cbid);

        sceKernelSleepThreadCB();

        return 0;
}


/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
        int thid = 0;

        thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
        if(thid >= 0)
        {
                sceKernelStartThread(thid, 0, 0);
        }

	MP3_module_start(0,0);

        return thid;
}

int user_main (SceSize args, void *argp); 

int main( int argc, char *argv[] )
{
        strcpy(currentpath,argv[0]);
        currentpath[strlen(currentpath)-10]=0;
	readConfig();

#ifndef GENERIC
	displayStartupLogo();
	generic=0;
	if (config.serialport!=1) { //no need to load/start usbgps modules..
        pspDebugScreenPrintf("- Loading USB Accessory module...");
	if (sceUtilityLoadUsbModule(PSP_USB_MODULE_ACC) < 0) {

        	pspDebugScreenSetBackColor(0x00000077);
                pspDebugScreenPrintf("\n\n\n\nERROR:COULD NOT LOAD usbacc.prx\nGPS WILL BE DISABLED\n\n\n\n");
		sceKernelDelayThread(4000000);
		goto skipgpsinit;
        }else
                pspDebugScreenPrintf("\t ok\n");

        pspDebugScreenPrintf("- Loading USB GPS module... ");
	if (sceUtilityLoadUsbModule(PSP_USB_MODULE_GPS) < 0)
        {
        	pspDebugScreenSetBackColor(0x00000077);
                pspDebugScreenPrintf("\n\n\n\nERROR:COULD NOT LOAD usbgps.prx\nGPS WILL BE DISABLED\n\n\n\n");
		sceKernelDelayThread(4000000);
		goto skipgpsinit;
        }else
                pspDebugScreenPrintf("\t ok\n");

                // Start usb drivers
                pspDebugScreenPrintf("- Starting USB BUS driver...");
                if (sceUsbStart(PSP_USBBUS_DRIVERNAME,0,0)) {
        		pspDebugScreenSetBackColor(0x00000077);
                	pspDebugScreenPrintf("\n\n\n\nERROR:COULD NOT START USB BUS DRIVER\nGPS WILL BE DISABLED\n\n\n\n");
			sceKernelDelayThread(4000000);
			goto skipgpsinit;
                } else
                        pspDebugScreenPrintf("\t\t ok\n");

                pspDebugScreenPrintf("- Starting USB Accessory driver...");
                if (sceUsbStart("USBAccBaseDriver",0,0)) {
        		pspDebugScreenSetBackColor(0x00000077);
                	pspDebugScreenPrintf("\n\n\n\nERROR:COULD NOT START USB ACC DRIVER\nGPS WILL BE DISABLED\n\n\n\n");
			sceKernelDelayThread(4000000);
			goto skipgpsinit;
                } else
                        pspDebugScreenPrintf("\tok\n");

                pspDebugScreenPrintf("- Starting USB GPS driver...");
                if (sceUsbStart(PSP_USBGPS_DRIVERNAME,0,0)) {
        		pspDebugScreenSetBackColor(0x00000077);
                	pspDebugScreenPrintf("\n\n\n\nERROR:COULD NOT START USB GPS DRIVER\nGPS WILL BE DISABLED\n\n\n\n");
			sceKernelDelayThread(4000000);
			goto skipgpsinit;
                } else
                        pspDebugScreenPrintf("\t\t ok\n");

                // Initialize GPS device
                pspDebugScreenPrintf("- Opening USB GPS device...");
                if (sceUsbGpsOpen()) {
        		pspDebugScreenSetBackColor(0x00000077);
                	pspDebugScreenPrintf("\n\n\n\nERROR:COULD NOT OPEN GPS DEVICE\nGPS WILL BE DISABLED\n\n\n\n");
			sceKernelDelayThread(4000000);
			goto skipgpsinit;
                } else
                        pspDebugScreenPrintf("\t\t  ok\n");
	}

skipgpsinit:

		if (config.loadwifi) {
                	pspDebugScreenPrintf("- Loading Inet Modules...");
			if (sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON) < 0) {
				fprintf(stdout,"Error, could not load PSP_NET_MODULE_COMMON\n");
				sceKernelDelayThread(4000000);
			}
			if (sceUtilityLoadNetModule(PSP_NET_MODULE_INET) < 0) {
				fprintf(stdout,"Error, could not load PSP_NET_MODULE_INET\n");
				sceKernelDelayThread(4000000);
			}

		}
#else
	generic=1;
#endif

	ms_write_log("\nmMapThis Started");
        sceRtcGetCurrentTick(&triptime);
	int umid = 0;
        SetupCallbacks();

	loadSioPrx();

        umid = sceKernelCreateThread("user_main_thread",user_main, 33, 32*1024, 0, 0);
        if(umid >= 0)
        {
                sceKernelStartThread(umid, 0, 0);
        } else {
		ms_write_log("Could not start main\n");
	}
	sceKernelDelayThread(2000000);
#ifdef GENERIC
	if (config.loadwifi ) {
		fprintf(stdout,"loading inet modules\n");
		pspSdkLoadInetModules();
	}
#endif

	if (generic==1 || config.serialport==1)
		getSioData();
	else
		holdMainFromExiting();
	sceKernelDelayThread(1000000);
        sceKernelExitGame();
return 0;
}




int user_main (SceSize args, void *argp) {
	memset(streetbuf,0,sizeof(streetbuf));


	char path[128];
	int  timer,  alerttimer;
	int	startupmaploaded=0;
	 tlock = sceKernelCreateSema("tlockSema", 0, 1, 1, 0); 
	 gpslock = sceKernelCreateSema("gpslockSema", 0, 1, 1, 0); 
	 linelock = sceKernelCreateSema("linelockSema", 0, 1, 1, 0); 
	scePowerSetClockFrequency(config.startupfrequency,config.startupfrequency,config.startupfrequency/2);


	initGraphics();
	font_init() ;
	splashscreen();
	initLatLon();

	int firstpass=1;
	int skipmenu;
	sceRtcGetCurrentTick(&triptime);


	while (1) {
	skipmenu=0;
	topmenu:
	marker=0;
	initPath();
	readMapListings(); 
	if (firstpass==1) {
		firstpass=0;
		int i;
		for ( i=0; i<optcount;i++) {
			if (strcmp(options[i],config.startupmap)==0) {
				sprintf(zipfile,"./maps/%s",options[i]);
				skipmenu=1;
				gpsOn=1;
				startupmaploaded=1;
				break;
			}
		
		}

	}
	qsort( options, (optcount-((config.loadwifi>0)?(1):(0))),sizeof(options[0]),opt_cmp);
	if (skipmenu==0) {
	Image * background=ldImage("system/background.png");
	blitAlphaImageToScreen(0,0,PSP_WIDTH,PSP_HEIGHT,background,0,0);
	char label[64]="::: SELECT MAP :::"; translate(label);
        int filenum =display_menu(label, options, optcount, 0 ) ;
	beep();
	if (filenum<0) {
		if (filenum<=-100000)
			break;
		else
		filenum=-filenum-1;
	}
	//fprintf(stdout,"[%s]\n",options[filenum]);
	if (strcmp(options[filenum],"===_WIFI_MAP_UPLOAD_===")==0) {
		if (config.loadwifi)
			map_upload_menu();
		goto topmenu;
	}
	if (background!=NULL)
		freeImage(background);
	sprintf(zipfile,"./maps/%s",options[filenum]);
	gpsOn=0;
	config.rotatemap=1;
	}


	strcpy(attr_file,"_NONE");
	memset(attr_ifile,0,sizeof(attr_ifile));


        display_message("","PLEASE WAIT WHILE MAP IS LOADING","",0);

	if (gpsfsOpen(zipfile)>0)
		datatype=1; //gpsfs 
	else
		datatype=0;



	TILE_NUM=detect_size(zipfile);
	if (TILE_NUM==1)
        	display_message("MAP SIZE SEEMS TO BE VERY SMALL!","CONSIDER RE-BUILDING THIS MAP.","PRESS [X] TO PROCEED..",30000);
	load_coords(zipfile);

	maxzoom=zoom=get_zoom(TILE_NUM);
	if (load_mapp_prefs()==-1){
		mapx = TILE_SIZE*TILE_NUM/2;
		mapy=mapx;
	}
        else
        {
                if(mapy<0 || mapy>TILE_SIZE*TILE_NUM)
                        mapy=(long)((TILE_SIZE*TILE_NUM)/2);
                if(mapx<0 || mapx>TILE_SIZE*TILE_NUM)
                        mapx=(long)((TILE_SIZE*TILE_NUM)/2);
        }
	zm = powerOf(zoom-1);
	newzm=zm;
	oldy=oldx=mapx;

	idle=0;


	if (firsttime==1) {
#ifndef GENERIC
	if (config.serialport==0) {
        thid_gpsmngr = sceKernelCreateThread("gps_manager_thread",gpsmngr, 30, 4096, 0, 0);
        if(thid_gpsmngr >= 0) {
                sceKernelStartThread(thid_gpsmngr, 0, 0);
        }
	else 
                ms_write_log("Failed to start gps_manager_thread!\n");
	}
#endif


        thid_cachemngr = sceKernelCreateThread("cachemngr_thread", cachemngr, 34,1024*8, 0, 0);
	ms_write_log("id=%d\n",thid_cachemngr);
        if(thid_cachemngr >= 0)
        {
                sceKernelStartThread(thid_cachemngr, 0, 0);
        }
	else   
                printf("Failed to start cachemngr_thread!\n");

	sceKernelDelayThread(500000);

        thid_garbagecoll = sceKernelCreateThread("garbage_collector_thread",garbage_collector, 35, 8192, 0, 0);
        if(thid_garbagecoll >= 0)
        {
                sceKernelStartThread(thid_garbagecoll, 0, 0);
        }
	else  
                ms_write_log("Failed to start garbage_collector_thread!\n");
	
	displayInit();

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	firsttime=0;
	}

	timer=0;
	atimer=0;
	alerttimer=50;
	while (!((cpad.Buttons & PSP_CTRL_SELECT) && timer>10/(333/Clock))) {
	timer++;
	atimer++;
	if (timer>100) timer=21;
	if (atimer>100) atimer=21;
	if (alerttimer>100) alerttimer=51;

	 sceCtrlPeekBufferPositive(&cpad, 1); 
	if(!(cpad.Buttons &  PSP_CTRL_HOLD))  
        {
 	 if (cpad.Ly <= 0x30)  {
		if (config.rotatemap==0) {course=0; config.rotatemap=1;}
                gpsOn=0;
                atimer=0;
                my=mapy-round(zm*config.step*cos(course*0.0174532925));
                mx=mapx-round(zm*config.step*sin((course+180)*0.0174532925));
                mapy=(long)my;
                mapx=(long)mx;

        }
        if (cpad.Ly >= 0xD0) {
		if (config.rotatemap==0) {course=0; config.rotatemap=1;}
                gpsOn=0;
                atimer=0;
                my=mapy+round(zm*config.step*cos(course*0.0174532925));
                mx=mapx+round(zm*config.step*sin((course+180)*0.0174532925));
                mapy=(long)my;
                mapx=(long)mx;
        }
        if (cpad.Lx <= 0x30) {
		if (config.rotatemap==0) {course=0; config.rotatemap=1;}
                gpsOn=0;
                atimer=0;
                mx=mapx-round(zm*config.step*cos(-course*0.0174532925));
                my=mapy-round(zm*config.step*sin((-course+180)*0.0174532925));
                mapy=(long)my;
                mapx=(long)mx;
        }
        if (cpad.Lx >= 0xD0) {
		if (config.rotatemap==0) {course=0; config.rotatemap=1;}
                gpsOn=0;
                atimer=0;
                mx=mapx+round(zm*config.step*cos(-course*0.0174532925));
                my=mapy+round(zm*config.step*sin((-course+180)*0.0174532925));
                mapy=(long)my;
                mapx=(long)mx;
         }
	}
/*
	if ((cpad.Buttons &  PSP_CTRL_CIRCLE) && (cpad.Buttons & PSP_CTRL_RTRIGGER) && timer>10/(333/Clock)) {
		timer=0;
		beep();
		if (stick) {
	        	display_message("","STICK TO MAP OFF","",400);
			stick=0;
		} else {
	        	display_message("","STICK TO MAP","",400);
			stick=1;
		}
	}
*/
	if ((cpad.Buttons &  PSP_CTRL_CROSS) && timer>10/(333/Clock)) {
		timer=0;
		if(marker)
		{
			marker=0;
			initPath();
	        	display_message("","MARKER OFF","",800);
		}
		else
		{
			marker=1;
			lat1=c.lat;
			lon1=c.lon;
			mapx1=mapx;
			mapy1=mapy;
			if (attraction[0]==NULL)
				attraction[0] = ldImage("system/icons/attraction.png");
	        	display_message("","MARKER SET","",800);
		}
	}

        if ((cpad.Buttons &  PSP_CTRL_LTRIGGER) && (cpad.Buttons &  PSP_CTRL_RTRIGGER) && timer>10/(333/Clock))
        {
                ms_write_log("\nThread Infos : thid_cachemngr=%d, thid_garbagecoll=%d, thid_usermain=%d thid_wifi=%d\n",thid_cachemngr,thid_garbagecoll,thid_usermain,thid_wifi);
                record_thread_info(thid_cachemngr);
                record_thread_info(thid_garbagecoll);
                record_thread_info(thid_usermain);
                record_thread_info(thid_wifi);
		course=0;
                timer=0;
        }
        else
        {

	if ((cpad.Buttons &  PSP_CTRL_LTRIGGER) && timer>10/(333/Clock) && config.rotatemap>0) {
                course+=2;
                if (course>=360)
                        course-=360;

        }
        if ((cpad.Buttons &  PSP_CTRL_RTRIGGER) && timer>10/(333/Clock) && config.rotatemap>0) {
                course-=2;
                if (course<0)
                        course+=360;

        }
	}

	if ((cpad.Buttons &  PSP_CTRL_SQUARE)||(startupmaploaded) ) {
		startupmaploaded=0;
		gpsOn=1;
		strcpy(warn,"BAD");
		mx=(double)mapx;
		my=(double)mapy;
		deltax=0;
		deltay=0;
	        display_message("G P S   M O D E","","ON",100);
	}



	if (cpad.Buttons &   PSP_CTRL_START) {
		playfile("open.mp3");
		timer=0;
        	int selected=menuMain( 5 ) ;
		switch (selected) {
			case 0:
				menuAddPoi();
				break;
			case 1:
				beep();
				readAttractionsListings();
				if (optcount==0)
					strcpy(options[optcount++],"_NONE");
				qsort( options, optcount,sizeof(options[0]),opt_cmp);
				int fn=display_menu("::: SELECT ATTRACTION TYPE :::",  options , optcount, 0 ); 
				if (fn!=-100000) {
					if (fn < 0 ) {
						
						fn=-fn-1;
						if (strcmp(options[fn],"_NONE")!=0)
							menuPoiSearch(options[fn]);

				} else {

					strcpy(attr_file,options[fn]);
					turn_off_attractions();
					flipScreen();
		       			display_message("","LOADING DATA","",0);
					if (attr_file!=NULL && strcmp(attr_file,"_NONE")!=0) {
						int attr_counter=load_attractions("_MY_POIS", zipfile, datatype,  mapx,  mapy, zoom, "");
				 		attr_counter=load_attractions(attr_file, zipfile, datatype,  mapx,  mapy, zoom, "");
					int k;
					for (k=0;k<=get_image_count(); k++) {
					//fprintf(stdout,"%d---->[%s]\n",get_image_count(),attr_ifile[k]);
					if (strlen(attr_ifile[k])==0)
       	                		 	attraction[k] = ldImage("system/icons/attraction.png");
       		      		   	else {
                              		  char filename1[128];
                     		           sprintf(filename1,"%s/%s", zipfile,attr_ifile[k]);
					if (my_access(filename1, F_OK) != 0)
                               			sprintf(filename1,"system/icons/%s", attr_ifile[k]);
					if (my_access(filename1, F_OK) != 0)
                               			sprintf(filename1,"system/icons/attraction.png");
                     		           attraction[k]=ldImage(filename1);
              		  		}
				}

				attr_flag=1;
				} else {
	       				display_message("WARNING!","NO ATTRACTION","TYPE SELECTED ",1000);
				}
				}
			}

			break;
			case 2:
				locate_address();
				break;
			case 3:
			sprintf(path,"%s/geodata.dat",zipfile);
			FILE * test = fopen( path,"r");
			if (test==NULL)
				test = fopen( "geodata.dat","r");
			if (test==NULL) {
				display_message("ERROR!","GEODATA FILE","IS MISSING!",1000);

			} else {
				fclose(test);
				flipScreen(); 
       				display_message("LOOKING FOR","NEAREST","ADDRESS LOCATIONS",0);
				memset(streetbuf,0,sizeof(streetbuf));
				reverse_lookup(c.lat, c.lon, streetbuf, zipfile);
				sceRtcGetCurrentTick(&displaycounter);
				streetbuf[99]=0;
			}
			break;
			case 5:
				flipScreen(); 
				if (Clock==333) {
                        		Clock=222;
                        		display_message("","CHANGING CLOCK TO 222MHZ","",700);
                        		scePowerSetClockFrequency(Clock,Clock,Clock/2);
			} else {
                        	Clock=333;
                        	scePowerSetClockFrequency(Clock,Clock,Clock/2);
                        	display_message("","CHANGING CLOCK TO 333MHZ","",700);
			}
			break;
			case 6:
				if (showpanels==1) {
					showpanels=0;
                        		display_message("","HIDING THE DATA PANELS","",600);
				} else {
					showpanels=1;
                        		display_message("","SHOWING THE DATA PANELS","",600);
				}
			break;
			case 8:
				if (generic==1 || config.serialport==1)
                                	menuGpsInfoGeneric();
				else
                                	menuGpsInfo290();
                                break;
                        case 9:
                                distance=0;
                                avgspeed=0;
				prevlat=0;
				prevlon=0;
                                sceRtcGetCurrentTick(&triptime);
                                break;
                        case 4:
				if (config.fakefeed==1)  {
					config.fakefeed=0;
					togglefeed=1;
                        		display_message("","READING THE DATA FROM GPS UNIT","",600);
				}else{
					config.fakefeed=1;
					if (gpsout!=NULL) {
						fclose(gpsout);
						gpsout=NULL;
					}
					togglefeed=1;
                        		display_message("","READING THE GPS DATA FROM FILE","",600);
				}
                                break;
			case 7 :
				{
					char filename2[256];
					time_t		ptime;
					struct tm	*MT;
					timer=0;
					ptime=time(NULL);
					MT=localtime(&ptime);
					sprintf(filename2,"ms0:/PSP/PHOTO/Lat_%8.4f__Lon_%8.4f__%02d%02d_%02d%02d%02d.png",c.lat,c.lon,MT->tm_mday,(MT->tm_mon)+1,MT->tm_hour,MT->tm_min,MT->tm_sec);
					savePngImage(filename2,getVramDisplayBuffer(),SCREEN_WIDTH,SCREEN_HEIGHT,PSP_LINE_SIZE,0);
				}
				break;
                        case 11:
                                menuConfiguration();
                                break;
                        case 10:
				menuHelp();
                                break;
		}
	}

	if ((cpad.Buttons & PSP_CTRL_CROSS) && timer>10/(333/Clock) ) {
		timer=0;
		if (config.fakefeed==1 && gpsOn==1) {
			ffreadspeed+=2;
			if (ffreadspeed>10)
				ffreadspeed=1;
		} 
		
	}
	if (cpad.Buttons &  PSP_CTRL_TRIANGLE) {
		beep();

		if (attr_flag==1) {
			turn_off_attractions();
	        	display_message("","POI OFF","",800);
		} else {
	       	display_message("","LOADING DATA","",0);
		if (attr_file!=NULL && strcmp(attr_file,"_NONE")!=0) 
			turn_on_attractions();
		else {
	       		display_message("WARNING!","NO ATTRACTION","TYPE SELECTED ",1000);
		}
		}
	}
	//toggle  gps nmea to file dump



	if ((cpad.Buttons & PSP_CTRL_VOLUP) && timer>10/(333/Clock)) {
		//empty for now
	}
	if ((cpad.Buttons & PSP_CTRL_VOLDOWN) && timer>10/(333/Clock)) {
		//empty for now
	}
	if ((cpad.Buttons & PSP_CTRL_LEFT) && timer>10/(333/Clock))
	{
		if(gpsOn)
		{
			if(!config.fakefeed) {
				beep();
				timer=0;
				if (gpsout!=NULL) {
		       			display_message("GPS DATA","","RECORDING OFF",400);
					fclose(gpsout);
					gpsout=NULL;
				} else {
					gpsout=fopen("gps.txt", "w");
		       			display_message("GPS DATA","","RECORDING ON",400);
				}
			}
		}
	}

	if (transitiontimer>1) { // do projection change animation
		if (config.rotatemap==2) {  	//3D ->2D
			zmfix-=0.17;
			anglefix+=0.033;
		} else {		//2D->3D
			zmfix+=0.17;
			anglefix-=0.033;
		}
		transitiontimer--;
	} else if(transitiontimer==1) {
			zmfix=0;
			anglefix=0;
			transitiontimer--;
			if (config.rotatemap==2) {
				switchcolor=1;
				if (!gpsOn) {
					config.rotatemap=1;
				} else {
					config.rotatemap=0;
					course=0;
				}

			} else {
				config.rotatemap=2;
			}
	}

	if (cpad.Buttons &   PSP_CTRL_RIGHT  && transitiontimer==0) {
				beep();
			if (config.rotatemap==2) {
				transitiontimer=20;
			} else if (config.rotatemap==1 ) {
				if (switchcolor) {
				if (config.nightmode==0)
					config.nightmode=1;
				else
					config.nightmode=0;
				setScene(config.rotatemap);	
				unload_group(4096,zipfile,datatype);
				sceKernelChangeThreadPriority(thid_cachemngr, 33);
				if (config.nightmode)
	       				display_message("SWITCHING TO","","NIGHT MODE",400);
				else
	       				display_message("SWITCHING TO","","DAY MODE",400);
				switchcolor=0;
				} else {
					transitiontimer=20;
					setScene(config.rotatemap);	
				}
			} else {
				config.rotatemap=1;
	       			display_message("SWITCHING TO","","TRACK UP MODE",400);
				setScene(config.rotatemap);	
			}

	}
	if (config.rotatemap==2)
		zvalue=Z;
	else
		zvalue=ZNORM;
	if (zoomtimer>1 && config.smoothzoom) {
		zoomtimer--;
		sceKernelDelayThread(1);
		if (newzm>zm)
			fixvar+=zvalue/2.5;
		else
			fixvar-=zvalue/5.2;
	} else {
		zm=newzm;
		if (zoomtimer>0) {
			fprintf(stdout,"unloading..\n");
			unload_group(zm,zipfile,datatype);
			imglist_garbagecollect();
		}
		sceKernelChangeThreadPriority(thid_cachemngr, 34);
		fixvar=0;
		zoomtimer=0;
	}

	if (cpad.Buttons &   PSP_CTRL_DOWN && zoomtimer==0 ) {
		if (zm<TILE_NUM/2) {
			atimer=0;
			beep();
			zoom++;
			newzm = powerOf(zoom-1);
			sceKernelChangeThreadPriority(thid_cachemngr, 33);
			if (config.smoothzoom)
				zoomtimer=20;
			else
				mysleep(200000);
		}
  	}

	if (cpad.Buttons &  PSP_CTRL_UP  && zoomtimer==0) {
		if (zoom>1) {
			atimer=0;
			beep();
			zoom--;
			newzm = powerOf(zoom-1);
			sceKernelChangeThreadPriority(thid_cachemngr, 33);
			if (config.smoothzoom)
				zoomtimer=20;
			else
				mysleep(200000);
		}

  	}
	c=getGPS(mapx,mapy);
	displayMap();
		

	if (gpsout!=NULL)
		draw_red_string ("REC: ON",27-gpsOn*20,20-gpsOn*15);


	flipScreen();
	if (gpsOn) {
		if (mx<1 && my<1) {
			mx=(double)mapx;
			my=(double)mapy;

		}
		frameDelta=frameTick;
		sceRtcGetCurrentTick(&frameTick);
		frameDelta=frameTick-frameDelta;
		//fprintf(stdout,"deltax:%f factor:%f deltax/:%f\n",deltax,speedfactor,deltax/avgframes);
		mx+=deltax/avgframes;
                my+=deltay/avgframes;
		deltax*=config.speedfactor;
		deltay*=config.speedfactor;
		mapx=(long)mx;
		mapy=(long)my;
		if (stick) {
			Coord cc=getDistanceToRoad(mapx, mapy, zm, TILE_NUM,100);
			mapx=mapx+cc.lat;
			mapy=mapy+cc.lon;
		}
		if (generic==1 || config.serialport==1)
			getGPSMessageGeneric();
		else
			getGPSMessage290();
	
		mysleep(90000);	
	}
//	Coord cc=getDistanceToRoad(mapx, mapy, zm, TILE_NUM,40);
//	mapx=mapx+cc.lat;
//	mapy=mapy+cc.lon;
	mysleep(12000);	
	sceRtcGetCurrentTick(&frameTimer);
	}
	if (gpsout!=NULL) {
		fclose(gpsout);
		gpsout=NULL;
	}
	beep();
	idle=1;
	sceKernelDelayThread(50);
	zm=newzm;
	fixvar=0;
	zoomtimer=0;
	zm++;
	unload_group(4096,zipfile,datatype);
        imglist_garbagecollect();
	sceKernelDelayThread(500000);
	save_map_prefs (mapx,mapy,  zoom); 

	if (datatype==1)
		gpsfsClose();
	if (attr_flag==1) 
              turn_off_attractions();
	}
	running=0;
	cleanup_output();
#ifdef GENERIC
  	sceKernelExitDeleteThread(0);
#endif
        sceKernelExitGame();
	return 0;

}



