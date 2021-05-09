#include "line.h"
#include "utils.h"
#include "main.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"

int icount=0;
int segcount=0;

LINE * lines1[MAXLINES];
extern double ftx;
extern double fty;
extern int basezoom;
//extern zoom;
extern int TILE_NUM;
extern int my_access(char *name,int ok) ;
double imax_dist=0;
int imax_index=0;

#define CHUNK 6800
#define SEGMAX 30  

SEGMENT * segs[SEGMAX];

extern long mapx ;
extern long mapy ;



int seg_cmp(const void * a1, const void * a2) {
        SEGMENT *seg1 = *(SEGMENT **) a1;
        SEGMENT *seg2 = *(SEGMENT **) a2;
        //fprintf(stdout, "comparing %s:::%s\n", seg1->area , seg2->area);
	double d1=(mapx-seg1->x)*(mapx-seg1->x)+(mapy-seg1->y)*(mapy-seg1->y);
	double d2=(mapx-seg2->x)*(mapx-seg2->x)+(mapy-seg2->y)*(mapy-seg2->y);
        return (int)(d1-d2);
}


void insert_segment(SEGMENT *seg) {
	int d=(mapx-seg->x)*(mapx-seg->x)+(mapy-seg->y)*(mapy-seg->y);
        if (segcount<SEGMAX){                      //just insert first N points
                segs[segcount++]=seg;

                if (d > imax_dist) {
                        imax_dist=d;
                        imax_index=segcount-1;
                }
        } else {                                 //may need to substitute stuff
                if(d<imax_dist) {    //substitute
                        free(segs[imax_index]);
                        segs[imax_index]=seg;
                        imax_dist=0;
                        int i;
                        for (i=0; i<segcount; i++) {
				d=(mapx-segs[i]->x)*(mapx-segs[i]->x)+(mapy-segs[i]->y)*(mapy-segs[i]->y);
                                if (d>imax_dist) {
                                        imax_dist=d;
                                        imax_index=i;
                                }
                        }

                }


        }
}


//inserts up to MAXLINE of closes street lines...


void insert_line(LINE *line) {
	sceKernelWaitSema(linelock,1,0);
	if (icount<MAXLINES){  			//just insert first N points 
		if (lines1[icount]!=NULL)
			free(lines1[icount]);
		lines1[icount++]=line;
		 //printf(" %d - %d %d|%d %d|%s,%d,%d,%d,%d,%d [%s]\n",icount-1,line->sx,line->sy,line->ex,line->ey,line->name, line->stype, line->fromaddrl, line->toaddrl, line->fromaddrr, line->toaddrr, line->postcode);
	}  					//may need to substitute stuff
 	sceKernelSignalSema(linelock, 1);
}

void free_line(int counter) {
	sceKernelWaitSema(linelock,1,0);
	if  (lines1[counter]!=NULL) {
		free(lines1[counter]);
		lines1[counter]=NULL;
	}
 	sceKernelSignalSema(linelock, 1);

}

int get_line(int counter, LINE *l) {
//	sceKernelWaitSema(linelock,1,0);
	if  (lines1[counter]!=NULL ) {

		memcpy(l,lines1[counter],sizeof(LINE));

 //		sceKernelSignalSema(linelock, 1);
		return 1;
	 }
//	sceKernelSignalSema(linelock, 1);
	return -1;
}


int load_lines(char * file, char * zipfile, int datatype, long mapx, long mapy, int zm ) {
	icount=0;
	int prevcount=0;
	int same=0;
	FILE *fp;
	char strline[256];
	LINE * line;
	double ox=ftx;
	double oy=fty;
	int i;
	int m=1;
	for (i=basezoom; i>0; i--) {
		ox*=2;	
		oy*=2;
		m*=2;	
	}
	ox*=256;
	oy*=256;
                sprintf(strline, "%s/%s",zipfile,file);
		if (my_access(strline, F_OK) != 0) {
#ifndef GENERIC
                	sprintf(strline, "ms0:/PSP/COMMON/%s",file);
#else
                	sprintf(strline, "/PSP/COMMON/%s",file);
#endif
                 	fp = fopen(strline, "r");
		} else
                 	fp = fopen(strline, "r");

	if (fp==NULL) {
		printf("couldn't open file: %s\n", strline); 
		return -1;

	}
	
	SEGMENT *seg;
	int bytes=1;
	int chunkcount=0;
	while (bytes>0) {
		seg=(SEGMENT*)malloc(sizeof(SEGMENT));
		if (seg==NULL) {
			fclose(fp);
			return -1;
		}
                bytes=fread (seg,sizeof(SEGMENT),1,fp);
		if (strncmp(seg->area,"#####",5)==0) {
			free(seg);
			break;
		}
		seg->x=(long)(seg->x-ox)/m;
		seg->y=(long)(seg->y-oy)/m;
		//fprintf(stdout,"%s, %ld %ld %ld %ld\n", seg->area,seg->x,seg->y,seg->fsz,seg->offset);
		insert_segment(seg);
		chunkcount++;
	}
	qsort(segs,segcount,sizeof(segs[0]),seg_cmp);
	
	/*fprintf(stdout,"=======%ld %ld=================\n",mapx, mapy);
	for (i=0; i<segcount; i++) {
		fprintf(stdout,"%s, %ld %ld %ld %ld\n", segs[i]->area,segs[i]->x,segs[i]->y,segs[i]->fsz,segs[i]->offset);

	}
	*/
	i=0;
	while (icount<MAXLINES && i<segcount) {
	fseek(fp,segs[i]->offset,SEEK_SET);
	int ret;
    	unsigned have;
    	z_stream strm;
    	unsigned char in[CHUNK];
    	unsigned char out[CHUNK];

    	/* allocate inflate state */
    	strm.zalloc = Z_NULL;
    	strm.zfree = Z_NULL;
    	strm.opaque = Z_NULL;
    	strm.avail_in = 0;
    	strm.next_in = Z_NULL;
    	ret = inflateInit(&strm);
    	if (ret != Z_OK)
       		return -1;

	/* decompress until deflate stream ends or end of file */

	have=0;
    do {
        strm.avail_in = fread(in, 1, CHUNK, fp);
        if (ferror(fp)) {
            (void)inflateEnd(&strm);
            return -1;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK-have;
            strm.next_out = out+have;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
		//fprintf(stdout,"avail=%d\n",strm.avail_out);
            	have = CHUNK  - strm.avail_out;
		memset(&out[have],0,CHUNK-have);
		// parse the data chunk
		unsigned char * start = out;
		while (have>=sizeof(LINE)) {

	 		line=(LINE*)malloc(sizeof(LINE));
	 		if (line==NULL) {
				fprintf(stdout,"couldn't allocate memory for line#: %d\n", icount); 
				ret = Z_STREAM_END;
				break;
			}
			memcpy(line,start,sizeof(LINE));
			 //printf("%d %d|%d %d|%s,%d,%d,%d,%d,%d\n",line->sx,line->sy,line->ex,line->ey,line->name, line->stype, line->fromaddrl, line->toaddrl, line->fromaddrr, line->toaddrr);
			line->sx=(line->sx-ox)/m;
			line->sy=(line->sy-oy)/m;
			line->ex=(line->ex-ox)/m;
			line->ey=(line->ey-oy)/m;
			if (zoom>4 && line->stype>40) {
				free(line);
				goto here;
			}
			if (abs(line->sx-mapx)/zm<300 && abs(line->sy-mapy)/zm<300) {
				insert_line(line);
			} else
				free(line);
			start+=sizeof(LINE);
			have-=sizeof(LINE);

		}
		//printf("now have:%d\n",have);
		if (have>0)
			memcpy(out,start,have);

		//fprintf(stdout,"[%s]\n",out);
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
	here:
    /* clean up and return */
    (void)inflateEnd(&strm);

	//fprintf(stdout,"loaded: %s %d\n", segs[i]->area, icount-prevcount);
	if (icount==prevcount)
		same++;
	else
		same=0;
	if (same>1 && zoom<=4)
		break;
	prevcount=icount;
	i++;
}
	fclose(fp);
	free_segments();

	fprintf(stdout,"loaded: %d records\n", icount);

	while (icount<MAXLINES)
		free_line(icount++);
	return 0;		
}

void free_segments() {
	int i;
	for (i=0; i<segcount; i++) {
		if (segs[i]!=NULL) {
			free(segs[i]);
		}
	}
	segcount=0;
}
void free_lines() {
	free_segments(); 
	int i;
	for (i=0; i<icount; i++)
		free_line(i);
	icount=0;
}


int line_count() {
	return icount;
}


int isOnLine(LINE l, int x, int y) {
        int sx=l.sx;
        int sy=l.sy;
        int ex=l.ex;
        int ey=l.ey;
        if (sx<ex) {
                sx=ex;
                ex=l.sx;
        }
        if (sy<ey) {
                sy=ey;
                ey=l.sy;
        }
        if (x<=sx && x>=ex && y<=sy && y>=ey  ) {
                if ( (y==l.ey && x==l.ex)  || sy-ey<10  )
                        return 1;
                //fprintf(stdout,"%f\n",fabs((double)(l.ex-l.sx)/(l.ey-l.sy) - (double)(l.ex-x)/(l.ey-y)));
                if (fabs((double)(l.ex-l.sx)/(l.ey-l.sy) - (double)(l.ex-x)/(l.ey-y))<0.3) {
                        //fprintf(stdout,"!!!!%f\n",fabs((double)(l.ex-l.sx)/(l.ey-l.sy) - (double)(l.ex-x)/(l.ey-y)));
                        return 1;
                } else
                        return 0;
        }else
                return 0;

}

