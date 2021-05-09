#ifndef _FONT_H_
#define _FONT_H_
#include "graphics.h" 
#include <string.h>


enum FONTS {FONT01,  FONT03, LAST_FONT };

#define FONT01NAME "localfont_small.png"
//#define FONT01CHARS "ÀÁÂÃÄÅ¨ÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞß"
#define FONT01W 8 
#define FONT01H 8 

#define FONT03NAME "bigfont.png"
#define FONT03CHARS "!\\\"#%$&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^                                        "
#define FONT03W 10 
#define FONT03H 16 



typedef struct {
	char filename[32];
	Image *fontimg;
	int w;
	int h;
	char charorder[256];
} FONT;

void font_init();
void draw_red_string(char *text,int x, int y);
void draw_comment_string(char *text,int x, int y);
void draw_string(char *text,int x, int y);
void draw_string_no_und(char *text,int x, int y);
void draw_string_no_ctrl(char *text,int x, int y);
void draw_big_string(char *text, int x, int y);
void draw_red_string(char *text, int x, int y);
void translate(char* phrase);
void draw_local_char_small(int ch,int x, int y);
#endif
