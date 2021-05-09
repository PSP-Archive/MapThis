#include "attractions.h"
#include "utils.h"
#include "main.h"
#include "basic.h"
#include "geocalc.h"
#include <math.h>
int count=0;

extern char attr_ifile[MAXIMAGE][128];
extern int attr_width;
extern int  attr_height;
extern int  waypoint;
extern int basezoom;
extern int TILE_NUM;
extern void addPathPoint(long x, long y);
extern void initPath();
double max_dist=0;
int max_index=0;
int image_count=0;
int curr_count=0;

int attr_cmp(const void * a1, const void * a2) {
        ATTRACTION *attr1 = *(ATTRACTION **) a1;
        ATTRACTION *attr2 = *(ATTRACTION **) a2;
        return (int)(attr1->d - attr2->d);
}


void readAttractionsListings() {
        optcount=0;
        DIR *dp;
        struct dirent *dir;
        char path[128];
        sprintf(path,"%s/%s",currentpath,zipfile);
#ifndef GENERIC
        if ( (dp = opendir(path)) == NULL ) {
#else
        if ( (dp = opendir(zipfile)) == NULL ) {
#endif
                //printf("Can't open directory: %s\n", zipfile);
                return;
        }
        while ((dir = readdir(dp)) != NULL) {
                if (dir->d_name[0]!='_') continue;
                strcpy(options[optcount++],dir->d_name);
        }
        closedir(dp);


#ifndef GENERIC
        sprintf(path,"ms0:/PSP/COMMON");
#else
        sprintf(path,"/PSP/COMMON");
#endif
        if ( (dp = opendir(path)) == NULL ) {
                //printf("Can't open directory: %s\n", path);
                return;
        }
        while ((dir = readdir(dp)) != NULL) {
                if (dir->d_name[0]!='_') continue;
                strcpy(options[optcount++],dir->d_name);
        }
        closedir(dp);
}


//inserts up to MAXNUM of closes attractions...
void insert_attraction(ATTRACTION *attr) {
	if (count<MAXNUM){  			//just insert first N points 
		attractions[count++]=attr;
		if (attr->d > max_dist) {
			max_dist=attr->d;
			max_index=count-1;
		}
	} else { 					//may need to substitute stuff
		if(attr->d<max_dist) {    //substitute
			free(attractions[max_index]);
			attractions[max_index]=attr;
			max_dist=0;
			int i;
			for (i=0; i<count; i++) {
				if (attractions[i]->d>max_dist) {
					max_dist=attractions[i]->d;
					max_index=i;
				}
			}

		}


	}
}



// MIB.42_4 to convert all different deg/min/sec to single fraction double

double ConvertLocationStringToLocationDouble(char *pline)
{
char line[64];
double muler=1.0f,fv=0.0f;
int	a,nosc=0;

	if(pline==NULL) return(fv);
	if(pline[0]=='\0') return(fv);

ms_write_log("\nConvertLocationStringToLocationDouble(%s)",pline);

	for(a=0;((pline[a]!='\0')&&(a<64));a++)		// substitute phase
	{
		switch(pline[a])
		{
			case 'N' :
			case 'E' :
				line[a]=' ';
			break;
		 	case 'S' :
			case 'W' :
				line[a]=' ';
				muler=-1.0f;
			break;
			case '\'':
			case '"':
			case '\260' :
				line[a]=';';
				++nosc;
			break;
			default:
				line[a]=pline[a];
			break;
		}
	}
	line[a]='\0';

ms_write_log("\n\t\t(%s) = %d",line,nosc);

	switch(nosc)
	{
		case 2 :		// N120°12.34567'   Now it looks : ' 120;12.34567;'
		{
		double	v1;
		char	*asp=&line[0],*bsp=&line[0];

			while((*asp!=';')&&(*asp!='\0')) ++asp;
			if(*asp==';') *asp='\0'; else --asp;
			fv=atof(bsp);
			++asp;bsp=asp;
			while((*asp!=';')&&(*asp!='\0')) ++asp;
			if(*asp==';') *asp='\0'; else --asp;
			v1=atof(bsp);
			v1/=60.0f;

ms_write_log("\t\t%f %f",fv,v1);

			if(v1<1.0f) fv+=v1;


		}
		break;
		case 3 :		// N120°12'34.567"   Now it looks : ' 120;12;34.567;'
		{
		double	v1,v2;
		char	*asp=&line[0],*bsp=&line[0];

			while((*asp!=';')&&(*asp!='\0')) ++asp;
			if(*asp==';') *asp='\0'; else --asp;
			fv=atof(bsp);
			++asp;bsp=asp;
			while((*asp!=';')&&(*asp!='\0')) ++asp;
			if(*asp==';') *asp='\0'; else --asp;
			v1=atof(bsp);
			++asp;bsp=asp;
			while((*asp!=';')&&(*asp!='\0')) ++asp;
			if(*asp==';') *asp='\0';
			v2=atof(bsp);
			v1/=60.0f;
			v2/=3600.0f;

ms_write_log("\t\t%f %f %f",fv,v1,v2);

			if(v1<1.0f) fv+=v1;
			if(v2<0.1f) fv+=v2;		
		}
		break;
		default :
			fv=atof(line);

ms_write_log("\t\t%f",fv);

		break;
	}

fv*=muler;

ms_write_log("\nReturning : %f",fv);

return(fv);
}


int load_attractions(char * file, char * zipfile, int datatype, long mapx, long mapy, int zoomlevel, char * filterstr) {
	waypoint=0;
	FILE *fp;
	char line[256];
	ATTRACTION * attr;
	Coord c;

	memset(attr_ifile,0,sizeof(attr_ifile));

                sprintf(line, "%s/%s",zipfile,file);
		if (my_access(line, F_OK) != 0) {
#ifndef GENERIC
                	sprintf(line, "ms0:/PSP/COMMON/%s",file);
#else
                	sprintf(line, "/PSP/COMMON/%s",file);
#endif
                 	fp = fopen(line, "r");
		} else
                 	fp = fopen(line, "r");

	if (fp==NULL) {
		//printf("couldn't open file: %s\n", line); 
		return -1;

	}

	while(fgets(line,256,fp) != NULL) {
		if (strlen(line) < 10 || line[0]=='#')
			continue;
		toUpperCase(line);

		if (strncmp(line,"!IMAGE:",7)==0 ) {
			//denis:: new logic to allow repetitive image declarations of same image..
			char * imagename=strtok(&line[7], ",");
			int j, foundflag=0;
			for ( j=0; j<image_count; j++) {
				if (strcmp(attr_ifile[j],imagename)==0) {
					curr_count=j;
					foundflag=1;
					break;
				}
			}
				
			if (image_count<MAXIMAGE && foundflag==0) {
				strcpy(attr_ifile[++image_count],strtok(&line[7], ","));
				curr_count=image_count;
			}
			//fprintf (stdout, "%s:::%d\n",attr_ifile[image_count],image_count);
			continue;
		}
		if (strncmp(line,"!WAYPOINT",9)==0) {
			waypoint=1;
			initPath();
			continue;		
		}
		if (strlen(filterstr)>0) {
			if (strstr(line, filterstr)==NULL)
				continue;
		}
// MIB.42_4 to handle different DMS expressions
		c=getXY(ConvertLocationStringToLocationDouble(strtok(line, ",")),ConvertLocationStringToLocationDouble(strtok(NULL, ",")),basezoom);
//		c=getXY(atof(strtok(line, ",")),atof(strtok(NULL, ",")),basezoom);


		if (waypoint==1)
			addPathPoint((long)c.lon, (long) c.lat);
//		if (c.lon>0 && c.lon<TILE_SIZE*TILE_NUM && c.lat>0 && c.lat<TILE_SIZE*TILE_NUM){ 
		 	attr=(ATTRACTION*)malloc(sizeof(ATTRACTION));
		 	if (attr==NULL) {
				printf("couldn't allocate memory for rec#: %d\n", count); 
				break;
		 	}
			attr->x=(int)c.lon;
			attr->y=(int)c.lat;
			attr->d=sqrt(((double)(attr->x)-(double)mapx)*((double)(attr->x)-(double)mapx)+((double)(attr->y)-(double)mapy)*((double)(attr->y)-(double)mapy));
			//fprintf(stdout,"%ld:%ld x %ld:%ld  :::%f\n", mapx,mapy, attr->x, attr->y, attr->d);
			attr->i=curr_count;
			char * ptr = strtok(NULL, ",");
			if (ptr!=NULL) {
				strncpy(attr->name,ptr,37);
				attr->name[37]=0;
				ptr = strtok(NULL, ",");
				if (ptr!=NULL) {
					strncpy(attr->details,ptr,59);
					attr->details[59]=0;
				} else {
					memset(attr->details, 0,59);
				}
			} else {
				memset(attr->details, 0,59);
				memset(attr->name, 0,37);
			}
			// parsing out the sound message marker
			ptr = strtok(NULL, ",");
			if (ptr!=NULL) 
				attr->message=atoi(ptr);
			 else 
				attr->message=0;
			attr->waypoint=waypoint;
			insert_attraction(attr);
			//attractions[count++]=attr;
			//printf ("%d %d %d %s %s\n", attr->x, attr->y,attr->i, attr->name, attr->details);
	//	}


	}
	fclose(fp);

	printf("loaded: %d records\n", count);
	return count;

}

void free_attractions() {
	int i;
	for (i=0; i<count; i++) {
		if (attractions[i]!=NULL) {
			free(attractions[i]);
		}
	}
	count=0;
	image_count=0;
	curr_count=0;
	memset(attr_ifile,0,sizeof(attr_ifile));
	max_index=0;
	max_dist=0;
}

int attractions_count() {
	return count;
}
int get_image_count() {
	return image_count;
}
