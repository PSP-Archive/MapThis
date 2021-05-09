//////////////////////////////////////////////////////////////////////////////
#include "display.h"
#include "main.h"
#include "geocalc.h"
#include "graphics.h"
#include "font.h"
#include "attractions.h"
#include "danzeff.h"
#include "utils.h"
#include "basic.h"
#include "line.h"
#include "menu.h"


//////////////////////////////////////////////////////////////////////////////
/** Local variables */

static Image *cursor=NULL, *scale=NULL, *zoomscale=NULL, *arrows=NULL, 
      	*compass=NULL, *arrow2mark=NULL, *battery=NULL, *topplate=NULL, 
      	*sideplate=NULL, *signalbar=NULL, *arrow=NULL, *messageplate, 
	*current=NULL, *blank=NULL;

static char speedlabel[16]="SPEED";
static char bearinglabel[16]="BEARING";
static char altitudelabel[16]="ALTITUDE";
static char triptimelabel[16]="TRIPTIME";
static char distancelabel[16]="DISTANCE";
static char avgspeedlabel[16]="AVG SPD"; 
static char tomarklabel[16]="TO MARK";
static u64 routeTick;

static int rdcolors[20] = { 0xff24bff2, 0xff73faff, 0xffffffff, 0xff000000, 0xff0f0f0f, 0xffcc7070 };
static int rdcolorlim=6;


//////////////////////////////////////////////////////////////////////////////

 
//////////////////////////////////////////////////////////////////////////////

extern ATTRACTION * attractions[MAXNUM];

void displayStartupLogo()
{
	pspDebugScreenInit();
        pspDebugScreenSetBackColor(0x00770000);
        pspDebugScreenClear();

        pspDebugScreenPrintf("\n_|      _|    _|_|    _|_|_|    _|_|_|_|_|  _|    _|  _|_|_|    _|_|_|\n\
_|_|  _|_|  _|    _|  _|    _|      _|      _|    _|    _|    _|      \n\
_|  _|  _|  _|_|_|_|  _|_|_|        _|      _|_|_|_|    _|      _|_|  \n\
_|      _|  _|    _|  _|            _|      _|    _|    _|          _|\n\
_|      _|  _|    _|  _|            _|      _|    _|  _|_|_|  _|_|_|  ");
        pspDebugScreenPrintf("\n\n\n\n");
}


void splashscreen() {
        Image * splashscreen=ldImage("system/splashscreen.png");
        blitAlphaImageToScreen(0,0,PSP_WIDTH,PSP_HEIGHT,splashscreen,0,0);
        flipScreen();
        sceKernelDelayThread(2000000);
        if (splashscreen!=NULL)
                freeImage(splashscreen);
}




void display_message(char* l1, char* l2, char* l3, int timeout) 
{
        char line1[64];
        char line2[64];
        char line3[64];
        strcpy(line1,l1);
        strcpy(line2,l2);
        strcpy(line3,l3);
        translate(line1);
        translate(line2);
        translate(line3);
        int len=30, counter=0;
	SceCtrlData cpad;
        Color * bg=NULL;
        if (timeout>0) {
                bg= (Color *) malloc ( FRAMEBUFFER_SIZE);
                if (bg!=NULL)
                        memcpy(bg, getVramDrawBuffer(), FRAMEBUFFER_SIZE);
        }

        Image * menuwindow=ldImage("system/menuwindow.png");

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

        flipScreen();
        if (timeout>0)
                sceKernelDelayThread(500000);
        cpad.Buttons=0;         // MIB.42_2 to clear last state
        while (!(cpad.Buttons & PSP_CTRL_CROSS) && counter < timeout && !(cpad.Buttons & PSP_CTRL_RIGHT) ) {
                sceCtrlPeekBufferPositive(&cpad, 1);
                sceKernelDelayThread(1);
                counter++;
                sceKernelDelayThread(1000);
        }
        if (timeout>0 && bg!=NULL) {
                memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
                memcpy( getVramDisplayBuffer(),bg, FRAMEBUFFER_SIZE);
                flipScreen();
                free(bg);
        }
        sceKernelDelayThread(1);

        free (messageline);
        if (menuwindow!=NULL)
                freeImage(menuwindow);

}



// displays the message at the buttom of the screen
void display_alert(char* line1, char* line2)
{
        char l[49];
        memset(l,0,sizeof l);
        //printf ("[%s][%s]\n", line1, line2);
        blitAlphaImageToScreen(0,0,480,68,messageplate,0,204);

        copy_pad(l,line1,48);
        draw_big_string (l,10,212);

        int i;
        memset(l,0,sizeof l);
        for (i=45; i>0; i--) {
                if(line2[i]==' ' || line2[i]==';') {
                        line2[i]='\0';
                        break;
                }
                if(line2[i]==0)
                        break;
        }
        copy_pad(l,line2,48);
        draw_big_string (l,10,230);
        if (i<10)
                return;
        char *line3 = &line2[i+1];
        memset(l,0,sizeof l);
        copy_pad(l,line3,48);
        draw_big_string (l,10,248);
        return;
}



void displayInit()
{
    cursor = ldImage("system/cursor.png");
    arrows = ldImage("system/arrows.png");
    arrow = ldImage("system/arrow.png");
    compass = ldImage("system/compass.png");
    arrow2mark = ldImage("system/arrow2mark.png");
    blank = ldImage("system/blank.png");
    scale = ldImage("system/scale.png");
    battery = ldImage("system/battery.png");
    topplate = ldImage("system/topplate.png");
    messageplate = ldImage("system/messageplate.png");
    sideplate = ldImage("system/sideplate.png");
    signalbar = ldImage("system/signalbar.png");
    zoomscale = ldImage("system/zoomscale.png");

    translate(speedlabel);
    translate(bearinglabel);
    translate(altitudelabel);
    translate(triptimelabel);
    translate(distancelabel);
    translate(avgspeedlabel);
    translate(tomarklabel);
}


void DisplayDeinit()
{
    if (cursor) freeImage(cursor);
    if (arrows) freeImage(arrows);
    if (arrow) freeImage(arrow);
    if (compass) freeImage(compass);
    if (arrow2mark) freeImage(arrow2mark);
    if (blank) freeImage(blank);
    if (scale) freeImage(scale);
    if (battery) freeImage(battery);
    if (topplate) freeImage(topplate);
    if (messageplate) freeImage(messageplate);
    if (sideplate) freeImage(sideplate);
    if (signalbar) freeImage(signalbar);
    if (zoomscale) freeImage(zoomscale);
}



void draw_zoom_scale(int x,int y,Image * zs, int zoom , int maxzoom) {
        int i=0;
        if (maxzoom<2)
                return;
        blitAlphaImageToScreen(0,0,19,3,zs,x,y);
        for (i=0; i<maxzoom; i++) {
                blitAlphaImageToScreen(0,3,19,12,zs,x,y+3+11*i);
        }
        blitAlphaImageToScreen(0,16,19,10,zs,x,y+3+11*(zoom-1));

}

void draw_compass() {
                if (gpsOn) {
                        if (config.rotatemap==1)
                                blitRotatedImage(compass,370+(1-showpanels)*80+16,150,(180+course)*0.0174532925,config.rotatemap);
                        else if (config.rotatemap==2)
                                blitRotatedImage(compass,330+(1-showpanels)*60+16,180,(180+course)*0.0174532925,config.rotatemap );
                } else {
                        if (config.rotatemap==1)
                                blitRotatedImage(compass,460,150,(180+course)*0.0174532925,config.rotatemap );
                        else if (config.rotatemap==2)
                                blitRotatedImage(compass,400,180,(180+course)*0.0174532925,config.rotatemap );

                }

}

void draw_cursor() {
        char printbuf[16];
        if (gpsOn) {
                double hdopflag= 30*hdop/(powerOf(basezoom+zoom+2));
                if (config.rotatemap>0) {
                        if (hdopflag>=15 && hdopflag<80) {
                                if (config.rotatemap==1)
                                        drawCircleScreen(240,136,hdopflag,0xff0000ff);
                                else
                                        if (hdopflag>=30)
                                                drawElipseScreen(240,140,hdopflag*2,0xff0000ff);

                                }
                                if (warn[0]=='O') {
                                        if (config.rotatemap==2)
                                                blitAlphaImageToScreen(0,0,50,35,arrows,215,116);
                                        else
                                                blitRotatedImage(arrow,256,256,180*0.0174532925,0);
                                }else {
                                        if (config.rotatemap==2)
                                                blitAlphaImageToScreen(50,0,50,35,arrows,215,116);
                                        else
                                                blitAlphaImageToScreen(0,0,23,23,cursor,228,124);
                        }
                } else {
                        if (warn[0]=='O') {
                                if (hdopflag>15 && hdopflag<80)
                                        drawCircleScreen(240,136,hdopflag,0xff0000ff);
                                blitRotatedImage(arrow,256,256,(-course+180)*0.0174532925,1);
                        } else
                                blitAlphaImageToScreen(0,0,23,23,cursor,228,124);

                }
        } else {
                if (marker) {
                        if (config.speedfix>1.8)
                                sprintf(printbuf,"[%.2f]", pathdist/1000);
                        else
                                sprintf(printbuf,"[%.2f]", pathdist/1609.344);
                        draw_string (printbuf, 220,114);
                }
                if (config.rotatemap<2)
                        blitAlphaImageToScreen(0,0,23,23,cursor,228,124);
                else
                        blitAlphaImageToScreen(50,0,50,35,arrows,215,116);
        }

}



void draw_top_panel() {
		double plat_min,plat_sec,plon_min,plon_sec;
                char printbuf[64];
                blitAlphaImageToScreen(0,0,14,16,topplate,0,0);
                blitAlphaImageToScreen(96,0,14,16,topplate,14,0);
                if (scePowerGetBatteryLifePercent()<=100 && scePowerGetBatteryLifePercent()>0)
                        blitAlphaImageToScreen(((100-scePowerGetBatteryLifePercent())/25)*16,0,16,10,battery,5,3);
                else
                        blitAlphaImageToScreen(0,0,16,10,battery,5,3);
        if (ftx>=0 || fty>=0) {
                double plat= c.lat;
                double plon= c.lon;
                char NS='N';
                char EW='E';
                if (plat<0) {
                        plat*=-1;
                        NS='S';
                } else if (plat==0)
                        NS=' ';
                if (plon<0) {
                        plon*=-1;
                        EW='W';
                }else if (plon==0)
                        EW=' ';

                switch(config.dms)
                {
                        case 1 :
                                sprintf(printbuf,"%c%3.0f\260%02.3f'  %c%3.0f\260%02.3f'",  NS, floor(plat), 60*(plat-floor(plat)), EW, floor(plon), 60*(plon-floor(plon)));
                        break;
                        case 2 :                                // \260=0xBO(degree) \042=0x22=" \047=0x27='
                                plat_min=60*(plat-floor(plat));
                                plat_sec=59*(plat_min-floor(plat_min));
                                plon_min=60*(plon-floor(plon));
                                plon_sec=59*(plon_min-floor(plon_min));
                                sprintf(printbuf,"%c%3.0f\260%02.0f\047%02.0f\042   %c%3.0f\260%02.0f\047%02.0f\042",  NS, floor(plat), floor(plat_min), plat_sec, EW, floor(plon), floor(plon_min), plon_sec);
                        break;
                        default :
                                sprintf(printbuf,"%c%10.6f\260  %c%10.6f\260", NS,plat, EW,plon);
                        break;
                }
                blitAlphaImageToScreen(0,0,110,16,topplate,28,0);
                blitAlphaImageToScreen(0,0,110,16,topplate,138,0);
                draw_string (printbuf, 38,4);
                if (marker){
                        double range=0;
                        double bearing=0;
                        GeoRangeAndBearing(c.lat, c.lon, lat1,lon1, &range, &bearing) ;
                        if (config.speedfix>1.8) //kilometers
                                sprintf(printbuf,"DIST:%6.2f", range/1000);
                        else //miles
                                sprintf(printbuf,"DIST:%6.2f", range/1609.344);
                        blitAlphaImageToScreen(0,0,110,16,topplate,248,0);
                        draw_string (printbuf, 256,4);

                }
        }
}



void draw_right_panel(char* attrbuf1) {
		double plat_min,plat_sec,plon_min,plon_sec;
                char printbuf[64];
                blitAlphaImageToScreen(0,0,80,34,sideplate,400,0);
                blitAlphaImageToScreen(0,0,80,34,sideplate,400,34);
                blitAlphaImageToScreen(0,0,80,34,sideplate,400,68);
                blitAlphaImageToScreen(0,0,80,34,sideplate,400,102);
                blitAlphaImageToScreen(0,0,80,34,sideplate,400,136);
                blitAlphaImageToScreen(0,0,80,34,sideplate,400,170);
                draw_string (speedlabel ,408,4);
                sprintf(printbuf,"%4.1f",speed*config.speedfix);
                draw_big_string(printbuf,416,15);

                draw_string (bearinglabel ,408,38);
                if (warn[0]=='B')
                        sprintf(printbuf,"  -- ");
                else
                        sprintf(printbuf,"  %2s ",heading[((int)course+22)%360/45]);
                draw_big_string(printbuf,416,47);
                draw_string (altitudelabel ,408,72);
                if (warn[0]=='B')
                        sprintf(printbuf,"  --");
                else
                        sprintf(printbuf," %4.1f",altitude*config.altfix);
                draw_big_string(printbuf,416,83);
                draw_string (triptimelabel ,408,106);
                u64 curTick;
                sceRtcGetCurrentTick(&curTick);
                curTick=(curTick-triptime)/1000000;
                sprintf(printbuf,"%2d:%02d",(int)curTick/60, (int)curTick%60);
                draw_big_string(printbuf,416,117);

                draw_string (distancelabel ,408,140);
                sprintf(printbuf,"%4.1f",config.speedfix*distance*0.53986935161690870809264158073746/1000);
                draw_big_string(printbuf,416,151);

                draw_string (avgspeedlabel ,408,174);
                sprintf(printbuf,"%4.1f",avgspeed);
                draw_big_string(printbuf,416,185);
                if (attrbuf1[0]==0) {
                blitAlphaImageToScreen(0,0,80,34,sideplate,400,204);
                if (marker) {
                        draw_string (tomarklabel ,408,208);
                        double range=0;
                        double bearing=0;
                        GeoRangeAndBearing(c.lat, c.lon, lat1,lon1, &range, &bearing) ;
                        if (config.speedfix>1.8) //kilometers
                                sprintf(printbuf,"%4.1f",range/1000);
                        else //miles
                                sprintf(printbuf,"%4.1f",range/1609.344);
                        draw_big_string(printbuf,410,217);
                        if (config.rotatemap>0)
                                blitRotatedImage(arrow2mark,480,340,(-bearing+course-180)*0.0174532925,1);
                        else
                                //blitAlphaImageToScreen((((int)bearing+22)%360/45)*20,0,20,20,arrow2mark,458,215);
                                blitRotatedImage(arrow2mark,480,340,(-bearing+180)*0.0174532925,1);

                } else {
                        double plat= c.lat;
                        double plon= c.lon;
                        char NS='N';
                        char EW='E';
                        if (plat<0) {
                                plat*=-1;
                                NS='S';
                        } else if (plat==0)
                                NS=' ';
                        if (plon<0) {
                                plon*=-1;
                                EW='W';
                        }else if (plon==0)
                                EW=' ';
                         switch(config.dms)
                        {
                        case 1 :
                                sprintf(printbuf,"%c%3.0f\260%2.2f", NS, floor(plat), 60*(plat-floor(plat)));
                        break;
                        case 2 :                                // \260 (degree) \042=0x22=" \047=0x27='
                                //double plat_min,plat_sec;
                                plat_min=60*(plat-floor(plat));
                                plat_sec=60*(plat_min-floor(plat_min));
                                sprintf(printbuf,"%c%3.0f\260%02.0f\047%02.0f\042",  NS, floor(plat), floor(plat_min), plat_sec);
                        break;
                        default :
                                sprintf(printbuf,"%c%8.4f", NS,plat);
                        break;
                        }

                        draw_string (printbuf ,404,212);
                        switch(config.dms)
                        {
                        case 1 :
                                sprintf(printbuf,"%c%3.0f\260%2.2f", EW, floor(plon), 60*(plon-floor(plon)));
                        break;
                        case 2 :                                // \260 degrees \042=0x22=" \047=0x27='
                                //double plon_min,plon_sec;
                                plon_min=60*(plon-floor(plon));
                                plon_sec=60*(plon_min-floor(plon_min));
                                sprintf(printbuf,"%c%3.0f\260%02.0f\047%02.0f\042",  EW, floor(plon), floor(plon_min), plon_sec);
                        break;
                        default :
                                sprintf(printbuf,"%c%8.4f", EW,plon);
                        break;
                        }

                        draw_string (printbuf ,404,225);

                }
                blitAlphaImageToScreen(0,0,80,34,sideplate,400,238);
                int slide=satellites-2;
                if (slide<0) slide=0;
                if (slide>6) slide=6;
                blitAlphaImageToScreen(slide*30,0,30,10,signalbar,410,256);
                if (scePowerGetBatteryLifePercent()<=100 && scePowerGetBatteryLifePercent()>0)
                        blitAlphaImageToScreen(((100-scePowerGetBatteryLifePercent())/25)*16,0,16,10,battery,456,256);
                else
                        blitAlphaImageToScreen(0,0,16,10,battery,456,256);

                int hours=ltime/10000+config.timezone;
                if (hours<0) hours+=24;
                if (hours>23) hours-=24;
                sprintf(printbuf,"%02d:%02ld:%02ld",   hours,ltime%10000/100,ltime%100);
                draw_string (printbuf,408,244);
}
}




void displayMap() {
    char filename[128];
    int tx,ty, tyfix, tmpx,tmpy;
    int counter=0;
    
    if (zoom<1) zoom = 1;            
	sceKernelDelayThread(10);
        tx=  (mapx/zm - PSP_WIDTH/2 - TILE_SIZE)/TILE_SIZE*TILE_SIZE;
        ty=  (mapy/zm - PSP_WIDTH/2 - TILE_SIZE)/TILE_SIZE*TILE_SIZE;

        if (mapx/zm>TILE_SIZE*TILE_NUM/zm-PSP_WIDTH/2)
                tx-=TILE_SIZE;
        if (mapy/zm>TILE_SIZE*TILE_NUM/zm-PSP_WIDTH/2)
                ty-=TILE_SIZE;

                tyfix=ty;
        while (tx < mapx/zm + PSP_WIDTH ) {
                ty=tyfix;
                while (ty < mapy/zm + PSP_WIDTH ) {
                        sceKernelDelayThread(1);
                        if (datatype==1)
                                        sprintf(filename,"%dx%04d%04d.GPS", zm, (int)ty/TILE_SIZE, (int) tx/TILE_SIZE);
                        else
                                sprintf(filename,"%s/%dx/%03d/%dx%03d%03d.%s",zipfile,zm, (int) ty/TILE_SIZE, zm, (int)ty/TILE_SIZE, (int) tx/TILE_SIZE,filetype);
                        current=loadfromcache(filename);
                        tmpx=tx-mapx/zm+PSP_WIDTH/2;
                        tmpy=ty-mapy/zm+PSP_HEIGHT/2;

                        if (current!=NULL) {
                                blitMapTile(current, tmpx+128+16,tmpy+128+120,(course-180)*0.0174532925,config.rotatemap, fixvar+zmfix,anglefix );
                        } else
                                blitMapTile(blank, tmpx+128+16,tmpy+128+120,(course-180)*0.0174532925,config.rotatemap, fixvar+zmfix, anglefix );


                        ty+=TILE_SIZE;
                sceKernelDelayThread(1);

                }
                tx+=TILE_SIZE;
        }




        counter++;

        if (marker)
                readPath();

        if (attr_flag && waypoint && gpsOn) {
                u64 newTick;
                sceRtcGetCurrentTick(&newTick);
                if (newTick-routeTick>600*1000000) { //check route every 10 min
                        turn_off_attractions();
                        turn_on_attractions();
                        routeTick=newTick;
                }
        }

        //display attractions....
        memset(attrbuf1,0,sizeof(attrbuf1));
        memset(attrbuf2,0,sizeof(attrbuf2));
        if (strlen(streetbuf)>0) {
                streetbuf[39]=0;
                strcpy(attrbuf1,streetbuf);
                strcpy(attrbuf2,&streetbuf[40]);
                u64 tm;
                sceRtcGetCurrentTick(&tm);
                if (tm-displaycounter>5000000) {
                        memset(streetbuf,0,128);
                }
        }
        guStart();
 int i, j;
        float thick=0.6;
        double ang= (180+course)*0.0174532925;
                //sceKernelWaitSema(linelock,1,0);
        for ( i=0; i<MAXLINES; i++) {

                if (get_line(i,&l)==-1)
                        break;
                //printf("!!! %d - %ld %ld|%ld %d|%s,%ld,%ld,%ld,%ld,%ld [%s]\n",i,l->sx,l->sy,l->ex,l->ey,l->name, l->stype, l->fromaddrl, l->toaddrl, l->fromaddrr, l->toaddrr, l->postcode);
                Color c=0x55ffffff;
                if (l.stype<41)
                        c=0x5500ffff;
                if (l.stype<0)
                        c=0x55f0000;
                if (atimer>10/(333/Clock)) {
                        if (isOnLine(l,mapx,mapy)) {
                                toUpperCase(l.name);
                                sprintf(attrbuf1,"%s (%ld)",l.name, l.stype);
                                sprintf(attrbuf2,"LEFT:%ld-%ld RIGHT:%ld-%ld",l.fromaddrl,l.toaddrl,l.fromaddrr,l.toaddrr);
                                c=0x5500ff00;

                        }
                }
                thick=0.6;
                if (zoom>4 && l.stype>=41 && l.stype<100)
                        continue;
                if (zoom==4 && l.stype>=41 && l.stype<100)
                        thick=0;
                DrawThickLine((l.sx/zm-4-mapx/zm)+PSP_WIDTH/2+20,(l.sy/zm-4-mapy/zm)+PSP_HEIGHT/2+124,(l.ex/zm-4-mapx/zm)+PSP_WIDTH/2+20,(l.ey/zm-4-mapy/zm)+PSP_HEIGHT/2+124,c,0.5f+thick, config.rotatemap,ang, fixvar+zmfix, anglefix);
        }
        // draw path to mark
        if (marker)
                DrawThickLine((mapx1/zm-4-mapx/zm)+PSP_WIDTH/2+4+16,(mapy1/zm-4-mapy/zm+4)+PSP_HEIGHT/2+120,PSP_WIDTH/2+16,PSP_HEIGHT/2+120,0x33ffffff,2, config.rotatemap,ang, fixvar+zmfix, anglefix);
        Color cc=0x220000ff;
        if (config.nightmode)
                cc=0x2200ff00;
        for ( i=1; i<pathcounter-1; i++) {
                DrawThickLine((pathx[i]/zm-4-mapx/zm)+PSP_WIDTH/2+20,(pathy[i]/zm-4-mapy/zm)+PSP_HEIGHT/2+124,(pathx[i+1]/zm-4-mapx/zm)+PSP_WIDTH/2+20,(pathy[i+1]/zm-4-mapy/zm)+PSP_HEIGHT/2+124,cc,2, config.rotatemap,ang, fixvar+zmfix, anglefix);
        }
        renderLines( config.rotatemap,(180+course)*0.0174532925, fixvar+zmfix, anglefix);
        guFinish();

        sceKernelDelayThread(10);
        guStart();
        // draw "TO" mark icon
        if ( marker && abs(mapx1-mapx)<PSP_WIDTH*zm && abs(mapy1-mapy) < PSP_HEIGHT*zm){
                attr_width=attraction[0]->imageWidth;
                attr_height=attraction[0]->imageHeight;
                blitSprite(attraction[0],(mapx1/zm-mapx/zm)+PSP_WIDTH/2+16,(mapy1/zm-mapy/zm)+PSP_HEIGHT/2+120,ang,config.rotatemap,0, fixvar+zmfix, anglefix);
        }
        if (attr_flag==1 && atimer>10/(333/Clock) ) {
                for (j=attractions_count()-1; j>=0; j--) {
                        if (attractions[j]!=NULL && abs(attractions[j]->x-mapx)<PSP_WIDTH*zm && abs(attractions[j]->y-mapy) < PSP_HEIGHT*zm){
                                if (attraction[attractions[j]->i]==NULL)
                                        continue;

                                attr_width=attraction[attractions[j]->i]->imageWidth;
                                attr_height=attraction[attractions[j]->i]->imageHeight;
                        if (attractions[j]!=NULL && abs(attractions[j]->x-mapx)<PSP_WIDTH*zm && abs(attractions[j]->y-mapy) < PSP_HEIGHT*zm){
                                blitSprite(attraction[attractions[j]->i],(attractions[j]->x/zm-mapx/zm)+PSP_WIDTH/2+16,(attractions[j]->y/zm-mapy/zm)+PSP_HEIGHT/2+120,ang,config.rotatemap,0,fixvar+zmfix, anglefix);
                                if (abs(attractions[j]->x/zm-mapx/zm)<attr_width/4+attractions[j]->waypoint*config.warn_dist/zm && abs(attractions[j]->y/zm-mapy/zm) < attr_height/2+attractions[j]->waypoint*config.warn_dist/zm) {
                                        if (attractions[j]->waypoint ) {
                                                if (attrbuf2[0]==0 || strncmp(attrbuf2,"MIDPOINT",8)==0) {
                                                        memset(attrbuf1,0,sizeof(attrbuf1));
                                                        memset(attrbuf2,0,sizeof(attrbuf2));
                                                        sprintf(attrbuf1,"%s",attractions[j]->name);
                                                sprintf(attrbuf2,"%s",attractions[j]->details);
                                        }
                                        } else {
                                                sprintf(attrbuf1,"POI: %s",attractions[j]->name);
                                                sprintf(attrbuf2,"%s",attractions[j]->details);
                                        }
                                if (attraction[attractions[j]->i]==NULL)
                                        continue;
                                        play_alert(attractions[j]->message);
                                        if (duration==0)
                                                attractions[j]->message=0;
                                        attr_width=attraction[attractions[j]->i]->imageWidth;
                                        attr_height=attraction[attractions[j]->i]->imageHeight;
                                        blitSprite(attraction[attractions[j]->i],(attractions[j]->x/zm-mapx/zm)+PSP_WIDTH/2+16,(attractions[j]->y/zm-mapy/zm)+PSP_HEIGHT/2+120,ang,config.rotatemap,1, fixvar+zmfix, anglefix);
                                }
                        }
                }
        }

        }
        guFinish();

	sceKernelDelayThread(10);
        if (transitiontimer==0) {
                draw_compass();
                draw_cursor();
        }
        if ((ftx>=0 || fty>=0) && config.rotatemap<2 && transitiontimer==0)  //draw scale
                blitAlphaImageToScreen(0,(zoom+basezoom+1)*30,123,30,scale,5,PSP_HEIGHT-35);
        draw_zoom_scale(5,20,zoomscale,zoom, maxzoom);

        if (showpanels) {                       // trigger panel display
                if (gpsOn) {
                        draw_right_panel(attrbuf1);
                        checkforspeed(speed*config.speedfix, config.speedlimit);
                } else
                        draw_top_panel();
        }
	
	if (strlen(attrbuf1)>0 && atimer>10/(333/Clock)) {
                display_alert(attrbuf1,attrbuf2);
        }
}


// this may be used for sticking to road function via color recognition
Coord getDistanceToRoad(int mapx,int mapy,int zm,int TILE_NUM,int size) 
{
        int tx=0, ty=0;
        int tyfix=0;
        int tmpx,tmpy;
        char filename[256];
        Image * newimage = createImage(size,size);
        tx=  (mapx/zm - size/2 - TILE_SIZE)/TILE_SIZE*TILE_SIZE;
        ty=  (mapy/zm - size/2 - TILE_SIZE)/TILE_SIZE*TILE_SIZE;

        if (mapx/zm>TILE_SIZE*TILE_NUM/zm-size/2)
                tx-=TILE_SIZE;
        if (mapy/zm>TILE_SIZE*TILE_NUM/zm-size/2)
                ty-=TILE_SIZE;

                tyfix=ty;
        while (tx < mapx/zm + size) {
                ty=tyfix;
                while (ty < mapy/zm + size) {
                        //sceKernelDelayThread(1);
                        if (datatype==1)
                                        sprintf(filename,"%dx%04d%04d.GPS", zm, (int)ty/TILE_SIZE, (int) tx/TILE_SIZE);
                        else
                                sprintf(filename,"%s/%dx/%03d/%dx%03d%03d.%s",zipfile,zm, (int) ty/TILE_SIZE, zm, (int)ty/TILE_SIZE, (int) tx/TILE_SIZE,filetype);
                        current=loadfromcache(filename);
                        tmpx=tx-mapx/zm+size/2;
                        tmpy=ty-mapy/zm+size/2;

                        if (newimage!=NULL) {
                        if (current!=NULL)
                                blitImageToImage(0,0,TILE_SIZE,TILE_SIZE,current,tmpx,tmpy,newimage);
                        else
                                blitImageToImage(0,0,TILE_SIZE,TILE_SIZE,blank,tmpx,tmpy,newimage);
                        }

                        ty+=TILE_SIZE;

                }
                tx+=TILE_SIZE;
        }

        int xx=0,yy=0;
        if (newimage !=NULL) {
                int rad=0;
                int match=0;
                double alpha;
                //Color pixel1=getPixelImage(size/2,size/2,newimage);
                //fprintf(stdout, "%X:: \n",pixel1);
                while (rad<size/2 && match==0) {
                for (alpha=90-course; alpha<=270-course; alpha+=180) {
                        //fprintf(stdout,"alpha :%f\n",alpha);
                        xx=(int)(size/2+rad*sin(alpha*0.0174532925));
                        yy=(int)(size/2+rad*cos(alpha*0.0174532925));
                        Color pixel=getPixelImage(xx,yy,newimage);

                        //xtoi("ff24bff2", &c1);
                        int c;
                        for (c=0; c<rdcolorlim; c++) {
                                if (pixel==rdcolors[c] ) {
                                //      fprintf(stdout, "%X:: x=%d, y=%d\n",pixel,xx-size/2, yy-size/2);
                                        match=1;
                                        break;
                                }
                        }
                        if (match==1)
                                break;
                }
                rad++;
                //fprintf(stdout,"HERE! :%d\n",rad);
        }

        if (match!=1) {
                xx=size/2;
                yy=size/2;
        }

        freeImage(newimage);
        }
        Coord c;
        c.lat=(double)(xx-size/2);
        c.lon=(double)(yy-size/2);
        if (abs(c.lat)<3 && abs(c.lon)<3) {
                c.lat=0.0;
                c.lon=0.0;
        }

        return c;
}

