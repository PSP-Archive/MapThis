#include "main.h"
#include "graphics.h"
#include "font.h"
#include "version.h"
#include "basic.h"
#include "danzeff.h"
#include "utils.h"
#include "menu.h"
#include "geocalc.h"

extern nmeap_gsv_t     gsv;
extern int gpslock;
int shownmea=0;


int menuMain(int cur ) {
	 SceCtrlData cpad;
        char label[64]="::: SELECT ACTION :::"; translate(label);
        int  timer=0;
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(1);

        Color * bg= (Color *) malloc ( FRAMEBUFFER_SIZE);
        memcpy(bg, getVramDrawBuffer(), FRAMEBUFFER_SIZE);

        Image * menubuttons=ldImage("system/menubuttons.png");
        Image * menuwindow=ldImage("system/menuwindow.png");

        while (!((cpad.Buttons & PSP_CTRL_CROSS )&& timer>10/(333/Clock))) {
                sceCtrlPeekBufferPositive(&cpad, 1);
                sceKernelDelayThread(1);


                if ((cpad.Buttons & PSP_CTRL_CIRCLE) && timer>7/(333/Clock)) {
                        timer=0;
                        break;
                }
                if ((cpad.Buttons & PSP_CTRL_SELECT) && timer>10/(333/Clock)) {
                        if (bg!=NULL)
                                free(bg);
                        if (menubuttons!=NULL)
                                freeImage(menubuttons);
                        if (menuwindow!=NULL)
                                freeImage(menuwindow);
                        return -100000;

                }

                if ((cpad.Buttons & PSP_CTRL_DOWN || cpad.Ly >= 0xD0) && timer>10/(333/Clock)) {
                        if (cur<8)
                                cur+=4;
                        timer=0;
                        beep();
                }

                if ((cpad.Buttons &  PSP_CTRL_UP || cpad.Ly <= 0x30) && timer>10/(333/Clock)) {
                        timer=0;
                        beep();
                        if (cur>3)
                                cur-=4;
                }
                if ((cpad.Buttons &  PSP_CTRL_LEFT || cpad.Lx <= 0x30) && timer>10/(333/Clock)) {
                        timer=0;
                        beep();
                        if (cur!=0 && cur!=4 && cur!=8)
                                cur--;

                }
                if ((cpad.Buttons &  PSP_CTRL_RIGHT || cpad.Lx >= 0xD0) && timer>10/(333/Clock)) {
                        timer=0;
                        beep();
                        if (cur!=3 && cur!=7 && cur!=11)
                                cur++;

                }
                timer++;
                if (timer>100) timer=11;
                if (bg!=NULL)
                        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
                if (menuwindow!=NULL)  {
                        blitAlphaImageToScreen(0,0,358,122,menuwindow,34,28);
                        blitAlphaImageToScreen(0,38,358,90,menuwindow,34,150);

                        blitAlphaImageToScreen(400,0,55,122,menuwindow,392,28);
                        blitAlphaImageToScreen(400,38,55,90,menuwindow,392,150);
                }
                draw_string( label, PSP_WIDTH/2-26*3, 33);
                if (menubuttons!=NULL) {
                        blitAlphaImageToScreen(0,0,400,60,menubuttons,40,50);
                        if (config.fakefeed==1)
                                blitAlphaImageToScreen(0,61,100,60,menubuttons,40,112);
                        else
                                blitAlphaImageToScreen(0,181,100,60,menubuttons,40,112);
                        if (Clock<=222)
                                blitAlphaImageToScreen(100,61,100,60,menubuttons,140,112);
                        else
                                blitAlphaImageToScreen(100,181,100,60,menubuttons,140,112);
                        if (showpanels==1)
                                blitAlphaImageToScreen(200,61,100,60,menubuttons,240,112);
                        else
                                blitAlphaImageToScreen(200,181,100,60,menubuttons,240,112);
                        blitAlphaImageToScreen(300,61,100,60,menubuttons,340,112);

                        blitAlphaImageToScreen(0,121,400,60,menubuttons,40,174);
                        blitAlphaImageToScreen(303,185,114,72,menubuttons,33+100*(cur%4),45+61*(cur/4));
                        if (timer%(30/(333/Clock))<15/(333/Clock))
                                blitAlphaImageToScreen(303,185,114,72,menubuttons,33+100*(cur%4),45+61*(cur/4));

                }

                flipScreen();
                sceKernelDelayThread(5/(333/Clock));


        }
        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
        memcpy( getVramDisplayBuffer(),bg, FRAMEBUFFER_SIZE);
        flipScreen();
        sceKernelDelayThread(1);

        if (bg!=NULL)
                free(bg);
        if (menubuttons!=NULL)
                freeImage(menubuttons);
        if (menuwindow!=NULL)
                freeImage(menuwindow);
                return cur;

}


int menuHelp() {
	SceCtrlData cpad;
        char label[64]="::: HELP :::"; translate(label);
        int i=0, timer=0;
        int topline=0;
        char line[60];
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(1);
        int lines=100;
        char * options[lines];
        FILE * fp=fopen("system/help.txt","r");
        if (fp!=NULL) {
        for (i=0; i<lines; i++)  {
                options[i]=(char*)malloc(64*sizeof(char));
                if (options[i]==NULL) {
                        int j;
                        for (j=0;j<i; j++)
                                free(options[j]);
                        fclose(fp);
                        return -1;
                } else
                        memset(options[i],0,64*sizeof(char));
        }
        Color * bg= (Color *) malloc ( FRAMEBUFFER_SIZE);
        memcpy(bg, getVramDrawBuffer(), FRAMEBUFFER_SIZE);

        i=0;
        memset(line,0,sizeof(line));
         while(fgets(line,60,fp) != NULL && i<lines )  {
                if (line[strlen(line)-1]=='\n' || line[strlen(line)-1]=='\r')
                        line[strlen(line)-1]=0;
                if (line[strlen(line)-1]=='\n' || line[strlen(line)-1]=='\r')
                        line[strlen(line)-1]=0;
                strcpy(options[i++],line);
                memset(line,0,sizeof(line));
        }


        int size=i;
        Image * menuwindow=ldImage("system/menuwindow.png");

         sceCtrlPeekBufferPositive(&cpad, 1);
        while (!(cpad.Buttons & PSP_CTRL_CIRCLE)) {
                sceCtrlPeekBufferPositive(&cpad, 1);
                sceKernelDelayThread(1);


                if ((cpad.Buttons & PSP_CTRL_CIRCLE) && timer>2) {
                        break;
                }
                if (cpad.Buttons & PSP_CTRL_SELECT)
                        break;

                if ((cpad.Buttons & PSP_CTRL_DOWN || cpad.Ly >= 0xD0) && timer>1) {
                                if (size-topline>17)
                                        topline++;
                        timer=0;
                }

                if ((cpad.Buttons &  PSP_CTRL_UP || cpad.Ly <= 0x30) && timer>1) {
                        timer=0;
                        if (topline>0)
                                topline--;
                }


                timer++;
                if (timer>100) timer=11;
                if (bg!=NULL)
                        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
                if (menuwindow!=NULL) {
                        blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
                        blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
                 } else
                        fillScreenRect(0x000000cc,12,28,455,232);
                draw_string( label, PSP_WIDTH/2- 12*4, 34);
                for (i=0; i<17 && i<size; i++) {
                        strcpy(line,options[i+topline]);
                        //toUpperCase(line);
                        draw_string( line, 26, i*12+51);

                }


                flipScreen();
                sceKernelDelayThread(1);


        }
	beep();
        for (i=0; i<lines; i++) {
                if (options[i]!=NULL)
                        free(options[i]);
        }
        if (bg!=NULL)
                free(bg);
        if (menuwindow!=NULL)
                freeImage(menuwindow);
        fclose(fp);

        } else return -1;
        return 1;
}


int edit_line(char * line) {
	SceCtrlData cpad;
        int timer=0;
        char templine[64];
        int changed=0;
        strcpy(templine, line);
        //fprintf(stdout,"{{%s}}\n", templine);
        danzeff_load();
        if (!(danzeff_isinitialized())) { return 0; }
        danzeff_moveTo(300,96);
        Color * bg= (Color *) malloc ( FRAMEBUFFER_SIZE);
        memcpy(bg, getVramDrawBuffer(), FRAMEBUFFER_SIZE);
        int edit_pos=strlen(line)-1;
        if (edit_pos<0) edit_pos=0;
        while (1) {
                sceCtrlPeekBufferPositive(&cpad, 1);
                sceKernelDelayThread(1);
                if (bg!=NULL)
                        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
                fillScreenRect(0x00000000,26,50,8*53,13);
                fillScreenRect(0xaaaaaaaa,25,49,8*53,13);

                danzeff_render();
                char key= danzeff_readInput(cpad);
                if (key==DANZEFF_SELECT) {
                        beep();
                        break;
                }
                if (key==DANZEFF_START) {
                        beep();
                        strcpy(line,templine);
                        changed=1;
                        break;
                }
                switch (key)
               {
          case 0:  break;
          case 9: templine[edit_pos]++;
                beep();
                break;
          case 10:   templine[edit_pos]--;
                beep();
                break; 
          //case '\n': beep();break;

         case DANZEFF_LEFT: if (edit_pos>0) edit_pos--;
                beep();
                break;
         case DANZEFF_RIGHT: if (edit_pos<53) edit_pos++;
                        if (edit_pos>strlen(templine))
                                strcat(templine," ");
                beep();
                break;
        case 8: //delete
                beep();
                deleteChar(templine,edit_pos);
                break;
        case 7: //backspace             // MIB.42_4
                beep();
                if(edit_pos>0)
                {
                        --edit_pos;
                        deleteChar(templine,edit_pos);
                }
                break;
         default:
                beep();
                if ((signed int)strlen(templine)<53) {
                        insertChar(templine,edit_pos,key);
                        edit_pos++;
                }
                toUpperCase(templine);
             break;
        }

                if (timer>100) timer=21;
                timer++;
                draw_string( templine, 26, 51);
                if (timer%20>10)
                        draw_string_no_ctrl( "_", 26+8*edit_pos, 53);
                flipScreen();

        }
        if (bg!=NULL)
                free(bg);
         danzeff_free();
        return changed;
}

void initConfig() {
	config.baud=38400;
        config.fakefeed=0;
        config.initLocation=1;
        config.dms=0;
        config.speedfix=1.151;
        config.speedlimit=1000;
        config.nightmode=0;
        config.altfix=1;
        config.timezone=0;
        config.warn_dist=35;
        config.speedfactor=0.99;
        config.turnspeed=2;
        memset(config.startupmap,0,sizeof(config.startupmap));
        config.loadwifi=1;
        config.startupfrequency=333;
        config.satinfofrequency=111;
        config.rotatemap=1;
        config.step=2.0f;
        config.smoothzoom=0;
        config.cachemapindex=0;
        config.debug_logging=0;
        config.cachedelay=0;
        config.serialport=0;
}


void readConfig() {
	initConfig();
        FILE * fp=fopen("system/config.txt", "r");
        if (fp==NULL)
                return;
        char line[128];
        pspTime rtime;
        sceRtcGetCurrentClockLocalTime(&rtime);
        int hhr = rtime.hour;
        int lochr = hhr;
        sceRtcGetCurrentClock(&rtime,0);
        int utchour = rtime.hour;
        int utchr = utchour;
        config.timezone = lochr - utchr;

        while(fgets(line,128,fp) != NULL) {
                if (line[0]=='#')
                        continue;
                if (strncmp(line,"BAUD=",5)==0) {
                        config.baud=atoi(&line[5]);
                }
                if (strncmp(line,"FAKEFEED=",9)==0) {
                        config.fakefeed=atoi(&line[9]);
                }
                if (strncmp(line,"INITLOCATION=",9)==0) {
                        config.initLocation=atol(&line[13]);
                }
                if (strncmp(line,"DMS=",4)==0) {
                        config.dms=atoi(&line[4]);
                }
                if (strncmp(line,"SPEEDFIX=",9)==0) {
                        config.speedfix=atof(&line[9]);
                }
                if (strncmp(line,"SPEEDLIMIT=",11)==0) {
                        config.speedlimit=atof(&line[11]);
                }
                if (strncmp(line,"NIGHTMODE=",10)==0) {
                        config.nightmode=atoi(&line[10]);
                }
                if (strncmp(line,"ALTFIX=",7)==0) {
                        config.altfix=atof(&line[7]);
                }
                if (strncmp(line,"WARNINGDISTANCE=",16)==0) {
                        config.warn_dist=atoi(&line[16]);
                }
                if (strncmp(line,"SPEEDFACTOR=",12)==0) {
                        config.speedfactor=atof(&line[12]);
                }
                if (strncmp(line,"TURNSPEED=",10)==0) {
                        config.turnspeed=atoi(&line[10]);
                }
                if (strncmp(line,"STARTUPMAP=",11)==0) {
                        strcpy(config.startupmap,&line[11]);
                                config.startupmap[strlen(config.startupmap)-1]='\0';
                }

                if (strncmp(line,"LOADWIFI=",9)==0) {
                        config.loadwifi=atoi(&line[9]);
                }
                if (strncmp(line,"STARTUPFREQUENCY=",17)==0) {
                        config.startupfrequency=atoi(&line[17]);
                }
                if (strncmp(line,"SATINFOFREQUENCY=",17)==0) {
                        config.satinfofrequency=atoi(&line[17]);
                }
                if (strncmp(line,"STARTUPSCREENMODE=",18)==0) {
                        config.rotatemap=atoi(&line[18]);
                }
                if (strncmp(line,"CURSORSPEED=",12)==0) {
                        config.step=atof(&line[12]);
                }
                if (strncmp(line,"SMOOTHZOOM=",11)==0) {
                        config.smoothzoom=atoi(&line[11]);
                }
                if (strncmp(line,"CACHEMAPINDEX=",14)==0) {
                        config.cachemapindex=atoi(&line[14]);
                }
                if (strncmp(line,"DEBUGLOG=",9)==0) {
                        config.debug_logging=atoi(&line[9]);
                }
                if (strncmp(line,"CACHEDELAY=",11)==0) {
                        config.cachedelay=atoi(&line[11]);
                }
                if (strncmp(line,"ENABLESERIALPORT=",17)==0) {
                        config.serialport=atoi(&line[17]);
                }
        }

        fclose(fp);
}


int menuConfiguration() {
	SceCtrlData cpad;
        char label[64]="::: CONFIGURATION :::"; translate(label);
        int i=0, timer=0;
        int topline=0;
        char line[60];
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(1);
        int lines=150;
        char * options[lines];
        FILE * fp=fopen("system/config.txt","r");
        if (fp!=NULL) {
        for (i=0; i<lines; i++)  {
                options[i]=(char*)malloc(64*sizeof(char));
                if (options[i]==NULL) {
                        int j;
                        for (j=0;j<i; j++)
                                free(options[j]);
                        fclose(fp);
                        return -1;
                } else
                        memset(options[i],0,64*sizeof(char));
        }
        Color * bg= (Color *) malloc ( FRAMEBUFFER_SIZE);
        memcpy(bg, getVramDrawBuffer(), FRAMEBUFFER_SIZE);

        i=0;
         while(fgets(line,60,fp) != NULL && i<lines )  {
                if (line[strlen(line)-1]=='\n')
                        line[strlen(line)-1]=0;
                strcpy(options[i++],line);
        }

        fclose(fp);
        int size=i;
        Image * menuwindow=ldImage("system/menuwindow.png");

        sceCtrlPeekBufferPositive(&cpad, 1);
        while (!(cpad.Buttons & PSP_CTRL_CIRCLE)) {
                sceCtrlPeekBufferPositive(&cpad, 1);
                sceKernelDelayThread(1);


                if ((cpad.Buttons & PSP_CTRL_CIRCLE) && timer>2) {
                        break;
                }
                if (cpad.Buttons & PSP_CTRL_SELECT)
                        break;
                if ((cpad.Buttons & PSP_CTRL_CROSS) && timer>10/(333/Clock)) {
                        timer=0;
                        if (edit_line(options[topline])==1){
                                //save file
                                fp=fopen("system/config.txt","w");
                                if (fp!=NULL) {
                                        for (i=0; i<size; i++)
                                                fprintf(fp,"%s\n",options[i]);
                                        fclose(fp);
                                        readConfig();
                                        unload_group(4096,zipfile,datatype);
                                }
                        }
                }

                if ((cpad.Buttons & PSP_CTRL_DOWN || cpad.Ly >= 0xD0) && timer>4/(333/Clock)) {
                                if (size-topline>17)
                                        topline++;
                        timer=0;
                }

                if ((cpad.Buttons &  PSP_CTRL_UP || cpad.Ly <= 0x30) && timer>2/(333/Clock)) {
                        timer=0;
                        if (topline>0)
                                topline--;
                }

                timer++;
                if (timer>100) timer=11;
                if (bg!=NULL)
                        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
                if (menuwindow!=NULL) {
                        blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
                        blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
                 } else
                        fillScreenRect(0x000000cc,12,28,455,232);
                draw_string( label, PSP_WIDTH/2- 12*8, 34);
                if (timer%10>5) {
                        draw_string( ">", 20, 51);
                        draw_string( ">", 15, 51);
                }
                for (i=0; i<17 && i<size; i++) {
                        strcpy(line,options[i+topline]);
                        toUpperCase(line);
                        if (line[0]=='#')
                                draw_comment_string( line, 26, i*12+51);
                        else
                                draw_string_no_ctrl( line, 26, i*12+51);

                }

                        
                flipScreen();
                sceKernelDelayThread(1);


        }
		beep();
        for (i=0; i<lines; i++) {
                if (options[i]!=NULL)
                        free(options[i]);
        }
        if (bg!=NULL)
                free(bg);
        if (menuwindow!=NULL)
                freeImage(menuwindow);

        } else return -1;
        return 1;
}

int menuPoiSearch(char * filename) {
	SceCtrlData cpad;
        char label1[64]="::: SEARCH POI FORM :::"; translate(label1);
        char label2[64]="ENTER SEARCH WORD OR LEAVE BLANK FOR PROXIMITY SEARCH"; translate(label2);
        int current=0;
        char headers[1][16] = { "SEARCH FOR:"}; translate(headers[0]);
        char notes[1][64] = { "PRESS START TO SEARCH......" }; translate(notes[0]);
        int lens[1]= { 30};
        int size=0;
        char values[1][64];
        Coord co=getGPS(mapx, mapy);
        memset(values[0],0,64);
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
                draw_string( label2,32, 52);
                int i,j;
                for ( i=0; i<1; i++) {
                        draw_string_no_und( headers[i], 20, 73+15*i);
                        if (i==current) {
                                for ( j=0; j<(signed int)strlen(values[i])+2; j++) {
                                        drawHiliteBar(hilite,102+j*8, 69+15*i,1);
                                        drawHiliteBar(hilite,102+j*8, 77+15*i,1);
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
                                break;

                }

                switch (key)
               {
          case 0:
          break;
         case 9: if (current>0) {
                        beep();
                        current--;
                        edit_pos=0;
                }
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
                break;
         case DANZEFF_RIGHT: if (edit_pos<lens[current]) edit_pos++;
                strcpy( error,"");
                beep();
                break;
         case DANZEFF_SELECT:
                beep();
                if (menuwindow!=NULL)
                        freeImage(menuwindow);
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
             //fprintf(stdout,"You have input the key %c\n",key);
             break;
        }
        flipScreen();

        }
        danzeff_free();
        strcpy(label1, "::: SEARCHING :::"); translate (label1);
        strcpy(label2, "PLEASE WAIT WHILE SEARCH IS IN PROGRESS"); translate (label2);
        if (bg!=NULL)
                       memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
        if (menuwindow!=NULL) {
                blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
                blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
        }else
                fillScreenRect(0x000000cc,12,28,455,232);
        draw_string( label1, PSP_WIDTH/2-26*4, 34);
        draw_string( label2, PSP_WIDTH/2-26*4-50, 112);

         flipScreen();
        turn_off_attractions();
         int attr_counter=load_attractions(filename, zipfile, datatype,  mapx,  mapy, zoom, values[0]);
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
        if (attr_counter==0) {
                strcpy(label1, "NO RECORDS FOUND"); translate (label1);
                draw_string(label1, PSP_WIDTH/2-26*4, 120);
                turn_off_attractions();
                flipScreen();
                sceKernelDelayThread(1500000);
                if (menuwindow!=NULL)
                        freeImage(menuwindow);
                if (hilite!=NULL)
                        freeImage(hilite);
                if (bg!=NULL)
                        free(bg);
                return 0;
        }
        qsort( attractions, attr_counter,sizeof(attractions[0]),attr_cmp);
        if (attr_counter>MAXLISTSIZE)
                size=MAXLISTSIZE;
        else
                size=attr_counter;

        int cur=0;
        int timer=0;
        int topline=0;
        strcpy(label1, "::: POI SEARCH RESULTS"); translate (label1);

        while (!(cpad.Buttons & PSP_CTRL_CROSS)) {
                sceCtrlPeekBufferPositive(&cpad, 1);
                sceKernelDelayThread(1);


                if ((cpad.Buttons & PSP_CTRL_CIRCLE) && timer>7/(333/Clock)) {
                        timer=0;
                        break;
                }
                if (cpad.Buttons & PSP_CTRL_SELECT) {

                        turn_off_attractions();
                        if (menuwindow!=NULL)
                                freeImage(menuwindow);
                        if (bg!=NULL)
                                free(bg);
                        return -100000;
                }

                if ((cpad.Buttons & PSP_CTRL_DOWN || cpad.Ly >= 0xD0) && timer>2/(333/Clock)) {
                        if (cur<size-1) {
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
                } else
                        fillScreenRect(0x000000cc,12,28,455,232);
                sprintf(values[0],"%s [%d] :::",label1,size);
                draw_string( values[0], PSP_WIDTH/2-26*4, 34);
                drawHiliteBar(hilite,20,(cur-topline)*15+57,55 );
                drawHiliteBar(hilite,20,(cur-topline)*15+65,55 );


                double dist=0;
                double bearing=0;
                int i;
                for ( i=0; i<13 && i<size; i++) {
                        Coord attrco=getGPS(attractions[i+topline]->x,attractions[i+topline]->y);
                        GeoRangeAndBearing(co.lat, co.lon, attrco.lat,attrco.lon, &dist, &bearing) ;


                        if (config.speedfix>1.8) //kilometers
                                sprintf(filename,"%-38s %7.2f KM %-2s",attractions[i+topline]->name,dist/1000,heading[((int)bearing+22)%360/45]);
                        else   
                                sprintf(filename,"%-38s %7.2f MI %-2s",attractions[i+topline]->name,dist/1609.344,heading[((int)bearing+22)%360/45]);
                        toUpperCase(filename);
                        draw_string( filename, 30, i*15+61);

                }


                flipScreen();
                sceKernelDelayThread(5);


        }

        mapx=attractions[cur]->x;
        mapy=attractions[cur]->y;
        attr_flag=1;

        if (menuwindow!=NULL)
                freeImage(menuwindow);
         if (bg!=NULL)
                free(bg);
         if (hilite!=NULL)
                freeImage(hilite);

        return 0;
}




int menuAddPoi() {
	SceCtrlData cpad;
        char label1[64]="::: ADD POI FORM :::"; translate(label1);
        char label2[64]="EDIT THE FIELDS BELOW AND PRESS [START] BUTTON"; translate(label2);
        int current=0;
        char headers[4][16] = { "POI NAME:" , "DESC:", "LATITUDE:", "LONGITUDE:"  };
        translate(headers[0]);
        translate(headers[1]);
        translate(headers[2]);
        translate(headers[3]);
        char notes[4][64] = { "NOTE: SOME CHARACTERS ARE NOT SUPPORTED" ,"NOTE: SOME CHARACTERS ARE NOT SUPPORTED" , "NOTE: MUST BE IN DECIMAL FORMAT", "NOTE: MUST BE IN DECIMAL FORMAT" };
        translate(notes[0]);
        translate(notes[1]);
        translate(notes[2]);
        translate(notes[3]);
        int lens[4]= { 37,43,18,18};
        char values[5][64];
        Coord co=getGPS(mapx, mapy);

        double plat;
        double plon;
                plat=co.lat;
                plon=co.lon;
                char NS='N';
                char EW='E';
                if(config.dms!=0)
                {
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
                }

                switch(config.dms)
                {
                        case 1 :
                                sprintf(values[0],"MY POI %c%1.0f\260%02.6f' X %c%1.0f\260%02.6f'",NS, floor(plat), 60*(plat-floor(plat)), EW, floor(plon), 60*(plon-floor(plon)));
                                sprintf(values[2],"%c%1.0f\260%02.6f'",NS, floor(plat), 60*(plat-floor(plat)));
                                sprintf(values[3],"%c%1.0f\260%02.6f'",EW, floor(plon), 60*(plon-floor(plon)));
                        break;
                        case 2 :                                // \260=deg \042=0x22=" \047=0x27='
                        {
                        double plat_min,plat_sec,plon_min,plon_sec;
                                plat_min=60*(plat-floor(plat));
                                plat_sec=60*(plat_min-floor(plat_min));
                                plon_min=60*(plon-floor(plon));
                                plon_sec=60*(plon_min-floor(plon_min));
                                sprintf(values[0],"MY POI %c%1.0f\260%02.0f\047%02.4f\042 X %c%1.0f\260%02.0f\047%02.4f\042",NS, floor(plat), floor(plat_min), plat_sec, EW, floor(plon), floor(plon_min), plon_sec);
                                sprintf(values[2],"%c%1.0f\260%02.0f\047%02.4f\042",NS, floor(plat), floor(plat_min), plat_sec);
                                sprintf(values[3],"%c%1.0f\260%02.0f\047%02.4f\042",EW, floor(plon), floor(plon_min), plon_sec);
                        }
                        break;
                        default :
                                sprintf(values[0],"MY POI %10.6f X %10.6f",co.lat, co.lon);
                                sprintf(values[2],"%f",co.lat);
                                sprintf(values[3],"%f",co.lon);
                        break;
                }
        sprintf(values[1]," ");


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
                draw_string( label2, 58, 48);
                int i,j;
                for ( i=0; i<4; i++) {
                        draw_string( headers[i], 20, 73+15*i);
                        if (i==current) {
                                for (j=0; j<(signed int)strlen(values[i])+2; j++) {
                                        drawHiliteBar(hilite,102+j*8, 69+15*i,1 );
                                        drawHiliteBar(hilite,102+j*8, 77+15*i,1 );
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
                        if (check_fields(values[0], ConvertLocationStringToLocationDouble(values[2]), ConvertLocationStringToLocationDouble(values[3]), 2, 4)==1 && check_fields(values[1], ConvertLocationStringToLocationDouble(values[2]), ConvertLocationStringToLocationDouble(values[3]), 2, 4)==1)
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
                if (hilite!=NULL)
                        freeImage(hilite);
                 if (bg!=NULL)
                        free(bg);
                danzeff_free();
                return 0;
         case DANZEFF_START: break;
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
             //fprintf(stdout,"You have input the key %c\n",key);
             break;
       }
                flipScreen();

        }

        danzeff_free();
        char filestr[128];
        sprintf(filestr,"%s/_MY_POIS",zipfile);
        FILE * pfp=fopen(filestr,"a");
        if (pfp!=NULL) {
                double plat;
                double plon;
                plat=ConvertLocationStringToLocationDouble(values[2]);
                plon=ConvertLocationStringToLocationDouble(values[3]);
                char NS='N';
                char EW='E';
                if(config.dms!=0)
                {
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
                }

                switch(config.dms)
                {
                        case 1 :
                                fprintf(pfp,"%c%1.0f\260%02.6f',%c%1.0f\260%02.6f',%s,%s,\n",  NS, floor(plat), 60*(plat-floor(plat)), EW, floor(plon), 60*(plon-floor(plon)), values[0], values[1]);
                        break;
                        case 2 :                                // \260=deg \042=0x22=" \047=0x27='
                        {
                        double plat_min,plat_sec,plon_min,plon_sec;

                                plat_min=60*(plat-floor(plat));
                                plat_sec=60*(plat_min-floor(plat_min));
                                plon_min=60*(plon-floor(plon));
                                plon_sec=60*(plon_min-floor(plon_min));
                                fprintf(pfp,"%c%1.0f\260%02.0f\047%02.4f\042,%c%1.0f\260%02.0f\047%02.4f\042,%s,%s,\n",  NS, floor(plat), floor(plat_min), plat_sec, EW, floor(plon), floor(plon_min), plon_sec, values[0], values[1]);
                        }
                        break;
                        default :
                                fprintf(pfp,"%f,%f,%s,%s,\n", plat, plon, values[0], values[1]);
                        break;
                }
//              fprintf(pfp,"%f,%f,%s,%s,\n",atof(values[2]), atof(values[3]), values[0], values[1]);
                fclose(pfp);
                turn_on_attractions();
        }

        if (menuwindow!=NULL)
                freeImage(menuwindow);
        if (hilite!=NULL)
                freeImage(hilite);
         if (bg!=NULL)
                free(bg);
        return 0;
}

int menuGpsInfo290() {
	SceCtrlData cpad;
        char label1[32]="GPS INFORMATION"; translate(label1);
        char label2[16]="GPS STATE"; translate(label2);
        char label3[64]="------------------ SATELLITE INFO --------------------"; translate(label3);
        int timer=0;
        int fix;
        int oldclock=Clock;
        Clock=config.satinfofrequency;
        scePowerSetClockFrequency(Clock,Clock,Clock/2);
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(1);
        char line[64];
        char state[4][11] = { "  OFF     ", "ACTIVATING","ACTIVATING", "  ON      " };
        int i;
        for (i=0; i<4; i++)
                translate(state[i]);
        char labels[17][16]={"SAT-IN-VIEW:", "FIXED:", "HDOP:", "SAT ID:", "ELEVATION:", "AZIMUTH:", "SNR:", "FIX:", "TIME:", "LAT:", "LON:", "ALT:", "SPEED:", "BEARING:", "LOC:", "CPU:"};
        for (i=0; i<16; i++)
                translate(labels[i]);
        Color * bg= (Color *) malloc ( FRAMEBUFFER_SIZE);
        memcpy(bg, getVramDrawBuffer(), FRAMEBUFFER_SIZE);

        Image * menuwindow=ldImage("system/menuwindow.png");

        while (!((cpad.Buttons & PSP_CTRL_SELECT )&& timer>5/(333/Clock))) {
                sceCtrlPeekBufferPositive(&cpad, 1);
                sceKernelDelayThread(1);
                timer++;
                if (timer>100)
                        timer=20;
                if ((cpad.Buttons &  PSP_CTRL_UP) && timer>4/(333/Clock)) {
                        beep();
                        if (Clock<333) {
                                if (Clock<111)
                                        Clock=111;
                                else
                                        Clock+=111;
                                scePowerSetClockFrequency(Clock,Clock,Clock/2);
                                timer=0;
                        }
                }
                if ((cpad.Buttons &  PSP_CTRL_DOWN) && timer>4/(333/Clock)) {
                        beep();
                        if (Clock>111) {
                                Clock-=111;
                                scePowerSetClockFrequency(Clock,Clock,Clock/2);
                                timer=0;
                        } else {
                                if (Clock==110)
                                        Clock=40;
                        }
                }
                if ((cpad.Buttons &  PSP_CTRL_LEFT) && timer>4/333/Clock) {
                        timer=0;
                        beep();
                        if (gpsState == 0x3) {
                                config.initLocation--;
                                sceUsbGpsSetInitDataLocation(config.initLocation);
                                sceUsbGpsGetInitDataLocation(&getLocation);
                        }
                }
                if ((cpad.Buttons &  PSP_CTRL_RIGHT) && timer>4/333/Clock) {
                        timer=0;
                        beep();
                        if (gpsState == 0x3) {
                                config.initLocation++;
                                sceUsbGpsSetInitDataLocation(config.initLocation);
                                sceUsbGpsGetInitDataLocation(&getLocation);
                        }
                }

                if (bg!=NULL)
                        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
                if (menuwindow!=NULL) {
                        blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
                        blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
                }else
                        fillScreenRect(0x000000cc,12,28,455,232);
                draw_string( label1, PSP_WIDTH/2-strlen(label1)*4, 34);
                sceKernelWaitSema(gpslock,1,0);
                sprintf(line,"%9s: %s   %s %02d/%02d/%04d %02d:%02d:%02d (GMT)",label2, state[gpsState],labels[8], gpsd.month, gpsd.date, gpsd.year, gpsd.hour, gpsd.minute, gpsd.second);
                draw_string(line, 20, 50);
                sprintf(line,"%s     %9.6f  %s %10.6f  %s  %5.1f",labels[9], gpsd.latitude,labels[10], gpsd.longitude, labels[11],gpsd.altitude);
                draw_string(line, 20, 70);
                sprintf(line,"%s     %5.2f        %s%03.0f      %s   %ld",labels[12], gpsd.speed,labels[13], gpsd.bearing,labels[14], getLocation);
                draw_string(line,  20, 80);
                sprintf(line,"%s     %d Mhz",labels[15], scePowerGetCpuClockFrequencyInt());
                draw_string(line,  20, 90);
                int j;
                fix=0;
                draw_string(label3, 20, 100);
                for (j=0; j<satd.satellites_in_view && j<16; j++) {
                        sprintf(line,"%s%03d  %s%02d  %s%03d  %s%02d  %s%d",labels[3], satd.satinf[j].id,labels[4], satd.satinf[j].elevation,labels[5], satd.satinf[j].azimuth,labels[6], satd.satinf[j].snr,labels[7], satd.satinf[j].good);
                        draw_string(line,  20, 110+j*9);
                if ( satd.satinf[j].good==1)
                        fix++;
                }
                sprintf(line,"%s %02d         %s %02d        %s %5.1f",labels[0], satd.satellites_in_view,labels[1],fix,labels[2], gpsd.hdop);
                sceKernelSignalSema(gpslock, 1);
                draw_string(line,  20, 60);

                flipScreen();
                sceKernelDelayThread(100000);


        }
        Clock=oldclock;
        scePowerSetClockFrequency(Clock,Clock,Clock/2);

        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
        memcpy( getVramDisplayBuffer(),bg, FRAMEBUFFER_SIZE);
        flipScreen();
        sceKernelDelayThread(1);

        if (bg!=NULL)
                free(bg);
        if (menuwindow!=NULL)
                freeImage(menuwindow);
        return 1;
}


int menuGpsInfoGeneric() {
	SceCtrlData cpad;
        int timer=0;
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(1);
        char line[64];

        Color * bg= (Color *) malloc ( FRAMEBUFFER_SIZE);
        memcpy(bg, getVramDrawBuffer(), FRAMEBUFFER_SIZE);

        Image * menuwindow=ldImage("system/menuwindow.png");

        while (!((cpad.Buttons & PSP_CTRL_SELECT )&& timer>5/(333/Clock))) {
                sceCtrlPeekBufferPositive(&cpad, 1);
                sceKernelDelayThread(1);
                timer++;
                if (timer>100)
                        timer=20;
                if ((cpad.Buttons &  PSP_CTRL_UP) && timer>4/(333/Clock)) {
                        beep();
                        if (Clock<333) {
                                Clock+=111;
                                scePowerSetClockFrequency(Clock,Clock,Clock/2);
                                timer=0;
                        }
                }
                if ((cpad.Buttons &  PSP_CTRL_DOWN) && timer>4/(333/Clock)) {
                        beep();
                        if (Clock>111) {
                                Clock-=111;
                                scePowerSetClockFrequency(Clock,Clock,Clock/2);
                                timer=0;
                        }
                }
                if ((cpad.Buttons &  PSP_CTRL_SQUARE) && timer>4/(333/Clock)) {
                        timer=0;
                        beep();
                        if (shownmea==1)
                                shownmea=0;
                        else
                                shownmea=1;

                }

                if (bg!=NULL)
                        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
                if (menuwindow!=NULL) {
                        blitAlphaImageToScreen(0,0,455,122,menuwindow,12,28);
                        blitAlphaImageToScreen(0,18,455,110,menuwindow,12,150);
                }else
                        fillScreenRect(0x000000cc,12,28,455,232);
                draw_string( "GPS INFORMATION", PSP_WIDTH/2-strlen("GPS INFORMATION")*4, 34);
                if (isconnected)
                        sprintf(line,"GPS STATE: ON     TIME: %02ld/%02ld/%02ld %02ld:%02ld:%02ld(GMT)",  date%10000/100,date/10000,date%100, ltime/10000, ltime%10000/100, ltime%100);
                else
                        sprintf(line,"GPS STATE: OFF    TIME: %02ld/%02ld/%02ld %02ld:%02ld:%02ld(GMT)",  date%10000/100,date/10000,date%100, ltime/10000, ltime%10000/100, ltime%100);
                draw_string(line,  20, 50);
                if (config.dms==1) {
                        double plat= latitude;
                        double plon= longitude;
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
                        sprintf(line,"LAT:      %c%3.0f.%02.05f LON: %c%3.0f.%02.05f ALT:  %5.1f", NS,floor(plat), 60*(plat-floor(plat)), EW, floor(plon),60*(plon-floor(plon)),  altitude);
                } else
                        sprintf(line,"LAT:         %9.6f  LON: %10.6f  ALT:  %5.1f", latitude, longitude, altitude);
                draw_string(line,  20, 70);
                 sprintf(line,"SPEED:     %5.2f        BEARING:%03.0f      QUALITY: %d", speed, course, quality);
                draw_string(line,  20, 80);
                sprintf(line,"CPU:     %d Mhz", scePowerGetCpuClockFrequencyInt());
                draw_string(line,  20, 90);
                int j,i;
                if (shownmea) {
                        sprintf(line,"------------------ NMEA SENTENCES --------------------");
                        draw_string(line, 20, 100);
                        char temp[60];
                        int ln=0;
                        j=0;i=0;
                        memset(temp,0,sizeof(temp));
                        //fprintf(stdout,"[%s]\n",buffer);
                        while (j<1024 && buffer[j]>0) {
                                temp[i]=buffer[j];
                                j++;
                                i++;
                                if (i>55 ||  buffer[j]==0  ||  buffer[j]=='\n' ) {
                                        temp[i]=0;
                                        //fprintf(stdout,"%s\n",temp);
                                        draw_string_no_ctrl(temp,  20, 110+(ln++*9));
                                        i=0;
                                        memset(temp,0,sizeof(temp));
                                }
                        }

                } else {
                        sprintf(line,"------------------ SATELLITE INFO --------------------");
                        draw_string(line, 20, 100);
                        int satc=0;
                        for (j=0; j<gsv.satellites_in_view && j<16; j++) {
                                sprintf(line," SAT ID:%03d    ELEVATION:%02d    AZIMUTH:%03d    SNR:%02d  ", gsv.satellites[j].prn, gsv.satellites[j].elevation, gsv.satellites[j].azimuth,gsv.satellites[j].snr);
                                draw_string(line,  20, 110+(satc++*9));
                        }
                }
                sprintf(line,"SAT-IN-VIEW: %02d         FIXED: %02d        HDOP: %5.1f",  satellites_in_view, satellites, hdop);
                draw_string_no_ctrl(line,  20, 60);

                flipScreen();
                sceKernelDelayThread(100000);


        }
        memcpy( getVramDrawBuffer(),bg, FRAMEBUFFER_SIZE);
        memcpy( getVramDisplayBuffer(),bg, FRAMEBUFFER_SIZE);
        flipScreen();
        sceKernelDelayThread(1);

        if (bg!=NULL)
                free(bg);
        if (menuwindow!=NULL)
                freeImage(menuwindow);
        return 1;
}



