#ifndef _LINE_H_
#define _LINE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINES		25000


typedef struct  {
        char area[20];
        long x;
        long y;
        long fsz;
        long offset;
} SEGMENT;


typedef struct  {
        long sx;
        long sy;
        long ex;
        long ey;
        long stype;
        long fromaddrl;
        long toaddrl;
        long fromaddrr;
        long toaddrr;
        char name[30];
        char postcode[8];
} LINE;

int linelock;
int load_lines(char * file, char * zipfile, int datatype, long mapx, long mapy, int zoomlevel );
void free_lines();
void free_line(int);
int get_line(int, LINE*);
int line_count();
void free_segments();
int isOnLine(LINE l, int x, int y);
#endif
