//////////////////////////////////////////////////////////////////////////////
/**
    menu.h: functions to display menus 
*/

//////////////////////////////////////////////////////////////////////////////

#ifndef MENU_H
#define MENU_H

//////////////////////////////////////////////////////////////////////////////


typedef struct  {
        int 	baud;
	int 	fakefeed;
	long 	initLocation;
	int  	dms;
	float	speedfix;
	float	speedlimit;
	int	nightmode;
	float	altfix;
	int	timezone;
	int	warn_dist;
	float	speedfactor;
	int	turnspeed;
	char	startupmap[128];
	int	loadwifi;
	int	startupfrequency;
	int	satinfofrequency;
	int	rotatemap;
	float	step;
	int	smoothzoom;
	int	cachemapindex;
	int	debug_logging;
	int	cachedelay;
	int	serialport;
} CONFIG;

CONFIG config;

int menuMain(int cur );
int menuHelp();
int menuConfiguration();
void readConfig();
int menuPoiSearch(char * filename);
int menuAddPoi();
int menuGpsInfo290();
int menuGpsInfoGeneric();

#endif // MENU_H
