/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
*/
/************************************************************************* 
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 2002 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 *************************************************************************/
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: XpmCreate.c /main/cde1_maint/4 1995/10/13 17:11:05 pascale $"
#endif
#endif
/* Copyright 1990-92 GROUPE BULL -- See license conditions in file COPYRIGHT */
/*****************************************************************************\
 * create.c:                                                                 *
 *  XPM library                                                              *
 *  Create an X image and possibly its related shape mask                    *
 *  from the given xpmInternAttrib.                                          *
 *  Developed by Arnaud Le Hors                                              *
 * Added for backward compatibility with 1.2 API			     *
\*****************************************************************************/

#include <Xm/Xm.h>
#include <Xm/XpmI.h>
#ifdef VMS
#include "sys$library:ctype.h"
#else
#include <ctype.h>
#endif

#ifdef XVPLIB
#include <Xvp/Xvplib.h>
#endif

#include "ColorObj.h"

#define MONO    2
#define GRAY4   3
#define GRAY    4
#define COLOR   5

typedef struct _XpmCachedColorPixelStruct{
    Display	*display;
    Colormap 	 colormap;
    XrmQuark 	 colorname;
    Pixel    	 pixel;
    unsigned int num_cached;
}XpmCachedColorPixelStruct;

typedef struct _XpmCachedColorPixelListStruct{
    int                         numEntries;
    int                         maxEntries;
    XpmCachedColorPixelStruct  *cache;
}XpmCachedColorPixelListStruct;


#if   !defined(_XtBoolean)
# if NeedWidePrototypes
#  define _XtBoolean	int
# else
#  define _XtBoolean	Boolean
# endif
#endif

LFUNC(xpmVisualType, int, (Visual *visual));

LFUNC(SetColor, int, (Display * display, Colormap colormap, char *colorname,
		      unsigned int color_index, Pixel * image_pixel,
		      Pixel * mask_pixel, unsigned int * mask_pixel_index,
		      unsigned int * num_image_pixels,
		      XpmAttributes *attributes));

LFUNC(CreateOrDestroyColors, int, (Display *display, XpmAttributes *attributes,
			  char ***ct, unsigned int ncolors, Pixel *ip,
			  Pixel *mp, unsigned int *mask_pixel,
			  unsigned int *num_image_pixels, _XtBoolean create));

LFUNC(CreateXImage, int, (Display * display, Visual * visual,
			  unsigned int depth, unsigned int width,
			  unsigned int height, XImage ** image_return));

#ifdef DONT_USE_INPLACE_IMAGES
LFUNC(SetImagePixels, void, (XImage * image, unsigned int width,
			     unsigned int height, unsigned int *pixelindex,
			     Pixel * pixels));

LFUNC(SetImagePixels32, void, (XImage * image, unsigned int width,
			       unsigned int height, unsigned int *pixelindex,
			       Pixel * pixels));

LFUNC(SetImagePixels16, void, (XImage * image, unsigned int width,
			       unsigned int height, unsigned int *pixelindex,
			       Pixel * pixels));

LFUNC(SetImagePixels8, void, (XImage * image, unsigned int width,
			      unsigned int height, unsigned int *pixelindex,
			      Pixel * pixels));

LFUNC(SetImagePixels1, void, (XImage * image, unsigned int width,
			      unsigned int height, unsigned int *pixelindex,
			      Pixel * pixels));
#else /* DONT_USE_INPLACE_IMAGES */
LFUNC(SetAllocateImagePixelsXY, void, (XImage * image, unsigned int pi_depth,
					unsigned int *pixelindex,
					Pixel * pixels));
LFUNC(SetImagePixelsXY, void, (XImage * image, unsigned int pi_depth,
					unsigned int *pixelindex,
					Pixel * pixels));
LFUNC(SetImagePixelsZ, void, (XImage * image, unsigned int pi_depth,
					unsigned int *pixelindex,
					Pixel * pixels));
#endif /* DONT_USE_INPLACE_IMAGES */

#ifdef NEED_STRCASECMP

LFUNC(strcasecmp, int, (char *s1, char *s2));

static int
strcasecmp(s1, s2)
    register char *s1, *s2;
{
    register int c1, c2;

    while (*s1 && *s2) {
	c1 = isupper((unsigned char)*s1) ? tolower((unsigned char)*s1) :
						   (unsigned char)*s1;
	c2 = isupper((unsigned char)*s2) ? tolower((unsigned char)*s2) :
						   (unsigned char)*s2;
	if (c1 != c2)
	    return (1);
	s1++;
	s2++;
    }
    if (*s1 || *s2)
	return (1);
    return (0);
}
#endif

static int
xpmVisualType(visual)
    Visual *visual;
{
    switch (visual->class) {
    case StaticGray:
    case GrayScale:
	switch (visual->map_entries) {
	case 2:
	    return (MONO);
	case 4:
	    return (GRAY4);
	default:
	    return (GRAY);
	}
    default:
	return (COLOR);
    }
}

static XpmCachedColorPixelListStruct colorPixCacheList;

#define GetCacheColorPixel(DISP,CMAP,NAME,PIXEL) \
		GetOrFreeCacheColorPixel(DISP,CMAP,NAME,PIXEL,True,(Boolean *)0)
#define FreeCacheColorPixel(DISP,CMAP,NAME,PIXEL,CANFREE) \
		GetOrFreeCacheColorPixel(DISP,CMAP,NAME,PIXEL,False,CANFREE)

static Boolean
#ifdef _NO_PROTO
GetOrFreeCacheColorPixel( display, colormap, colorname, pixel, get, can_free)
	Display *display;
	Colormap colormap;
	char *colorname;
	Pixel *pixel;
	_XtBoolean get;
	Boolean * can_free;
#else
GetOrFreeCacheColorPixel( Display *display, Colormap colormap,
                    char *colorname, Pixel *pixel, _XtBoolean get, Boolean * can_free)
#endif
{
    static Boolean firstTime = True;
    XrmQuark colorname_q = XrmStringToQuark(colorname);
    int i;

    if ( firstTime )
    {
       colorPixCacheList.numEntries = colorPixCacheList.maxEntries = 0;
       colorPixCacheList.cache = NULL;
       firstTime = False; 
       return False;
    }

    for (i = 0; i < colorPixCacheList.numEntries ; i++)
    {
	if (colorPixCacheList.cache[i].colorname == colorname_q &&
            colorPixCacheList.cache[i].colormap == colormap &&
            colorPixCacheList.cache[i].display == display)
        {
	   *pixel = colorPixCacheList.cache[i].pixel;
           if (get)
		colorPixCacheList.cache[i].num_cached++;
	   else {
		colorPixCacheList.cache[i].num_cached--;
		if (colorPixCacheList.cache[i].num_cached == 0) {
		    int j;
		    for (j = i + 1; j < colorPixCacheList.numEntries; i++, j++)
			colorPixCacheList.cache[i] = colorPixCacheList.cache[j];
		    colorPixCacheList.numEntries--;
		    *can_free = True;
		} else
		    *can_free = False;
	   }
           return True;
        }
    }
    return False;
}

static void
#ifdef _NO_PROTO
CacheColorPixel( display, colormap, colorname, pixel)
	Display *display;
	Colormap colormap;
	char *colorname;
	Pixel pixel;
#else
CacheColorPixel( Display *display, Colormap colormap,
                 char *colorname, Pixel pixel)
#endif
{
    int numEntries = colorPixCacheList.numEntries;

    if (numEntries == colorPixCacheList.maxEntries)
    {
        colorPixCacheList.maxEntries += 25;
	colorPixCacheList.cache = (XpmCachedColorPixelStruct *)
				  XtRealloc((char *)colorPixCacheList.cache,
					    colorPixCacheList.maxEntries *
				            sizeof(XpmCachedColorPixelStruct));
    }

    colorPixCacheList.cache[numEntries].display = display;
    colorPixCacheList.cache[numEntries].colormap = colormap;
    colorPixCacheList.cache[numEntries].colorname = XrmStringToQuark(colorname);
    colorPixCacheList.cache[numEntries].pixel = pixel;
    colorPixCacheList.cache[numEntries].num_cached = 1;
                                                      
    colorPixCacheList.numEntries++;
}

static int
#ifdef _NO_PROTO
SetColor(display, colormap, colorname, color_index,
	 image_pixel, mask_pixel, mask_pixel_index,
	 num_image_pixels, attributes)
    Display *display;
    Colormap colormap;
    char *colorname;
    unsigned int color_index;
    Pixel *image_pixel, *mask_pixel;
    unsigned int *mask_pixel_index;
    unsigned int *num_image_pixels;
    XpmAttributes *attributes;
#else
SetColor(Display *display, Colormap colormap, char *colorname,
	unsigned int color_index, Pixel *image_pixel, Pixel *mask_pixel,
	unsigned int *mask_pixel_index, unsigned int *num_image_pixels,
	XpmAttributes *attributes)
#endif
{
    XColor xcolor;
    Pixel pixel;

    if (strcasecmp(colorname, TRANSPARENT_COLOR)) {
        if (!GetCacheColorPixel(display, colormap, colorname, &pixel))
        {
	   if (!XParseColor(display, colormap, colorname, &xcolor)) return(1);
	   else if (!XAllocColor(display, colormap, &xcolor))
	   {
	     if (attributes && (attributes->valuemask & XpmCloseness) &&
		 attributes->closeness != 0)
	     {
	       XColor *cols;
	       unsigned int ncols,i,closepix;
	       long int closediff,closeness = attributes->closeness;

	       if (attributes && attributes->valuemask & XpmDepth)
		 ncols = 1 << attributes->depth;
	       else
		 ncols = 1 << DefaultDepth(display, DefaultScreen(display));

	       cols = (XColor*)calloc(ncols,sizeof(XColor));
	       for (i = 0; i < ncols; ++i) cols[i].pixel = i;
	       XQueryColors(display,colormap,cols,ncols);

	       for (i = 0, closediff = 0x7FFFFFFF; i < ncols; ++i)
	       {
#define COLOR_FACTOR       3
#define BRIGHTNESS_FACTOR  1

		 long int newclosediff = 
		   COLOR_FACTOR * (
		     abs(xcolor.red - cols[i].red)   + /* Wyoming 64-bit fix */ 
		     abs(xcolor.green - cols[i].green) + /* Wyoming 64-bit fix */ 
		     abs(xcolor.blue  - cols[i].blue)) + /* Wyoming 64-bit fix */ 
		   BRIGHTNESS_FACTOR * abs(
		    (xcolor.red+xcolor.green+xcolor.blue) - /* Wyoming 64-bit fix */ 
		    (cols[i].red+cols[i].green+cols[i].blue)); /* Wyoming 64-bit fix */ 

		 if (newclosediff < closediff)
		 { closepix = i; closediff = newclosediff; }
	       }

	       if ((long)cols[closepix].red >= (long)xcolor.red - closeness &&
		   (long)cols[closepix].red <= (long)xcolor.red + closeness &&
		   (long)cols[closepix].green >= (long)xcolor.green-closeness &&
		   (long)cols[closepix].green <= (long)xcolor.green+closeness &&
		   (long)cols[closepix].blue >= (long)xcolor.blue - closeness &&
		   (long)cols[closepix].blue <= (long)xcolor.blue + closeness)
	       {
		 xcolor = cols[closepix]; free(cols);
		 
		 /***********************************************************
		   Fix for bug 1216907, don't return 1 if XAllocColor fails.
		   If a value of 1 is returned then the pixmap is flagged 
		   invalid, and not displayed.
		   **********************************************************/
		 if (!XAllocColor(display, colormap, &xcolor)) {

		     XColor almostXC, exactXC;
		     XGCValues gcval;
		     PixelSet pixels[MAX_NUM_COLORS];
		     int coloruse, index;
		     short i2, i3, i4, i5;

		     /* Can't allocate the exact color. Try the default
			background color of the desktop */
#define FILLER_COLOR_INDEX 2

		     index = (FILLER_COLOR_INDEX >= MAX_NUM_COLORS ? 
			         MAX_NUM_COLORS : FILLER_COLOR_INDEX);

		     if (_XmGetPixelData(DefaultScreen(display), &coloruse, 
					 pixels, &i2, &i3, &i4, &i5) &&
			 (pixels[FILLER_COLOR_INDEX].bg <= 255)) {
			 /* we want to make sure that the pixel value
			    is less than 255, because if we got to this
			    part of the code we're dealing with an 8 bit
			    display */
			 xcolor.pixel = pixels[FILLER_COLOR_INDEX].bg;
		     } else {
			 xcolor.pixel = BlackPixel(display, 
						   DefaultScreen(display));
		     }
		 }
	       }
	       else { free(cols); return (1); }
	     }
	     else return (1);
	   }
           pixel = (Pixel) xcolor.pixel;
           CacheColorPixel(display, colormap, colorname, pixel);
	   /* Since we introduce the colorCache functionality,
	    * the original purpose of pixels/npixels is gone. see below.
	    *
	   (*pixels)[*npixels] = pixel;
	   (*npixels)++;
	    */
        }

	*image_pixel = pixel;
	*mask_pixel = 1;
       
    } else {
	if ((attributes->valuemask & XpmColorSymbols)
	    && (attributes->numsymbols > 0)) {
		*image_pixel = attributes->colorsymbols[0].pixel;
	} else {
	    *image_pixel = 0;
    	}
	*mask_pixel = 0;
	*mask_pixel_index = color_index;/* store the color table index */
    }
    return (0);
}

static int
#ifdef _NO_PROTO
FreeColor(display, colormap, colorname)
    Display *display;
    Colormap colormap;
    char *colorname;
#else
FreeColor(Display *display, Colormap colormap, char *colorname)
#endif
{
    Pixel pixel;
    Boolean can_free;
    if (strcasecmp(colorname, TRANSPARENT_COLOR))
	if (FreeCacheColorPixel(display, colormap, colorname, &pixel, &can_free)) {
	    if (can_free) {
		XFreeColors (display, colormap, &pixel, 1, 0);
	    }
	    return (0);
	}
    return (1);
}

#define CreateColors(DISP,ATTR,CT,NCOLORS,IP,MP,MASK,NPIXELS) \
	CreateOrDestroyColors(DISP,ATTR,CT,NCOLORS,IP,MP,MASK,NPIXELS, True)
#define DestroyColors(DISP,ATTR,CT,NCOLORS) \
	CreateOrDestroyColors(DISP,ATTR,CT,NCOLORS, \
		(Pixel *)0,(Pixel *)0,(unsigned int *)0,(unsigned int *)0, False)

static int
#ifdef _NO_PROTO
CreateOrDestroyColors(display, attributes, ct, ncolors,
	     ip, mp, mask_pixel, num_image_pixels, create)
    Display *display;
    XpmAttributes *attributes;
    char ***ct;
    unsigned int ncolors;
    Pixel *ip;
    Pixel *mp;
    unsigned int *mask_pixel;		/* mask pixel index */
    unsigned int *num_image_pixels;	/* number of available pixels */
    _XtBoolean create;
#else
CreateOrDestroyColors(
	Display *display,
	XpmAttributes *attributes,
	char ***ct,
	unsigned int ncolors,
	Pixel *ip,
	Pixel *mp,
	unsigned int *mask_pixel, /* mask pixel index */
	unsigned int *num_image_pixels, /* number of available pixels */
    	_XtBoolean create )
#endif
{
    /* variables stored in the XpmAttributes structure */
    Visual *visual;
    Colormap colormap;
    XpmColorSymbol *colorsymbols;
    unsigned int numsymbols;

    char *colorname;
    unsigned int a, b, l;
    Boolean pixel_defined;
    unsigned int key;
    XpmColorSymbol *cs;
    char **cts;
    int ErrorStatus = XpmSuccess;
    char *s;
    int cts_index;

#define SetOrFreeColor(DISP,CMAP,NAME,INDEX,IP,MP,MPI,NPIXELS,ATTR,CREATE) \
	(CREATE? SetColor(DISP,CMAP,NAME,INDEX,IP,MP,MPI,NPIXELS,ATTR) \
	       : FreeColor(DISP,CMAP,NAME))

    /*
     * retrieve information from the XpmAttributes 
     */
    if (attributes && attributes->valuemask & XpmColorSymbols) {
	colorsymbols = attributes->colorsymbols;
	numsymbols = attributes->numsymbols;
    } else
	numsymbols = 0;

    if (attributes && attributes->valuemask & XpmVisual)
	visual = attributes->visual;
    else
	visual = DefaultVisual(display, DefaultScreen(display));

    if (attributes && attributes->valuemask & XpmColormap)
	colormap = attributes->colormap;
    else
	colormap = DefaultColormap(display, DefaultScreen(display));

    key = xpmVisualType(visual);
    switch(key)
    {
     case MONO:  cts_index = 2; break;
     case GRAY4: cts_index = 3; break;
     case GRAY:  cts_index = 4; break;
     case COLOR: cts_index = 5; break;
    }

    for (a = 0; a < ncolors; a++, ct++) {
	colorname = NULL;
	pixel_defined = False;
	cts = *ct;

	/*
	 * look for a defined symbol 
	 */
	if (numsymbols && cts[1]) {
	    s = cts[1];
	    for (l = 0, cs = colorsymbols; l < numsymbols; l++, cs++)
		if ((!cs->name && cs->value && cts[cts_index] &&
		     !strcasecmp(cs->value,cts[cts_index])) ||
		    cs->name && !strcmp(cs->name, s))
		    break;
	    if (l != numsymbols) {
		if (cs->name && cs->value)
		    colorname = cs->value;
		else
		    pixel_defined = True;
	    }
	}
	if (!pixel_defined) {		/* pixel not given as symbol value */
	    if (colorname) {		/* colorname given as symbol value */
		if (!SetOrFreeColor(display, colormap, colorname, a, ip, mp,
			      mask_pixel, num_image_pixels, attributes, create))
		    pixel_defined = True;
		else
		    ErrorStatus = XpmColorError;
	    }
	    b = key;
	    while (!pixel_defined && b > 1) {
		if (cts[b]) {
		    if (!SetOrFreeColor(display, colormap, cts[b], a, ip, mp,
				  mask_pixel, num_image_pixels, attributes, create)) {
			pixel_defined = True;
			break;
		    } else
			ErrorStatus = XpmColorError;
		}
		b--;
	    }
	    b = key + 1;
	    while (!pixel_defined && b < NKEYS + 1) {
		if (cts[b]) {
		    if (!SetOrFreeColor(display, colormap, cts[b], a, ip, mp,
				  mask_pixel, num_image_pixels, attributes, create)) {
			pixel_defined = True;
			break;
		    } else
			ErrorStatus = XpmColorError;
		}
		b++;
	    }
	    if (!pixel_defined)
		return (XpmColorFailed);
	} else if (create) {
	    *ip = colorsymbols[l].pixel;
	    *mp = 1;
	}
	   /* Since we introduce the colorCache functionality,
	    * the original purpose of pixels/npixels, ie. for later-on
	    * freeing those newly allocated pixels upon certain failures in 
	    * _XmxpmCreateImage(), is gone.
	    * All the allocated pixels now are registered into the colorCache
	    * with the colorname as the index and each reference (or use) of
	    * one color increases its reference count in Cache by 1.
	    * npixels (renamed as num_image_pixels) is now the number of colors 	    * in the colorTable of the Image that are successfully found so far.
	    */
	if (create)
	    ip++, mp++, (*num_image_pixels)++;
    }
    return (ErrorStatus);
#undef SetOrFreeColor
}    

#undef RETURN
#ifdef Debug
#define RETURN(status) \
    { if (image) { \
	free(image->data); \
	XDestroyImage(image); } \
    if (shapeimage) { \
	free(shapeimage->data); \
	XDestroyImage(shapeimage); } \
    if (image_pixels) free(image_pixels); \
    if (mask_pixels) free(mask_pixels); \
    if (num_image_pixels) CreateOrDestroyColors(display, attributes, \
	attrib->colorTable, num_image_pixels, (Pixel *)0,(Pixel *)0, \
	(unsigned int *)0, (unsigned int *)0, False); \
    return (status); }
#else

#define RETURN(status) \
    { if (image) XDestroyImage(image); \
    if (shapeimage) XDestroyImage(shapeimage); \
    if (image_pixels) free(image_pixels); \
    if (mask_pixels) free(mask_pixels); \
    if (num_image_pixels) CreateOrDestroyColors(display, attributes, \
        attrib->colorTable, num_image_pixels, (Pixel *)0,(Pixel *)0, \
        (unsigned int *)0, (unsigned int *)0, False); \
    return (status); }
#endif

#ifdef _NO_PROTO
_XmxpmCreateImage(display, attrib, image_return, shapeimage_return, attributes)
    Display *display;
    xpmInternAttrib *attrib;
    XImage **image_return;
    XImage **shapeimage_return;
    XpmAttributes *attributes;
#else
_XmxpmCreateImage(
    Display *display,
    xpmInternAttrib *attrib,
    XImage **image_return,
    XImage **shapeimage_return,
    XpmAttributes *attributes )
#endif
{
    Visual *visual;
    Colormap colormap;
    unsigned int depth;

    XImage *image = NULL;
    XImage *shapeimage = NULL;
    unsigned int mask_pixel;
    int ErrorStatus;

    Pixel *image_pixels = NULL;
    Pixel *mask_pixels = NULL;
    unsigned int num_image_pixels = 0;	/* number of available pixels */

    if (attributes && attributes->valuemask & XpmVisual)
	visual = attributes->visual;
    else
	visual = DefaultVisual(display, DefaultScreen(display));

    if (attributes && attributes->valuemask & XpmColormap)
	colormap = attributes->colormap;
    else
	colormap = DefaultColormap(display, DefaultScreen(display));

    if (attributes && attributes->valuemask & XpmDepth)
	depth = attributes->depth;
    else
	depth = DefaultDepth(display, DefaultScreen(display));

    ErrorStatus = XpmSuccess;

    image_pixels = (Pixel *) malloc(sizeof(Pixel) * attrib->ncolors);
    if (!image_pixels)
	return(XpmNoMemory);

    mask_pixels = (Pixel *) malloc(sizeof(Pixel) * attrib->ncolors);
    if (!mask_pixels)
	RETURN(ErrorStatus);

    mask_pixel = UNDEF_PIXEL;

    ErrorStatus = CreateColors(display, attributes, attrib->colorTable,
			       attrib->ncolors, image_pixels, mask_pixels,
			       &mask_pixel, &num_image_pixels);
    if (ErrorStatus != XpmSuccess && (ErrorStatus < 0 || attributes &&
	(attributes->valuemask & XpmExactColors) && attributes->exactColors))
	RETURN(ErrorStatus);

#ifndef DONT_USE_INPLACE_IMAGES

    goto DO_SHAPE_IMAGE_FIRST;
    DONE_WITH_SHAPEIMAGE:
#endif /* DONT_USE_INPLACE_IMAGES */

    if (image_return) {
	ErrorStatus = CreateXImage(display, visual, depth,
				    attrib->width, attrib->height, &image);
	if (ErrorStatus != XpmSuccess)
	    RETURN(ErrorStatus);

#ifdef DONT_USE_INPLACE_IMAGES
	if (image->depth == 1)
	    SetImagePixels1(image, attrib->width, attrib->height,
			    attrib->pixelindex, image_pixels);
	else if (image->bits_per_pixel == 8)
	    SetImagePixels8(image, attrib->width, attrib->height,
			    attrib->pixelindex, image_pixels);
	else if (image->bits_per_pixel == 16)
	    SetImagePixels16(image, attrib->width, attrib->height,
			     attrib->pixelindex, image_pixels);
	else if (image->bits_per_pixel == 32)
	    SetImagePixels32(image, attrib->width, attrib->height,
			     attrib->pixelindex, image_pixels);
	else
	    SetImagePixels(image, attrib->width, attrib->height,
			   attrib->pixelindex, image_pixels);
#else /* DONT_USE_INPLACE_IMAGES */

	if (image->depth == 1) {

	    SetAllocateImagePixelsXY(image, image->bitmap_pad,
		attrib->pixelindex, image_pixels);
		
	} else {
	    image->data = (char *) attrib->pixelindex;

	    SetImagePixelsZ(image, image->bitmap_pad,
		attrib->pixelindex, image_pixels);
	    
	    attrib->pixelindex = NULL;
	}
#endif /* DONT_USE_INPLACE_IMAGES */
    }

#ifndef DONT_USE_INPLACE_IMAGES
    goto ALREADY_DID_SHAPEIMAGE;
    DO_SHAPE_IMAGE_FIRST:
#endif /* DONT_USE_INPLACE_IMAGES */

    if (mask_pixel != UNDEF_PIXEL && shapeimage_return) {
	ErrorStatus = CreateXImage(display, visual, 1, attrib->width,
				    attrib->height, &shapeimage);
	if (ErrorStatus != XpmSuccess)
	    RETURN(ErrorStatus);

#ifdef DONT_USE_INPLACE_IMAGES
	SetImagePixels1(shapeimage, attrib->width, attrib->height,
			attrib->pixelindex, mask_pixels);
#else /* DONT_USE_INPLACE_IMAGES */
	{
	    int dpth;
	    if (depth <= 8)
		dpth = 8;
	    else if (depth <= 16)
		dpth = 16;
	    else 
		dpth = 32;

	    SetAllocateImagePixelsXY(shapeimage, dpth,
		attrib->pixelindex, mask_pixels);
	}
#endif /* DONT_USE_INPLACE_IMAGES */
    }
    free(mask_pixels);

#ifndef DONT_USE_INPLACE_IMAGES
    goto DONE_WITH_SHAPEIMAGE;
    ALREADY_DID_SHAPEIMAGE:
#endif /* DONT_USE_INPLACE_IMAGES */

    if (attributes &&
	(attributes->valuemask & XpmReturnInfos
	 || attributes->valuemask & XpmReturnPixels)) {
	if (mask_pixel != UNDEF_PIXEL) {
	    Pixel *pixels, *p1, *p2;
	    unsigned int a;

	    attributes->npixels = attrib->ncolors - 1;
	    pixels = (Pixel *) malloc(sizeof(Pixel) * attributes->npixels);
	    if (pixels) {
		p1 = image_pixels;
		p2 = pixels;
		for (a = 0; a < attrib->ncolors; a++, p1++)
		    if (a != mask_pixel)
			*p2++ = *p1;
		attributes->pixels = pixels;
	    } else {
		/* if error just say we can't return requested data */
		attributes->valuemask &= ~XpmReturnPixels;
		attributes->valuemask &= ~XpmReturnInfos;
		attributes->pixels = NULL;
		attributes->npixels = 0;
	    }
	    free(image_pixels);
	} else {
	    attributes->pixels = image_pixels;
	    attributes->npixels = attrib->ncolors;
	}
	attributes->mask_pixel = mask_pixel;
    } else
	free(image_pixels);

    if (image_return)
	*image_return = image;

    if (shapeimage_return)
	*shapeimage_return = shapeimage;

    return (ErrorStatus);
}

#ifdef _NO_PROTO
_XmxpmDestroyImage(display, attrib, attributes)
    Display *display;
    xpmInternAttrib *attrib;
    XpmAttributes *attributes;
#else
_XmxpmDestroyImage(
    Display *display,
    xpmInternAttrib *attrib,
    XpmAttributes *attributes)
#endif
{
    int ErrorStatus;
    ErrorStatus = DestroyColors(display, attributes, attrib->colorTable, attrib->ncolors);
    return (ErrorStatus);
}

static int
#ifdef _NO_PROTO
CreateXImage(display, visual, depth, width, height, image_return)
    Display *display;
    Visual *visual;
    unsigned int depth;
    unsigned int width;
    unsigned int height;
    XImage **image_return;
#else
CreateXImage(
    Display *display,
    Visual *visual,
    unsigned int depth,
    unsigned int width,
    unsigned int height,
    XImage **image_return)
#endif
{
    int bitmap_pad;

    if (depth > 16)
	bitmap_pad = 32;
    else if (depth > 8)
	bitmap_pad = 16;
    else
	bitmap_pad = 8;

    *image_return = XCreateImage(display, visual, depth, ZPixmap, 0, 0,
				 width, height, bitmap_pad, 0);
    if (!*image_return)
	return (XpmNoMemory);

#ifdef DONT_USE_INPLACE_IMAGES
    (*image_return)->data =
	(char *) malloc((*image_return)->bytes_per_line * height);

    if (!(*image_return)->data) {
	XDestroyImage(*image_return);
	*image_return = NULL;
	return (XpmNoMemory);
    }
    memset((*image_return)->data,0,((*image_return)->bytes_per_line * height));
#else /* DONT_USE_INPLACE_IMAGES */
    (*image_return)->data = NULL;
#endif /* DONT_USE_INPLACE_IMAGES */
    return (XpmSuccess);
}

#ifdef DONT_USE_INPLACE_IMAGES
LFUNC(_putbits, void, (register char *src, int dstoffset,
		       register int numbits, register char *dst));

#endif /* DONT_USE_INPLACE_IMAGES */

#ifdef DONT_USE_INPLACE_IMAGES
static unsigned char Const _lomask[0x09] = {
		     0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
static unsigned char Const _himask[0x09] = {
		     0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};

static void
#ifdef _NO_PROTO
_putbits(src, dstoffset, numbits, dst)
    register char *src;			/* address of source bit string */
    int dstoffset;			/* bit offset into destination;
					 * range is 0-31 */
    register int numbits;		/* number of bits to copy to
					 * destination */
    register char *dst;			/* address of destination bit string */
#else
_putbits(
    register char *src,			/* address of source bit string */
    int dstoffset,			/* bit offset into destination;
					 * range is 0-31 */
    register int numbits,		/* number of bits to copy to
					 * destination */
    register char *dst)			/* address of destination bit string */
#endif
{
    register unsigned char chlo, chhi;
    int hibits;

    dst = dst + (dstoffset >> 3);
    dstoffset = dstoffset & 7;
    hibits = 8 - dstoffset;
    chlo = *dst & _lomask[dstoffset];
    for (;;) {
	chhi = (*src << dstoffset) & _himask[dstoffset];
	if (numbits <= hibits) {
	    chhi = chhi & _lomask[dstoffset + numbits];
	    *dst = (*dst & _himask[dstoffset + numbits]) | chlo | chhi;
	    break;
	}
	*dst = chhi | chlo;
	dst++;
	numbits = numbits - hibits;
	chlo = (unsigned char) (*src & _himask[hibits]) >> hibits;
	src++;
	if (numbits <= dstoffset) {
	    chlo = chlo & _lomask[numbits];
	    *dst = (*dst & _himask[numbits]) | chlo;
	    break;
	}
	numbits = numbits - dstoffset;
    }
}

static void
#ifdef _NO_PROTO
SetImagePixels(image, width, height, pixelindex, pixels)
    XImage *image;
    unsigned int width;
    unsigned int height;
    unsigned int *pixelindex;
    Pixel *pixels;
#else
SetImagePixels(
    XImage *image,
    unsigned int width,
    unsigned int height,
    unsigned int *pixelindex,
    Pixel *pixels)
#endif
{
    register char *src;
    register char *dst;
    register unsigned int *iptr;
    register int x, y, i;
    register char *data;
    Pixel pixel, px;
    int nbytes, depth, ibu, ibpp;

    data = image->data;
    iptr = pixelindex;
    depth = image->depth;
    if (image->depth == 1) {
	ibu = image->bitmap_unit;
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		pixel = pixels[*iptr];
		for (i = 0, px = pixel;
		     i < sizeof(unsigned long); i++, px >>= 8)
		    ((unsigned char *) &pixel)[i] = px;
		src = &data[XYINDEX(x, y, image)];
		dst = (char *) &px;
		px = 0;
		nbytes = ibu >> 3;
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
		XYNORMALIZE(&px, image);
		_putbits((char *) &pixel, (x % ibu), 1, (char *) &px);
		XYNORMALIZE(&px, image);
		src = (char *) &px;
		dst = &data[XYINDEX(x, y, image)];
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
	    }
    } else {
	ibpp = image->bits_per_pixel;
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		pixel = pixels[*iptr];
		if (depth == 4)
		    pixel &= 0xf;
		for (i = 0, px = pixel;
		     i < sizeof(unsigned long); i++, px >>= 8)
		    ((unsigned char *) &pixel)[i] = px;
		src = &data[ZINDEX(x, y, image)];
		dst = (char *) &px;
		px = 0;
		nbytes = (ibpp + 7) >> 3;
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
		ZNORMALIZE(&px, image);
		_putbits((char *) &pixel, (x * ibpp) & 7, ibpp, (char *) &px);
		ZNORMALIZE(&px, image);
		src = (char *) &px;
		dst = &data[ZINDEX(x, y, image)];
		for (i = nbytes; --i >= 0;)
		    *dst++ = *src++;
	    }
    }
}

#ifndef WORD64
static unsigned long byteorderpixel = MSBFirst << 24;
#endif

static void
#ifdef _NO_PROTO
SetImagePixels32(image, width, height, pixelindex, pixels)
    XImage *image;
    unsigned int width;
    unsigned int height;
    unsigned int *pixelindex;
    Pixel *pixels;
#else
SetImagePixels32(
    XImage *image,
    unsigned int width,
    unsigned int height,
    unsigned int *pixelindex,
    Pixel *pixels)
#endif
{
    register unsigned char *addr;
    register unsigned char *data;
    register unsigned int *iptr;
    register int x, y;
    Pixel pixel;

    data = (unsigned char *) image->data;
    iptr = pixelindex;
#ifndef WORD64
    if (*((char *) &byteorderpixel) == image->byte_order) {
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &data[ZINDEX32(x, y, image)];
		*((unsigned long *)addr) = pixels[*iptr];
	    }
    } else
#endif
    if (image->byte_order == MSBFirst)
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &data[ZINDEX32(x, y, image)];
		pixel = pixels[*iptr];
		addr[0] = pixel >> 24;
		addr[1] = pixel >> 16;
		addr[2] = pixel >> 8;
		addr[3] = pixel;
	    }
    else
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &data[ZINDEX32(x, y, image)];
		pixel = pixels[*iptr];
		addr[0] = pixel;
		addr[1] = pixel >> 8;
		addr[2] = pixel >> 16;
		addr[3] = pixel >> 24;
	    }
}

static void
#ifdef _NO_PROTO
SetImagePixels16(image, width, height, pixelindex, pixels)
    XImage *image;
    unsigned int width;
    unsigned int height;
    unsigned int *pixelindex;
    Pixel *pixels;
#else
SetImagePixels16(
    XImage *image,
    unsigned int width,
    unsigned int height,
    unsigned int *pixelindex,
    Pixel *pixels)
#endif
{
    register unsigned char *addr;
    register unsigned char *data;
    register unsigned int *iptr;
    register int x, y;

    data = (unsigned char *) image->data;
    iptr = pixelindex;
    if (image->byte_order == MSBFirst)
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &data[ZINDEX16(x, y, image)];
		addr[0] = pixels[*iptr] >> 8;
		addr[1] = pixels[*iptr];
	    }
    else
	for (y = 0; y < height; y++)
	    for (x = 0; x < width; x++, iptr++) {
		addr = &data[ZINDEX16(x, y, image)];
		addr[0] = pixels[*iptr];
		addr[1] = pixels[*iptr] >> 8;
	    }
}

static void
#ifdef _NO_PROTO
SetImagePixels8(image, width, height, pixelindex, pixels)
    XImage *image;
    unsigned int width;
    unsigned int height;
    unsigned int *pixelindex;
    Pixel *pixels;
#else
SetImagePixels8(
    XImage *image,
    unsigned int width,
    unsigned int height,
    unsigned int *pixelindex,
    Pixel *pixels)
#endif
{
    register char *data;
    register unsigned int *iptr;
    register int x, y;

    data = image->data;
    iptr = pixelindex;
    for (y = 0; y < height; y++)
	for (x = 0; x < width; x++, iptr++)
	    data[ZINDEX8(x, y, image)] = pixels[*iptr];
}

static void
#ifdef _NO_PROTO
SetImagePixels1(image, width, height, pixelindex, pixels)
    XImage *image;
    unsigned int width;
    unsigned int height;
    unsigned int *pixelindex;
    Pixel *pixels;
#else
SetImagePixels1(
    XImage *image,
    unsigned int width,
    unsigned int height,
    unsigned int *pixelindex,
    Pixel *pixels)
#endif
{
    register unsigned int *iptr;
    register int x, y;
    register char *data;

    if (image->byte_order != image->bitmap_bit_order)
	SetImagePixels(image, width, height, pixelindex, pixels);
    else {
	data = image->data;
	iptr = pixelindex;
	if (image->bitmap_bit_order == MSBFirst)
	    for (y = 0; y < height; y++)
		for (x = 0; x < width; x++, iptr++) {
		    if (pixels[*iptr] & 1)
			data[ZINDEX1(x, y, image)] |= 0x80 >> (x & 7);
		    else
			data[ZINDEX1(x, y, image)] &= ~(0x80 >> (x & 7));
		}
	else
	    for (y = 0; y < height; y++)
		for (x = 0; x < width; x++, iptr++) {
		    if (pixels[*iptr] & 1)
			data[ZINDEX1(x, y, image)] |= 1 << (x & 7);
		    else
			data[ZINDEX1(x, y, image)] &= ~(1 << x & 7);
		}
    }
}

#else /* DONT_USE_INPLACE_IMAGES */

static void
SetAllocateImagePixelsXY(XImage *image, unsigned int pi_depth,
    unsigned int *pixelindex, Pixel *pixels)
{
    /*
     * This is a bitmap unit of 8.
     */
    image->data = (char *) malloc (image->bytes_per_line * image->height);

    SetImagePixelsXY(image, pi_depth, pixelindex, pixels);
}

static void
SetImagePixelsXY(XImage *image, unsigned int pi_depth,
    unsigned int *pixelindex, Pixel *pixels)
{
    int x, start_y_line, total_y_lines, chunk, total_chunks;
    int final_piece;

    total_y_lines = image->height * image->bytes_per_line;
    total_chunks = image->width >> 0x3;
    final_piece = total_chunks << 0x3;

    if (image->bitmap_bit_order == MSBFirst)
    {
	/*
	 * This code looks really long, but it is just 3 copies
	 * of the same thing, with px being the correct size
	 * for the pixel
	 */
	switch (pi_depth) {
	case 8:
	    /*
	     * pixelindex is really an array of unsigned char's
	     */
	    {
		unsigned char *px = (unsigned char *) pixelindex;

		for (start_y_line = 0; start_y_line < total_y_lines;
		    start_y_line += image->bytes_per_line)
		{
		    for (chunk = 0; chunk < total_chunks;
			chunk++, px += 8)
		    {
			/*
			 * Do the full bytes as chunks
			 */
			image->data[start_y_line + chunk] =
			    ((pixels[*(px)] & 0x1) << 7)
			    | ((pixels[*(px + 1)] & 0x1) << 6)
			    | ((pixels[*(px + 2)] & 0x1) << 5)
			    | ((pixels[*(px + 3)] & 0x1) << 4)
			    | ((pixels[*(px + 4)] & 0x1) << 3)
			    | ((pixels[*(px + 5)] & 0x1) << 2)
			    | ((pixels[*(px + 6)] & 0x1) << 1)
			    | (pixels[*(px + 7)] & 0x1);
		    }

		    if (final_piece < image->width) {
			image->data[start_y_line + total_chunks] = 0;
			for (x = final_piece; x < image->width; x++, px++) {
			    if (pixels[*px] & 1)
				image->data[start_y_line + total_chunks]
				    |= 0x80 >> (x & 7);
			}
		    }
		}
	    } 
	    break;
	case 16:
	    /*
	     * pixelindex is really an array of unsigned short's
	     */
	    {
		unsigned short *px = (unsigned short *) pixelindex;

		for (start_y_line = 0; start_y_line < total_y_lines;
		    start_y_line += image->bytes_per_line)
		{
		    for (chunk = 0; chunk < total_chunks;
			chunk++, px += 8)
		    {
			/*
			 * Do the full bytes as chunks
			 */
			image->data[start_y_line + chunk] =
			    ((pixels[*(px)] & 0x1) << 7)
			    | ((pixels[*(px + 1)] & 0x1) << 6)
			    | ((pixels[*(px + 2)] & 0x1) << 5)
			    | ((pixels[*(px + 3)] & 0x1) << 4)
			    | ((pixels[*(px + 4)] & 0x1) << 3)
			    | ((pixels[*(px + 5)] & 0x1) << 2)
			    | ((pixels[*(px + 6)] & 0x1) << 1)
			    | (pixels[*(px + 7)] & 0x1);
		    }

		    if (final_piece < image->width) {
			image->data[start_y_line + total_chunks] = 0;
			for (x = final_piece; x < image->width; x++, px++) {
			    if (pixels[*px] & 1)
				image->data[start_y_line + total_chunks]
				    |= 0x80 >> (x & 7);
			}
		    }
		}
	    } 
	    break;
	case 32:
	    /*
	     * pixelindex is ok
	     */
	    {
		unsigned int *px = pixelindex;

		for (start_y_line = 0; start_y_line < total_y_lines;
		    start_y_line += image->bytes_per_line)
		{
		    for (chunk = 0; chunk < total_chunks;
			chunk++, px += 8)
		    {
			/*
			 * Do the full bytes as chunks
			 */
			image->data[start_y_line + chunk] =
			    ((pixels[*(px)] & 0x1) << 7)
			    | ((pixels[*(px + 1)] & 0x1) << 6)
			    | ((pixels[*(px + 2)] & 0x1) << 5)
			    | ((pixels[*(px + 3)] & 0x1) << 4)
			    | ((pixels[*(px + 4)] & 0x1) << 3)
			    | ((pixels[*(px + 5)] & 0x1) << 2)
			    | ((pixels[*(px + 6)] & 0x1) << 1)
			    | (pixels[*(px + 7)] & 0x1);
		    }

		    if (final_piece < image->width) {
			image->data[start_y_line + total_chunks] = 0;
			for (x = final_piece; x < image->width; x++, px++) {
			    if (pixels[*px] & 1)
				image->data[start_y_line + total_chunks]
				    |= 0x80 >> (x & 7);
			}
		    }
		}
	    } 
	    break;
	}
    } else
    {
	switch (pi_depth) {
	case 8:
	    /*
	     * pixelindex is really an array of unsigned char's
	     */
	    {
		unsigned char *px = (unsigned char *) pixelindex;

		for (start_y_line = 0; start_y_line < total_y_lines;
		    start_y_line += image->bytes_per_line)
		{
		    for (chunk = 0; chunk < total_chunks;
			chunk++, px += 8)
		    {
			/*
			 * Do the full bytes as chunks
			 */
			image->data[start_y_line + chunk] =
			    (pixels[*(px)] & 0x1)
			    | ((pixels[*(px + 1)] & 0x1) << 1)
			    | ((pixels[*(px + 2)] & 0x1) << 2)
			    | ((pixels[*(px + 3)] & 0x1) << 3)
			    | ((pixels[*(px + 4)] & 0x1) << 4)
			    | ((pixels[*(px + 5)] & 0x1) << 5)
			    | ((pixels[*(px + 6)] & 0x1) << 6)
			    | ((pixels[*(px + 7)] & 0x1) << 7);
		    }

		    if (final_piece < image->width) {
			image->data[start_y_line + total_chunks] = 0;
			for (x = final_piece; x < image->width; x++, px++) {
			    if (pixels[*px] & 1)
				image->data[start_y_line + total_chunks]
				    |= 1 << (x & 7);
			}
		    }
		}
	    }
	    break;
	case 16:
	    /*
	     * pixelindex is really an array of unsigned short's
	     */
	    {
		unsigned short *px = (unsigned short *) pixelindex;

		for (start_y_line = 0; start_y_line < total_y_lines;
		    start_y_line += image->bytes_per_line)
		{
		    for (chunk = 0; chunk < total_chunks;
			chunk++, px += 8)
		    {
			/*
			 * Do the full bytes as chunks
			 */
			image->data[start_y_line + chunk] =
			    (pixels[*(px)] & 0x1)
			    | ((pixels[*(px + 1)] & 0x1) << 1)
			    | ((pixels[*(px + 2)] & 0x1) << 2)
			    | ((pixels[*(px + 3)] & 0x1) << 3)
			    | ((pixels[*(px + 4)] & 0x1) << 4)
			    | ((pixels[*(px + 5)] & 0x1) << 5)
			    | ((pixels[*(px + 6)] & 0x1) << 6)
			    | ((pixels[*(px + 7)] & 0x1) << 7);
		    }

		    if (final_piece < image->width) {
			image->data[start_y_line + total_chunks] = 0;
			for (x = final_piece; x < image->width; x++, px++) {
			    if (pixels[*px] & 1)
				image->data[start_y_line + total_chunks]
				    |= 1 << (x & 7);
			}
		    }
		}
	    }
	    break;
	case 32:
	    /*
	     * pixelindex is ok
	     */
	    {
		unsigned int *px = pixelindex;

		for (start_y_line = 0; start_y_line < total_y_lines;
		    start_y_line += image->bytes_per_line)
		{
		    for (chunk = 0; chunk < total_chunks;
			chunk++, px += 8)
		    {
			/*
			 * Do the full bytes as chunks
			 */
			image->data[start_y_line + chunk] =
			    (pixels[*(px)] & 0x1)
			    | ((pixels[*(px + 1)] & 0x1) << 1)
			    | ((pixels[*(px + 2)] & 0x1) << 2)
			    | ((pixels[*(px + 3)] & 0x1) << 3)
			    | ((pixels[*(px + 4)] & 0x1) << 4)
			    | ((pixels[*(px + 5)] & 0x1) << 5)
			    | ((pixels[*(px + 6)] & 0x1) << 6)
			    | ((pixels[*(px + 7)] & 0x1) << 7);
		    }

		    if (final_piece < image->width) {
			image->data[start_y_line + total_chunks] = 0;
			for (x = final_piece; x < image->width; x++, px++) {
			    if (pixels[*px] & 1)
				image->data[start_y_line + total_chunks]
				    |= 1 << (x & 7);
			}
		    }
		}
	    }
	    break;
	}
    }
}

static void
SetImagePixelsZ(XImage *image, unsigned int pi_depth,
    unsigned int *pixelindex, Pixel *pixels)
{
/*
 * WARNING! This code is intended only for Sparc and x86 Solaris
 * platforms in that it only handles MSB byte-MSB bit (sparc) or
 * LSB byte-MSB bit (x86) integers.
 */
    int x;
    int total;

    total = image->height * image->width;

    if (image->data && pixelindex) {
	switch (pi_depth) {
	case 8:
	    {
		unsigned char *c_pi = (unsigned char *) pixelindex;
		unsigned char *c_dt = (unsigned char *) image->data;

		for (x = 0; x < total; x++)
		    *(c_dt++) = (unsigned char) pixels[*(c_pi++)];
	    }
	    break;
	case 16:
	    {
		unsigned short *s_pi = (unsigned short *) pixelindex;
		unsigned short *s_dt = (unsigned short *) image->data;

#if defined(i386)   /* LSB machine */
		/*
		 * We're a LSB machine displaying on an MSB machine,
		 * so byte swap
		 */
		if (image->byte_order == MSBFirst) {
#else /* MSB machine */
		/*
		 * We're a MSB machine displaying on an LSB machine
		 */
		if (image->byte_order == LSBFirst) {
#endif /* MSB machine */
		    for (x = 0; x < total; x++) {
			*s_dt = (unsigned short) pixels[*(s_pi++)];
			*s_dt = ((*s_dt >> 8) & 0xff)
			    | ((*s_dt << 8) & 0xff00);
			s_dt++;
		    }
		} else {
		    for (x = 0; x < total; x++)
			*(s_dt++) = (unsigned short) pixels[*(s_pi++)];
		}
	    }
	    break;
	case 32:
	    {
		unsigned int *i_pi = (unsigned int *) pixelindex;
		unsigned int *i_dt = (unsigned int *) image->data;

#if defined(i386) /* LSB machine */
		if (image->byte_order == MSBFirst) {
#else /* MSB machine */
		if (image->byte_order == LSBFirst) {
#endif /* MSB machine */
		    for (x = 0; x < total; x++) {
			*i_dt = pixels[*(i_pi++)];
			*i_dt = ((*i_dt >> 24) & 0xff)
			    | ((*i_dt >> 8) & 0xff00)
			    | ((*i_dt << 8) & 0xff0000)
			    | (*i_dt << 24);
			i_dt++;
		    }
		} else {
		    for (x = 0; x < total; x++)
			*(i_dt++) = pixels[*(i_pi++)];
		}
	    }
	    break;
	}
    }
}
#endif /* DONT_USE_INPLACE_IMAGES */
