#include "font.h"
#include "main.h"

FONT fonts[LAST_FONT];

void readLocalizationSettings() {
	strcpy(fonts[FONT03].charorder,FONT03CHARS);
        FILE * fp=fopen("system/localization.txt", "r");
        if (fp==NULL)
                return;

        char line[128];
        while(fgets(line,128,fp) != NULL) {
                if (line[0]=='#')
                        continue;
                if (strncmp(line,"LOCAL_CHARSET=",14)==0) {
			strcpy(fonts[FONT01].charorder,&line[14]);
			strcat(fonts[FONT03].charorder,&line[14]);
                }
	}

	fclose(fp);
}

void translate(char* phrase) {
	if (strlen(phrase)==0)
		return;
        FILE * fp=fopen("system/localization.txt", "r");
        if (fp==NULL)
                return ;
        char line[64];
        while(fgets(line,64,fp) != NULL) {
                if (line[0]=='#')
                        continue;
                if (strncmp(line,phrase,strlen(phrase))==0) {
			line[strlen(line)-1]=0;
			//fprintf(stdout,"[%s]\n",&line[strlen(phrase)+1]);
			fclose(fp);
			 strcpy(phrase, &line[strlen(phrase)+1]);
			return;
                }
	}

	fclose(fp);
	return;
}

void font_init()
{
	int i;
	char buffer[32];

	readLocalizationSettings(); 

	strcpy(fonts[FONT03].filename,FONT03NAME);
	fonts[FONT03].w=FONT03W;
	fonts[FONT03].h=FONT03H;
	//strcpy(fonts[FONT03].charorder,FONT03CHARS);
	fonts[FONT03].charorder[4]=-80;

	strcpy(fonts[FONT01].filename,FONT01NAME);
	fonts[FONT01].w=FONT01W;
	fonts[FONT01].h=FONT01H;

	for(i=0;i<LAST_FONT;i++) {
		strcpy(buffer,"system/");
		strcat(buffer,fonts[i].filename);
		fonts[i].fontimg=ldImage(buffer);
	}
}

void draw_comment_string(char *text, int x, int y)
{

	int i;
	int len=strlen(text);
	for (i=0;i<len; i++)
		if (text[i]=='_')
			text[i]=' ';
	printTextScreen(x+2,y+1, text, 0x00000000);
	printTextScreen(x,y, text, 0x55555555);
	printTextScreen1(x+1,y, text, 0xffbbbbbb,fonts[FONT01].fontimg);
}

void draw_red_string(char *text, int x, int y)
{

	int i;
	int len=strlen(text);
	for (i=0;i<len; i++)
		if (text[i]=='_')
			text[i]=' ';
	//printTextScreen(x+2,y+1, text, 0x00000000);
	printTextScreen(x,y, text, 0xaa0000aa);
	printTextScreen(x+1,y, text, 0xff0000ff);
}

void draw_string(char *text, int x, int y)
{

	printTextScreen(x+2,y+1, text, 0x00000000);
	printTextScreen(x,y, text, 0xaaaaaaaa);
	printTextScreen1(x+1,y, text, 0xffffffff,fonts[FONT01].fontimg);
}

void draw_string_no_und(char *text, int x, int y)
{

	int i;
	int len=strlen(text);
	for (i=0;i<len; i++)
		if (text[i]=='_')
			text[i]=' ';
	draw_string(text,x, y);
}

void draw_string_no_ctrl(char *text, int x, int y)
{
	int i;
	int len=strlen(text);
	for (i=0;i<len; i++)
		if (text[i]<32 && text[i]>0)
			text[i]=' ';
	draw_string(text,x, y);
}

void draw_local_char_small(int ch,int x, int y) {
	unsigned int j;
	for(j=0;j<strlen(fonts[FONT01].charorder);j++)
		if(ch==fonts[FONT01].charorder[j]) break;
	if (fonts[FONT01].fontimg!=NULL)
		blitAlphaImageToScreen(j*fonts[FONT01].w,0,fonts[FONT01].w,fonts[FONT01].h,fonts[FONT01].fontimg,x,y);

}

void draw_big_string(char *text, int x, int y)
{
	unsigned int i,j,k;

	for(i=0;i<strlen(text);i++) {

		for(j=0;j<strlen(fonts[FONT03].charorder);j++) 
			if(text[i]==fonts[FONT03].charorder[j]) break;
		if(j>101) {
			j-=102;
			k=32;
		}else if(j>50) {
			j-=51;
			k=16;
		} else
			k=0;
	
		if (fonts[FONT03].fontimg!=NULL)
			blitAlphaImageToScreen(j*fonts[FONT03].w,k,fonts[FONT03].w,fonts[FONT03].h,fonts[FONT03].fontimg,i*fonts[FONT03].w+x,y);
	}
}
