#ifndef GRAPHICS_H
#define GRAPHICS_H

//#include <ft2build.h>
//#include FT_FREETYPE_H
//#include <freetype2/freetype/freetype.h> 

#include <psptypes.h>



#define	PSP_LINE_SIZE 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define FRAMEBUFFER_SIZE (PSP_LINE_SIZE*SCREEN_HEIGHT*4)
#define FARS    68000.0f
#define NEARS   .91f
#define DIST    8.0
#define Z       -0.21f
#define ZNORM   -0.65f
#define FOA     75.0f
#define ROTX    5.57

typedef u32 Color;
#define A(color) ((u8)(color >> 24 & 0xFF))
#define B(color) ((u8)(color >> 16 & 0xFF))
#define G(color) ((u8)(color >> 8 & 0xFF))
#define R(color) ((u8)(color & 0xFF))
#define ARGB8888(_A_,_R_,_G_,_B_)   ((   ((u32)_A_&0xFF)<<24)   |   (         ((u32)_R_&0xFF)<<16)   |   (         ((u32)_G_&0xFF)<<8)      |   (      ((u32)_B_&0xFF)<<0)         )




typedef struct
{
	int textureWidth;  // the real width of data, 2^n with n>=0
	int textureHeight;  // the real height of data, 2^n with n>=0
	int imageWidth;  // the image width
	int imageHeight;
	Color* data;
} Image;

/**
 * Load a PNG or JPEG image (depends on the filename suffix).
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image to load
 * @return pointer to a new allocated Image struct, or NULL on failure
 */
extern Image* loadImage(const char* filename);
extern Image* loadImage1(const char* filename,int negate);

/**
 * Load a PNG image.
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image to load
 * @return pointer to a new allocated Image struct, or NULL on failure
 */
extern Image* loadPngImage(const char* filename);

/**
 * Load a JPEG image.
 *
 * @pre filename != NULL
 * @param filename - filename of the JPEG image to load
 * @return pointer to a new allocated Image struct, or NULL on failure
 */
extern Image* loadJpegImage(const char* filename);

/**
 * Load a PNG or JPEG image from in-memory data (with auto detection of the format).
 *
 * @pre data != NULL && len >0
 * @param data - the in-memory bytes of the image
 * @param len - the number of valid bytes at the data pointer
 * @return pointer to a new allocated Image struct, or NULL on failure
 */
extern Image* loadImageFromMemory(const unsigned char* data, int len);

/**
 * Blit a rectangle part of an image to another image.
 *
 * @pre source != NULL && destination != NULL &&
 *      sx >= 0 && sy >= 0 &&
 *      width > 0 && height > 0 &&
 *      sx + width <= source->width && sy + height <= source->height &&
 *      dx + width <= destination->width && dy + height <= destination->height
 * @param sx - left position of rectangle in source image
 * @param sy - top position of rectangle in source image
 * @param width - width of rectangle in source image
 * @param height - height of rectangle in source image
 * @param source - pointer to Image struct of the source image
 * @param dx - left target position in destination image
 * @param dy - top target position in destination image
 * @param destination - pointer to Image struct of the destination image
 */
extern void blitImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination);
extern void blitImageToImage2(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination);

/**
 * Blit a rectangle part of an image to screen.
 *
 * @pre source != NULL && destination != NULL &&
 *      sx >= 0 && sy >= 0 &&
 *      width > 0 && height > 0 &&
 *      sx + width <= source->width && sy + height <= source->height &&
 *      dx + width <= SCREEN_WIDTH && dy + height <= SCREEN_HEIGHT
 * @param sx - left position of rectangle in source image
 * @param sy - top position of rectangle in source image
 * @param width - width of rectangle in source image
 * @param height - height of rectangle in source image
 * @param source - pointer to Image struct of the source image
 * @param dx - left target position in destination image
 * @param dy - top target position in destination image
 */
extern void blitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy);

/**
 * Blit a rectangle part of an image to another image without alpha pixels in source image.
 *
 * @pre source != NULL && destination != NULL &&
 *      sx >= 0 && sy >= 0 &&
 *      width > 0 && height > 0 &&
 *      sx + width <= source->width && sy + height <= source->height &&
 *      dx + width <= destination->width && dy + height <= destination->height
 * @param sx - left position of rectangle in source image
 * @param sy - top position of rectangle in source image
 * @param width - width of rectangle in source image
 * @param height - height of rectangle in source image
 * @param source - pointer to Image struct of the source image
 * @param dx - left target position in destination image
 * @param dy - top target position in destination image
 * @param destination - pointer to Image struct of the destination image
 */
extern void blitAlphaImageToImage(int sx, int sy, int width, int height, Image* source, int dx, int dy, Image* destination);

/**
 * Blit a rectangle part of an image to screen without alpha pixels in source image.
 *
 * @pre source != NULL && destination != NULL &&
 *      sx >= 0 && sy >= 0 &&
 *      width > 0 && height > 0 &&
 *      sx + width <= source->width && sy + height <= source->height &&
 *      dx + width <= SCREEN_WIDTH && dy + height <= SCREEN_HEIGHT
 * @param sx - left position of rectangle in source image
 * @param sy - top position of rectangle in source image
 * @param width - width of rectangle in source image
 * @param height - height of rectangle in source image
 * @param source - pointer to Image struct of the source image
 * @param dx - left target position in destination image
 * @param dy - top target position in destination image
 */
extern void blitAlphaImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy);

/**
 * Create an empty image.
 *
 * @pre width > 0 && height > 0 && width <= 512 && height <= 512
 * @param width - width of the new image
 * @param height - height of the new image
 * @return pointer to a new allocated Image struct, all pixels initialized to color 0, or NULL on failure
 */
extern Image* createImage(int width, int height);

/**
 * Frees an allocated image.
 *
 * @pre image != null
 * @param image a pointer to an image struct
 */
extern void freeImage(Image* image);

/**
 * Initialize all pixels of an image with a color.
 *
 * @pre image != NULL
 * @param color - new color for the pixels
 * @param image - image to clear
 */
extern void clearImage(Color color, Image* image);

/**
 * Initialize all pixels of the screen with a color.
 *
 * @param color - new color for the pixels
 */
extern void clearScreen(Color color);

/**
 * Fill a rectangle of an image with a color.
 *
 * @pre image != NULL
 * @param color - new color for the pixels
 * @param x0 - left position of rectangle in image
 * @param y0 - top position of rectangle in image
 * @param width - width of rectangle in image
 * @param height - height of rectangle in image
 * @param image - image
 */
extern void fillImageRect(Color color, int x0, int y0, int width, int height, Image* image);

/**
 * Fill a rectangle of an image with a color.
 *
 * @pre image != NULL
 * @param color - new color for the pixels
 * @param x0 - left position of rectangle in image
 * @param y0 - top position of rectangle in image
 * @param width - width of rectangle in image
 * @param height - height of rectangle in image
 */
extern void fillScreenRect(Color color, int x0, int y0, int width, int height);

/**
 * Set a pixel on screen to the specified color.
 *
 * @pre x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT
 * @param color - new color for the pixels
 * @param x - left position of the pixel
 * @param y - top position of the pixel
 */
extern void putPixelScreen(Color color, int x, int y);

/**
 * Set a pixel in an image to the specified color.
 *
 * @pre x >= 0 && x < image->imageWidth && y >= 0 && y < image->imageHeight && image != NULL
 * @param color - new color for the pixels
 * @param x - left position of the pixel
 * @param y - top position of the pixel
 */
extern void putPixelImage(Color color, int x, int y, Image* image);

/**
 * Get the color of a pixel on screen.
 *
 * @pre x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT
 * @param x - left position of the pixel
 * @param y - top position of the pixel
 * @return the color of the pixel
 */
extern Color getPixelScreen(int x, int y);

/**
 * Get the color of a pixel of an image.
 *
 * @pre x >= 0 && x < image->imageWidth && y >= 0 && y < image->imageHeight && image != NULL
 * @param x - left position of the pixel
 * @param y - top position of the pixel
 * @return the color of the pixel
 */
extern Color getPixelImage(int x, int y, Image* image);

/**
 * Print a text (pixels out of the screen or image are clipped).
 *
 * @param x - left position of text
 * @param y - top position of text
 * @param text - the text to print
 * @param color - text color
 */
extern void printTextScreen(int x, int y, const char* text, Color color);
extern void printTextScreen1(int x, int y, const char* text, Color color, Image *);

/**
 * Print a text (pixels out of the screen or image are clipped).
 *
 * @param x - left position of text
 * @param y - top position of text
 * @param text - the text to print
 * @param color - text color
 * @param image - image
 */
extern void printTextImage(int x, int y, const char* text, Color color, Image* image);

/**
 * Print a text, which was rendered to a bitmap with Freetype.
 *
 * @param x - left position of text
 * @param y - top position of text
 * @param text - the text to print
 * @param color - text color
 * @param image - image
 */
//extern void fontPrintTextImage(FT_Bitmap* bitmap, int x, int y, Color color, Image* image);

/**
 * Print a text, which was rendered to a bitmap with Freetype.
 *
 * @param x - left position of text
 * @param y - top position of text
 * @param text - the text to print
 * @param color - text color
 */
//extern void fontPrintTextScreen(FT_Bitmap* bitmap, int x, int y, Color color);

/**
 * Save an image or the screen in PNG or JPEG format (depends on the filename suffix).
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image
 * @param data - start of Color type pixel data (can be getVramDisplayBuffer())
 * @param width - logical width of the image or SCREEN_WIDTH
 * @param height - height of the image or SCREEN_HEIGHT
 * @param lineSize - physical width of the image or PSP_LINE_SIZE
 * @param saveAlpha - if 0, image is saved without alpha channel
 */
extern void saveImage(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha);

/**
 * Save an image or the screen in PNG format.
 *
 * @pre filename != NULL
 * @param filename - filename of the PNG image
 * @param data - start of Color type pixel data (can be getVramDisplayBuffer())
 * @param width - logical width of the image or SCREEN_WIDTH
 * @param height - height of the image or SCREEN_HEIGHT
 * @param lineSize - physical width of the image or PSP_LINE_SIZE
 * @param saveAlpha - if 0, image is saved without alpha channel
 */
extern void savePngImage(const char* filename, Color* data, int width, int height, int lineSize, int saveAlpha);

/**
 * Save an image or the screen in JPEG format.
 *
 * @pre filename != NULL
 * @param filename - filename of the JPEG image
 * @param data - start of Color type pixel data (can be getVramDisplayBuffer())
 * @param width - logical width of the image or SCREEN_WIDTH
 * @param height - height of the image or SCREEN_HEIGHT
 * @param lineSize - physical width of the image or PSP_LINE_SIZE
 */
extern void saveJpegImage(const char* filename, Color* data, int width, int height, int lineSize);

/**
 * Exchange display buffer and drawing buffer.
 */
extern void flipScreen();

/**
 * Initialize the graphics.
 */
extern void initGraphics();

/**
 * Disable graphics, used for debug text output.
 */
extern void disableGraphics();

/**
 * Draw a line to screen.
 *
 * @pre x0 >= 0 && x0 < SCREEN_WIDTH && y0 >= 0 && y0 < SCREEN_HEIGHT &&
 *      x1 >= 0 && x1 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT
 * @param x0 - x line start position
 * @param y0 - y line start position
 * @param x1 - x line end position
 * @param y1 - y line end position
 */
void drawLineScreen(int x0, int y0, int x1, int y1, Color color, int thick);

/**
 * Draw a line to screen.
 *
 * @pre x0 >= 0 && x0 < image->imageWidth && y0 >= 0 && y0 < image->imageHeight &&
 *      x1 >= 0 && x1 < image->imageWidth && y1 >= 0 && y1 < image->imageHeight
 * @param x0 - x line start position
 * @param y0 - y line start position
 * @param x1 - x line end position
 * @param y1 - y line end position
 */
extern void drawLineImage(int x0, int y0, int x1, int y1, Color color, Image* image,int thick);

/**
 * Get the current draw buffer for fast unchecked access.
 *
 * @return the start address of the current draw buffer
 */
extern Color* getVramDrawBuffer();

/**
 * Get the current display buffer for fast unchecked access.
 *
 * @return the start address of the current display buffer
 */
extern Color* getVramDisplayBuffer();

void drawCircleScreen(int x, int y, int radius, Color color);
void drawElipseScreen(int x, int y, int radius, Color color);

extern void guStart();
extern void guFinish();

void nightMode();
void renderLines(int mode, double angle, float fixvar, float anglevar);
void drawLine3(float x1, float y1, float x2, float y2, Color c,int mode, double angle, float fixvar, float anglevar);
void drawGuLine(float x1, float y1, float x2, float y2, Color c,float width, int mode, double angle);
void blitSprite(Image *img, float x, float y, double angle, int mode, int hilite,float fixvar, float anglevar);
void blitRotatedImage(Image *img, float x, float y, double angle, int mode);
void blitMapTile(Image *img, float x, float y, double angle, int mode , float fixvar, float anglevar);
void swizzleImage(Image *);
void setScene(int mode);
void FillPolygon(float* x, float* y, int count, Color color, int mode, double angle); 
void DrawThickLine(float x1, float y1, float x2, float y2, Color color, float lineWidth, int mode, double angle, float fixvar, float anglevar);

Image* ldImage(const char* filename);
Image* ldImage1(const char* filename,int negate);
void drawHiliteBar(Image * hilite,int x,int y, int width);
#endif
