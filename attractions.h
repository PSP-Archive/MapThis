#ifndef _ATTRACTIONS_H_
#define _ATTRACTIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXNUM	 400 
#define MAXIMAGE 10

typedef struct  {
	int x;
	int y;
	double d;
        char name[38];
        char details[60];
	int i;
	int message;
	int waypoint;
} ATTRACTION;

ATTRACTION * attractions[MAXNUM];

int load_attractions(char * file, char * zipfile, int datatype, long mapx, long mapy, int zoomlevel, char * filterstr);
void free_attractions();
int attractions_count();
int get_image_count();
double ConvertLocationStringToLocationDouble(char *pline);
int attr_cmp(const void * a1, const void * a2);
void readAttractionsListings();
#endif
