#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "main.h"
#include "geocalc.h"
#include "geodata.h"

int zipcounter;
char nearbyzips[6][7];
double dists[6];
long sizes[6];
long offs[6];
int zcount=0; 
int zmax_index=0;
double zmax_dist=0;


int substitute(char *buf,char *what, char *with) {
        char tmp[256];
        strcpy(tmp,buf);
        char *ptr=strstr(tmp,what);
        if (ptr==NULL)
                return 0;
        *ptr=0;
        ptr+=strlen(what);
        sprintf(buf,"%s%s%s",tmp,with,ptr);
        return 1;
}

void alias (char * buf) {
char what[130][24] = {
" EAST ",
" WEST ",
" NORTH ",
" SOUTH ",
" NORTHEAST ",
" NORTHWEST ",
" SOUTHEAST ",
" SOUTHWEST ",
" N E ",
" N W ",
" S E ",
" S W ",
" AVENUE ",
" STREET ",
" PARKWAY ",
" DRIVE ",
" ROAD ",
" ISL ",
" ISLA ",
" ISLAND ",
" ISLANDS ",
" ONE ",
" TWO ",
" THREE ",
" FOUR ",
" FIVE ",
" SIX ",
" SEVEN ",
" EIGHT ",
" NINE ",
" TEN ",
" FIRST ",
" SECOND ",
" THIRD ",
" FOURTH ",
" FIFTH ",
" SIXTH ",
" SEVENTH ",
" EIGHTH ",
" NINTH ",
" TENTH ",
" ALABAMA ",
" ALASKA ",
" ARIZONA ",
" ARKANSAS ",
" AMERICAN SAMOA ",
" CALIFORNIA ",
" COLORADO ",
" CONNECTICUT ",
" DELAWARE ",
" DISTRICT OF COLUMBIA ",
" FEDERAL STATES OF FM ",
" MICRONESIA ",
" FLORIDA ",
" GEORGIA ",
" GUAM ",
" HAWAII ",
" IDAHO ",
" ILLINOIS ",
" INDIANA ",
" IOWA ",
" KANSAS ",
" KENTUCKY ",
" LOUISIANA ",
" MAINE ",
" MARSHALL ISL ",
" MARYLAND ",
" MASSACHUSETTS ",
" MICHIGAN ",
" MINNESOTA ",
" MISSISSIPPI ",
" MISSOURI ",
" MONTANA ",
" NEBRASKA ",
" NEVADA ",
" NEW HAMPSHIRE ",
" NEW JERSEY ",
" NEW MEXICO ",
" NEW YORK ",
" N CAROLINA ",
" N DAKOTA ",
" N MARIANA ISL ",
" OHIO ",
" OKLAHOMA ",
" OREGON ",
" PALAU ",
" PENNSYLVANIA ",
" PUERTO RICO ",
" RHODE IS ",
" S CAROLINA ",
" S DAKOTA ",
" TENNESSEE ",
" TEXAS ",
" UTAH ",
" VERMONT ",
" VIRGINIA ",
" VIRGIN IS ",
" US VI ",
" WASHINGTON ",
" W VIRGINIA ",
" WISCONSIN ",
" WYOMING ",
" UNITED STATES ",
"1ST",
"1TH",
"2ND",
"2TH",
"3RD",
"3TH",
"4TH",
"5TH",
"6TH",
"7TH",
"8TH",
"9TH",
"0TH",
"  ",
"| ",
" ,",
", ",
"0-",
"1-",
"2-",
"3-",
"4-",
"5-",
"6-",
"7-",
"8-",
"9-"
};

char with[130][6]= {
" E ",
" W ",
" N ",
" S ",
" NE ",
" NW ",
" SE ",
" SW ",
" NE ",
" NW ",
" SE ",
" SW ",
" AVE ",
" ST ",
" PKY ",
" DR ",
" RD ",
" IS ",
" IS ",
" IS ",
" IS ",
" 1 ",
" 2 ",
" 3 ",
" 4 ",
" 5 ",
" 6 ",
" 7 ",
" 8 ",
" 9 ",
" 10 ",
" 1 ",
" 2 ",
" 3 ",
" 4 ",
" 5 ",
" 6 ",
" 7 ",
" 8 ",
" 9 ",
" 10 ",
" AL ",
" AK ",
" AZ ",
" AR ",
" AS ",
" CA ",
" CO ",
" CT ",
" DE ",
" DC ",
" FM ",
" FM ",
" FL ",
" GA ",
" GU ",
" HI ",
" ID ",
" IL ",
" IN ",
" IA ",
" KS ",
" KY ",
" LA ",
" ME ",
" MH ",
" MD ",
" MA ",
" MI ",
" MN ",
" MS ",
" MO ",
" MT ",
" NE ",
" NV ",
" NH ",
" NJ ",
" NM ",
" NY ",
" NC ",
" ND ",
" MP ",
" OH ",
" OK ",
" OR ",
" PW ",
" PA ",
" PR ",
" RI ",
" SC ",
" SD ",
" TN ",
" TX ",
" UT ",
" VT ",
" VA ",
" VI ",
" VI ",
" WA ",
" WV ",
" WI ",
" WY ",
" US ",
"1",
"1",
"2",
"2",
"3",
"3",
"4",
"5",
"6",
"7",
"8",
"9",
"0",
" ",
"|",
",",
",",
"0",
"1",
"2",
"3",
"4",
"5",
"6",
"7",
"8",
"9"
};
int i;
	for (i=0; i<130; i++) {
		while (substitute(buf,what[i],with[i])!=0);
	}
}


void insert_zip(char *zip, double dist, long sz, long offset) {
	if (zcount<6){
		strcpy(nearbyzips[zcount],zip);
		dists[zcount]=dist;
		sizes[zcount]=sz;
		offs[zcount]=offset;
		if (dist > zmax_dist) {
                        zmax_dist=dist;
                        zmax_index=zcount;
                }
		zcount++;
	} else {
		if(dist<zmax_dist) {    //substitute
                        strcpy(nearbyzips[zmax_index],zip);
			dists[zmax_index]=dist;
			sizes[zmax_index]=sz;
			offs[zmax_index]=offset;
                        zmax_dist=0;
			int i;
                        for ( i=0; i<zcount; i++) {
                                if (dists[i]>zmax_dist) {
                                        zmax_dist=dists[i];
                                        zmax_index=i;
                                }
                        }

                }


	}
}

void copy_address(ADDRESS dest, ADDRESS source) {
		dest.lat=source.lat;
		dest.lon=source.lon;
		dest.dist=source.dist;
		dest.bearing=source.bearing;
		dest.num=source.num;
		strcpy( dest.zipcode , source.zipcode );
		strcpy( dest.street , source.street );
		strcpy( dest.city , source.city );
		strcpy( dest.state , source.state );
}



int reverse_lookup(double lat, double lon, char * streetbuf, char * zipfile) {

	FILE *fp;
	char line[256];
        sprintf(line, "%s/geodata.dat",zipfile);
        fp = fopen(line, "r");
	if (fp==NULL)
        	fp = fopen("geodata.dat", "r");
	if (fp==NULL) {
                printf("couldn't open file: %s\n", line);
                return -1;
        }
	zcount=0;
        zmax_index=0;
        zmax_dist=0;
	char zip[6];
	char junk[23];
	memset(nearbyzips,0,sizeof(nearbyzips));
	memset(zip,0,6);
	long la,lo, sz, offset;
	fread (zip,5,1,fp);
	while (zip[0]!='#') {
		fread (&la,4,1,fp);
        	fread (&lo,4,1,fp);
        	fread (junk,22,1,fp); //no need for state and city here
        	fread (&sz,4,1,fp);
        	fread (&offset,4,1,fp);
		double zlat=(double)(la)/1000000;
		double zlon=(double)(lo)/1000000;

		if (fabs(zlat-lat)<0.25 && fabs(zlon-lon)<0.25) {
                	double dist, bearing;
                	GeoRangeAndBearing(lat, lon, zlat,zlon, &dist, &bearing) ;
			//fprintf(stdout,"===>%s:%f\n",zip,dist);
			insert_zip(zip,dist,sz,offset);
		}
		fread (zip,5,1,fp);
	}

	ADDRESS s1,s2,s3;
        s1.dist=100000;
        s2.dist=100000;
        s3.dist=100000;

	int i;
	for ( i=0; i<6 && i<zcount; i++)  {
		//printf("[%s]:::[%f]\n", nearbyzips[i],dists[i]);
		fseek(fp,offs[i],SEEK_SET);
		unsigned char *in = (unsigned char*) malloc(sizes[i]+1);
		memset(in,0,sizes[i]+1);
		long rs=fread(in,1,sizes[i],fp);
		char *out =(char*) malloc(sizes[i]*10+1);
        	if (out==NULL) {
                	printf("Could not allocate space for decompressed data\n");
			fclose(fp);
                	return (-1);
        	}
		memset(out,0,sizes[i]*10+1);
        	z_stream strm;
        	int ret;
        	/* allocate inflate state */
        	strm.zalloc = Z_NULL;
        	strm.zfree = Z_NULL;
        	strm.opaque = Z_NULL;
       		strm.avail_in = 0;
        	strm.next_in = Z_NULL;
        	ret = inflateInit(&strm);
        	if (ret != Z_OK) {
			fclose(fp);
			(void)inflateEnd(&strm);
                	return ret;
		}
        	strm.avail_in =rs;
        	strm.next_in = in;

        	strm.avail_out = sizes[i]*10+1;

		strm.next_out = (unsigned char*)out;
        	ret = inflate(&strm, Z_NO_FLUSH);
        	assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
        	switch (ret) {
            		case Z_NEED_DICT:
           		ret = Z_DATA_ERROR;     /* and fall through */
            		case Z_DATA_ERROR:
            		case Z_MEM_ERROR:
            		(void)inflateEnd(&strm);
			fclose(fp);
                	return ret;
            	}
		//printf("%s",out);
		free(in);

		//parse out...
		char* street = strtok(out,",");
		char* nums = strtok(NULL,",");
		char*  lonstr= strtok(NULL,"+");
		char*  latstr= strtok(NULL,"\n");

		while( street!=NULL && nums!=NULL && lonstr!=NULL && latstr!=NULL) {
			double zlon=atof(lonstr)/1000000;
			double zlat=atof(latstr)/1000000;
			double d, bearing;
			//printf("<<<%d %s, %f %f>>>\n", atoi(nums), street,zlat,zlon);
			if (fabs(zlon-lon)<0.0015 && fabs(zlat-lat)<0.0015) {
			//printf("{{{{%f:::%f}}}\n",fabs(zlon-lon),fabs(zlat-lat));
                        GeoRangeAndBearing(lat, lon, zlat,zlon, &d, &bearing) ;
			if (d<s1.dist) {
                                copy_address(s3,s2);
                                copy_address(s2,s1);
                                s1.dist=d;
                                s1.bearing=bearing;
                                s1.lat=zlat;
                                s1.lon=zlon;
                                strcpy(s1.street,street);
                                strcpy(s1.zipcode,nearbyzips[i]);
                                s1.num=atoi(nums);

                        } else if (d<s2.dist) {
                                copy_address(s3,s2);
                                s2.dist=d;
                                s2.bearing=bearing;
                                s2.lat=zlat;
                                s2.lon=zlon;
                                strcpy(s2.street,street);
                                strcpy(s2.zipcode,nearbyzips[i]);
                                s2.num=atoi(nums);
                        } else if (d<s3.dist) {
                                s3.dist=d;
                                s3.bearing=bearing;
                                s3.lat=zlat;
                                s3.lon=zlon;
                                strcpy(s3.street,street);
                                strcpy(s3.zipcode,nearbyzips[i]);
                                s3.num=atoi(nums);
                        }
			}
			street = strtok(NULL,",");
			nums = strtok(NULL,",");
			lonstr= strtok(NULL,"+");
			latstr= strtok(NULL,"\n");

		}	
		free(out);
		(void)inflateEnd(&strm);
	}
	fclose(fp);
	//fprintf(stdout,"1: %d %s %3.0f\n", s1.num, s1.street, s1.dist);
	//fprintf(stdout,"2: %d %s %3.0f\n", s2.num, s2.street, s2.dist);
	//fprintf(stdout,"3: %d %s %3.0f\n", s3.num, s3.street, s3.dist);

	if (s1.dist>300) {
		sprintf (streetbuf, "NO GEO DATA AVAILABLE FOR THIS AREA");
		return 1;
	}
	if (strcmp(s1.street,s2.street)==0 ) {
		if (s1.dist==s3.dist)
			sprintf (streetbuf, "STREET LOOKUP: INTERSECTION [%3.0fM]      %d %s,%s & %d %s,%s", s1.dist,s1.num, s1.street, s1.zipcode,  s3.num, s3.street, s3.zipcode);
		else
			sprintf (streetbuf, "STREET LOOKUP: NEARBY STREET [%3.0fM]     %d %s,%s", s1.dist,s1.num, s1.street, s1.zipcode);
		return 1;
	}
	if (s1.dist==s2.dist)
			sprintf (streetbuf, "STREET LOOKUP: INTERSECTION [%3.0fM]      %d %s,%s & %d %s,%s", s1.dist,s1.num, s1.street, s1.zipcode,  s2.num, s2.street, s2.zipcode);
	else
			sprintf (streetbuf, "STREET LOOKUP: NEARBY STREET [%3.0fM]     %d %s,%s", s1.dist,s1.num, s1.street, s1.zipcode);
	return 1;

}







// MIB.42_8 : added 'tolerance' to this call for second 'relaxed' pass  1 - relaxed search, 2 - wide search - just streetname
int address_geolookup( char * zipfile, char *addr, char* zipcode, char * city_filter, char * state_filter , double minlat, double minlon, double zmaxlat, double zmaxlon, int tolerance)
{
FILE *fp;
char line[256];
char streetbuf[61];
char *streetname;

	sprintf(streetbuf," %s ",addr);
	alias(streetbuf);
	char * street=streetbuf;
	while (street[0]==' ') street++;
	while (strlen(street)>0 && street[strlen(street)-1]==' ')
		street[strlen(street)-1]=0;
	//printf("converted:[%s]\n",street);
	int num=0;
	char strbuf[61];
	strcpy(strbuf,street);
	char * numstr = strtok(strbuf," ");		// numstr contains the number (find first entry in strbuf until the ' ' space
	if(numstr==NULL)				// MIB.42_8
	{
		if(tolerance!=2)
	                return -1;
	}
	if(tolerance<2)					// MIB.42_8
	{
		num=atoi(numstr);
		if (num==0)
			return -1;
		streetname = &street[strlen(numstr)+1];
	}
	else
	{
		num=0;
		streetname = street;
	}

	while (streetname[0]==' ') streetname++;
	if (streetname==NULL || strlen(streetname)<3)
		return -1;


	sprintf(line, "%s/geodata.dat",zipfile);
	zipcounter=0;
        fp = fopen(line, "r");

	if (fp==NULL)
        	fp = fopen("geodata.dat", "r");
	if (fp==NULL) {
                printf("couldn't open file: %s\n", line);
                return -1;

        }
	char zip[6];
	char state[3];
	char city[21];
	memset(zip,0,6);
	memset(state,0,3);
	memset(city,0,21);
	long la,lo, sz, offset, current_offset=0;
	double lat,lon;
	fread (zip,5,1,fp);
	current_offset+=5;
	while (zip[0]!='#')
	{
                fread (&la,4,1,fp);
                fread (&lo,4,1,fp);
                fread (state,2,1,fp);
                fread (city,20,1,fp);
                fread (&sz,4,1,fp);
                fread (&offset,4,1,fp);
                lat=(double)(la)/1000000;
                lon=(double)(lo)/1000000;
		current_offset+=38;
		//fprintf(stdout,"%f>%f>%f,%f>%f>%f,%s,%s,%s\n", zmaxlat,lat,minlat,zmaxlon,lon,minlon,zip,city,state);
		if ( strlen(zipcode)>0 && strstr(zip, zipcode)==NULL) {
			fread (zip,5,1,fp);
			current_offset+=5;
			continue;
		}
		if ( strlen(state_filter)>0 && strstr(city, city_filter)==NULL) {
			fread (zip,5,1,fp);
			current_offset+=5;
			continue;
		}
		if (strlen(city_filter)>0 && strstr(city,city_filter)==NULL){
			fread (zip,5,1,fp);
			current_offset+=5;
                        continue;
		}
		if (lat<minlat) {
			fread (zip,5,1,fp);
			current_offset+=5;
                        continue;
		}
                if (lat>zmaxlat) {
			fread (zip,5,1,fp);
			current_offset+=5;
                        continue;
		}

		if (lon>zmaxlon || lon<minlon) {
			fread (zip,5,1,fp);
			current_offset+=5;
                        continue;
		}
//	ms_write_log("%s; %s; %s; %f; %f\n", zip,state,city,lat,lon);
	//fprintf(stdout,"%s; %s; %s; %f; %f\n", zip,state,city,lat,lon);
		fseek(fp,offset,SEEK_SET);
                unsigned char *in = (unsigned char*) malloc(sz+1);
                if (in==NULL) {
                        printf("Could not allocate space for compressed data\n");
                        fclose(fp);
                        return (-1);
                }
                memset(in,0,sz+1);
                long rs=fread(in,1,sz,fp);
                char *out =(char*) malloc(sz*10+1);
                if (out==NULL) {
                        printf("Could not allocate space for decompressed data\n");
			free(in);
                        fclose(fp);
                        return (-1);
                }
                memset(out,0,sz*10+1);
                z_stream strm;
                int ret;
                /* allocate inflate state */
                strm.zalloc = Z_NULL;
                strm.zfree = Z_NULL;
                strm.opaque = Z_NULL;
                strm.avail_in = 0;
                strm.next_in = Z_NULL;
                ret = inflateInit(&strm);
                if (ret != Z_OK) {
                        fclose(fp);
                        (void)inflateEnd(&strm);
                        return ret;
                }
                strm.avail_in =rs;
                strm.next_in = in;
		
		strm.avail_out = sz*10+1;

                strm.next_out = (unsigned char*)out;
                ret = inflate(&strm, Z_NO_FLUSH);
                assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
                switch (ret) {
                        case Z_NEED_DICT:
                        ret = Z_DATA_ERROR;     /* and fall through */
                        case Z_DATA_ERROR:
                        case Z_MEM_ERROR:
                        (void)inflateEnd(&strm);
                        fclose(fp);
                        return ret;
                }
                //printf("%s",out);
                free(in);
                (void)inflateEnd(&strm);

		//parse out...
                char* strt = strtok(out,",");
                char* nums = strtok(NULL,",");
                char*  lonstr= strtok(NULL,"+");
                char*  latstr= strtok(NULL,"\n");


		int lowmatch=0;
        	double lowmatchlat=0;
        	double lowmatchlon=0;
        	int himatch=0;
        	double himatchlat=0;
        	double himatchlon=0;
        	char numberline[128];
	
		int done=0;
		int cmpres=0;
		int cmpreslng=strlen(streetname);

//	ms_write_log("\nComparison starts tol=%d (cmpreslng=%d  streetname='%s')",tolerance,cmpreslng,streetname);

                while( strt!=NULL && nums!=NULL && lonstr!=NULL && latstr!=NULL)
		{
			switch(tolerance)
			{
				case 0 :
					cmpres=strncmp(strt,streetname,strlen(street));
				break;
				case 1 :
				case 2 :
					cmpres=0;
					while(strt[cmpres]==streetname[cmpres]) ++cmpres;
					if(cmpres==0) cmpres=1;
					else
					if(cmpres>=cmpreslng) cmpres=0;
				break;
			}
			if(cmpres!=0)
			{
                		strt = strtok(NULL,",");
                		nums = strtok(NULL,",");
                		lonstr= strtok(NULL,"+");
                		latstr= strtok(NULL,"\n");
                        	continue;
			}
			strcpy(numberline,nums);
			char *geonum=strtok(numberline,";");
                	while (geonum!=NULL) {
                		//printf ("<%s>\n",geonum);
                		int geonumber=atoi(geonum);
                		if((num!=0)&&(geonumber%2!=num%2))		// MIB.42_8
				{
					geonum=strtok(NULL,";");
					continue;
                		}
				if((abs(geonumber-num)<2)||(num==0))		// MIB.42_8
				{
					strcpy(addrs[zipcounter].zipcode,zip);
					switch(tolerance)
					{
						case 0 :
							strcpy(addrs[zipcounter].street,street);
						break;
						case 1 :
							sprintf(addrs[zipcounter].street,"%d %s",num,strt);		// MIB.42 changed to accomodate relaxed search
						break;
						case 2 :
							sprintf(addrs[zipcounter].street,"%d %s",geonumber,strt);	// MIB.42_8
						break;
					}
                        		strcpy(addrs[zipcounter].city,city);
                        		strcpy(addrs[zipcounter].state,state);
                        		addrs[zipcounter].lat=atof(latstr)/1000000;
                        		addrs[zipcounter].lon=atof(lonstr)/1000000;
					zipcounter++;
					done=1;
					break;
                		}

                		if (geonumber>num) {
                        		if (lowmatch==0) {
						done=1;
                                		break;
                        		} else {
                                		himatch=geonumber;
                                		himatchlon=atof(lonstr)/1000000;
                                		himatchlat=atof(latstr)/1000000;
                                		//printf ("!{%f}{%f}{%d}{%s}\n",himatchlat, himatchlon,himatch, strt);
                                		//printf ("!{%f}{%f}{%d}{%s}\n",lowmatchlat, lowmatchlon,lowmatch, strt);
                                		double percent_off=(double)(num-lowmatch)/(double)(himatch-lowmatch);
						strcpy(addrs[zipcounter].zipcode,zip);
						if(tolerance)	// MIB.42 
							sprintf(addrs[zipcounter].street,"%d %s",num,strt);	// MIB.42 changed to accomodate relaxed search
						else
							strcpy(addrs[zipcounter].street,street);
                        			strcpy(addrs[zipcounter].city,city);
                        			strcpy(addrs[zipcounter].state,state);
                                		addrs[zipcounter].lat=lowmatchlat+(himatchlat-lowmatchlat)*percent_off;
                                		addrs[zipcounter].lon=lowmatchlon+(himatchlon-lowmatchlon)*percent_off;
						zipcounter++;
						done=1;
                               			break; 
                        		}
				} else {
                        		lowmatch=geonumber;
                          		lowmatchlon=atof(lonstr)/1000000;
                           		lowmatchlat=atof(latstr)/1000000;
                		}
                		geonum=strtok(NULL,";");
                		//printf("[%d][%s]\n", geonumber,line);
               		}
                	strt = strtok(&latstr[strlen(latstr)+1],",");
                	nums = strtok(NULL,",");
                	lonstr= strtok(NULL,"+");
                	latstr= strtok(NULL,"\n");
		
			if (done==1)
				break;
		}


                free(out);	


		//printf("!!!{%ld}\n",current_offset);
		fseek(fp,current_offset,SEEK_SET);
		memset(zip,0,6);
                fread (zip,5,1,fp);
		current_offset+=5;
        }

	fclose(fp);
	return 0;
}
