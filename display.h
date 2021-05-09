//////////////////////////////////////////////////////////////////////////////
/**
    Display.h: functions to display info on screen
*/

//////////////////////////////////////////////////////////////////////////////

#ifndef DISPLAY_H
#define DISPLAY_H

//////////////////////////////////////////////////////////////////////////////
#include "graphics.h"
#include "attractions.h"

void displayStartupLogo();
void displayMap();
void displayInit();
void draw_zoom_scale(int x,int y,Image * zs, int zoom , int maxzoom);
void draw_compass();
void draw_cursor();
void draw_top_panel();
void draw_right_panel(char* buf);
void display_message(char* l1, char* l2, char* l3, int timeout);
void splashscreen();

#endif // DISPLAY_H

