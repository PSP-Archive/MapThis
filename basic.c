//////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdio.h>
#include "basic.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////////

/** basic c functions */

int my_access(char *name,int ok) { //mimic after access()
        FILE * f=fopen (name,"r");
        if (f==NULL)
                return -1;
        else {
                fclose(f);
                return 0;
        }
}


void save_map_prefs (int x,int y, int zoom) {
        char filestr[128];
        sprintf(filestr,"%s/map_pref.txt",zipfile);
        FILE * fp=fopen(filestr,"w");
        if (fp!=NULL) {
                fprintf(fp,"%d %d %d\n",x,y,zoom);
        fclose(fp);
        }
}

int load_mapp_prefs () {
        char filestr[128];
        int z,rd;
        char xstr[30], ystr[30];
        sprintf(filestr,"%s/map_pref.txt",zipfile);
        FILE * fp=fopen(filestr,"r");
        if (fp!=NULL) {
                rd=fscanf(fp,"%s %s %d",  xstr, ystr , &z);
                if (rd<3)
                        return -1;

                mapx=atoi(xstr);
                mapy=atoi(ystr);
                zoom=z;

        fclose(fp);
        return 1;
        }
        return -1;
}


void deleteChar(char * str, int pos) 
{
    if (pos>=(signed int)strlen(str))
        return;
    int i;
    for ( i=pos; i<(signed int)strlen(str); i++)
        str[i]=str[i+1];
}

void insertChar(char * str, int pos, char ch) 
{
    int len=strlen(str);
    if (pos>len)
        return;
    str[len+1]=0;
    int i;
    for (i=len; i>=pos; i--)
        str[i+1]=str[i];
    str[pos]=ch;
}

// ZZZ: use toupper() instead
void toUpperCase(char * str) 
{
    char *ptr=str;
    for(ptr=str;*ptr;ptr++)
    {
        if (*ptr >= 97 && *ptr <= 122)
        {
            *ptr-=32;
        }
    }
}

int opt_cmp(const void * a1, const void * a2) 
{
    return strcmp((char*)a1, (char*)a2);
}

// ZZ: better implmentation of powerOf
int powerOf(int p)
{
    return 1<<p;
}


int get_zoom(int tile_num) 
{
    int tn=tile_num;
    int power=0;
    while (tn!=1) 
    {
        tn=tn/2;
        power++;
    }
    return power;
}

int calc_mapsize(int sz) 
{
    int i=sz;
    int total=0;
    while ( i>1) 
    {
        total+=i*i;
        i/=2;
    }
    return total;
}

void copy_pad(char * dest, char * src, int len) 
{
    //center the message...
    int l=0, half=0;
    int slen=strlen(src);
    if (slen>len) {
        src[len]=0;
        slen=len;
    }
    l=len - slen;
    half=(int)l/2;
    memset(dest,' ', half);
    strcpy(&dest[half], src);
    memset(&dest[half+slen],' ', len-half-slen);
    dest[len]=0;
}

void pad(char * str) 
{
    int i;
    for ( i=0; i<(signed int)strlen(str); i++) {
        if (str[i]==' ' || str[i]=='/' || str[i]=='\\' || str[i]=='?' || str[i]=='<' || str[i]=='>')
            str[i]='_';
        
    }
}


int xtoi(const char* xs, unsigned int* result)
{
 size_t szlen = strlen(xs);
 int i, xv, fact;

 if (szlen > 0)
 {
  // Converting more than 32bit hexadecimal value?
  if (szlen>8) return 2; // exit

  // Begin conversion here
  *result = 0;
  fact = 1;

  // Run until no more character to convert
  for(i=szlen-1; i>=0 ;i--)
  {
   if (isxdigit(*(xs+i)))
   {
    if (*(xs+i)>=97)
    {
     xv = ( *(xs+i) - 97) + 10;
    }
    else if ( *(xs+i) >= 65)
    {
     xv = (*(xs+i) - 65) + 10;
    }
    else
    {
     xv = *(xs+i) - 48;
    }
    *result += (xv * fact);
    fact *= 16;
   }
   else
   {
    // Conversion was abnormally terminated
    // by non hexadecimal digit, hence
    // returning only the converted with
    // an error value 4 (illegal hex character)
    return 4;
   }
  }
 }

 // Nothing to convert
 return 1;
}

int check_fields(char * name, double lat, double lon, int bz, int sz) {
        pad (name);
        if (lat==0.0 || lon==0.0)
                return -1;
        if (bz<0 || bz>14)
                return -1;
        if (sz!=4 && sz!=8 && sz!=16 && sz!=32 && sz!=64 && sz!=128 && sz!=256 && sz!=512)
                return -1;
        return 1;

}



/*
int readchar(char *pvec)
{
    int ch;
    if (*pvec == 0) {
        ch = -1;
    }
    else {
        ch = *pvec++;
    }
    return ch;
}
*/

//////////////////////////////////////////////////////////////////////////////



