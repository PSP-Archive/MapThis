#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <math.h>
#include "main.h"
#include "menu.h"
#include "basic.h"

IMGLIST *img_list=NULL;
Image *imglist_search(char *name);
void imglist_add(Image *s, char *name);
FILE * gpsfsfp[5];
long *offsets;
long totallines=0;
short *files;
extern Image * blank;
// MIB.42_2
void ms_write_log(char *fmt,...)
{
va_list args;
FILE    *fp;

        if(!config.debug_logging) return;
        if((fp=fopen("loadlogs.txt","a+"))==NULL) return;
        va_start(args, fmt);
        vfprintf(fp, fmt, args);
        fflush(fp);
        fclose(fp);
        va_end(args);
return;
}
// MIB.42_2 end of edit

Image *loadfromcache(char *filename)
{
	return imglist_search(filename);
}
		



Image *loadfromdir(char *filename)
{
 
	Image * s1;

	if((s1=imglist_search(filename))!=NULL) {
        	return s1;
       	}
        s1=ldImage(filename);
	if (s1!=NULL)
		imglist_add(s1,filename);
	return s1;
}

Image *loadfromdir_check4blank(char *filename,int negate)
{
 
	Image * s1;

	if((s1=imglist_search(filename))!=NULL) {
        	return s1;
       	}
        s1=ldImage1(filename,negate);
	if (s1!=NULL)
		imglist_add(s1,filename);
	return s1;
}




void cleanup (int mapx, int mapy, int zoom,char * zipfile,int datatype) {
	char filename[128];
	int cmplen, tx,ty;
	sceKernelWaitSema(tlock,1,0); 
        IMGLIST *i=img_list;
        while(i!=NULL) {
		memset(filename,0,sizeof(filename));
		if (datatype==1) { //GPSFS
			cmplen=strlen(i->name);
			filename[0]=i->name[cmplen-8];
			filename[1]=i->name[cmplen-7];
			filename[2]=i->name[cmplen-6];
			filename[3]=i->name[cmplen-5];
			tx=atoi(filename)*TILE_SIZE;
			memset(filename,0,sizeof(filename));
			filename[0]=i->name[cmplen-12];
			filename[1]=i->name[cmplen-11];
			filename[2]=i->name[cmplen-10];
			filename[3]=i->name[cmplen-9];
			ty=atoi(filename)*TILE_SIZE;
		} else { //dir
			cmplen=strlen(i->name);
			filename[0]=i->name[cmplen-7];
			filename[1]=i->name[cmplen-6];
			filename[2]=i->name[cmplen-5];
			tx=atoi(filename)*TILE_SIZE;
			memset(filename,0,sizeof(filename));
			filename[0]=i->name[cmplen-10];
			filename[1]=i->name[cmplen-9];
			filename[2]=i->name[cmplen-8];
			ty=atoi(filename)*TILE_SIZE;

		}

		if (tx < mapx/zoom - PSP_WIDTH/2 - TILE_SIZE-RADIUS ||  tx > mapx/zoom + PSP_WIDTH/2+RADIUS || ty < mapy/zoom - PSP_WIDTH - TILE_SIZE-RADIUS || ty > mapy/zoom + PSP_WIDTH+RADIUS)  { 

			if(i->refcount) {
				//ms_write_log("cleaning [%s] %d\n", i->name, zoom);
                               i->refcount=0;
                        }

		}
		
                i=i->next;
        }
	sceKernelSignalSema(tlock, 1); 
}



void unload_group (int zoom,char * zipfile,int datatype) {
	char filename[128];
	int cmplen;
        IMGLIST *i=img_list;
	sceKernelWaitSema(tlock,1,0); 
	  if (datatype==1) {
		sprintf(filename,"%dx",zoom);
		cmplen=2;
	  } else {
		sprintf(filename,"%s/%dx",zipfile,zoom);
		cmplen=strlen(zipfile)+3;
	}

        while(i!=NULL) {
		//printf("%d:%s:%s:%d\n",zoom, i->name,filename,cmplen);
                if(strncmp(i->name,filename,cmplen)!=0) {
                        if(i->refcount) {
				//ms_write_log("%d:%s:%s:%d\n",zoom, i->name,filename,cmplen);
                                i->refcount=0;
                        }
                }
                i=i->next;
        }
	sceKernelSignalSema(tlock, 1); 
}

void unloadmap(char *name)
{
	sceKernelWaitSema(tlock,1,0); 
        IMGLIST *i=img_list;
	
        while(i!=NULL) {
                if(!strcmp(i->name,name)) {
                        if(i->refcount) {
                                i->refcount=0;
                        }
			//fprintf(stdout,"unloading %s\n",name);
			sceKernelSignalSema(tlock, 1); 
                        return;
                }
                i=i->next;
        }
	sceKernelSignalSema(tlock, 1); 
}


void imglist_add(Image *s, char *name)
{
	sceKernelWaitSema(tlock,1,0); 
        IMGLIST *nu;
	if((nu=(IMGLIST* )malloc(sizeof(IMGLIST)))==NULL)
        {
                ms_write_log("Failed to allocate memory in imglist_add(%s) for %d bytes...\n",name,(sizeof(IMGLIST)));  // MIB.42_2
		sceKernelSignalSema(tlock, 1); 
                return;
        }
        strcpy(nu->name,name);
        nu->refcount=1;
        nu->img=s;

        if(img_list==NULL) {
                img_list=nu;
                nu->next=NULL;
        } else {
                nu->next=img_list;
                img_list=nu;
        }
	sceKernelSignalSema(tlock, 1); 
}

Image *imglist_search(char *name)
{
	sceKernelWaitSema(tlock,1,0); 
        IMGLIST *i=img_list;

        while(i!=NULL) {
                if(!strcmp(name,i->name)) {
                        //i->refcount++;
			sceKernelSignalSema(tlock, 1); 
                        return (i->img);
                }
                i=i->next;
        }
	sceKernelSignalSema(tlock, 1); 
        return(NULL);
}


void imglist_garbagecollect()
{
        IMGLIST *s=img_list,*p=NULL,*n=NULL;
	sceKernelWaitSema(tlock,1,0); 

        while(s!=NULL) {
                n=s->next;

                if(s->refcount==0) {
                        if(p==NULL) {
                                img_list=n;
                        } else {
                                p->next=n;
                        }
                        freeImage(s->img);
                        free(s);
                } else {
                        p=s;
                }
                s=n;
        }
	sceKernelSignalSema(tlock, 1); 
     //   ms_write_log("garbage collect done\n");
}


long totalTiles(int sz) {  //returns total number of tiles, in map of sz size
        switch (sz) {
                case 2: return 4;
                case 4: return 20;
                case 8: return 84;
                case 16: return 340;
                case 32: return 1364;
                case 64: return 5460;
                case 128: return 21844;
                case 256: return 87380;
                case 512: return 349524;
                case 1024: return 1398100;
                case 2048: return 5592404;
                case 4096: return 22369620;
                default: return 0;
        }

}

long translateCoords(int x, int y, int zm, int dimension) {
        return x+y*dimension/zm+totalTiles(dimension)-totalTiles(dimension/zm);
}


int gpsfsGetX () {
        char buf[5];
        fseek(gpsfsfp[0], 8, SEEK_SET);
        memset(buf,0, sizeof(buf));
        fread(buf, 4, 1, gpsfsfp[0]);
        return *((int *)buf);

}

int gpsfsGetY () {
        char buf[5];
        fseek(gpsfsfp[0], 16, SEEK_SET);
        memset(buf,0, sizeof(buf));
        fread(buf, 4, 1, gpsfsfp[0]);
        return *((int *)buf);

}

int gpsfsGetBaseZoom () {
        char buf[5];
        fseek(gpsfsfp[0], 24, SEEK_SET);
        memset(buf,0, sizeof(buf));
        fread(buf, 4, 1, gpsfsfp[0]);
        return *((int *)buf);

}

int gpsfsGetFileType () {
        char buf[5];
        fseek(gpsfsfp[0], 32, SEEK_SET);
        memset(buf,0, sizeof(buf));
        fread(buf, 4, 1, gpsfsfp[0]);
        return *((int *)buf);

}



long gpsfsGetTotalTiles () {
	if (totallines<=0) {
        char buf[5];
        fseek(gpsfsfp[0], 40, SEEK_SET);
        memset(buf,0, sizeof(buf));
        fread(buf, 4, 1, gpsfsfp[0]);
        return *((long *)buf);
	}else
		return totallines;

}

int gpsfsGetMapDimension() {
        long maxsize=gpsfsGetTotalTiles ();
        long sz=4;
        long sum=4;
        while (maxsize>sum) {
                sz=sz*4;
                sum+=sz;
        }
        return (int)sqrt(sz);
}

long gpsfsGetFileSize(int i) {
        fseek(gpsfsfp[i], 0, SEEK_END);
        return ftell(gpsfsfp[i]);
}

Image * gpsfsGetTile(int offset, int swizzle) {
        char buf[9];
int     totaltiles;
	int i;
        for ( i=0; i<5; i++) {
                if (gpsfsfp[i]==NULL)
                {
                        ms_write_log("gpsfsGetTile gpsfsfp[%d]==NULL\n",i);     // MIB.42_2
                        return NULL;
                }
        }

        totaltiles=gpsfsGetTotalTiles();
        if(offset>=totaltiles)
        {
                if(offset>(totaltiles+1))
                        ms_write_log("gpsfsGetTile offset[%d]>=gpsfsGetTotalTiles()[%d] \n",offset,gpsfsGetTotalTiles());               // MIB.42_2
                return NULL;
        }


	long x,y,size;
	short whichfile; 
	if (offsets!=NULL && files!=NULL) {
		x = offsets[offset];
        	if (x<0)
                	return NULL;
		y = offsets[offset+1];
		if (y==0) 
			size=gpsfsGetFileSize(files[offset])-x;
        	else if (y<0)
                	size=-y;
        	else
                	size=y-x;

		whichfile=files[offset];

	} else {
        	fseek(gpsfsfp[0], 48+offset*8, SEEK_SET);
        	memset(buf,0, sizeof(buf));
        	fread(buf, 4, 1, gpsfsfp[0]);
		whichfile=*((short *)buf);
		//printf("getting whichfile:%d\n",whichfile);
        	memset(buf,0, sizeof(buf));
        	fread(buf, 4, 1, gpsfsfp[0]);
        	x = *((long *)buf);
        	if (x<0)
                	return NULL;
        	//fseek(fp, 48+offset*8+8, SEEK_SET);
        	fread(buf, 4, 1, gpsfsfp[0]);
        	memset(buf,0, sizeof(buf));
        	fread(buf, 4, 1, gpsfsfp[0]);
        	y = *((long *)buf);

		if (y==0)
                        size=gpsfsGetFileSize(whichfile)-x;
                else if (y<0)
                        size=-y;
                else
                        size=y-x;



	}
      //fprintf(stdout,"offset=%ld x=%ld:::y=%ld:::%ld, %d\n",offset, x,y, size, whichfile);
        fseek(gpsfsfp[whichfile], x, SEEK_SET);
        unsigned char* data=(unsigned char*)malloc(size+1);
	if (data==NULL)
        {
                ms_write_log("Failed to allocate memory in gpsfsGetTile for %d bytes... offset=%ld x=%ld:::y=%ld:::%ld, %d\n",(size+1),offset, x,y, size, whichfile);   // MIB.42_2
                return NULL;
        }
        fread(data, size, 1, gpsfsfp[whichfile]);
        Image* img=loadImageFromMemory(data, size);
        free(data);
	if (swizzle)
		swizzleImage(img);
        return img;
}

int gpsfsOpen(char * dirname) {
	char filename[128];
	char buf[4];
	sprintf(filename,"%s/GPSFS",dirname);
	gpsfsfp[0]=fopen(filename,"rb");
	if (gpsfsfp[0]==NULL)
        {
                ms_write_log("Failed gpsfsOpen('%s') dirname='%s'\n",filename,dirname); // MIB.42_2
                return -1;
        }
	totallines=0;
	long t=gpsfsGetTotalTiles();
	//fprintf(stdout,"TotalTiles: %ld\n",t);
	if (config.cachemapindex==0)
		offsets=NULL;
	else {
		offsets = (long *) malloc ((t+2)*sizeof(long));
		files = (short *) malloc ((t+2)*sizeof(short));
	}
	if (offsets==NULL || files==NULL) {
		if (config.cachemapindex==1)
			ms_write_log("Failed to allocate memory. offsets=%x (size=%d)  file=%x (size=%d)\n",offsets,((t+2)*sizeof(long)),files,((t+2)*sizeof(short)));  // MIB.42_2
		if (offsets!=NULL) free(offsets);
		if (files!=NULL) free(files);
	} else {
	//populate the offsets table
	long i;
	for ( i=0; i<=t; i++) {
        	fseek(gpsfsfp[0], 48+i*8, SEEK_SET);
        	memset(buf,0, sizeof(buf));
        	fread(buf, 4, 1, gpsfsfp[0]);
        	files[i] = *((short *)buf);

        	memset(buf,0, sizeof(buf));
        	fread(buf, 4, 1, gpsfsfp[0]);
        	offsets[i] = *((long *)buf);
	}
	}
	int i;
	for (i=1; i<5; i++) {
		sprintf(filename,"%s/GPSFS%d",dirname,i);
        	gpsfsfp[i]=fopen(filename,"rb");
		if (gpsfsfp[i]==NULL){
			ms_write_log("Failed to open %d. mapfile '%s'\n",i,filename);   // MIB.42_2
			return -1;
		}
	}	
	return 1;	
}

void gpsfsClose() {
	int i;
	for (i=0; i<5; i++) {
		if (gpsfsfp[i]!=NULL)
			fclose(gpsfsfp[i]);
	}
	totallines=0;
	if (offsets!=NULL)
		free (offsets);
	if (files!=NULL)
		free (files);
}

Image *loadfromgpsfs(int x , int y, int zm, int sz) {
	char filename[32];
	sprintf(filename,"%dx%04d%04d.GPS",zm,y,x);
	Image * s1;

	if((s1=imglist_search(filename))!=NULL) {
		return s1;
       	}
	//printf("%d:%d:%d:::%ld\n",x,y,zm,translateCoords(x,y,zm,sz));
	s1=gpsfsGetTile(translateCoords(x,y,zm,sz),1);
	if (s1!=NULL) {
                imglist_add(s1,filename);
        	return s1;
	}


	if (zm>1) {
		Image *q1=gpsfsGetTile(translateCoords(x*2,y*2,zm/2,sz),0);
		if (q1!=NULL) {
			s1=createImage(256,256);
			if (s1!=NULL)
				blitImageToImage2(0,0,128,128,q1,0,0,s1);
			freeImage(q1);
		} else
			return NULL;
		q1=gpsfsGetTile(translateCoords(x*2+1,y*2,zm/2,sz),0);
		if (q1!=NULL) {
			if (s1!=NULL)
				blitImageToImage2(0,0,128,128,q1,128,0,s1);
			freeImage(q1);
		} else
			return NULL;
		q1=gpsfsGetTile(translateCoords(x*2,y*2+1,zm/2,sz),0);
		if (q1!=NULL) {
			if (s1!=NULL)
				blitImageToImage2(0,0,128,128,q1,0,128,s1);
			freeImage(q1);
		} else
			return NULL;
		q1=gpsfsGetTile(translateCoords(x*2+1,y*2+1,zm/2,sz),0);
		if (q1!=NULL) {
			if (s1!=NULL)
				blitImageToImage2(0,0,128,128,q1,128,128,s1);
			freeImage(q1);
		}
	}
	swizzleImage(s1);
	if (s1!=NULL)
        	imglist_add(s1,filename);
        return s1;
}
