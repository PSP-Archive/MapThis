//////////////////////////////////////////////////////////////////////////////
/**
    Basic.h: Basic C functions
*/

//////////////////////////////////////////////////////////////////////////////

#ifndef BASIC_H
#define BASIC_H

#include <ctype.h>
//////////////////////////////////////////////////////////////////////////////


int my_access(char *name,int ok);

void deleteChar(char * str, int pos);

void insertChar(char * str, int pos, char ch);

void toUpperCase(char * str); 

int calc_mapsize(int sz); 

int get_zoom(int tile_num); 

inline int powerOf(int p);

inline int opt_cmp(const void * a1, const void * a2); 


void copy_pad(char * dest, char * src, int len);

void pad(char * str); 


int load_mapp_prefs ();
void save_map_prefs (int x,int y, int zoom);
int check_fields(char * name, double lat, double lon, int bz, int sz);

/* not used

int readchar(char *pvec);
*/
//////////////////////////////////////////////////////////////////////////////

#endif // BASIC_H

