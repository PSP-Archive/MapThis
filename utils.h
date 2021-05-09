#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <stdio.h>
#include "graphics.h"



typedef struct _imglist {
        char name[256];
        int refcount;
        Image *img;
        struct _imglist *next;
} IMGLIST;

int tlock;

int unzipFile( char* zipfile, char* filename, char* outfile );
void closezipfile();
Image *loadfromzip(char *filename, char * zipfile);
Image *loadfromdir(char *filename );
Image *loadfromdir_check4blank(char *filename, int negate );
void imglist_garbagecollect();
void unloadmap(char *filename);
Image *loadfromcache(char *filename );
void unload_group (int zoom, char* zipfile,int datatype);
int unzipReadDir( char* zipfile, char options[][60]);
int gpsfsGetX ();
int gpsfsGetY ();
int gpsfsGetBaseZoom ();
int gpsfsGetFileType ();
long gpsfsGetTotalTiles ();
int gpsfsGetMapDimension();
long gpsfsGetFileSize(int i);
Image *loadfromgpsfs(int x , int y, int zm, int sz);
int gpsfsOpen(char * dirname);
void gpsfsClose();
void cleanup (int mapx, int mapy, int zoom,char * zipfile,int datatype);

void ms_write_log(char *fmt,...);
