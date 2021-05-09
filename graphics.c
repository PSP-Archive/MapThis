#include <stdlib.h>
#include <malloc.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <png.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdebug.h>
#include <math.h>
#include "font.h"
#include "menu.h"

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define true 1
#define false 0
#define MAXL 5000
#define PI      3.14159265358979326433
//extern "C" {
#include <jpeglib.h>
#include <jerror.h>
//}
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <pspgu.h>

#include "graphics.h"
#include "main.h"

#include "utils.h"		// MIB.42_2 
//#include "framebuffer.h"
#define IS_ALPHA(color) (((color)&0xff000000)==0xff000000?0:1)
#define FRAMEBUFFER_SIZE (PSP_LINE_SIZE*SCREEN_HEIGHT*4)
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
static unsigned int __attribute__((aligned(16))) dList[262144];


void drawHiliteBar(Image * hilite,int x,int y, int width) {
        int i;
        if (hilite==NULL)
                return;
        for ( i=0; i<width; i++)
                blitAlphaImageToScreen(0,0,8,8,hilite,x+i*8,y);
}

Image* ldImage(const char* filename) {
        Image *img = loadImage(filename);
	ms_write_log("opening image: %s\n", filename);		// MIB.42_2 to log
        swizzleImage(img);
        return img;
}


Image* ldImage1(const char* filename,int negate) {
        Image *img = loadImage1(filename,negate);
        swizzleImage(img);
        return img;
}


Color* g_vram_base = (Color*) (0x40000000 | 0x04000000);

typedef struct
{
	unsigned short u, v;
	short x, y, z;
} Vertex;

typedef struct {
        float u,v;
        unsigned int color;
        float x, y, z;
} Vertex1;

typedef struct {
	unsigned int color;
	float x, y, z;
} Vertex2;

//unsigned int __attribute__((aligned(16))) list[262144];
//void *dList;            // display List, used by sceGUStart

unsigned int color = GU_COLOR( 0.0f, 0.0f, 1.0f, 0.0f );



Vertex1 __attribute__((aligned(16))) quad[4] =
{
        {1,1, 0x00000000, -1.0f, 1.0f, 0.0f },     // top-left point
        {0,1, 0x00000000, 1.0f, 1.0f, 0.0f },     // top-right point
        {1,0, 0x00000000, -1.0f, -1.0f, 0.0f } ,    // bottom-left point
        {0,0, 0x00000000, 1.0f, -1.0f, 0.0f }      // bottom-right point
};

Vertex1 __attribute__((aligned(16))) verts[4]=
{
        {1,1, 0x00000000, -.1f, .1f, 0.0f },     // top-left point
        {0,1, 0x00000000, .1f, .1f, 0.0f },     // top-right point
        {1,0, 0x00000000, -.1f, -.1f, 0.0f } ,    // bottom-left point
        {0,0, 0x00000000, .1f, -.1f, 0.0f }      // bottom-right point
};

int curline=0;
Vertex2 __attribute__((aligned(16))) lines[2*MAXL];

enum {IS_INSIDE=0,IS_TOP=1,IS_BOTTOM=2,IS_LEFT=4,IS_RIGHT=8};

void drawLine3(float x1,float y1,float x2,float y2, Color c ,int mode,double angle, float fixvar, float anglevar) {
	if (curline>2*MAXL-2)
		renderLines(mode,  angle, fixvar, anglevar); 
	lines[curline].color=c;
	lines[curline+1].color=c;

	lines[curline].x=(1.0f-x1/256)*DIST;
	lines[curline].y=(y1/256-1.0f)*DIST;

	lines[curline+1].x=(1.0f-x2/256)*DIST;
	lines[curline+1].y=(y2/256-1.0f)*DIST;
	curline+=2;
}


void FillPolygon(float* x, float* y, int count, Color color, int mode, double angle) {
	int i;
	float z=ZNORM;
	if (mode==0) 
		angle=PI;
	else if(mode==2){
		z=Z;
	}
/*
	guStart( GU_DIRECT, dList );

	 sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadIdentity();
        sceGumPerspective( FOA, 16.4f/9.0f, NEARS, FARS);

        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();

        sceGuClearColor( GU_COLOR( 0.0f, 0.0f, 0.0f, 1.0f ) );
        sceGuClearDepth(0);

        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        ScePspFVector3 move1 = { 0.0f, -0.0f,  z };
        sceGumTranslate( &move1 );
	if (mode==2)
        	sceGumRotateX(ROTX);
        sceGumRotateZ(angle);
*/
	Vertex2* vertices = (Vertex2*)sceGuGetMemory(count * sizeof(Vertex2));
	//sceKernelDcacheWritebackInvalidateAll();
	for(i=0; i<count; i++)
	{
		vertices[i].color = color;
		vertices[i].x = 1.0f - x[i]/256;
		vertices[i].y = y[i]/256 - 1.0f;
		vertices[i].z = 0.0f;
		//fprintf(stdout, "%d ->%f:%f\n",i,vertices[i].x,vertices[i].y);
	}
	//sceGuFrontFace(GU_CCW);
	//sceGuDisable(GU_TEXTURE_2D);
	sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, count, 0, lines);
	//sceGuEnable(GU_TEXTURE_2D);
	//sceGuFrontFace(GU_CW);
 //       guFinish();
}

void DrawThickLine1(float x1, float y1, float x2, float y2, Color color, float lineWidth, int mode, double angle)
{
	float dy=y2-y1;
	float dx=x2-x1;
	if(dy==0 && dx==0)
		return;

	float l=(float)hypot(dx,dy);

	float x[4];
	float y[4];

	x[0]=x1+lineWidth*(y2-y1)/l;
	y[0]=y1-lineWidth*(x2-x1)/l;

	x[1]=x1-lineWidth*(y2-y1)/l;
	y[1]=y1+lineWidth*(x2-x1)/l;

	x[2]=x2-lineWidth*(y2-y1)/l;
	y[2]=y2+lineWidth*(x2-x1)/l;

	x[3]=x2+lineWidth*(y2-y1)/l;
	y[3]=y2-lineWidth*(x2-x1)/l;

	FillPolygon(x, y, 4, color, mode,angle);
}








void DrawThickLine(float x0,float y0,float x1,float y1, Color c, float w,int mode, double angle ,float fixvar, float anglevar) {
	float i,j;
	int width=512;
	w=w/2;
int tmp0,tmp1,tmpP,x=0,y=0;

	tmp0=IS_INSIDE;			// determine where x0:y0 is compared to screen
	if(y0>=(width-1)) tmp0|=IS_TOP;
	else if(y0<1) tmp0|=IS_BOTTOM;
	if(x0>=(width-1)) tmp0|=IS_RIGHT;
	else if(x0<1) tmp0|=IS_LEFT;
	tmp1=IS_INSIDE;			// determine where x1:y1 is compared to screen
	if(y1>=(width+1)) tmp1|=IS_TOP;
	else if(y1<1) tmp1|=IS_BOTTOM;
	if(x1>=(width-1)) tmp1|=IS_RIGHT;
	else if(x1<1) tmp1|=IS_LEFT;

	while(tmp0|tmp1)		// We need to keep clipping until we get inside the screen
	{
		if(tmp0&tmp1) return;		// not intersecting the screen, so no need to render
// Ok, we need to clip...
		if(tmp0) tmpP=tmp0; else tmpP=tmp1;	// x0:y0 point is not on screen
		if(tmpP&IS_TOP)				// Does it need top clipping?
		{
			if(y1!=y0)
				x=x0+(x1-x0)*((width-1)-y0)/(y1-y0);
			else
				x=x0+(x1-x0)*((width-1)-y0);
			y=width-2;
		}
		else
			if(tmpP&IS_BOTTOM)		// Does it need bottom clipping?
			{
				if(y1!=y0)
					x=x0+(x1-x0)*(1-y0)/(y1-y0);
				else
					x=x0+(x1-x0)*(1-y0);
				y=1;
			}
			else
				if(tmpP&IS_RIGHT)		// Does it need to be clip'd on the right?
				{
					if(x1!=x0)
						y=y0+(y1-y0)*((width-1)-x0)/(x1-x0);
					else
						y=y0+(y1-y0)*((width-1)-x0);
					x=width-2;
				}
				else
					if(tmpP&IS_LEFT)	// Does it need to be clip'd on the left?
					{
						if(x1!=x0)
							y=y0+(y1-y0)*(1-x0)/(x1-x0);
						else
							y=y0+(y1-y0)*(1-x0);
						x=1;
					}
		if(tmpP==tmp0)
		{
			x0=x;y0=y;
			tmp0=IS_INSIDE;			// determine where x0:y0 is compared to screen
			if(y0>=(width-1)) tmp0|=IS_TOP;
			else if(y0<1) tmp0|=IS_BOTTOM;
			if(x0>=(width-1)) tmp0|=IS_RIGHT;
			else if(x0<1) tmp0|=IS_LEFT;		
		}
		else
		{
			x1=x;y1=y;
			tmp1=IS_INSIDE;			// determine where x1:y1 is compared to screen
			if(y1>=(width-1)) tmp1|=IS_TOP;
			else if(y1<1) tmp1|=IS_BOTTOM;
			if(x1>=(width-1)) tmp1|=IS_RIGHT;
			else if(x1<1) tmp1|=IS_LEFT;
		}
	}

	//DrawThickLine1(x0+i, y0+j,x1+i,y1+j,c,w,mode,angle);
	if ( w<=.5f)
		drawLine3(x0, y0,x1,y1,c,mode,angle, fixvar, anglevar);
	else {
	for (i=-w; i<=w; i+=0.3f) {
		for (j=-w; j<=w; j+=0.3f)
			drawLine3(x0+i, y0+j,x1+i,y1+j,c,mode,angle, fixvar, anglevar);
	}
	}


}

void renderLines(int mode, double angle, float fixvar, float anglevar) {

	float xangle = 0.0;
	float z=ZNORM;
	if (mode==0) 
		angle=PI;
	else if (mode==2) {
		z=Z;
		xangle = ROTX;
	}
	z*=DIST;	
	sceKernelDcacheWritebackInvalidateAll();

        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        ScePspFVector3 move1 = { 0.0f, -0.0f,  z+fixvar };
        sceGumTranslate( &move1 );
       	sceGumRotateX(xangle+anglevar);
        sceGumRotateZ(angle);
	sceGuDisable(GU_TEXTURE_2D);
	sceGumDrawArray( GU_LINES,  GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, curline, 0, lines );
	sceGuEnable(GU_TEXTURE_2D);
	sceKernelDcacheWritebackInvalidateAll();
	curline=0;
//	 guFinish();
}


extern u8 msx[];


static int dispBufferNumber;
static int initialized = 0;
Image* loadJpegImage1(const char* filename,int negate);
Image* loadPngImage1(const char* filename,int negate);

static int getNextPower2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

Color* getVramDrawBuffer()
{
	Color* vram = (Color*) g_vram_base;
	if (dispBufferNumber == 0) vram += FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

Color* getVramDisplayBuffer()
{
	Color* vram = (Color*) g_vram_base;
	if (dispBufferNumber == 1) vram += FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
}

static int isJpegFile(const char* filename)
{
	char* suffix = strrchr(filename, '.');
	if (suffix) {
		if (stricmp(suffix, ".jpg") == 0 || stricmp(suffix, ".jpeg") == 0) return true;
	}
	return false;
}


void blitRotatedImage(Image *img, float x, float y, double angle, int mode ) {
	float w = (float)(img->imageWidth)/512.0f*DIST;
	float h = (float)(img->imageHeight)/512.0f*DIST;	
	if (x<0 || x >512)
		return;
	if (y<0 || y >512)
		return;

	float z=ZNORM;

	if (mode==0) {
		angle=PI;
	} else if (mode==2) {
		z=-0.49f;
	}
	z*=DIST;

	verts[0].x=-w;
	verts[0].y=h;

	verts[1].x=w;
	verts[1].y=h;

	verts[2].x=-w;
	verts[2].y=-h;

	verts[3].x=w;
	verts[3].y=-h;

	sceKernelDcacheWritebackInvalidateAll();
	guStart();
        sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadIdentity();
        sceGumPerspective( FOA, 16.4f/9.0f, NEARS, FARS);

        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();

        sceGuClearColor( GU_COLOR( 0.0f, 0.0f, 0.0f, 1.0f ) );
        sceGuClearDepth(0);

        sceGuTexScale( 1.0f, 1.0f );
        sceGuTexImage( 0, img->textureWidth, img->textureHeight, img->textureWidth, img->data );
	//sceKernelDcacheWritebackAll();
	
	
        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        ScePspFVector3 move1 = { 0.0f, -0.0f, z };
        sceGumTranslate( &move1 );
        ScePspFVector3 move = { DIST*(x/256-1),DIST*(1-y/256), 0.0 };
        sceGumTranslate( &move );
	if (mode==2)
        	sceGumRotateX(5.75f);
        sceGumRotateZ(angle);
	sceGumDrawArray( GU_TRIANGLE_STRIP,  GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 4, 0, verts );
	 guFinish();

}


void setScene( int mode) {

        sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadIdentity();
        sceGumPerspective( FOA, 16.4f/9.0f, NEARS, FARS);

        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();

        sceGuClearColor( GU_COLOR( 0.0f, 0.0f, 0.0f, 1.0f ) );
        sceGuClearDepth(0);


}

void blitSprite(Image *img, float x, float y, double angle, int mode, int hilite, float fixvar, float anglevar) {
	float xangle = 0.0;
	float w = (float)(img->imageWidth)/512.0f*DIST; //trying to fix gapsbetween tiles
	float h = (float)(img->imageHeight/2)/512.0f*DIST;	
	if (x<0 || x >512)
		return;
	if (y<0 || y >512)
		return;


	float z=ZNORM;

	if (mode==0) {
		angle=PI;
	} else if (mode==2) {
		z=Z;
		w/=1.5;
		h/=1.5;
		xangle=ROTX;	
	}

	z*=DIST;
	if (hilite) {
		w*=1.5;
		h*=1.5;
		z+=0.000001;
	}


	verts[0].x=-w;
	verts[0].y=h;

	verts[1].x=w;
	verts[1].y=h;

	verts[2].x=-w;
	verts[2].y=-h;

	verts[3].x=w;
	verts[3].y=-h;

	sceKernelDcacheWritebackInvalidateAll();
	//guStart();
       	sceGuTexScale( 1.0f, 1.0f );
	if (!hilite)
        	sceGuTexImage( 0, img->textureWidth, img->textureHeight/2, img->textureWidth, img->data );
	else
        	sceGuTexImage( 0, img->textureWidth, img->textureHeight/2, img->textureWidth, &(img->data[(int)(img->textureHeight/2* img->textureWidth)]) );

        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        ScePspFVector3 move1 = { 0.0f, -0.0f, z+fixvar };
        sceGumTranslate( &move1 );
       	sceGumRotateX(xangle+anglevar);
        sceGumRotateZ(angle+PI);
        ScePspFVector3 move = { DIST*(x/256-1), DIST*(1-y/256), 0.0 };
        sceGumTranslate( &move );
        sceGumRotateZ(-angle);
	sceGumDrawArray( GU_TRIANGLE_STRIP,  GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 4, 0, verts );
	 //guFinish();

}



void blitMapTile(Image *img, float x, float y, double angle, int mode, float fixvar, float anglevar)  {
	if (img==NULL || img->data==NULL || img->textureWidth!=256 || img->textureHeight!=256)
		return;
	float z=ZNORM;
	float xangle = 0.0;

	if (mode==0) {
		angle=PI;
	} else if (mode==2) {
		z=Z;
		xangle = ROTX;
	}
	float chunks=16;
	float w=(.5+0.00001)*DIST;
	float h=(0.5/chunks+0.00001)*DIST;
	z*=DIST;

	verts[0].x=-w;
	verts[0].y=h;

	verts[1].x=w;
	verts[1].y=h;

	verts[2].x=-w;
	verts[2].y=-h;

	verts[3].x=w;
	verts[3].y=-h;

	sceKernelDcacheWritebackInvalidateAll();
	guStart();
	if (mode==2 && fixvar>0) {
        	sceGumMatrixMode(GU_PROJECTION);
        	sceGumLoadIdentity();
        	sceGumPerspective( 75.0f, 16.4f/9.0f, NEARS-fixvar/2.1, FARS);
	}
	sceGuTexWrap(GU_CLAMP,GU_CLAMP);
        //sceGuTexScale( 1.0f, 1.0f );
        sceGuTexScale( 1.0000001f, 1.0000001f );
	sceGuTexOffset(0,0);

	int i;
	for (i=0; i<chunks; i++) {
        sceGuTexImage( 0, img->textureWidth, img->textureHeight/chunks, img->textureWidth, &(img->data[(int)(i*img->textureHeight/chunks* img->textureWidth)]) );
	sceKernelDcacheWritebackAll();

        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        ScePspFVector3 move1 = { 0.0f, 0.0, z+fixvar };
        sceGumTranslate( &move1 );
       	sceGumRotateX(xangle+anglevar);
        sceGumRotateZ(angle);
	
        ScePspFVector3 move = { DIST*(1-x/256.0f), DIST*((y-img->textureHeight/2*(1-1/chunks)+i*img->textureHeight/chunks)/256.0f-1), 0 };
        sceGumTranslate( &move );

	sceGumDrawArray( GU_TRIANGLE_STRIP,  GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 4, 0, verts );
	}
	guFinish();
}





void blitSprite2(Image *img, float x, float y, double angle, int mode) {
	x=x+img->imageWidth/2;
	y=y+img->imageHeight/2;
	verts[0].x=1.0f-x/256;
	verts[0].y=y/256-1;

	verts[1].x=1.0f-x/256+(float)img->imageWidth/256;
	verts[1].y=y/256-1;

	verts[2].x=1.0f-x/256;
	verts[2].y=y/256-(float)img->imageHeight/256-1;

	verts[3].x=1.0f-x/256+(float)img->imageWidth/256;
	verts[3].y=y/256-(float)img->imageHeight/256-1;

	verts[0].u=1;
	verts[1].u=0;
	verts[2].u=1;
	verts[3].u=0;
	verts[0].v=1;
	verts[1].v=1;
	verts[2].v=0;
	verts[3].v=0;

	sceKernelDcacheWritebackInvalidateAll();
	guStart();
        sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadIdentity();
        sceGumPerspective( 75.0f, 16.4f/9.0f, NEARS, FARS);

        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();

        sceGuClearColor( GU_COLOR( 0.0f, 0.0f, 0.0f, 1.0f ) );
        sceGuClearDepth(0);

        sceGuTexScale( 1.0f, 1.0f );
         sceGuTexImage( 0, img->textureWidth, img->textureHeight, img->textureWidth, img->data );
	sceKernelDcacheWritebackAll();

	
        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        ScePspFVector3 move1 = { 0.0f, -0.0f, -0.69f };
        sceGumTranslate( &move1 );
        sceGumRotateZ(angle);

	sceGumDrawArray( GU_TRIANGLE_STRIP,  GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D, 4, 0, verts );
	 guFinish();
}



Image* loadImage1(const char* filename, int negate)
{
	if (isJpegFile(filename)) {
		return loadJpegImage1(filename,negate);
	} else {
		return loadPngImage1(filename,negate);
	}
}

Image* loadImage(const char* filename)
{
	if (isJpegFile(filename)) {
		return loadJpegImage(filename);
	} else {
		return loadPngImage(filename);
	}
}

Image* loadPngImageImpl(png_structp png_ptr, int negate)
{
unsigned int sig_read = 0;
png_uint_32 width, height, x, y;
int bit_depth, color_type, interlace_type;
u32* line;
png_infop info_ptr;

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		ms_write_log("Failed to create info_struct in loadPngImageImpl\n");		// MIB.42_2 to log
		return NULL;
	}
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	if (width > 512 || height > 512) {
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		ms_write_log("Width=%d or Height=%d is bigger than 512 in loadPngImageImpl\n",width,height);		// MIB.42_2 to log
		return NULL;
	}
	Image* image = (Image*) malloc(sizeof(Image));
	if(image==NULL)
	{
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		ms_write_log("Failed to allocate %d bytes for image in loadPngImageImpl\n",sizeof(Image));		// MIB.42_2 to log
		return NULL;	
	}
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	image->data = (Color*) memalign(16, image->textureWidth * image->textureHeight * sizeof(Color));
	if(image->data==NULL)
	{
		free(image);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		ms_write_log("Failed to allocate %d bytes for image->data in loadPngImageImpl\n",(image->textureWidth * image->textureHeight * sizeof(Color)));		// MIB.42_2 to log
		return NULL;
	}
	line = (u32*) malloc(width * 4);
	if(line==NULL)
	{
		free(image->data);
		free(image);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		ms_write_log("Failed to allocate %d bytes for line in loadPngImageImpl\n",(width*4));		// MIB.42_2 to log
		return NULL;
	}
	for (y = 0; y < height; y++) {
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0; x < width; x++) {
			u32 color = line[x];
			if (negate) {
				int a=A(color);
				int r=255-R(color); if (r<0) r=0;
				int g=255-G(color); if (g<0) g=0;
				int b=255-B(color); if (b<0) b=0;
				image->data[x + y * image->textureWidth] =  ARGB8888(a,r,g,b);
			} else
				image->data[x + y * image->textureWidth] =  color;
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	return image;
}

Image* loadPngImage1(const char* filename,int negate)
{
	png_structp png_ptr;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		ms_write_log("Failed to open file '%s' in loadPngImage\n",filename);		// MIB.42_2 to log
		return NULL;
	}
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		fclose(fp);
		ms_write_log("Failed to create read_structure in loadPngImage\n",filename);		// MIB.42_2 to log
		return NULL;
	}
	png_init_io(png_ptr, fp);
	Image* image = loadPngImageImpl(png_ptr,negate);
	fclose(fp);
return image;
}


Image* loadPngImage(const char* filename)
{
	png_structp png_ptr;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		ms_write_log("Failed to open file '%s' in loadPngImage\n",filename);		// MIB.42_2 to log
		return NULL;
	}
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		fclose(fp);
		ms_write_log("Failed to create read_structure in loadPngImage\n",filename);		// MIB.42_2 to log
		return NULL;
	}
	png_init_io(png_ptr, fp);
	Image* image = loadPngImageImpl(png_ptr,0);
	fclose(fp);
return image;
}

Image* loadJpegImageImpl(struct jpeg_decompress_struct dinfo, int negate)
{
	jpeg_read_header(&dinfo, TRUE);
	int width = dinfo.image_width;
	int height = dinfo.image_height;
	jpeg_start_decompress(&dinfo);
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) {
		jpeg_destroy_decompress(&dinfo);
		ms_write_log("Failed to allocate %d bytes for image in loadJpegImageImpl\n",sizeof(Image));		// MIB.42_2 to log
		return NULL;
	}
	if (width > 512 || height > 512)
	{
		jpeg_destroy_decompress(&dinfo);
		ms_write_log("Width=%d or Height=%d is bigger than 512 in loadJpegImageImpl\n",width,height);		// MIB.42_2 to log
		return NULL;
	}
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	image->data = (Color*) memalign(16, image->textureWidth * image->textureHeight * sizeof(Color));
	if(image->data==NULL)	// MIB.42_2 extra checks
	{
		jpeg_destroy_decompress(&dinfo);
		free(image);
		ms_write_log("Failed to allocate %d bytes for image->data in loadJpegImageImpl\n",(image->textureWidth * image->textureHeight * sizeof(Color)));		// MIB.42_2 to log
		return NULL;
	}
	u8* line = (u8*) malloc(width * 3);
	if(line==NULL)	// MIB.42_2 extra checks
	{
		free(image->data);
		free(image);
		jpeg_destroy_decompress(&dinfo);
		ms_write_log("Failed to allocate %d bytes for line in loadJpegImageImpl\n",(width*3));		// MIB.42_2 to log
		return NULL;
	}
	if (dinfo.jpeg_color_space == JCS_GRAYSCALE) {
		while (dinfo.output_scanline < dinfo.output_height) {
			int y = dinfo.output_scanline;
			jpeg_read_scanlines(&dinfo, &line, 1);
			int x;
			for ( x = 0; x < width; x++) {
				Color c = line[x];
				image->data[x + image->textureWidth * y] = c | (c << 8) | (c << 16) | 0xff000000;;
			}
		}
	} else {
		while (dinfo.output_scanline < dinfo.output_height) {
			int y = dinfo.output_scanline;
			jpeg_read_scanlines(&dinfo, &line, 1);
			u8* linePointer = line;
			int x;
			for ( x = 0; x < width; x++) {
				Color c = *(linePointer++);
				c |= (*(linePointer++)) << 8;
				c |= (*(linePointer++)) << 16;
				if (negate) {
					int a=A(c);
					int r=255-R(c); if (r<0) r=0;
					int g=255-G(c); if (g<0) g=0;
					int b=255-B(c); if (b<0) b=0;
					image->data[x + y * image->textureWidth] =  ARGB8888(a,r,g,b)|0xff000000;
				} else 
					image->data[x + image->textureWidth * y] = c | 0xff000000;
			}
		}
	}
	jpeg_finish_decompress(&dinfo);
	jpeg_destroy_decompress(&dinfo);
	free(line);
return image;
}

Image* loadJpegImage1(const char* filename,int negate)
{
	struct jpeg_decompress_struct dinfo;
	struct jpeg_error_mgr jerr;
	dinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&dinfo);
	FILE* inFile = fopen(filename, "rb");
	if (!inFile) {
		ms_write_log("Failed to open file '%s' in loadJpegImage\n",filename);		// MIB.42_2 to log
		jpeg_destroy_decompress(&dinfo);
		return NULL;
	}
	jpeg_stdio_src(&dinfo, inFile);
	Image* image = loadJpegImageImpl(dinfo,negate);
	fclose(inFile);
	return image;
}

Image* loadJpegImage(const char* filename)
{
	struct jpeg_decompress_struct dinfo;
	struct jpeg_error_mgr jerr;
	dinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&dinfo);
	FILE* inFile = fopen(filename, "rb");
	if (!inFile) {
		ms_write_log("Failed to open file '%s' in loadJpegImage\n",filename);		// MIB.42_2 to log
		jpeg_destroy_decompress(&dinfo);
		return NULL;
	}
	jpeg_stdio_src(&dinfo, inFile);
	Image* image = loadJpegImageImpl(dinfo,0);
	fclose(inFile);
	return image;
}


// code for jpeg memory source
typedef struct {
	struct jpeg_source_mgr pub;	/* public fields */

	const unsigned char* membuff;	/* The input buffer */
	int location;			/* Current location in buffer */ 
	int membufflength;            /* The length of the input buffer */
	JOCTET * buffer;		/* start of buffer */
	boolean start_of_buff;	/* have we gotten any data yet? */
} mem_source_mgr;

typedef mem_source_mgr* mem_src_ptr;

#define INPUT_BUF_SIZE  4096	/* choose an efficiently fread'able size */


METHODDEF(void) mem_init_source (j_decompress_ptr cinfo) {
	mem_src_ptr src;

	src = (mem_src_ptr) cinfo->src;

	/* We reset the empty-input-file flag for each image,
	* but we don't clear the input buffer.
	* This is correct behavior for reading a series of images from one source.
	*/
	src->location = 0;
	src->start_of_buff = TRUE;
}


METHODDEF(boolean) mem_fill_input_buffer (j_decompress_ptr cinfo) {
mem_src_ptr src;
size_t bytes_to_read;
size_t nbytes;

	src = (mem_src_ptr) cinfo->src;

	if((src->location)+INPUT_BUF_SIZE >= src->membufflength)
		bytes_to_read = src->membufflength - src->location;
	else
		bytes_to_read = INPUT_BUF_SIZE;

	memcpy(src->buffer, (src->membuff)+(src->location), bytes_to_read);
	nbytes = bytes_to_read;
	src->location += (int) bytes_to_read;

	if (nbytes <= 0) {
		if (src->start_of_buff)	/* Treat empty input file as fatal error */
			ERREXIT(cinfo, JERR_INPUT_EMPTY);
		WARNMS(cinfo, JWRN_JPEG_EOF);
		/* Insert a fake EOI marker */
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_buff = FALSE;

return TRUE;
}


METHODDEF(void) mem_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
mem_src_ptr src;

	src = (mem_src_ptr) cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long) src->pub.bytes_in_buffer) {
			num_bytes -= (long) src->pub.bytes_in_buffer;
			mem_fill_input_buffer(cinfo);
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}


METHODDEF(void) mem_term_source (j_decompress_ptr cinfo) {
}


GLOBAL(void) jpeg_mem_src (j_decompress_ptr cinfo, const unsigned char *mbuff, int mbufflen) {
	mem_src_ptr src;

	if (cinfo->src == NULL) {	/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			sizeof(mem_source_mgr));
		src = (mem_src_ptr) cinfo->src;
		src->buffer = (JOCTET *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
			INPUT_BUF_SIZE * sizeof(JOCTET));
	}

	src = (mem_src_ptr) cinfo->src;
	src->pub.init_source = mem_init_source;
	src->pub.fill_input_buffer = mem_fill_input_buffer;
	src->pub.skip_input_data = mem_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart;
	src->pub.term_source = mem_term_source;
	src->membuff = mbuff;
	src->membufflength = mbufflen;
	src->pub.bytes_in_buffer = 0;    /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
}

typedef struct {
	const unsigned char *data;
	png_size_t size;
	png_size_t seek;
} PngData;
	
static void ReadPngData(png_structp png_ptr, png_bytep data, png_size_t length)
{
PngData *pngData = (PngData*) png_get_io_ptr(png_ptr);
	if (pngData) {
		png_size_t i;
		for (i = 0; i < length; i++) {
			if (pngData->seek >= pngData->size) break;
			data[i] = pngData->data[pngData->seek++];
		}
	}
}

Image* loadImageFromMemory(const unsigned char* data, int len)
{
	if (len<8)
	{
		ms_write_log("Not enough data for loadImageFromMemory(%d)\n",len);		// MIB.42_2 to log
		return NULL;
	}
	
	// test for PNG
	if (data[0] == 137 && data[1] == 80 && data[2] == 78 && data[3] == 71
		&& data[4] == 13 && data[5] == 10 && data[6] == 26 && data[7] ==10)
	{
		png_structp png_ptr;
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL)
		{
			ms_write_log("Failed to read png image in loadImageFromMemory(%d)\n",len);		// MIB.42_2 to log
			return NULL;
		}
		PngData pngData;
		pngData.data = data;
		pngData.size = len;
		pngData.seek = 0;
		png_set_read_fn(png_ptr, (void *) &pngData, ReadPngData);
		Image* image = loadPngImageImpl(png_ptr,config.nightmode);
		if(image == NULL)	// MIB.42_2 extra check to log
		{
			ms_write_log("Read png image returned NULL in loadImageFromMemory(%d)\n",len);		// MIB.42_2 to log
		}
		return image;
	} else {
		// assume JPG
		struct jpeg_decompress_struct dinfo;
		struct jpeg_error_mgr jerr;
		dinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&dinfo);
		jpeg_mem_src(&dinfo, data, len);
		Image* image = loadJpegImageImpl(dinfo, config.nightmode);
		if(image == NULL)	// MIB.42_2 extra check to lg
		{
			ms_write_log("Read jpg image returned NULL in loadImageFromMemory(%d)\n",len);		// MIB.42_2 to log
		}
		return image;
	}
}

void blitImageToImage2(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination)
{

	if (dx<(-width) || dy<(-height))
		return;

	if (dx<0) {
		sx+= -dx;
		width=width+dx;
		dx=0;
	}
	if (dy<0) {
		sy+= -dy;
		height=height+dy;
		dy=0;
	}
	if (dx>destination->textureWidth || dy>destination->textureHeight)
		return;	
	if (dx+width>destination->textureWidth)
		width=destination->textureWidth-dx;


 	if (dy+height>destination->textureHeight)
		height=destination->textureHeight-dy;

	Color* destinationData = &destination->data[destination->textureWidth * dy + dx];
	int destinationSkipX = destination->textureWidth - width;
	Color* sourceData = &source->data[source->textureWidth * sy + sx];
	int sourceSkipX = source->textureWidth - width;
	int x, y;
	for (y = 0; y < height; y++, destinationData += destinationSkipX, sourceData += sourceSkipX*2) {
		for (x = 0; x < width; x++, destinationData++, sourceData+=2) {
			Color color = *sourceData;
                        if (!IS_ALPHA(color))
				*destinationData = *sourceData;
		}
	}
}


void blitImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination)
{

	if (dx<(-width) || dy<(-height))
		return;

	if (dx<0) {
		sx+= -dx;
		width=width+dx;
		dx=0;
	}
	if (dy<0) {
		sy+= -dy;
		height=height+dy;
		dy=0;
	}
	if (dx>destination->textureWidth || dy>destination->textureHeight)
		return;	
	if (dx+width>destination->textureWidth)
		width=destination->textureWidth-dx;


 	if (dy+height>destination->textureHeight)
		height=destination->textureHeight-dy;

	Color* destinationData = &destination->data[destination->textureWidth * dy + dx];
	int destinationSkipX = destination->textureWidth - width;
	Color* sourceData = &source->data[source->textureWidth * sy + sx];
	int sourceSkipX = source->textureWidth - width;
	int x, y;
	for (y = 0; y < height; y++, destinationData += destinationSkipX, sourceData += sourceSkipX) {
		for (x = 0; x < width; x++, destinationData++, sourceData++) {
			Color color = *sourceData;
                        if (!IS_ALPHA(color))
				*destinationData = *sourceData;
		}
	}
}



void blitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized) return;
	Color* vram = getVramDrawBuffer();
	sceKernelDcacheWritebackInvalidateAll();
	guStart();
	//sceKernelDcacheWritebackAll();
	sceGuCopyImage(GU_PSM_8888, sx, sy, width, height, source->textureWidth, source->data, dx, dy, PSP_LINE_SIZE, vram);
	 guFinish();
}
void blitAlphaImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination)
{
	Color* destinationData = &destination->data[destination->textureWidth * dy + dx];
	int destinationSkipX = destination->textureWidth - width;
	Color* sourceData = &source->data[source->textureWidth * sy + sx];
	int sourceSkipX = source->textureWidth - width;
	int x, y;
	s32 rcolorc, gcolorc, bcolorc, acolorc,rcolord, gcolord, bcolord, acolord;
	for (y = 0; y < height; y++, destinationData += destinationSkipX, sourceData += sourceSkipX) {
		for (x = 0; x < width; x++, destinationData++, sourceData++) {
			Color color = *sourceData;
			if (!IS_ALPHA(color)) {
				*destinationData = color;
			} else {
				rcolorc = color & 0xff;
				gcolorc = (color >> 8) & 0xff;
				bcolorc = (color >> 16) & 0xff;
				acolorc = (color >> 24) & 0xff;
				rcolord = *destinationData & 0xff;
				gcolord = (*destinationData >> 8) & 0xff;
				bcolord = (*destinationData >> 16) & 0xff;
				acolord = (*destinationData >> 24) & 0xff;
				
				rcolorc = ((acolorc*rcolorc)>>8) + (((255-acolorc) * rcolord)>>8);
				if (rcolorc > 255) rcolorc = 255;
				gcolorc = ((acolorc*gcolorc)>>8) + (((255-acolorc) * gcolord)>>8);
				if (gcolorc > 255) gcolorc = 255;
				bcolorc = ((acolorc*bcolorc)>>8) + (((255-acolorc) * bcolord)>>8);
				if (bcolorc > 255) bcolorc = 255;
				if (acolord + acolorc < 255) {
					acolorc = acolord+acolorc;
				} else {
					acolorc = 255;
				}
				*destinationData = rcolorc | (gcolorc << 8) | (bcolorc << 16) | (acolorc << 24);
			}
		}
	}
}

void blitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized) return;

	sceKernelDcacheWritebackInvalidateAll();
	guStart();

	sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, (void*) source->data);
	float u = 1.0f / ((float)source->textureWidth);
	float v = 1.0f / ((float)source->textureHeight);
	sceGuTexScale(u, v);
	int j = 0;
	while (j < width) {
		Vertex* vertices = (Vertex*) sceGuGetMemory(2 * sizeof(Vertex));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 0;
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
	 guFinish();	
}


void swizzle_fast(u8* out, const u8* in, unsigned int width, unsigned int height)
{
   unsigned int blockx, blocky;
   unsigned int j;
 
   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);
 
   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;
 
   const u8* ysrc = in;
   u32* dst = (u32*)out;
 
   for (blocky = 0; blocky < height_blocks; ++blocky)
   {
      const u8* xsrc = ysrc;
      for (blockx = 0; blockx < width_blocks; ++blockx)
      {
         const u32* src = (u32*)xsrc;
         for (j = 0; j < 8; ++j)
         {
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            src += src_pitch;
         }
         xsrc += 16;
     }
     ysrc += src_row;
   }
}

Image* createImage(int width, int height)
{
	Image* image = (Image*) malloc(sizeof(Image));

	if(image==NULL)	// MIB.42_2 extra check to log
	{
		ms_write_log("Failed to allocate %d bytes in createImage(%d,%d)\n",sizeof(Image),width,height);		// MIB.42_2 to log
		return NULL;
	}
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	image->data = (Color*) memalign(16, image->textureWidth * image->textureHeight * sizeof(Color));
	if(!image->data)	// MIB.42_2 extra check to log
	{
		ms_write_log("Failed to allocate %d bytes for data in createImage(%d,%d)\n",(image->textureWidth * image->textureHeight * sizeof(Color)),width,height);		// MIB.42_2 to log
		free(image);
		return NULL;
	}
	memset(image->data, 0, image->textureWidth * image->textureHeight * sizeof(Color));
return image;
}

void swizzleImage( Image* img )
{
	if (img==NULL)
		return;
      long size = img->textureWidth * img->textureHeight * 4;
      u8* temp = (u8*)malloc(size);
	if (temp!=NULL) {
      		swizzle_fast( temp, (u8*)img->data, (img->textureWidth * 4), img->textureHeight );
      
        	free(img->data);
        	img->data = (u32*)temp;
	 } else  {
        	free(img->data);
        	free(img);
		fprintf(stdout,"Failed to allocate %ld bytes for data in swizzleImage\n",size);
		img=NULL;
	}
} 


void freeImage(Image* image)
{
	if(image!=NULL)		// MIB.42_2 extra check!
	{
		//fprintf(stdout,"Free image\n");
		free(image->data);
		free(image);
	}
}

void clearImage(Color color, Image* image)
{
	int i;
	int size = image->textureWidth * image->textureHeight;
	Color* data = image->data;
	for (i = 0; i < size; i++, data++) *data = color;
}

void clearScreen(Color color)
{
	if (!initialized) return;
	guStart();
	sceGuClearColor(color);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	guFinish();
}

void fillImageRect(Color color, int x0, int y0, int width, int height, Image* image)
{
	int skipX;
	int x, y;

	if(image==NULL)	// MIB.42_2 extra check
	{
		ms_write_log("fillImageRect has NULL as image...\n");
		return;
	}
	skipX = image->textureWidth - width;
	Color* data = image->data + x0 + y0 * image->textureWidth;
	for (y = 0; y < height; y++, data += skipX) {
		for (x = 0; x < width; x++, data++) *data = color;
	}
}

void fillScreenRect(Color color, int x0, int y0, int width, int height)
{
	if (!initialized) return;
	int skipX = PSP_LINE_SIZE - width;
	int x, y;
	Color* data = getVramDrawBuffer() + x0 + y0 * PSP_LINE_SIZE;
	for (y = 0; y < height; y++, data += skipX) {
		for (x = 0; x < width; x++, data++) *data = color;
	}
}

void putPixelScreen(Color color, int x, int y)
{
	Color* vram = getVramDrawBuffer();
	vram[PSP_LINE_SIZE * y + x] = color;
}

void putPixelImage(Color color, int x, int y, Image* image)
{
	image->data[x + y * image->textureWidth] = color;
}

void nightMode(){
	int xx,yy;
	Color pixel;
	for ( xx=0; xx<480; xx++) {
		for ( yy=0; yy<272; yy++) {
			pixel=getPixelScreen(xx,yy);
			pixel/=2;
			putPixelScreen(pixel,xx,yy);
		}
	}
}

Color getPixelScreen(int x, int y)
{
	Color* vram = getVramDrawBuffer();
	return vram[PSP_LINE_SIZE * y + x];
}

Color getPixelImage(int x, int y, Image* image)
{
	return image->data[x + y * image->textureWidth];
}


void printTextScreen(int x, int y, const char* text, u32 color)
{
	int i, j, l;
	u8 *font;
	Color *vram_ptr;
	Color *vram;
// MIB.42_4
u8 degsgn[8] = {96,144,144,96,0,0,0,0};
	//fprintf(stdout,"{%s}\n", text);	
	if (!initialized) return;
	size_t c;
	for ( c = 0; c < strlen(text); c++) {
		if (x < 0 || x + 8 > SCREEN_WIDTH || y < 0 || y + 8 > SCREEN_HEIGHT) break;
		u8 ch = text[c];
		unsigned int newch = text[c];
		vram = getVramDrawBuffer() + x + y * PSP_LINE_SIZE;
// MIB.42_4 - changed the string rendering code to match the 0xB0 ASCII for unified input/output
		if ((newch<128 && newch>=32) || ch==(u8)'\260') {	
		if(ch!=(u8)'\260')
			font = &msx[ (int)ch * 8];
		else
			font = degsgn;
		for (i = l = 0; i < 8; i++, l += 8, font++) {
			vram_ptr  = vram;
			for (j = 0; j < 8; j++) {
				if ((*font & (128 >> j))) *vram_ptr = color;
				vram_ptr++;
			}
			vram += PSP_LINE_SIZE;
		}
		}
		x += 8;
	}
}

void printTextScreen1(int x, int y, const char* text, u32 color, Image * fontimg)
{
	int i, j, l;
	u8 *font;
	Color *vram_ptr;
	Color *vram;
// MIB.42_4
u8 degsgn[8] = {96,144,144,96,0,0,0,0};
	
	if (!initialized) return;
	size_t c;
	for ( c = 0; c < strlen(text); c++) {
		if (x < 0 || x + 8 > SCREEN_WIDTH || y < 0 || y + 8 > SCREEN_HEIGHT) break;
		u8 ch = text[c];
		unsigned int newch = text[c];
		vram = getVramDrawBuffer() + x + y * PSP_LINE_SIZE;
// MIB.42_4 - changed the string rendering code to match the 0xB0 ASCII for unified input/output
		if ((newch<128 && newch>=32) || ch==(u8)'\260') {	
		if(ch!=(u8)'\260')
			font = &msx[ (int)ch * 8];
		else
			font = degsgn;
		for (i = l = 0; i < 8; i++, l += 8, font++) {
			vram_ptr  = vram;
			for (j = 0; j < 8; j++) {
				if ((*font & (128 >> j))) *vram_ptr = color;
				vram_ptr++;
			}
			vram += PSP_LINE_SIZE;
		}
		}else{
			draw_local_char_small(newch,x,y);
		}
		x += 8;
	}
}

void printTextImage(int x, int y, const char* text, u32 color, Image* image)
{
	int i, j, l;
	u8 *font;
	Color *data_ptr;
	Color *data;
	
	if (!initialized) return;
	size_t c;
	for ( c = 0; c < strlen(text); c++) {
		if (x < 0 || x + 8 > image->imageWidth || y < 0 || y + 8 > image->imageHeight) break;
		char ch = text[c];
		data = image->data + x + y * image->textureWidth;
		
		font = &msx[ (int)ch * 8];
		for (i = l = 0; i < 8; i++, l += 8, font++) {
			data_ptr  = data;
			for (j = 0; j < 8; j++) {
				if ((*font & (128 >> j))) *data_ptr = color;
				data_ptr++;
			}
			data += image->textureWidth;
		}
		x += 8;
	}
}
/*
static void fontPrintTextImpl(FT_Bitmap* bitmap, int xofs, int yofs, Color color, Color* framebuffer, int width, int height, int lineSize)
{
	u8 rf = color & 0xff; 
	u8 gf = (color >> 8) & 0xff;
	u8 bf = (color >> 16) & 0xff;
	u8 af = (color >> 24) & 0xff;
	
	u8* line = bitmap->buffer;
	Color* fbLine = framebuffer + xofs + yofs * lineSize;
	for (int y = 0; y < bitmap->rows; y++) {
		u8* column = line;
		Color* fbColumn = fbLine;
		for (int x = 0; x < bitmap->width; x++) {
			if (x + xofs < width && x + xofs >= 0 && y + yofs < height && y + yofs >= 0) {
				u8 val = *column;
				color = *fbColumn;
				u8 r = color & 0xff; 
				u8 g = (color >> 8) & 0xff;
				u8 b = (color >> 16) & 0xff;
				u8 a = (color >> 24) & 0xff;
				r = rf * val / 255 + (255 - val) * r / 255;
				g = gf * val / 255 + (255 - val) * g / 255;
				b = bf * val / 255 + (255 - val) * b / 255;
				a = af * val / 255 + (255 - val) * a / 255;
				*fbColumn = r | (g << 8) | (b << 16) | (a << 24);
			}
			column++;
			fbColumn++;
		}
		line += bitmap->pitch;
		fbLine += lineSize;
	}
}

void fontPrintTextImage(FT_Bitmap* bitmap, int x, int y, Color color, Image* image)
{
	fontPrintTextImpl(bitmap, x, y, color, image->data, image->imageWidth, image->imageHeight, image->textureWidth);
}

void fontPrintTextScreen(FT_Bitmap* bitmap, int x, int y, Color color)
{
	fontPrintTextImpl(bitmap, x, y, color, getVramDrawBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT, PSP_LINE_SIZE);
}

*/
void saveImage(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha)
{
	if (isJpegFile(filename)) {
		saveJpegImage(filename, data, width, height, lineSize);
	} else {
		savePngImage(filename, data, width, height, lineSize, saveAlpha);
	}
}

void savePngImage(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha)
{
	png_structp png_ptr;
	png_infop info_ptr;
	FILE* fp;
	int i, x, y;
	u8* line;
	
	if ((fp = fopen(filename, "wb")) == NULL)		// MIB.42_2
	{
		ms_write_log("Can not create '%s' to save png image\n",filename);
		return;
	}
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)		// MIB.42_2
	{
		ms_write_log("Can not create png_write_struct to save '%s' png image\n",filename);
		return;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)		// MIB.42_2
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		ms_write_log("Can not create png_info_struct to save '%s' png image\n",filename);
		return;
	}
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8,
		saveAlpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	line = (u8*) malloc(width * (saveAlpha ? 4 : 3));
	if(line==NULL)		// MIB.42_2
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);		
		ms_write_log("Can not create line buffer with size of %d to save '%s' png image\n",(width * (saveAlpha ? 4 : 3)),filename);
		return;
	}
	for (y = 0; y < height; y++) {
		for (i = 0, x = 0; x < width; x++) {
			Color color = data[x + y * lineSize];
			u8 r = color & 0xff; 
			u8 g = (color >> 8) & 0xff;
			u8 b = (color >> 16) & 0xff;
			u8 a = saveAlpha ? (color >> 24) & 0xff : 0xff;
			line[i++] = r;
			line[i++] = g;
			line[i++] = b;
			if (saveAlpha) line[i++] = a;
		}
		png_write_row(png_ptr, line);
	}
	free(line);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	fclose(fp);
}

void saveJpegImage(const char* filename, Color* data, int width, int height, int lineSize)
{
	FILE* outFile = fopen(filename, "wb");
	if (!outFile)			// MIB.42_2
	{
		ms_write_log("Can not create '%s' jpg file.\n",filename);
		return;
	}
	struct jpeg_error_mgr jerr;
	struct jpeg_compress_struct cinfo;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outFile);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 100, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	u8* row = (u8*) malloc(width * 3);
	if(row==NULL)
	{
		ms_write_log("Can not allocate row buffer with size of %d to save '%s' jpg image\n",(width * 3),filename);
		return;
	}
	int y;
	for ( y = 0; y < height; y++) {
		u8* rowPointer = row;
		int x;
		for ( x = 0; x < width; x++) {
			Color c = data[x + cinfo.next_scanline * lineSize];
			*(rowPointer++) = c & 0xff;
			*(rowPointer++) = (c >> 8) & 0xff;
			*(rowPointer++) = (c >> 16) & 0xff;
		}
		jpeg_write_scanlines(&cinfo, &row, 1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(outFile);
	free(row);
}


void flipScreen()
{
	if (!initialized) return;
	//sceGuFinish();
        //sceGuSync(0,0);
	sceGuSwapBuffers();
	dispBufferNumber ^= 1;
	//sceGuStart(GU_DIRECT, dList);
}



// New line drawings with proper clipping
static void drawLine1(int x0, int y0, int x1, int y1, int color, Color* destination, int width, int thick)
{
int tmp0,tmp1,tmpP,x=0,y=0;

	tmp0=IS_INSIDE;			// determine where x0:y0 is compared to screen
	if(y0>=(width-1)) tmp0|=IS_TOP;
	else if(y0<1) tmp0|=IS_BOTTOM;
	if(x0>=(width-1)) tmp0|=IS_RIGHT;
	else if(x0<1) tmp0|=IS_LEFT;
	tmp1=IS_INSIDE;			// determine where x1:y1 is compared to screen
	if(y1>=(width+1)) tmp1|=IS_TOP;
	else if(y1<1) tmp1|=IS_BOTTOM;
	if(x1>=(width-1)) tmp1|=IS_RIGHT;
	else if(x1<1) tmp1|=IS_LEFT;

	while(tmp0|tmp1)		// We need to keep clipping until we get inside the screen
	{
		if(tmp0&tmp1) return;		// not intersecting the screen, so no need to render
// Ok, we need to clip...
		if(tmp0) tmpP=tmp0; else tmpP=tmp1;	// x0:y0 point is not on screen
		if(tmpP&IS_TOP)				// Does it need top clipping?
		{
			if(y1!=y0)
				x=x0+(x1-x0)*((width-1)-y0)/(y1-y0);
			else
				x=x0+(x1-x0)*((width-1)-y0);
			y=width-2;
		}
		else
			if(tmpP&IS_BOTTOM)		// Does it need bottom clipping?
			{
				if(y1!=y0)
					x=x0+(x1-x0)*(1-y0)/(y1-y0);
				else
					x=x0+(x1-x0)*(1-y0);
				y=1;
			}
			else
				if(tmpP&IS_RIGHT)		// Does it need to be clip'd on the right?
				{
					if(x1!=x0)
						y=y0+(y1-y0)*((width-1)-x0)/(x1-x0);
					else
						y=y0+(y1-y0)*((width-1)-x0);
					x=width-2;
				}
				else
					if(tmpP&IS_LEFT)	// Does it need to be clip'd on the left?
					{
						if(x1!=x0)
							y=y0+(y1-y0)*(1-x0)/(x1-x0);
						else
							y=y0+(y1-y0)*(1-x0);
						x=1;
					}
		if(tmpP==tmp0)
		{
			x0=x;y0=y;
			tmp0=IS_INSIDE;			// determine where x0:y0 is compared to screen
			if(y0>=(width-1)) tmp0|=IS_TOP;
			else if(y0<1) tmp0|=IS_BOTTOM;
			if(x0>=(width-1)) tmp0|=IS_RIGHT;
			else if(x0<1) tmp0|=IS_LEFT;		
		}
		else
		{
			x1=x;y1=y;
			tmp1=IS_INSIDE;			// determine where x1:y1 is compared to screen
			if(y1>=(width-1)) tmp1|=IS_TOP;
			else if(y1<1) tmp1|=IS_BOTTOM;
			if(x1>=(width-1)) tmp1|=IS_RIGHT;
			else if(x1<1) tmp1|=IS_LEFT;
		}
	}

	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	
	if (dy < 0) { dy = -dy;  stepy = -width; } else { stepy = width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	y0 *= width;
	y1 *= width;
	destination[x0+y0] = color;
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			if ((x0+y0+width+2)<width*width) {
				destination[x0+y0] = color;
				if (thick){
					destination[x0+y0+1] = color;
					destination[x0+y0+width] = color;
					destination[x0+y0+width+1] = color;
				}
			}

		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			if ((x0+y0+width+2)<width*width) {
				destination[x0+y0] = color;
				if (thick) {
					destination[x0+y0+1] = color;
					destination[x0+y0+width] = color;
					destination[x0+y0+width+1] = color;
				}
			}
		}
	}
}

static void drawLine(int x0, int y0, int x1, int y1, int color, Color* destination, int width, int thick)
{
int tmp0,tmp1,tmpP,x=0,y=0;

	tmp0=IS_INSIDE;			// determine where x0:y0 is compared to screen
	if(y0>=(SCR_HEIGHT-1)) tmp0|=IS_TOP;
	else if(y0<1) tmp0|=IS_BOTTOM;
	if(x0>=(SCR_WIDTH-1)) tmp0|=IS_RIGHT;
	else if(x0<1) tmp0|=IS_LEFT;
	tmp1=IS_INSIDE;			// determine where x1:y1 is compared to screen
	if(y1>=(SCR_HEIGHT+1)) tmp1|=IS_TOP;
	else if(y1<1) tmp1|=IS_BOTTOM;
	if(x1>=(SCR_WIDTH-1)) tmp1|=IS_RIGHT;
	else if(x1<1) tmp1|=IS_LEFT;

	while(tmp0|tmp1)		// We need to keep clipping until we get inside the screen
	{
		if(tmp0&tmp1) return;		// not intersecting the screen, so no need to render
// Ok, we need to clip...
		if(tmp0) tmpP=tmp0; else tmpP=tmp1;	// x0:y0 point is not on screen
		if(tmpP&IS_TOP)				// Does it need top clipping?
		{
			if(y1!=y0)
				x=x0+(x1-x0)*((SCREEN_HEIGHT-1)-y0)/(y1-y0);
			else
				x=x0+(x1-x0)*((SCREEN_HEIGHT-1)-y0);
			y=SCREEN_HEIGHT-2;
		}
		else
			if(tmpP&IS_BOTTOM)		// Does it need bottom clipping?
			{
				if(y1!=y0)
					x=x0+(x1-x0)*(1-y0)/(y1-y0);
				else
					x=x0+(x1-x0)*(1-y0);
				y=1;
			}
			else
				if(tmpP&IS_RIGHT)		// Does it need to be clip'd on the right?
				{
					if(x1!=x0)
						y=y0+(y1-y0)*((SCREEN_WIDTH-1)-x0)/(x1-x0);
					else
						y=y0+(y1-y0)*((SCREEN_WIDTH-1)-x0);
					x=SCREEN_WIDTH-2;
				}
				else
					if(tmpP&IS_LEFT)	// Does it need to be clip'd on the left?
					{
						if(x1!=x0)
							y=y0+(y1-y0)*(1-x0)/(x1-x0);
						else
							y=y0+(y1-y0)*(1-x0);
						x=1;
					}
		if(tmpP==tmp0)
		{
			x0=x;y0=y;
			tmp0=IS_INSIDE;			// determine where x0:y0 is compared to screen
			if(y0>=(SCR_HEIGHT-1)) tmp0|=IS_TOP;
			else if(y0<1) tmp0|=IS_BOTTOM;
			if(x0>=(SCR_WIDTH-1)) tmp0|=IS_RIGHT;
			else if(x0<1) tmp0|=IS_LEFT;		
		}
		else
		{
			x1=x;y1=y;
			tmp1=IS_INSIDE;			// determine where x1:y1 is compared to screen
			if(y1>=(SCR_HEIGHT-1)) tmp1|=IS_TOP;
			else if(y1<1) tmp1|=IS_BOTTOM;
			if(x1>=(SCR_WIDTH-1)) tmp1|=IS_RIGHT;
			else if(x1<1) tmp1|=IS_LEFT;
		}
	}

	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	
	if (dy < 0) { dy = -dy;  stepy = -width; } else { stepy = width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	y0 *= width;
	y1 *= width;
	destination[x0+y0] = color;
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			if ((x0+y0+width+2)<width*272) {
				destination[x0+y0] = color;
				if (thick) { 
					destination[x0+y0+1] = color;
					destination[x0+y0+width] = color;
					destination[x0+y0+width+1] = color;
				}
			}
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			if ((x0+y0+width+2)<width*272) {
				destination[x0+y0] = color;
				if (thick) {
					destination[x0+y0+1] = color;
					destination[x0+y0+width] = color;
					destination[x0+y0+width+1] = color;
				}
			}

			
		}
	}
}


void drawLineScreen(int x0, int y0, int x1, int y1, Color color,int thick)
{
	drawLine(x0, y0, x1, y1, color, getVramDrawBuffer(), PSP_LINE_SIZE,thick);
}

void drawLineImage(int x0, int y0, int x1, int y1, Color color, Image* image, int thick)
{
	drawLine1(x0, y0, x1, y1, color, image->data, image->textureWidth, thick);
}

void drawCircleScreen(int x, int y, int radius, Color color) {
        double angle;
        int ox,oy;
	double step=4.0/radius;
	Color *destination=getVramDrawBuffer();
        for (angle=0; angle<6.28; angle+=step) {
                ox=radius*sin(angle)+x;
                oy=radius*cos(angle)+y;
		destination[ox+oy*PSP_LINE_SIZE]=color;
		destination[ox+oy*PSP_LINE_SIZE+1]=color;
        }
}

void drawElipseScreen(int x, int y, int radius, Color color) {
        double angle;
        int ox,oy;
	double step=4.0/radius;
	Color *destination=getVramDrawBuffer();
        for (angle=0; angle<6.28; angle+=step) {
                ox=radius*sin(angle)+x;
                oy=radius/4*cos(angle)+y;
		destination[ox+oy*PSP_LINE_SIZE]=color;
		destination[ox+oy*PSP_LINE_SIZE+1]=color;
        }
}


#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

void initGraphics()
{
	dispBufferNumber = 0;
	//dList = memalign( 16, 1024 );

	sceGuInit();

	guStart();
	sceGuDrawBuffer(GU_PSM_8888, (void*)FRAMEBUFFER_SIZE, PSP_LINE_SIZE);
	sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, (void*)0, PSP_LINE_SIZE);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	sceGuDepthBuffer((void*) (FRAMEBUFFER_SIZE*2), PSP_LINE_SIZE);
	sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);
	sceGuDepthRange( 65535, 0);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	//sceGuAlphaFunc(GU_GREATER, 0, 0xff);
	//sceGuEnable(GU_ALPHA_TEST);

	//sceGuDepthFunc(GU_GEQUAL);
	//sceGuEnable(GU_DEPTH_TEST);

	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	//sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexMode(GU_PSM_8888, 0, 0, GU_TRUE);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter( GU_LINEAR, GU_LINEAR );
	sceGuAmbientColor(0x00000000);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	guFinish();

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	 // setup matrices for the triangle
        //sceGumMatrixMode(GU_PROJECTION);
        //sceGumLoadIdentity();
        //sceGumPerspective( 75.0f, 16.0f/9.0f, 0.1f, 500.0f);


	initialized = 1;
}

void disableGraphics()
{
	initialized = 0;
}

void guStart()
{
	sceGuStart(GU_DIRECT, dList);
}

void guFinish() {
        sceGuFinish();
	sceGuSync(0,0);
}




// MIB.42_3   -  I kept these for backup

#if 0
static void drawLine1(int x0, int y0, int x1, int y1, int color, Color* destination, int width)
{
	if (x0<0 && x1<0) return;
	if (y0<0 && y1<0) return;
	if (x0>=width && x1>=width) return;
	if (y0>=width && y1>=width) return;
	if(destination==NULL) return;

	//if (x0<0 || x1<0 || y0<0 || y1<0)
	//	printf(" %d,%d,%d,%d\n",x0,y0,x1,y1);

	if (x0<0) {
		if (x0==x1) return;
		if((y0=y1 - (x1*(y1-y0)/(x1-x0)))<0) y0=0;		// MIB.42  This is needed as vRam is not tolerant for misses
		x0=0;
	}
	if (x0>=width) {
		if (x0==x1) return;
		if((y0=y1 - ((x1-width+1)*(y1-y0)/(x1-x0)))<0) y0=0;	// MIB.42  This is needed as vRam is not tolerant for misses
		x0=width-1;
	}
	if (y0<0) {
		if (y1==y0) return;
		if((x0=x1 - (y1*(x1-x0)/(y1-y0)))<0) x0=0;		// MIB.42  This is needed as vRam is not tolerant for misses
		y0=0;
	}
	if (y0>=width) {
		if (y1==y0) return;
		if((x0=x1 - ((y1-width+1)*(x1-x0)/(y1-y0)))<0) x0=0;	// MIB.42  This is needed as vRam is not tolerant for misses
		y0=width-1;
	}

	if (x1>=width) {
		if (x0==x1) return;
		if((y1=y0+((width-1-x0)*(y1-y0)/(x1-x0)))<0) y1=0;	// MIB.42  This is needed as vRam is not tolerant for misses
		x1=width-1;
	}
	if (y1>=width) {
		if (y1==y0) return;
		if((x1=x0 + (x1-x0)*(width-1-y0)/(y1-y0))<0) x1=0;	// MIB.42  This is needed as vRam is not tolerant for misses
		y1=width-1;
	}
	if (x1<0) {
		if (x0==x1) return;
		if((y1=y0+((0-x0)*(y1-y0)/(x1-x0)))<0) y1=0;			// MIB.42  This is needed as vRam is not tolerant for misses
		x1=0;
	}
	if (y1<0) {
		if (y1==y0) return;
		if((x1=x0 + (x1-x0)*(0-y0)/(y1-y0))<0) x1=0;		// MIB.42  This is needed as vRam is not tolerant for misses
		y1=0;
	}
	//if (x0<0 || x1<0 || y0<0 || y1<0)
	//printf("! %d,%d,%d,%d\n",x0,y0,x1,y1);

	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	
	if (dy < 0) { dy = -dy;  stepy = -width; } else { stepy = width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	y0 *= width;
	y1 *= width;
	destination[x0+y0] = color;
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			destination[x0+y0] = color;
			destination[x0+y0+1] = color;
			destination[x0+y0+width] = color;
			destination[x0+y0+width+1] = color;
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			destination[x0+y0] = color;
			destination[x0+y0+1] = color;
			destination[x0+y0+width] = color;
			destination[x0+y0+width+1] = color;
		}
	}
}

static void drawLine(int x0, int y0, int x1, int y1, int color, Color* destination, int width)
{
	if (x0<0 && x1<0) return;
	if (y0<0 && y1<0) return;
	if (x0>=SCR_WIDTH && x1>=SCR_WIDTH) return;
	if (y0>=SCR_HEIGHT && y1>=SCR_HEIGHT) return;

	if (x0<0) {
		if (x0==x1) return;
		if((y0=y1 - (x1*(y1-y0)/(x1-x0)))<0) y0=0;			// MIB.42  This is needed as vRam is not tolerant for misses
		x0=0;
	}
	if (x0>=SCR_WIDTH) {
		if (x0==x1) return;
		if((y0=y1 - ((x1-SCR_WIDTH+1)*(y1-y0)/(x1-x0)))<0) y0=0;	// MIB.42  This is needed as vRam is not tolerant for misses
		x0=SCR_WIDTH-1;
	}
	if (y0<0) {
		if (y1==y0) return;
		if((x0=x1 - (y1*(x1-x0)/(y1-y0)))<0) x0=0;			// MIB.42  This is needed as vRam is not tolerant for misses
		y0=0;
	}
	if (y0>=SCR_HEIGHT) {
		if (y1==y0) return;
		if((x0=x1 - ((y1-SCR_HEIGHT+1)*(x1-x0)/(y1-y0)))<0) x0=0;	// MIB.42  This is needed as vRam is not tolerant for misses
		y0=SCR_HEIGHT-1;
	}

	if (x1>=SCR_WIDTH) {
		if (x0==x1) return;
		if((y1=y0+((SCR_WIDTH-1-x0)*(y1-y0)/(x1-x0)))<0) y1=0;		// MIB.42  This is needed as vRam is not tolerant for misses
		x1=SCR_WIDTH-1;
	}
	if (y1>=SCR_HEIGHT) {
		if (y1==y0) return;
		if((x1=x0 + (x1-x0)*(SCR_HEIGHT-1-y0)/(y1-y0))<0) x1=0;		// MIB.42  This is needed as vRam is not tolerant for misses
		y1=SCR_HEIGHT-1;
	}
	if (x1<0) {
		if (x0==x1) return;
		if((y1=y0+((0-x0)*(y1-y0)/(x1-x0)))<0) y1=0;			// MIB.42  This is needed as vRam is not tolerant for misses
		x1=0;
	}
	if (y1<0) {
		if (y1==y0) return;
		if((x1=x0 + (x1-x0)*(0-y0)/(y1-y0))<0) x1=0;			// MIB.42  This is needed as vRam is not tolerant for misses
		y1=0;
	}
	//if (x0<0 || x1<0 || y0<0 || y1<0)
	//printf("! %d,%d,%d,%d\n",x0,y0,x1,y1);
	//if (x0>SCR_WIDTH-10 || x1>SCR_WIDTH-10 || y0>SCR_HEIGHT-10 || y1>SCR_HEIGHT-10)
	//	printf(" %d,%d,%d,%d\n",x0,y0,x1,y1);

//	if(y0>(SCR_HEIGHT-3)) y0=(SCR_HEIGHT-3);
//	if(y1>(SCR_HEIGHT-3)) y1=(SCR_HEIGHT-3);
//	if(x0>(SCR_WIDTH-3)) x0=(SCR_WIDTH-3);
//	if(x1>(SCR_WIDTH-3)) x1=(SCR_WIDTH-3);

	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	
	if (dy < 0) { dy = -dy;  stepy = -width; } else { stepy = width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;
	
	y0 *= width;
	y1 *= width;
	destination[x0+y0] = color;
	if (dx > dy) {
		int fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			destination[x0+y0] = color;
			destination[x0+y0+1] = color;
			destination[x0+y0+width] = color;
			destination[x0+y0+width+1] = color;
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			destination[x0+y0] = color;
			destination[x0+y0+1] = color;
			destination[x0+y0+width] = color;
			destination[x0+y0+width+1] = color;
		}
	}
}
#endif

