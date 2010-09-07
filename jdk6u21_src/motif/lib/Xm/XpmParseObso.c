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
static char rcsid[] = "$XConsortium: XpmParse.c /main/cde1_maint/3 1995/10/05 12:16:42 lehors $"
#endif
#endif
/* Copyright 1990-92 GROUPE BULL -- See license conditions in file COPYRIGHT */
/*****************************************************************************\
 * parse.c:                                                                  *
 *  XPM library                                                              *
 *  Parse an XPM file or array and store the found informations              *
 *  in an an xpmInternAttrib structure which is returned.                    *
 *  Developed by Arnaud Le Hors                                              *
 *  Added for backward compatibility with 1.2 API			     *
\*****************************************************************************/

#include <Xm/XpmI.h>
#ifdef VMS
#include "sys$library:ctype.h"
#else
#include <ctype.h>
#endif

LFUNC(ParseValues, int, (xpmData *data, unsigned int *width,
			 unsigned int *height, unsigned int *ncolors,
			 unsigned int *cpp,  unsigned int *x_hotspot,
			 unsigned int *y_hotspot, unsigned int *hotspot,
			 unsigned int *extensions));

LFUNC(ParseColors, int, (xpmData *data, unsigned int ncolors, unsigned int cpp,
			 char ****colorTablePtr, xpmHashTable *hashtable));

LFUNC(ParsePixels, int, (xpmData *data, unsigned int width, 
			 unsigned int height, unsigned int ncolors,
			 unsigned int cpp, char ***colorTable,
			 xpmHashTable *hashtable, unsigned int **pixels,
			 XpmAttributes *attributes));

LFUNC(ParseExtensions, int, (xpmData *data, XpmExtension **extensions,
			     unsigned int *nextensions));

char *_XmxpmColorKeys[] =
{
 "s",					/* key #1: symbol */
 "m",					/* key #2: mono visual */
 "g4",					/* key #3: 4 grays visual */
 "g",					/* key #4: gray visual */
 "c",					/* key #5: color visual */
};

#undef RETURN
#define RETURN(status) \
  { if (colorTable) _XmxpmFreeColorTable(colorTable, ncolors); \
    if (pixelindex) free(pixelindex); \
    if (hints_cmt)  free(hints_cmt); \
    if (colors_cmt) free(colors_cmt); \
    if (pixels_cmt) free(pixels_cmt); \
    return(status); }

int
_XmxpmParseData(data, attrib_return, attributes)
    xpmData *data;
    xpmInternAttrib *attrib_return;
    XpmAttributes *attributes;
{
    /* variables to return */
    unsigned int width, height, ncolors, cpp;
    unsigned int x_hotspot, y_hotspot, hotspot = 0, extensions = 0;
    char ***colorTable = NULL;
    unsigned int *pixelindex = NULL;
    char *hints_cmt = NULL;
    char *colors_cmt = NULL;
    char *pixels_cmt = NULL;

    int ErrorStatus;
    xpmHashTable hashtable;

    ErrorStatus = ParseValues(data, &width, &height, &ncolors, &cpp,
			      &x_hotspot, &y_hotspot, &hotspot, &extensions);
    if (ErrorStatus != XpmSuccess)
	return(ErrorStatus);

    if (attributes && (attributes->valuemask & XpmReturnInfos))
	_XmxpmGetCmt(data, &hints_cmt);

    if (USE_HASHTABLE) {
	ErrorStatus = _XmxpmHashTableInit(&hashtable);
	if (ErrorStatus != XpmSuccess)
	    return(ErrorStatus);
    }
    
    ErrorStatus = ParseColors(data, ncolors, cpp, &colorTable, &hashtable);
    if (ErrorStatus != XpmSuccess)
	RETURN(ErrorStatus);

    if (attributes && (attributes->valuemask & XpmReturnInfos))
	_XmxpmGetCmt(data, &colors_cmt);

    ErrorStatus = ParsePixels(data, width, height, ncolors, cpp, colorTable,
			      &hashtable, &pixelindex, attributes);

    if (USE_HASHTABLE)
	_XmxpmHashTableFree(&hashtable);
    
    if (ErrorStatus != XpmSuccess)
	RETURN(ErrorStatus);

    if (attributes && (attributes->valuemask & XpmReturnInfos))
	_XmxpmGetCmt(data, &pixels_cmt);

    if (attributes && (attributes->valuemask & XpmReturnExtensions)) 
	if (extensions) {
	    ErrorStatus = ParseExtensions(data, &attributes->extensions,
					  &attributes->nextensions);
	    if (ErrorStatus != XpmSuccess)
		RETURN(ErrorStatus);
	} else {
	    attributes->extensions = NULL;
	    attributes->nextensions = 0;
	}

    attrib_return->width = width;
    attrib_return->height = height;
    attrib_return->cpp = cpp;
    attrib_return->ncolors = ncolors;
    attrib_return->colorTable = colorTable;
    attrib_return->pixelindex = pixelindex;

    if (attributes) {
	if (attributes->valuemask & XpmReturnInfos) {
	    attributes->hints_cmt = hints_cmt;
	    attributes->colors_cmt = colors_cmt;
	    attributes->pixels_cmt = pixels_cmt;
	}
	if (hotspot) {
	    attributes->x_hotspot = x_hotspot;
	    attributes->y_hotspot = y_hotspot;
	    attributes->valuemask |= XpmHotspot;
	}
    }
    return (XpmSuccess);
}

static int
ParseValues(data, width, height, ncolors, cpp,
	    x_hotspot, y_hotspot, hotspot, extensions)
    xpmData *data;
    unsigned int *width, *height, *ncolors, *cpp;
    unsigned int *x_hotspot, *y_hotspot,  *hotspot;
    unsigned int *extensions;
{
    unsigned int l;
    char buf[BUFSIZ];

    /*
     * read values: width, height, ncolors, chars_per_pixel 
     */
    if (!(_XmxpmNextUI(data, width) && _XmxpmNextUI(data, height)
	  && _XmxpmNextUI(data, ncolors) && _XmxpmNextUI(data, cpp)))
	return(XpmFileInvalid);

    /*
     * read optional information (hotspot and/or XPMEXT) if any 
     */
    l = _XmxpmNextWord(data, buf, BUFSIZ);
    if (l) {
	*extensions = l == 6 && !strncmp("XPMEXT", buf, 6);
	if (*extensions)
	    *hotspot = _XmxpmNextUI(data, x_hotspot)
		&& _XmxpmNextUI(data, y_hotspot);
	else {
	    *hotspot = _XmXpmatoui(buf, l, x_hotspot) && _XmxpmNextUI(data, y_hotspot);
	    l = _XmxpmNextWord(data, buf, BUFSIZ);
	    *extensions = l == 6 && !strncmp("XPMEXT", buf, 6);
	}
    }
    return (XpmSuccess);
}

static int
ParseColors(data, ncolors, cpp, colorTablePtr, hashtable)
    xpmData *data;
    unsigned int ncolors;
    unsigned int cpp;
    char ****colorTablePtr;		/* Jee, that's something! */
    xpmHashTable *hashtable;
{
    unsigned int key, l, a, b;
    unsigned int curkey;		/* current color key */
    unsigned int lastwaskey;		/* key read */
    char buf[BUFSIZ];
    char curbuf[BUFSIZ];		/* current buffer */
    char ***ct, **cts, **sptr, *s;
    char ***colorTable;
    int ErrorStatus;

    colorTable = (char ***) calloc(ncolors, sizeof(char **));
    if (!colorTable)
	return(XpmNoMemory);

    for (a = 0, ct = colorTable; a < ncolors; a++, ct++) {
	_XmxpmNextString(data);		/* skip the line */
	cts = *ct = (char **) calloc((NKEYS + 1), sizeof(char *));
	if (!cts) {
	    _XmxpmFreeColorTable(colorTable, ncolors);
	    return(XpmNoMemory);
	}

	/*
	 * read the character(s) used in the data for this color
	 *
	 * Here are some sample color definitions:
	 *   "       s none  m none  c none",
	 *   ".    s iconGray1     m white c #dededededede",
	 *
	 * so *cts is going to be "." for the second one.
	 */
	*cts = (char *) malloc(cpp + 1); /* + 1 for null terminated */
	if (!*cts) {
	    _XmxpmFreeColorTable(colorTable, ncolors);
	    return(XpmNoMemory);
	}
	for (b = 0, s = *cts; b < cpp; b++, s++)
	    *s = _XmxpmGetC(data);
	*s = '\0';

	/*
	 * store the string in the hashtable with its color index number
	 *
	 * We don't use the hashtable in ParseData() unless we
	 * have more than 2 characters per pixel
	 */
	if (USE_HASHTABLE && (cpp > 2)) {
	    ErrorStatus = _XmxpmHashIntern(hashtable, *cts, HashAtomData(a));
	    if (ErrorStatus != XpmSuccess) {
		_XmxpmFreeColorTable(colorTable, ncolors);
		return(ErrorStatus);
	    }
	}

	/*
	 * read color keys and values 
	 */
	curkey = 0;
	lastwaskey = 0;
	while (l = _XmxpmNextWord(data, buf, BUFSIZ)) {
	    if (!lastwaskey) {
		for (key = 0, sptr = _XmxpmColorKeys; key < NKEYS; key++, sptr++)
		    if ((strlen(*sptr) == l) && (!strncmp(*sptr, buf, l)))
			break;
	    }
	    if (!lastwaskey && key < NKEYS) { /* open new key */
		if (curkey) {		/* flush string */
		    s = cts[curkey] = (char *) malloc(strlen(curbuf) + 1);
		    if (!s) {
			_XmxpmFreeColorTable(colorTable, ncolors);
			return(XpmNoMemory);
		    }
		    strcpy(s, curbuf);
		}
		curkey = key + 1;	/* set new key  */
		*curbuf = '\0';		/* reset curbuf */
		lastwaskey = 1;
	    } else {
		if (!curkey) {		/* key without value */
		    _XmxpmFreeColorTable(colorTable, ncolors);
		    return(XpmFileInvalid);
		}
		if (!lastwaskey)
		    strcat(curbuf, " "); /* append space */
		buf[l] = '\0';
		strcat(curbuf, buf);	/* append buf */
		lastwaskey = 0;
	    }
	}
	if (!curkey) {			/* key without value */
	    _XmxpmFreeColorTable(colorTable, ncolors);
	    return(XpmFileInvalid);
	}
	s = cts[curkey] = (char *) malloc(strlen(curbuf) + 1);
	if (!s) {
	    _XmxpmFreeColorTable(colorTable, ncolors);
	    return(XpmNoMemory);
	}
	strcpy(s, curbuf);
    }
    *colorTablePtr = colorTable;
    return(XpmSuccess);
}

static int
ParsePixels(data, width, height, ncolors, cpp, colorTable, hashtable, pixels,
    attributes)
    xpmData *data;
    unsigned int width;
    unsigned int height;
    unsigned int ncolors;
    unsigned int cpp;
    char ***colorTable;
    xpmHashTable *hashtable;
    unsigned int **pixels;
    XpmAttributes *attributes;
{
    unsigned int *iptr, *iptr2;
    unsigned int a, x, y;
#ifndef DONT_USE_INPLACE_IMAGES
    unsigned int bitmap_pad_bytes;
    unsigned char *c_iptr = NULL, *c_iptr2 = NULL;
    unsigned short *s_iptr = NULL, *s_iptr2 = NULL;

    iptr = iptr2 = NULL;
#endif /* DONT_USE_INPLACE_IMAGES */

#ifdef DONT_USE_INPLACE_IMAGES
    iptr2 = (unsigned int *) malloc(sizeof(unsigned int) * width * height);

    if (!iptr2)
	return(XpmNoMemory);

    iptr = iptr2;
#else /* DONT_USE_INPLACE_IMAGES */
    {
	/*
	 * We're going to need an XImage->data block of pixels that has
	 * depth of whatever the drawable is. Currently, we only
	 * use the DefaultDepth of the display, so we know this depth
	 * now and can therefore reuse the space allocated to 
	 * xpmInternAttrib->pixelindex (*pixels) as the XImage->data
	 * later (see XpmCreate.c).
	 *
	 * The original code and this code have problems when the .pm
	 * has more colors than the display is deep. The CreateColors()
	 * call in _XmxpmCreateImage() fails (and as a side effect
	 * it hogs the rest of the colormap).  Since XpmCreate has all
	 * the image reduction code, we'll allocate a block that is too
	 * big and let that code deal with it, if it ever does.
	 */
	extern Display * _XmGetDefaultDisplay( void ); /* VendorSP.h */
	Display *dpy = _XmGetDefaultDisplay ();
	int dpth = 0;

        if (attributes && (attributes->valuemask & XpmDepth)) {
	    dpth = attributes->depth;
	} else {
	    dpth = DefaultDepth (dpy, DefaultScreen (dpy));
	}

	if ((dpth <= 8) && (ncolors <= 0xff)) {
	    bitmap_pad_bytes = sizeof (unsigned char);
	    c_iptr = c_iptr2 = (unsigned char *) malloc (bitmap_pad_bytes
		* width * height);
    
	    if (!c_iptr2)
		return(XpmNoMemory);
	} else if ((dpth <= 16) && (ncolors <= 0xffff)) {
	    bitmap_pad_bytes = sizeof (unsigned short);
	    s_iptr = s_iptr2 = (unsigned short *) malloc (bitmap_pad_bytes
		* width * height);

	    if (!s_iptr2)
		return(XpmNoMemory);
	} else {
	    bitmap_pad_bytes = sizeof (unsigned int);
	    iptr = iptr2 = (unsigned int *) malloc (bitmap_pad_bytes
		* width * height);

	    if (!iptr2)
		return(XpmNoMemory);
	}
    }
#endif /* DONT_USE_INPLACE_IMAGES */


    switch (cpp) {

       case (1): /* Optimize for single character colors */
       {
	   unsigned short colidx[256];

#ifndef SVR4
	   bzero((char *) colidx, 256 * sizeof(short));
#else
	   memset(colidx, 0, 256 * sizeof(short));
#endif

	   for (a = 0; a < ncolors; a++)
	       colidx[ colorTable[a][0][0] ] = a + 1;
                
#ifdef DONT_USE_INPLACE_IMAGES
	   for (y = 0; y < height; y++) 
	   {
	       _XmxpmNextString(data);
	       for (x = 0; x < width; x++, iptr++)
	       {
		   int idx = colidx[_XmxpmGetC(data)];
		   if ( idx != 0 )
		       *iptr = idx - 1;
		   else {
		       free(iptr2);
		       return(XpmFileInvalid);
		   }
	       }
	   }
#else /* DONT_USE_INPLACE_IMAGES */
	   /*
	    * DO NOT COMBINE THESE STATEMENTS! We don't want the switch
	    * to get hit on every single pixel.
	    */
	   switch (bitmap_pad_bytes) {
	   case 1:
	       for (y = 0; y < height; y++) 
	       {
		   _XmxpmNextString(data);
		   for (x = 0; x < width; x++, c_iptr++)
		   {
		       int idx = colidx[_XmxpmGetC(data)];
		       if ( idx != 0 )
			   *c_iptr = (unsigned char) idx - 1;
		       else {
			   free(c_iptr2);
			   return(XpmFileInvalid);
		       }
		   }
	       }
	       break;
	    case 2:
	       for (y = 0; y < height; y++) 
	       {
		   _XmxpmNextString(data);
		   for (x = 0; x < width; x++, s_iptr++)
		   {
		       int idx = colidx[_XmxpmGetC(data)];
		       if ( idx != 0 )
			   *s_iptr = (unsigned short) idx - 1;
		       else {
			   free(s_iptr2);
			   return(XpmFileInvalid);
		       }
		   }
	       }
	       break;
	    case 4:
	       for (y = 0; y < height; y++) 
	       {
		   _XmxpmNextString(data);
		   for (x = 0; x < width; x++, iptr++)
		   {
		       int idx = colidx[_XmxpmGetC(data)];
		       if ( idx != 0 )
			   *iptr = idx - 1;
		       else {
			   free(iptr2);
			   return(XpmFileInvalid);
		       }
		   }
	       }
	       break;
	    }
#endif /* DONT_USE_INPLACE_IMAGES */
       }
       break;

       case (2): /* Optimize for double character colors */
       {
	   unsigned short cidx[256][256];

#ifndef SVR4
	   bzero((char *)cidx, 256 * 256 * sizeof(short));
#else
	   memset(cidx, 0, 256 * 256 * sizeof(short));
#endif


	   for (a = 0; a < ncolors; a++)
	       cidx [ colorTable[a][0][0] ][ colorTable[a][0][1] ] = a + 1;

#ifdef DONT_USE_INPLACE_IMAGES
	   for (y = 0; y < height; y++) 
	   {
	       _XmxpmNextString(data);
	       for (x = 0; x < width; x++, iptr++)
	       {
		   int cc1 = _XmxpmGetC(data);
		   int idx = cidx[cc1][ _XmxpmGetC(data) ];
		   if ( idx != 0 )
		       *iptr = idx - 1;
		   else {
		       free(iptr2);
		       return(XpmFileInvalid);
		   }
	       }
	   }
#else /* !DONT_USE_INPLACE_IMAGES */
	   switch (bitmap_pad_bytes) {
	   case 1:
	       /*
		* Yes, this is possible and likely (2 cpp fitting in an 8bit
		* image)
		*/
	       for (y = 0; y < height; y++) 
	       {
		   _XmxpmNextString(data);
		   for (x = 0; x < width; x++, c_iptr++)
		   {
		       int cc1 = _XmxpmGetC(data);
		       int idx = cidx[cc1][ _XmxpmGetC(data) ];
		       if ( idx != 0 )
			   *c_iptr = (unsigned char) idx - 1;
		       else {
			   free(c_iptr2);
			   return(XpmFileInvalid);
		       }
		   }
	       }
	       break;
	    case 2:
	       for (y = 0; y < height; y++) 
	       {
		   _XmxpmNextString(data);
		   for (x = 0; x < width; x++, s_iptr++)
		   {
		       int cc1 = _XmxpmGetC(data);
		       int idx = cidx[cc1][ _XmxpmGetC(data) ];
		       if ( idx != 0 )
			   *s_iptr = (unsigned short) idx - 1;
		       else {
			   free(s_iptr2);
			   return(XpmFileInvalid);
		       }
		   }
	       }
	       break;
	    case 4:
	       for (y = 0; y < height; y++) 
	       {
		   _XmxpmNextString(data);
		   for (x = 0; x < width; x++, iptr++)
		   {
		       int cc1 = _XmxpmGetC(data);
		       int idx = cidx[cc1][ _XmxpmGetC(data) ];
		       if ( idx != 0 )
			   *iptr = idx - 1;
		       else {
			   free(iptr2);
			   return(XpmFileInvalid);
		       }
		   }
	       }
	    }
#endif /* DONT_USE_INPLACE_IMAGES */
       }
       break;

       default : /* Non-optimized case of long color names */
       {
	   char *s;
	   char buf[BUFSIZ];

	   buf[cpp] = '\0';
	   if (USE_HASHTABLE) {
	       xpmHashAtom *slot;

#ifdef DONT_USE_INPLACE_IMAGES
	       for (y = 0; y < height; y++) {
		   _XmxpmNextString(data);
		   for (x = 0; x < width; x++, iptr++) {
		       for (a = 0, s = buf; a < cpp; a++, s++)
			   *s = _XmxpmGetC(data);
		       slot = _XmxpmHashSlot(hashtable, buf);
		       if (!*slot) {		/* no color matches */
			   free(iptr2);
			   return(XpmFileInvalid);
		       }
		       *iptr = HashColorIndex(slot);
		   }
	       }
#else /* !DONT_USE_INPLACE_IMAGES */
	       switch (bitmap_pad_bytes) {
	       case 1:
		   for (y = 0; y < height; y++) {
		       _XmxpmNextString(data);
		       for (x = 0; x < width; x++, c_iptr++) {
			   for (a = 0, s = buf; a < cpp; a++, s++)
			       *s = _XmxpmGetC(data);
			   slot = _XmxpmHashSlot(hashtable, buf);
			   if (!*slot) {		/* no color matches */
			       free(c_iptr2);
			       return(XpmFileInvalid);
			   }
			   *c_iptr = (unsigned char) HashColorIndex(slot);
		       }
		   }
		   break;
	       case 2:
		   for (y = 0; y < height; y++) {
		       _XmxpmNextString(data);
		       for (x = 0; x < width; x++, s_iptr++) {
			   for (a = 0, s = buf; a < cpp; a++, s++)
			       *s = _XmxpmGetC(data);
			   slot = _XmxpmHashSlot(hashtable, buf);
			   if (!*slot) {		/* no color matches */
			       free(s_iptr2);
			       return(XpmFileInvalid);
			   }
			   *s_iptr = (unsigned short) HashColorIndex(slot);
		       }
		   }
		   break;
	       case 4:
		   for (y = 0; y < height; y++) {
		       _XmxpmNextString(data);
		       for (x = 0; x < width; x++, iptr++) {
			   for (a = 0, s = buf; a < cpp; a++, s++)
			       *s = _XmxpmGetC(data);
			   slot = _XmxpmHashSlot(hashtable, buf);
			   if (!*slot) {		/* no color matches */
			       free(iptr2);
			       return(XpmFileInvalid);
			   }
			   *iptr = HashColorIndex(slot);
		       }
		   }
		   break;
	       }
#endif /* DONT_USE_INPLACE_IMAGES */
	   } else {
#ifdef DONT_USE_INPLACE_IMAGES
	       for (y = 0; y < height; y++) {
		   _XmxpmNextString(data);
		   for (x = 0; x < width; x++, iptr++) {
		       for (a = 0, s = buf; a < cpp; a++, s++)
			   *s = _XmxpmGetC(data);
		       for (a = 0; a < ncolors; a++)
			   if (!strcmp(colorTable[a][0], buf))
			       break;
		       if (a == ncolors) {	/* no color matches */
			   free(iptr2);
			   return(XpmFileInvalid);
		       }
		       *iptr = a;
		   }
	       }
#else /* !DONT_USE_INPLACE_IMAGES */
	       switch (bitmap_pad_bytes) {
	       case 1:
		   for (y = 0; y < height; y++) {
		       _XmxpmNextString(data);
		       for (x = 0; x < width; x++, c_iptr++) {
			   for (a = 0, s = buf; a < cpp; a++, s++)
			       *s = _XmxpmGetC(data);
			   for (a = 0; a < ncolors; a++)
			       if (!strcmp(colorTable[a][0], buf))
				   break;
			   if (a == ncolors) {	/* no color matches */
			       free(c_iptr2);
			       return(XpmFileInvalid);
			   }
			   *c_iptr = (unsigned char) a;
		       }
		   }
		   break;
	       case 2:
		   for (y = 0; y < height; y++) {
		       _XmxpmNextString(data);
		       for (x = 0; x < width; x++, s_iptr++) {
			   for (a = 0, s = buf; a < cpp; a++, s++)
			       *s = _XmxpmGetC(data);
			   for (a = 0; a < ncolors; a++)
			       if (!strcmp(colorTable[a][0], buf))
				   break;
			   if (a == ncolors) {	/* no color matches */
			       free(s_iptr2);
			       return(XpmFileInvalid);
			   }
			   *s_iptr = (unsigned short) a;
		       }
		   }
		   break;
	       case 4:
		   for (y = 0; y < height; y++) {
		       _XmxpmNextString(data);
		       for (x = 0; x < width; x++, iptr++) {
			   for (a = 0, s = buf; a < cpp; a++, s++)
			       *s = _XmxpmGetC(data);
			   for (a = 0; a < ncolors; a++)
			       if (!strcmp(colorTable[a][0], buf))
				   break;
			   if (a == ncolors) {	/* no color matches */
			       free(iptr2);
			       return(XpmFileInvalid);
			   }
			   *iptr = a;
		       }
		   }
		   break;
	       }
#endif /* !DONT_USE_INPLACE_IMAGES */
	   }
       }
       break;
   }

#ifdef DONT_USE_INPLACE_IMAGES
    *pixels = iptr2;
#else /* DONT_USE_INPLACE_IMAGES */
    switch (bitmap_pad_bytes) {
    case 1:
	*pixels = (unsigned int *) c_iptr2;
	break;
    case 2:
	*pixels = (unsigned int *) s_iptr2;
	break;
    case 4:
	*pixels = iptr2;
	break;
    }
#endif /* DONT_USE_INPLACE_IMAGES */
    return (XpmSuccess);
}

static int
ParseExtensions(data, extensions, nextensions)
    xpmData *data;
    XpmExtension **extensions;
    unsigned int *nextensions;
{
    XpmExtension *exts = NULL, *ext;
    unsigned int num = 0;
    unsigned int nlines, a, l, notstart, notend = 0;
    int status;
    char *string, *s, *s2, **sp;

    _XmxpmNextString(data);
    exts = (XpmExtension *) malloc(sizeof(XpmExtension));
    /* get the whole string */
    status = _XmxpmGetString(data, &string, &l);
    if (status != XpmSuccess) {
	free(exts);
	return(status);
    }
    /* look for the key word XPMEXT, skip lines before this */
    while ((notstart = strncmp("XPMEXT", string, 6))
	   && (notend = strncmp("XPMENDEXT", string, 9))) {
	free(string);
	_XmxpmNextString(data);
	status = _XmxpmGetString(data, &string, &l);
	if (status != XpmSuccess) {
	    free(exts);
	    return(status);
	}
    }
    if (!notstart)
	notend = strncmp("XPMENDEXT", string, 9);
    while (!notstart && notend) {
	/* there starts an extension */
	ext = (XpmExtension *) realloc(exts, (num + 1) * sizeof(XpmExtension));
	if (!ext) {
	    free(string);
	    _XmXpmFreeExtensions(exts, num);
	    return(XpmNoMemory);
	}
	exts = ext;
	ext += num;
	/* skip whitespace and store its name */
	s2 = s = string + 6;
	while (isspace(*s2))
	    s2++;
	a = s2 - s;
	ext->name = (char *) malloc(l - a - 6);
	if (!ext->name) {
	    free(string);
	    ext->lines = NULL;
	    ext->nlines = 0;
	    _XmXpmFreeExtensions(exts, num + 1);
	    return(XpmNoMemory);
	}
	strncpy(ext->name, s + a, l - a - 6);
	free(string);
	/* now store the related lines */
	_XmxpmNextString(data);
	status = _XmxpmGetString(data, &string, &l);
	if (status != XpmSuccess) {
	    ext->lines = NULL;
	    ext->nlines = 0;
	    _XmXpmFreeExtensions(exts, num + 1);
	    return(status);
	}
	ext->lines = (char **) malloc(sizeof(char *));
	nlines = 0;
	while ((notstart = strncmp("XPMEXT", string, 6))
	       && (notend = strncmp("XPMENDEXT", string, 9))) {
	    sp = (char **) realloc(ext->lines, (nlines + 1) * sizeof(char *));
	    if (!sp) {
		free(string);
		ext->nlines = nlines;
		_XmXpmFreeExtensions(exts, num + 1);
		return(XpmNoMemory);
	    }
	    ext->lines = sp;
	    ext->lines[nlines] = string;
	    nlines++;
	    _XmxpmNextString(data);
	    status = _XmxpmGetString(data, &string, &l);
	    if (status != XpmSuccess) {
		ext->nlines = nlines;
		_XmXpmFreeExtensions(exts, num + 1);
		return(status);
	    }
	}
	if (!nlines) {
	    free(ext->lines);
	    ext->lines = NULL;
	}
	ext->nlines = nlines;
	num++;
    }
    if (!num) {
	free(string);
	free(exts);
	exts = NULL;
    } else if (!notend)
	free(string);
    *nextensions = num;
    *extensions = exts;
    return(XpmSuccess);
}
