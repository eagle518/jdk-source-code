/*
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)XmXOC.c	1.59 02/06/13
 */

/*
 *  @(#)XmXOC.c 1.55 99/10/27 SMI
 */

/*********************************************************************************
  This file is a temporary implementation of a pseudo-XOC (XmXOC), with Complex 
  Text Layout (CTL) Portable Layout Services (PLS) enabled.

  It is intended ultimately that this code be integrated with the reference
  implementation of X11.

  The implementation approach used below is to wrap an XmXOC around a standard
  (platform dependent) Xlib XOC. The XmXOC is both an enhanced XOC object in
  its own right and contains a pointer to the Xlib XOC. The XmXOC methods table
  contains pointers to methods in this file, which merely wrap calls to the PLS
  m_{w}transform_layout() function around calls to the methods in the Xlib XOC.
*********************************************************************************/

#ifdef SUN_CTL /* fix for bug 4195719 - leob */

/* changes for 64 bit */
#include <sys/isa_defs.h>
/* end of 64 bit header files */

#include <locale.h>		/* for LC_ALL */
#include <X11/keysym.h>		/* for XK_* */
#include <strings.h>		/* for bcmp */
#include <stdio.h>		/* for stderr */
#include "XmI.h"		/* for MAX */
#include "XmRenderTI.h"		/* XmRendition #defines */
#include "XmXOC.h"

/* changes for 64 bit */
#ifdef  _LP64

#if defined(__sparcv9)
#define MACH_ARCH     "sparcv9"
#elif defined(__ia64)
#define MACH_ARCH     "ia64"
#else
#error "unknown architecture"
#endif /* __sparcv9 */

#else /* _LP64 */
#define MACH_ARCH ""
#endif /* end of _LP64 */
/* end of 64 bit */

static XlcResource xm_xoc_resources[] = {
    { XNLayoutAttrObject, NULLQUARK, sizeof(AttrObject),
      XOffsetOf(XmXOCRec, layout_attr_object), XlcCreateMask | XlcGetMask },
    { XNLayoutModifier, NULLQUARK, sizeof(char*),
      XOffsetOf(XmXOCRec, layout_modifier), XlcCreateMask | XlcSetMask | XlcGetMask }
};

typedef struct {
    u_int	 len;		/* Length of a chunk */
    unsigned int gid;		/* CTL Multifont id */
    Boolean	 is_wchar;	/* wchar or multibyte */
    void	 *str;		/* Data stripped of fid */
} chunkDat;

struct chunk {
    chunkDat	 item;
    struct chunk *next;
};

typedef struct {
    struct chunk *head;
    struct chunk *cur;
    u_int	 numChunks;
} chunkList;

#define WC_MFONT		-1
#define NONCTLID		100
#define ONEBYTEWC		101
#define TWOBYTEWC		102
#define ONEBYTEMB		103
#define TWOBYTEMB		104
#define NUMCHARS		(is_wchar ? num_wchars : num_chars)
#define TWO_BYTE_FONT(f)	(((f)->min_byte1 != 0 || (f)->max_byte1 != 0))
#define MULTIFONT(xm_xoc)	(xm_xoc->ule_active == True)
#define CHARSIZE(is_wchar)      (is_wchar ? sizeof(wchar_t) : 1)
#define ISWORD_DELIMITOR(ch)	isspace(ch)
#define IS2BYTE_FONT(xm_xoc)	(xm_xoc->layout_shape_charset_size == 2)
#define GET_DIRECTION(props)	((props & 0x01) ? XmcdRTL : XmcdLTR)

/* Constants for XmXOC Cache handling */
#define MIN_CACHE_SIZE		1024
#define CACHE_INCREMENT         256
#define CACHE_DOUBLING_LIMIT    CTL_MAX_BUF_SIZE

/* static function declarations */
static Status
XocVisualCharScan(XFontSet               fontset,
		  char                  *string,
		  Boolean                is_wchar, 
		  int                    num_chars,
		  int                    position,
		  XmTextScanDirection    dir,
		  XmTextPosition        *new_pos);
static Status
XocVisualWordScan(XFontSet               fontset,
		  char                  *string,
		  Boolean                is_wchar, 
		  int                    num_chars,
		  int                    position,
		  XmTextScanDirection    dir,
		  Boolean                include_ws,
		  XmTextPosition        *new_pos);
static Status
XocVisualLineScan(XFontSet               fontset,
		  char                  *string,
		  Boolean                is_wchar, 
		  int                    num_chars,
		  int                    position,
		  XmTextScanDirection    dir,
		  XmTextPosition        *new_pos);
static Status
XocVisualConstCharScan(XFontSet		fontset,
		       char		*string,
		       Boolean		is_wchar, 
		       int		num_chars,
		       XmTextPosition	position,
		       XmTextPosition	*new_pos);
static int
XocVisualConstWordScan(XFontSet		fontset,
		       char		*string,
		       Boolean		is_wchar, 
		       int		num_chars,
		       XmTextPosition	position,
		       XmTextPosition	*new_pos);
static int
_XmbCTLTextEscapement(XFontSet		fontset,
		      _Xconst char	*text,
		      int		bytes_text);
static int
_XwcCTLTextEscapement(XFontSet		fontset,
		      _Xconst wchar_t	*text,
		      int		num_wchars);
static int
_XmbCTLTextExtents(XFontSet		fontset,
		   _Xconst char 	*text,
		   int			bytes_text,
		   XRectangle		*overall_ink_return,
		   XRectangle		*overall_logical_return);
static int
_XwcCTLTextExtents(XFontSet		fontset,
		   _Xconst wchar_t	*text,
		   int 			num_wchars,
		   XRectangle		*overall_ink_return,
		   XRectangle		*overall_logical_return);
static int
_XmbCTLDrawString(Display		*display,
		  Drawable		d,
		  XFontSet		fontset,
		  GC			gc,
		  int			x,
		  int			y,
		  _Xconst char		*text,
		  int			bytes_text);
static int
_XwcCTLDrawString(Display	  	*display,
		  Drawable	  	d,
		  XFontSet	  	fontset,
		  GC		  	gc,
		  int		  	x,
		  int		  	y,
		  _Xconst wchar_t 	*text,
 		  int		  	num_wchars);
static void
_XmbCTLDrawImageString(Display		*display,
		       Drawable 	d,
		       XFontSet 	fontset,
		       GC		gc,
		       int		x,
		       int		y,
		       _Xconst char	*text,
		       int		bytes_text);
static void
_XwcCTLDrawImageString(Display		*display,
		       Drawable		d,
		       XFontSet		fontset,
		       GC		gc,
		       int		x,
		       int		y,
		       _Xconst wchar_t	*text,
		       int		num_wchars);
static Status
_XmbCTLTextPerCharExtents(XFontSet 	fontset,
			  _Xconst char	*text,
			  int		bytes_text,
			  XRectangle	*ink_array_return,
			  XRectangle	*logical_array_return,
			  int		array_size,
			  int		*num_chars_return,
			  XRectangle	*overall_ink_return,
			  XRectangle	*overall_logical_return);
static Status
_XwcCTLTextPerCharExtents(XFontSet	fontset,
			  _Xconst wchar_t	*text,
			  int		num_wchars,
			  XRectangle	*ink_array_return,
			  XRectangle	*logical_array_return,
			  int		array_size,
			  int		*num_chars_return,
			  XRectangle	*overall_ink_return,
			  XRectangle	*overall_logical_return);
static Status
_XocCTLTextPerCharExtents_mb(XOC 	xoc,
			  _Xconst char	*text,
			  int		num_chars,
			  XSegment	*ink_array_return,
			  XSegment	*logical_array_return,
			  int		array_size,
			  int		*num_chars_return,
			  XSegment	*overall_ink_return,
			  XSegment	*overall_logical_return);
static Status
_XocCTLTextPerCharExtents_wc(XOC	xoc,
			     _Xconst wchar_t	*text,
			     int	num_chars,
			     XSegment	*ink_array_return,
			     XSegment	*logical_array_return,
			     int	array_size,
			     int	*num_chars_return,
			     XSegment	*overall_ink_return,
			     XSegment	*overall_logical_return);
static Status
_XocCTLTextPerCharExtents(XOC           oc,
			  _Xconst void  *string,
			  Boolean       is_wchar, 
			  int           num_chars,
			  XSegment      *ink_array_return,
			  XSegment      *logical_array_return,
			  int           array_size,
			  int           *num_chars_return,
			  XSegment      *overall_ink_return,
			  XSegment      *overall_logical_return);

/************************** START CTL MFONT *************************
 * The functions below are used to add multiscript support to Motif *
 * Used primarily in en_US.UTF-8 locale for Thai(th), Hebrew(He) &  *
 * Arabic (ar) support. Needs  base CTL functionality to exist.	    *
 ********************************************************************/
static int		mfontAdd(XmXOC xmxoc, mfontRec mfRec);
static int		parseShape(XmXOC xmxoc, char *modifier);
static unsigned int	ctlChar(XmXOC xmxoc, void *text, int pos);
static XFontStruct*	getCtlFont(XmXOC xmxoc, unsigned int ctlId);
static void		fillNonCTLChunk(struct chunk *myChunk, void *glyphs,
					short beg, short end, unsigned int id, int iswc);
static void		fillCTLChunk(struct chunk *myChunk, void *glyphs,
				     short beg, short end, unsigned int id, int iswc);
static void		freeList(chunkList *myList);
static int		mbMakeChunk(XmXOC xmxoc, chunkList *myList, char *glyphs, int glyph_bytes);
static int		wcMakeChunk(XmXOC xmxoc, chunkList *myList, wchar_t *glyphs, int glyph_num);
static int              parseOrientation(XmXOC xmxoc, char *modifier, LayoutValues layout_values,
					 LayoutTextDescriptor text_desc);
static int              _dummy_transform_layout( XmXOC xm_xoc, Boolean is_wChar, _Xconst void *inpBuffer, 
						 size_t inpSize, void *outBuf, size_t *outSize,
						 size_t *i2o, size_t *o2i, unsigned char *props,
						 size_t *in_buf_index);
/* *************** END CTL MFONT *************** */
/* End of Static function declarations */

XOC XmDestroyXmXOC(XOC xoc);
static void XmRealDestroyXmXOC(XOC xoc);

/* Macros as temporary storage is allocated on stack */ 
#define INIT_TRANS_DATA \
  int		status = -1; \
  size_t	index = 0;   \
  XmXOC		xm_xoc = (XmXOC)fontset; \
  LayoutObject	lo = xm_xoc->layout_object; \
  void 		*oBuf  = NULL; \
  size_t 	*i2o   = NULL; \
  size_t 	*o2i   = NULL; \
  unsigned char *props = NULL;

#define FREE_TRANS_DATA \
  if (oBuf) XtFree((char*)oBuf);	\
  if (i2o) XtFree((char*)i2o);	\
  if (o2i) XtFree((char*)o2i);	\
  if (props) XtFree((char*)props);

#define INIT_CACHE_DATA	\
  size_t	i2o_cache[CTL_CACHE_SIZE];	\
  size_t	o2i_cache[CTL_CACHE_SIZE];	\
  unsigned char props_cache[CTL_CACHE_SIZE];

/* Used to deallocate buffers allocated during a transformation */
/* ALWAYS maintain naming convention of props, o2i, i2o & oBuf */
#define FREE_CACHE_DATA \
  if (oBuf) XmStackFree((char*)oBuf, oBuf_cache);	\
  if (i2o) XmStackFree((char*)i2o, i2o_cache);	\
  if (o2i) XmStackFree((char*)o2i, o2i_cache);	\
  if (props) XmStackFree((char*)props, props_cache);

#define FREE_DUAL_CACHE_DATA \
  if (oBuf) {					\
      if (is_wchar) {				\
	XmStackFree((char*)oBuf, oBuf_wc);	\
      }						\
      else {					\
	XmStackFree((char*)oBuf, oBuf_mb);	\
      }						\
    }						\
  if (i2o) XmStackFree((char*)i2o, i2o_cache);	\
  if (o2i) XmStackFree((char*)o2i, o2i_cache);	\
  if (props) XmStackFree((char*)props, props_cache);

void
_XmXOC_InvalidateCache(XmXOC xm_xoc)
{
    if (xm_xoc->layout_cache.char_buf)
	XtFree((char*)xm_xoc->layout_cache.char_buf);
    if (xm_xoc->layout_cache.GlyphBuf)
	XtFree((char*)xm_xoc->layout_cache.GlyphBuf);
    if (xm_xoc->layout_cache.CPos2GPos)
	XtFree((char*)xm_xoc->layout_cache.CPos2GPos);
    if (xm_xoc->layout_cache.GPos2CPos)
	XtFree((char*)xm_xoc->layout_cache.GPos2CPos);
    if (xm_xoc->layout_cache.CharProps)
	XtFree((char*)xm_xoc->layout_cache.CharProps);
    
    xm_xoc->layout_cache.ret 		= -1;
    xm_xoc->layout_cache.BufSize	= 0;
    xm_xoc->layout_cache.CharNum	= 0;
    xm_xoc->layout_cache.GlyphNum	= 0;
}

static size_t next_cache_size(size_t curr_cache_size)
{
    if (curr_cache_size < MIN_CACHE_SIZE)
	return MIN_CACHE_SIZE;
    else if (curr_cache_size < CACHE_DOUBLING_LIMIT)
	return curr_cache_size * 2;
    return curr_cache_size + CACHE_INCREMENT;
}

int 
_XmXOC_transform_layout(XmXOC		xm_xoc,
			LayoutObject	layout_object,
			Boolean		is_wchar,
			_Xconst void	*inpBuffer,
			size_t		inpSize,
			void		*outBuf,
			size_t		*outSize,
			size_t		*i2o,
			size_t		*o2i,
			unsigned char	*props,
			size_t		*in_buf_index)
{
  unsigned int	sizet = sizeof(size_t);
  XtEnum	cpState;
  
  if ((layout_object	== xm_xoc->layout_object) &&
      (is_wchar		== xm_xoc->layout_cache.is_wchar) &&
      (inpSize		== xm_xoc->layout_cache.CharNum) &&
      ((inpSize > 0) && (is_wchar ?
       (wcsncmp((wchar_t*)inpBuffer, (wchar_t*)xm_xoc->layout_cache.char_buf, inpSize) == 0) :
       (strncmp((char*)inpBuffer, (char*)xm_xoc->layout_cache.char_buf, inpSize) == 0)))) {    
    if (props == NULL)
      cpState = COPYPARTIAL; /* Props, I2O, O2I Not copied/Needed */
    else if (xm_xoc->layout_cache.cacheState == COPYALL)
      cpState = COPYALL; /* 100 % hit */
    else
      cpState = COPYNONE; /* Transform */
  }
  else
    cpState = COPYNONE;
  
  if (cpState == COPYNONE) {
    int		ret = -1;    
    size_t	glyph_num, glyph_bytes;    
    size_t	char_num = inpSize;
    size_t	char_bytes = char_num * CHARSIZE(is_wchar);
    size_t      index_save= *in_buf_index;

    /* Perform appropriate transformation based on wchar, mb */
    if (is_wchar) {
      ret = (*xm_xoc->fnRec->m_wtransform_layout)(layout_object, (wchar_t*)inpBuffer,
						  inpSize, outBuf, outSize, i2o, o2i,
						  props, in_buf_index);
      glyph_bytes = *outSize * xm_xoc->layout_shape_charset_size;
      glyph_num = *outSize;
    }
    else {
      ret = (*xm_xoc->fnRec->m_transform_layout)(layout_object, (char*)inpBuffer, inpSize, 
						 outBuf, outSize, i2o, o2i,
						 props, in_buf_index);
      glyph_bytes = *outSize;
      glyph_num = *outSize / xm_xoc->layout_shape_charset_size;
    }
    
    if (ret !=0){ /*Language Engine Failure, call dummy transform*/
      *in_buf_index = index_save; /*get the original index value*/
      ret = _dummy_transform_layout(xm_xoc , is_wchar, inpBuffer, 
				    inpSize, outBuf, outSize, i2o, o2i, props, in_buf_index);
      glyph_bytes = *outSize;
      glyph_num = *outSize / xm_xoc->layout_shape_charset_size;
    }
    
    if (ret == 0) { /* Update the cache, Reallocate Buffers if needed */      
      if (xm_xoc->layout_cache.BufSize < inpSize * CHARSIZE(is_wchar)) {	
	size_t cache_num = next_cache_size(inpSize);
	size_t charbuf_size = cache_num * CHARSIZE(is_wchar);	
	size_t new_glyph_num = cache_num * xm_xoc->layout_max_expand;
	size_t glyphbuf_size = new_glyph_num * xm_xoc->layout_shape_charset_size;
	
	xm_xoc->layout_cache.char_buf = (void*)XtRealloc((void*)xm_xoc->layout_cache.char_buf, 
							 charbuf_size);
	xm_xoc->layout_cache.GlyphBuf = (void*)XtRealloc((void*)xm_xoc->layout_cache.GlyphBuf, 
							 glyphbuf_size);
	xm_xoc->layout_cache.CPos2GPos = (size_t*)XtRealloc((void*)xm_xoc->layout_cache.CPos2GPos,
							    cache_num * sizet);
	xm_xoc->layout_cache.GPos2CPos = (size_t*)XtRealloc((void*)xm_xoc->layout_cache.GPos2CPos,
							    new_glyph_num * sizet);
	xm_xoc->layout_cache.CharProps = (BYTE*)XtRealloc((void*)xm_xoc->layout_cache.CharProps,
							  cache_num);
	xm_xoc->layout_cache.BufSize = charbuf_size;
      }
            
      memcpy((void*)xm_xoc->layout_cache.char_buf, (const void*)inpBuffer, char_bytes);
      memcpy((void*)xm_xoc->layout_cache.GlyphBuf, (const void*)outBuf, glyph_bytes);
      xm_xoc->layout_cache.cacheState = COPYPARTIAL;
      if (props) {	
	memcpy((void*)xm_xoc->layout_cache.CPos2GPos, (const void*)i2o, char_num * sizet);
	if (is_wchar)
	  memcpy((void*)xm_xoc->layout_cache.GPos2CPos, (const void*)o2i, glyph_num * sizet);
	else
	  memcpy((void*)xm_xoc->layout_cache.GPos2CPos, (const void*)o2i, glyph_bytes * sizet);
	memcpy((void*)xm_xoc->layout_cache.CharProps, (const void*)props, char_num);
	xm_xoc->layout_cache.cacheState = COPYALL;
      }
      
      xm_xoc->layout_cache.ret = ret;
      xm_xoc->layout_cache.is_wchar = is_wchar;
      xm_xoc->layout_cache.char_buf_index = *in_buf_index;
      xm_xoc->layout_cache.CharNum = char_num;
      xm_xoc->layout_cache.GlyphNum = glyph_num;
    }
    else
      _XmXOC_InvalidateCache(xm_xoc); /* Transform error - Invalidate cache */
  }
  else { 
    size_t GlyphNum = xm_xoc->layout_cache.GlyphNum;
    size_t GlyphBytes = GlyphNum * xm_xoc->layout_shape_charset_size;
    size_t CharNum = xm_xoc->layout_cache.CharNum;

    memcpy((void*)outBuf, (const void*)xm_xoc->layout_cache.GlyphBuf, GlyphBytes);    
    /* For wchar we return the num of chars in the outSize */
    /* For multi byte we return the num of bytes in outSize */
    if (is_wchar)
      *outSize = GlyphNum;
    else
      *outSize = GlyphBytes;

    if (cpState == COPYALL) {
      memcpy((void*)i2o, (const void*)xm_xoc->layout_cache.CPos2GPos, CharNum * sizet);
      if (is_wchar)
	memcpy((void*)o2i, (const void*)xm_xoc->layout_cache.GPos2CPos, GlyphNum * sizet);
      else
	memcpy((void*)o2i, (const void*)xm_xoc->layout_cache.GPos2CPos, GlyphBytes * sizet);
      memcpy((void*)props, (const void*)xm_xoc->layout_cache.CharProps, CharNum);
    }
  }
  return (xm_xoc->layout_cache.ret);
}

/************************************************************************
 * The routines below that are used to obtain the escapement, extents,	*
 * percharextents and all drawing routines have been enhanced to handle *
 * the presentation form obtained from the Unicode Layout Engine (ULE). *
 * The format of data obtained from ULE is UCS4 with font id (FID) info	*
 * in the first two bytes & shapecharset info in the last two : Prabhat	*
 ************************************************************************/
Boolean ctlLocale(XmXOCFuncRec *fnRec)
{
    int			index;
    char		libpath[100]; /* Path name for liblayout presumed <= 100 */
    LayoutObject	tst_LO;
    BooleanValue	Active_dir = FALSE, Active_shape = FALSE;
    LayoutValueRec	layout_val[5];
    
    layout_val[0].name = ActiveShapeEditing;
    layout_val[0].value = &Active_dir;
    layout_val[1].name = ActiveDirectional;
    layout_val[1].value = &Active_shape;
    layout_val[2].name = 0;
    
    sprintf(libpath, "/usr/lib/%s/liblayout.so", MACH_ARCH);
    fnRec->so = dlopen(libpath, RTLD_LAZY);
    if (fnRec->so == NULL)
	return False;
    
    /* Add a check at the end to make sure all functions are found : Prabhat */
    fnRec->m_create_layout = (LayoutObject(*)())dlsym(fnRec->so, "m_create_layout");
    fnRec->m_destroy_layout = (int(*)())dlsym(fnRec->so, "m_destroy_layout");
    fnRec->m_getvalues_layout = (int(*)())dlsym(fnRec->so, "m_getvalues_layout");
    fnRec->m_setvalues_layout = (int(*)())dlsym(fnRec->so, "m_setvalues_layout");
    fnRec->m_transform_layout = (int(*)())dlsym(fnRec->so, "m_transform_layout");
    fnRec->m_wtransform_layout = (int(*)())dlsym(fnRec->so, "m_wtransform_layout");

    /*Bug 4461493 -xnllanguage does not affect on S8
	if we comment the setlocale(LC_ALL, ""); line out, that works just fine*/ 
    /*setlocale(LC_ALL, "");  Set Current Locale */

    tst_LO = (LayoutObject) (*fnRec->m_create_layout)(NULL, NULL);
    if (tst_LO) {
	(*fnRec->m_getvalues_layout)(tst_LO, &layout_val, &index);
	(*fnRec->m_destroy_layout)(tst_LO);
	if ((Active_dir == TRUE) || (Active_shape == TRUE))
	    return True;
    }
    return False;
}

static void
fillNonCTLChunk(struct chunk *myChunk, void *glyphs, short beg, short end,
		unsigned int id, int iswc)
{
    wchar_t	*wcBuf, *tmpWcBuf;
    char	*mbBuf;	
    int		ct, glyphCt;
    
    myChunk->item.len = end - beg;
    myChunk->item.gid = id; /* NONCTLID indicates not a CTL Character */
    myChunk->item.is_wchar = (iswc == WC_MFONT); /* -1 indicates WCHAR */
    
    if (!myChunk->item.is_wchar) {
	myChunk->item.len /= 4; /* Convert Mb to Wc */
	wcBuf = (wchar_t*)XtMalloc(myChunk->item.len * sizeof(wchar_t));
	mbBuf = (char*)glyphs;
	for (ct = beg, glyphCt = 0; ct < end; glyphCt++) {
	    wcBuf[glyphCt] = ((0x00FFFFFF | (mbBuf[ct] << 24)) &
			      (0xFF00FFFF | (mbBuf[ct + 1] << 16)) &
			      (0xFFFF00FF | (mbBuf[ct + 2] << 8)) &
			      (0xFFFFFF00 | mbBuf[ct + 3]));
	    ct += 4;
	}
    }
    else {
	tmpWcBuf = (wchar_t*)glyphs;
	wcBuf = (wchar_t*)(&tmpWcBuf[beg]);
    }    
    
    myChunk->item.str = (void*)wcBuf;
    myChunk->next = NULL;
}

static void
fillCTLChunk(struct chunk *myChunk, void *glyphs, short beg, short end, 
	     unsigned int id, int iswc)
{
    char	*mbBuf, *tmpMbBuf;
    wchar_t	*tmpWcBuf;
    int		glyphCt = 0, numBytes, ct;
    
    myChunk->item.len = end - beg;
    myChunk->item.gid = id; /* NONCTLID indicates not a CTL Character */
    myChunk->item.is_wchar = ((iswc == ONEBYTEWC) || (iswc == TWOBYTEWC));
    if (!myChunk->item.is_wchar) {
	myChunk->item.len /= 4;
	tmpMbBuf = (char*)glyphs;
    }
    else 
	tmpWcBuf = (wchar_t*)glyphs;
    
    if ((iswc == TWOBYTEWC) || (iswc == TWOBYTEMB)) {
	numBytes = (myChunk->item.is_wchar) ? ((end - beg)*2) : ((end - beg)/2);
    }
    else {
	numBytes = (myChunk->item.is_wchar) ? (end - beg) : ((end - beg)/4);
    }
    mbBuf = (char*)XtMalloc(numBytes);
    
    for (ct = beg, glyphCt = 0; ct < end;) {
	
	if ((iswc == TWOBYTEWC) || (iswc == TWOBYTEMB)) {
	    if (myChunk->item.is_wchar) {
		mbBuf[glyphCt++] = (u_int)((tmpWcBuf[ct] & 0x0000FF00) >> 8);
		mbBuf[glyphCt++] = (u_int)(tmpWcBuf[ct++] & 0x000000FF);
	    }
	    else {
		ct +=2;
		memcpy(&mbBuf[glyphCt], &tmpMbBuf[ct], 2);
		ct += 2; glyphCt += 2;
	    }
	}
	else {
	    if (myChunk->item.is_wchar)
		mbBuf[glyphCt++] = (u_int)(tmpWcBuf[ct++] & 0x000000FF);
	    else {
		ct += 3;
		memcpy(&mbBuf[glyphCt++], &tmpMbBuf[ct++], 1);
	    }
	}
    }
    myChunk->item.str = (void*)mbBuf;
    myChunk->next = NULL;
}

static void
freeList(chunkList *myList)
{
    struct chunk *ptr;
    struct chunk *tmpPtr;

    if ((myList) && (myList->numChunks > 0)) {
	ptr = myList->head;
	while (ptr != NULL) {
	    tmpPtr = ptr;
	    ptr = ptr->next;
	    if (!((tmpPtr->item.gid == NONCTLID) && tmpPtr->item.is_wchar))
		XtFree(tmpPtr->item.str);
	    XtFree((char*)tmpPtr);
	}
	XtFree((char*)myList);
    }
}

static int
mbMakeChunk(XmXOC xmxoc, chunkList *myList, char *glyphs, int glyph_bytes)
{
    int ct = 0;
    
    for (ct = 0; ct < glyph_bytes;) {
	int		start, glyphType;
	Boolean		sameid;
	struct chunk	*tmpChunk;
	unsigned int	tstId = NONCTLID;
	unsigned int	newId, curId;
	
	start = ct;
	tstId = ctlChar(xmxoc, (void*)glyphs, ct); ct += 4;
	tmpChunk = (struct chunk*)XtMalloc(sizeof(struct chunk));
	if (myList->numChunks == 0)
	    myList->head = tmpChunk;
	else
	    myList->cur->next = tmpChunk;
	myList->cur = tmpChunk;
	myList->numChunks++;
	
	sameid = True;
	while (sameid && ct < glyph_bytes) {
	    newId = ctlChar(xmxoc, (void*)glyphs, ct);
	    sameid = (newId == tstId);
	    if (sameid) ct += 4;
	}
	
	if (tstId == NONCTLID)
	    fillNonCTLChunk(tmpChunk, glyphs, start, ct, tstId, tstId);
	else {
	    XFontStruct *tmpFs = NULL;
	    tmpFs = getCtlFont(xmxoc, tstId);
	    if (TWO_BYTE_FONT(tmpFs))
		glyphType = TWOBYTEMB;
	    else
		glyphType = ONEBYTEMB;
	    fillCTLChunk(tmpChunk, glyphs, start, ct, tstId, glyphType);
	}
    }    
    return myList->numChunks;
}

static int
wcMakeChunk(XmXOC xmxoc, chunkList *myList, wchar_t *glyphs, int glyph_num)
{
    int ct = 0;
    
    for (ct = 0; ct < glyph_num;) {
	
	unsigned int	tstId = NONCTLID;
	int		start, glyphType;
	Boolean		sameid;
	unsigned int	newId, curId;
	struct chunk	*tmpChunk;
	
	start = ct;
	tstId = ctlChar(xmxoc, (void*)&glyphs[ct], WC_MFONT); ct++;
	tmpChunk = (struct chunk*)XtMalloc(sizeof(struct chunk));
	if (myList->numChunks == 0)
	    myList->head = tmpChunk;
	else
	    myList->cur->next = tmpChunk;
	myList->cur = tmpChunk;
	myList->numChunks++;
	
	sameid = True;
	while (sameid && ct < glyph_num) {
	    newId = ctlChar(xmxoc, (void*)&glyphs[ct], WC_MFONT);
	    sameid = (newId == tstId);
	    if (sameid) ct++;
	}
	
	if (tstId == NONCTLID)
	    fillNonCTLChunk(tmpChunk, glyphs, start, ct, tstId, WC_MFONT);
	else {
	    XFontStruct *tmpFs = NULL;
	    tmpFs = getCtlFont(xmxoc, tstId);
	    if (TWO_BYTE_FONT(tmpFs))
		glyphType = TWOBYTEWC;
	    else
		glyphType = ONEBYTEWC;
	    fillCTLChunk(tmpChunk, glyphs, start, ct, tstId, glyphType);
	}
    }
    return myList->numChunks;
}

static int
_XmbCTLTextEscapement(XFontSet		fontset,
		      _Xconst char	*text,
		      int		bytes_text)
{
    int		ret_escapement = 0, glyph_num = 0;
    char	*glyph_text = NULL;
    char	oBuf_cache[CTL_CACHE_SIZE];
    size_t	glyph_bytes = 0;
    INIT_TRANS_DATA
      
    if (text[bytes_text - 1] == '\n')
	bytes_text --;
    if ((bytes_text <= 0) || (!(lo && xm_xoc->layout_active))) 
	return 0;
    
    /* Can query ULE to find out no of bytes to allocate */
    glyph_bytes = bytes_text * xm_xoc->layout_max_expand *
		  xm_xoc->layout_shape_charset_size;
    oBuf = (void*)XmStackAlloc(glyph_bytes, oBuf_cache);
    
    status = _XmXOC_transform_layout(xm_xoc,
                                     lo,
                                     False,
				     text, (size_t)bytes_text,
				     oBuf, &glyph_bytes,
				     NULL, NULL, NULL, &index);
    if (status == 0) /* OK */
	glyph_text = (char*)oBuf;
    else {
        perror("Warning! XmXOC transform_layout failure\n");
	XmStackFree((char*)oBuf, oBuf_cache);
	return 0;
    }

    if (IS2BYTE_FONT(xm_xoc) || MULTIFONT(xm_xoc))
	glyph_num = glyph_bytes / xm_xoc->layout_shape_charset_size;
    else
	glyph_num = glyph_bytes;
    
    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	chunkList	*glyphChunk = (chunkList*)XtMalloc(sizeof(chunkList));
	struct chunk	*aChunk;
	int		len, tmp_esc = 0;
	
	glyphChunk->numChunks = 0;
	mbMakeChunk(xm_xoc, glyphChunk, glyph_text, glyph_bytes);
	aChunk = glyphChunk->head;
	while (aChunk != NULL) {
	    unsigned int tstId = NONCTLID;
	    
	    len = aChunk->item.len;
	    tstId = aChunk->item.gid;
	    if (tstId == NONCTLID)
		tmp_esc += xm_xoc->xoc->methods->wc_escapement(xm_xoc->xoc,
				       (wchar_t*)aChunk->item.str, len);
	    else {
		int		dir = 0, ascent = 0, descent = 0;
		XFontStruct	*tmpFs = NULL;
		XCharStruct	overall;
		
		tmpFs = getCtlFont(xm_xoc, tstId);
		if (TWO_BYTE_FONT(tmpFs))
		    XTextExtents16(tmpFs, (XChar2b*)(aChunk->item.str), len, &dir,
				   &ascent, &descent, &overall);
		else
		    XTextExtents(tmpFs, (char *)(aChunk->item.str), len, &dir,
				 &ascent, &descent, &overall);
		tmp_esc += overall.width;
	    }
	    aChunk = aChunk->next;
	}
	ret_escapement = tmp_esc;
	freeList(glyphChunk);
    }
    else if (IS2BYTE_FONT(xm_xoc)) {
	int	    fonts_in_fs = 0;
	int	    dir = 0, ascent = 0, descent = 0;
	char	    **font_name_list = NULL;
	XFontStruct **font_struct_list = NULL;
	XCharStruct overall;
	
	fonts_in_fs = XFontsOfFontSet(fontset, &font_struct_list, &font_name_list);
	if (fonts_in_fs <= 0)
	    XmeWarning(NULL, "_XmbCTLTextEscapement - no fonts in fontset\n");
	else { /* Note: We only handle one font per fontset */
	    XTextExtents16(*font_struct_list, (XChar2b*)glyph_text, glyph_num, &dir,
			   &ascent, &descent, &overall);
	    ret_escapement = overall.width;
	}
    } 
    else  /* single byte font */
	ret_escapement = xm_xoc->xoc->methods->mb_escapement(xm_xoc->xoc,
							     glyph_text,
							     glyph_bytes);
    XmStackFree((char*)oBuf, oBuf_cache);
    return ret_escapement;
}

static int
_XwcCTLTextEscapement(XFontSet		fontset,
		      _Xconst wchar_t	*text,
		      int		num_wchars)
{
    int		ret_escapement = 0;
    size_t	glyph_num = 0;
    wchar_t	*glyph_text = NULL;
    wchar_t	oBuf_cache[CTL_CACHE_SIZE];
    INIT_TRANS_DATA
    
    if (text[num_wchars - 1] == (wchar_t)'\n')
        num_wchars--;
    if ((num_wchars <= 0) || (!(lo && xm_xoc->layout_active)))
	return 0;
    
    glyph_num = num_wchars * xm_xoc->layout_max_expand;
    oBuf = (void*)XmStackAlloc(glyph_num * sizeof(wchar_t), oBuf_cache);
    
    /* Can query ULE to find out no of bytes to allocate */
    status = _XmXOC_transform_layout(xm_xoc,
                                     lo,
                                     True,
				     text, (size_t)num_wchars,
				     oBuf, &glyph_num,
				     NULL, NULL, NULL, &index);
    if (status == 0) /* OK */
	glyph_text = (wchar_t*)oBuf;
    else {
	XmeWarning(NULL, "XwcCTLTextEscapement-Transform failure\n");
	XmStackFree((char*)oBuf, oBuf_cache);
	return 0;
    }
    
    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	chunkList	*glyphChunk = (chunkList*)XtMalloc(sizeof(chunkList));
	int		len, tmp_esc = 0;
	struct chunk	*aChunk;
	
	glyphChunk->numChunks = 0;
	wcMakeChunk(xm_xoc, glyphChunk, glyph_text, glyph_num);
	aChunk = glyphChunk->head;
	while (aChunk != NULL) {
	    unsigned int tstId = NONCTLID;
	    
	    len = aChunk->item.len;
	    tstId = aChunk->item.gid;
	    if (tstId == NONCTLID)
		tmp_esc += xm_xoc->xoc->methods->wc_escapement(xm_xoc->xoc,
				       (wchar_t*)aChunk->item.str, len);
	    else {
		int		dir = 0, ascent = 0, descent = 0;
		XFontStruct	*tmpFs = NULL;
		XCharStruct	overall;
		
		tmpFs = getCtlFont(xm_xoc, tstId);
		if (TWO_BYTE_FONT(tmpFs))
		    XTextExtents16(tmpFs, (XChar2b*)aChunk->item.str, len,
				   &dir, &ascent, &descent, &overall);
		else
		    XTextExtents(tmpFs, (char*)aChunk->item.str, len, &dir,
				 &ascent, &descent, &overall);
		tmp_esc += overall.width;
	    }
	    aChunk = aChunk->next;
	}
	ret_escapement = tmp_esc;
	freeList(glyphChunk);
    }	
    else
	ret_escapement = xm_xoc->xoc->methods->wc_escapement(xm_xoc->xoc,
							     glyph_text,
							     glyph_num);
    XmStackFree((char*)oBuf, oBuf_cache);
    return ret_escapement;
}

static int
_XmbCTLTextExtents(XFontSet	fontset,
		   _Xconst char *text,
		   int		bytes_text,
		   XRectangle	*overall_ink_return,
		   XRectangle	*overall_logical_return)
{
    int		ret_extent = 0;
    char  	*glyph_text = NULL;
    size_t	glyph_bytes = 0;
    char	oBuf_cache[CTL_CACHE_SIZE];
    INIT_TRANS_DATA
    
    if (text && (bytes_text > 0) && text[bytes_text - 1] == '\n')
	bytes_text --;
    if ((bytes_text <= 0) || (!(lo && xm_xoc->layout_active))) {
	memset(overall_ink_return, 0, sizeof(XRectangle));
	memset(overall_logical_return, 0, sizeof(XRectangle));
	return 0;
    }
    
    /* Can query ULE to find out no of bytes to allocate */
    glyph_bytes = bytes_text * xm_xoc->layout_max_expand *
		  xm_xoc->layout_shape_charset_size;
    oBuf = (void*)XmStackAlloc(glyph_bytes, oBuf_cache);
    
    status = _XmXOC_transform_layout(xm_xoc,
                                     lo,
                                     False,
				     text, (size_t)bytes_text,
				     oBuf, &glyph_bytes,
				     NULL, NULL, NULL, &index);
    if (status == 0)
	glyph_text = (char*)oBuf;
    else {
	XmeWarning(NULL, "XmbCTLTextExtents-Transform failure\n");
	XmStackFree((char*)oBuf, oBuf_cache);
	return 0;
    }

    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	unsigned int	tstId = NONCTLID;
	int		tmp_xtent = 0, ct = 0;
	int		dir = 0, ascent = 0, descent = 0;
	wchar_t		byte1, byte2, byte3, byte4, to_draw_wc;
	XFontStruct	*tmp_fstr = NULL;
	XCharStruct	overall;
	int             fontset_ascent;
	int             fontset_descent;
	XFontSetExtents *fs_extents = NULL;

	/*fill the vertical components of the string with the max values in the fontset */
	fs_extents      = XExtentsOfFontSet((XFontSet)fontset);
	fontset_ascent  = -fs_extents->max_logical_extent.y;
	fontset_descent =  fs_extents->max_logical_extent.height + fs_extents->max_logical_extent.y;
	
	for (ct = 0; ct < glyph_bytes;) {
	    dir = 0; ascent = 0; descent = 0;
	    tstId = ctlChar(xm_xoc, (void*)glyph_text, ct);
	    
	    if (tstId != NONCTLID) {
		tmp_fstr = getCtlFont(xm_xoc, tstId);
		ct += 2;
		
		if (TWO_BYTE_FONT(tmp_fstr))
		    XTextExtents16(tmp_fstr, (XChar2b*)&glyph_text[ct], 1, &dir,
				   &ascent, &descent, &overall);
		else
		    XTextExtents(tmp_fstr, (char*)&glyph_text[ct+1], 1, &dir,
				 &ascent, &descent, &overall);
		tmp_xtent += overall.width;
		ct += 2;
	    }
	    else {  /* Not a CTL Character */
		byte4 = 0x00FFFFFF | (glyph_text[ct] << 24);
		byte3 = 0xFF00FFFF | (glyph_text[ct+1] << 16);
		byte2 = 0xFFFF00FF | (glyph_text[ct+2] << 8);
		byte1 = 0xFFFFFF00 | glyph_text[ct+3];
		to_draw_wc = byte4 & byte3 & byte2 & byte1;
		tmp_xtent += xm_xoc->xoc->methods->wc_extents(xm_xoc->xoc,
					      &to_draw_wc, 1,
					      overall_ink_return,
					      overall_logical_return);
		ct += 4;
	    }
	}
	overall_logical_return->width = tmp_xtent;
	overall_logical_return->height = fontset_descent+fontset_ascent;/* set using the font set values */
	overall_logical_return->y=   -fontset_ascent; /* set using the font set values */
	ret_extent = tmp_xtent;
    }
    else if (IS2BYTE_FONT(xm_xoc))
	ret_extent = xm_xoc->xoc->methods->mb_extents(xm_xoc->xoc,
					      glyph_text, glyph_bytes/2, 
					      overall_ink_return,
					      overall_logical_return);
    else
	ret_extent = xm_xoc->xoc->methods->mb_extents(xm_xoc->xoc,
					      glyph_text, glyph_bytes, 
					      overall_ink_return,
					      overall_logical_return);
    XmStackFree((char*)oBuf, oBuf_cache);
    return ret_extent;
}

static int
_XwcCTLTextExtents(XFontSet	fontset,
		   _Xconst wchar_t *text,
		   int 		num_wchars,
		   XRectangle	*overall_ink_return,
		   XRectangle	*overall_logical_return)
{
    int		ret_extent = 0;
    size_t	glyph_num = 0;
    wchar_t	*glyph_text = NULL, oBuf_cache[CTL_CACHE_SIZE];
    INIT_TRANS_DATA
    
    if (text[num_wchars - 1] == (wchar_t)'\n')
        num_wchars--;
    if ((num_wchars <= 0) || (!(lo && xm_xoc->layout_active))) {
	memset(overall_ink_return, 0, sizeof(XRectangle));
	memset(overall_logical_return, 0, sizeof(XRectangle));
	return 0;
    }
   
    glyph_num = num_wchars * xm_xoc->layout_max_expand;
    oBuf = (void*)XmStackAlloc(glyph_num * sizeof(wchar_t), oBuf_cache);

    /* Can query ULE to find out no of bytes to allocate */
    status = _XmXOC_transform_layout(xm_xoc,
                                     lo,
                                     True,
				     text, (size_t)num_wchars,
				     oBuf, &glyph_num,
				     NULL, NULL, NULL, &index);
    if (status == 0) /* OK */
	glyph_text = (wchar_t*)oBuf;
    else {
	XmeWarning(NULL, "XwcCTLTextExtents-Transform failure\n");
	XmStackFree((char*)oBuf, oBuf_cache);
	return 0;
    }

    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	chunkList	*glyphChunk = (chunkList*)XtMalloc(sizeof(chunkList));
	int		tmp_xtent = 0;
	struct chunk	*aChunk;
	int             fontset_ascent;
	int             fontset_descent;
	XFontSetExtents *fs_extents = NULL;

	/* in multi font fill the vertical components of the string with the max values in the fontset */
	fs_extents      = XExtentsOfFontSet((XFontSet)fontset);
	fontset_ascent  = -fs_extents->max_logical_extent.y;
	fontset_descent =  fs_extents->max_logical_extent.height + fs_extents->max_logical_extent.y; 
	
	glyphChunk->numChunks = 0;
	wcMakeChunk(xm_xoc, glyphChunk, glyph_text, glyph_num);
	
	aChunk = glyphChunk->head;
	while (aChunk != NULL) {
	    int		 len;
	    unsigned int tstId = NONCTLID;
	    
	    tstId = aChunk->item.gid;
	    len = aChunk->item.len;
	    if (tstId == NONCTLID)
		tmp_xtent += xm_xoc->xoc->methods->wc_extents(xm_xoc->xoc,
				      (wchar_t*)aChunk->item.str, len,
				      overall_ink_return,
				      overall_logical_return);
	    else {
		int		dir = 0, ascent = 0, descent = 0;
		XCharStruct	overall;
		XFontStruct	*tmpFs = NULL;
		
		tmpFs = getCtlFont(xm_xoc, tstId);
		if (TWO_BYTE_FONT(tmpFs))
		    XTextExtents16(tmpFs, (XChar2b*)aChunk->item.str, len,
				   &dir, &ascent, &descent, &overall);
		else
		    XTextExtents(tmpFs, (char*)aChunk->item.str, len,
				 &dir, &ascent, &descent, &overall);
		tmp_xtent += overall.width;
	    }
	    aChunk = aChunk->next;
	}
	overall_logical_return->width = tmp_xtent;
	overall_logical_return->y=  -fontset_ascent;                    /* set using the font set values */
	overall_logical_return->height = fontset_descent+fontset_ascent; /* set using the font set values */
	ret_extent = tmp_xtent;
	freeList(glyphChunk);
    }
    else
	ret_extent = xm_xoc->xoc->methods->wc_extents(xm_xoc->xoc,
						      glyph_text, glyph_num,
						      overall_ink_return,
						      overall_logical_return);
    XmStackFree((char*)oBuf, oBuf_cache);
    return ret_extent;
}

static int
_XmbCTLDrawString(Display	*display,
		Drawable	d,
		XFontSet	fontset,
		GC		gc,
		int		x,
		int		y,
		_Xconst char	*text,
		int		bytes_text)
{
    char	*glyph_text = NULL;
    char	oBuf_cache[CTL_CACHE_SIZE];
    size_t	glyph_bytes = 0;
    INIT_TRANS_DATA
    
    /* Refrain from drawing the last newline character */
    if (text[bytes_text - 1] == '\n')
	bytes_text --;
    if ((bytes_text <= 0) || (!(lo && xm_xoc->layout_active)))
	return 0;
    
    /* Can query ULE to find out no of bytes to allocate */
    glyph_bytes = bytes_text * xm_xoc->layout_max_expand *
		  xm_xoc->layout_shape_charset_size;
    oBuf = (void*)XmStackAlloc(glyph_bytes, oBuf_cache);
    status = _XmXOC_transform_layout(xm_xoc,
                                     lo,
                                     False,
				     text, (size_t)bytes_text,
				     oBuf, &glyph_bytes,
				     NULL, NULL, NULL, &index);
    if (status == 0) /* OK */
	glyph_text = (char*)oBuf;
    else {
	XmeWarning(NULL, "XmbCTLDrawString-Transform\n");
	XmStackFree((char*)oBuf, oBuf_cache);
	return 0;
    }
    
    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	chunkList	*glyphChunk = (chunkList*)XtMalloc(sizeof(chunkList));
	struct chunk	*aChunk;
	
	glyphChunk->numChunks = 0;
	mbMakeChunk(xm_xoc, glyphChunk, glyph_text, glyph_bytes);
	
	aChunk = glyphChunk->head;
	while (aChunk != NULL) {
	    int		 len;
	    unsigned int tstId = NONCTLID;
	    
	    tstId = aChunk->item.gid;
	    len = aChunk->item.len;
	    if (tstId == NONCTLID) {
		XRectangle	ink_ret, logic_ret;
		xm_xoc->xoc->methods->wc_draw_string(display, d,
				     xm_xoc->xoc, gc, x, y,
				     (wchar_t*)aChunk->item.str, len);
		x += xm_xoc->xoc->methods->wc_extents(xm_xoc->xoc,
				      (wchar_t*)aChunk->item.str, len,
				      &ink_ret, &logic_ret);
	    }
	    else {
		XFontStruct	*tmpFs = NULL;
		tmpFs = getCtlFont(xm_xoc, tstId);
		XSetFont(display, gc, tmpFs->fid);
		
		if (TWO_BYTE_FONT(tmpFs)) {
		    XDrawString16(display, d, gc, x, y, (XChar2b*)aChunk->item.str, len);
		    x += XTextWidth16(tmpFs, (XChar2b*)aChunk->item.str, len);
		}
		else {
		    XDrawString(display, d, gc, x, y, (char*)aChunk->item.str, len);
		    x += XTextWidth(tmpFs, (char*)aChunk->item.str, len);
		}
	    }
	    aChunk = aChunk->next;
	}
	freeList(glyphChunk);
    }
    else if (IS2BYTE_FONT(xm_xoc)) {
	int	    fonts_in_fs;
	char	    **font_name_list;
	XFontStruct **font_struct_list;
	
	fonts_in_fs = XFontsOfFontSet(fontset, &font_struct_list, &font_name_list);
	
	if (fonts_in_fs <= 0)
	    XmeWarning(NULL, "XmbDrawString in XmXOC.c\n");
	else {  
	    /* save the font in GC and restore it after drawing if needed */
	    Font	  orig_gc_font;
	    XGCValues	  values;
	    unsigned long valuemask;
	    XFontStruct	  *temp_font_struct;
	    Boolean	  is_font_already_set;
	    
	    valuemask = GCFont; /* Get the old GC font id */
	    XGetGCValues(display, gc, valuemask, &values);
	    orig_gc_font = values.font;
	    
	    XSetFont(display, gc, (*font_struct_list)->fid); /* Set new font id */
	    XDrawString16(display, d, gc, x, y, (XChar2b*)glyph_text, glyph_bytes/2); 
	    
	    /* restore the font id in the gc if the gc had a proper font earlier */
	    temp_font_struct = XQueryFont(display, orig_gc_font);
	    is_font_already_set = (temp_font_struct != NULL && orig_gc_font !=(*font_struct_list)->fid);
	    if (is_font_already_set) { /* Set the new font id */
		values.font = orig_gc_font;
		valuemask = GCFont;
		XChangeGC(display, gc, valuemask, &values);
		/*freeFont(display, temp_font_struct);*/
	    }
	}
    }
    else /* 1 byte font */
      xm_xoc->xoc->methods->mb_draw_string(display, d, xm_xoc->xoc, gc,
					   x, y, glyph_text, glyph_bytes);
    XmStackFree((char*)oBuf, oBuf_cache);
    return 0;
}

static int
_XwcCTLDrawString(Display	  *display,
		  Drawable	  d,
		  XFontSet	  fontset,
		  GC		  gc,
		  int		  x,
		  int		  y,
		  _Xconst wchar_t *text,
 		  int		  num_wchars)
{
    size_t	glyph_num = 0;
    wchar_t	*glyph_text = NULL, oBuf_cache[CTL_CACHE_SIZE];
    INIT_TRANS_DATA

    if (text[num_wchars - 1] == (wchar_t)'\n')
        num_wchars--;
    if ((num_wchars <= 0) || (!(lo && xm_xoc->layout_active)))
	return 0;
    
    glyph_num = num_wchars * xm_xoc->layout_max_expand;
    oBuf = (void*)XmStackAlloc(glyph_num * sizeof(wchar_t), oBuf_cache);
    /* Can query ULE to find out no of bytes to allocate */
    status = _XmXOC_transform_layout(xm_xoc,
                                     lo,
                                     True,
				     text, (size_t)num_wchars,
				     oBuf, &glyph_num,
				     NULL, NULL, NULL, &index);
    if (status == 0) /* OK */
	glyph_text = (wchar_t*)oBuf;
    else {
	XmeWarning(NULL, "XwcCTLDrawString-Transform failure\n");
	XmStackFree((char*)oBuf, oBuf_cache);
	return 0;
    }
    
    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	chunkList	*glyphChunk = (chunkList*)XtMalloc(sizeof(chunkList));
	struct chunk	*aChunk;
	
	glyphChunk->numChunks = 0;
	wcMakeChunk(xm_xoc, glyphChunk, glyph_text, glyph_num);
	
	aChunk = glyphChunk->head;
	while (aChunk != NULL) {
	    int		 len;
	    unsigned int tstId = NONCTLID;
	    
	    tstId = aChunk->item.gid;
	    len = aChunk->item.len;
	    if (tstId == NONCTLID) {
		XRectangle	ink_ret, logic_ret;
		xm_xoc->xoc->methods->wc_draw_string(display, d,
				     xm_xoc->xoc, gc, x, y,
				     (wchar_t*)aChunk->item.str, len);
		x += xm_xoc->xoc->methods->wc_extents(xm_xoc->xoc,
				      (wchar_t*)aChunk->item.str,
				      len, &ink_ret, &logic_ret);
	    }
	    else {
		XFontStruct	*tmpFs = NULL;
		
		tmpFs = getCtlFont(xm_xoc, tstId);
		XSetFont(display, gc, tmpFs->fid);
		if (TWO_BYTE_FONT(tmpFs)) {
		    XDrawString16(display, d, gc, x, y, (XChar2b*)aChunk->item.str, len);
		    x += XTextWidth16(tmpFs, (XChar2b*)aChunk->item.str, len); 
		}
		else {
		    XDrawString(display, d, gc, x, y, (char*)aChunk->item.str, len);
		    x += XTextWidth(tmpFs, (char*)aChunk->item.str, len);
		}
	    }
	    aChunk = aChunk->next;
	}
	freeList(glyphChunk);
    }
    else
      xm_xoc->xoc->methods->wc_draw_string(display, d, xm_xoc->xoc, gc,
					   x, y, glyph_text, glyph_num);
    XmStackFree((char*)oBuf, oBuf_cache);
    return 0;
}

static void
_XmbCTLDrawImageString(Display		*display,
		       Drawable 	d,
		       XFontSet 	fontset,
		       GC		gc,
		       int		x,
		       int		y,
		       _Xconst char	*text,
		       int		bytes_text)
{
    char	*glyph_text = NULL;
    char	oBuf_cache[CTL_CACHE_SIZE];
    size_t	glyph_bytes = 0;
    INIT_TRANS_DATA

    /* Refrain from drawing the last newline character */
    if (text[bytes_text - 1] == '\n')
	bytes_text --;
    if (bytes_text == 0) return;
    if (!(lo && xm_xoc->layout_active)) return;
    
    /* Can query ULE to find out no of bytes to allocate */
    glyph_bytes = bytes_text * xm_xoc->layout_max_expand *
		  xm_xoc->layout_shape_charset_size;
    oBuf = (void*)XmStackAlloc(glyph_bytes, oBuf_cache);
    status = _XmXOC_transform_layout(xm_xoc,
                                     lo,
                                     False,
				     text, (size_t)bytes_text,
				     oBuf, &glyph_bytes,
				     NULL, NULL, NULL, &index);
    if (status == 0) /* OK */
	glyph_text = (char*)oBuf;
    else {
        XmeWarning(NULL, "XmbCTLDrawImageString-Transform failure\n");
	XmStackFree((char*)oBuf, oBuf_cache);
	return;
    }

    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	chunkList	*glyphChunk = (chunkList*)XtMalloc(sizeof(chunkList));
	struct chunk	*aChunk;
	
	glyphChunk->numChunks = 0;
	mbMakeChunk(xm_xoc, glyphChunk, glyph_text, glyph_bytes);
	
	aChunk = glyphChunk->head;
	while (aChunk != NULL) {
	    int		 len;
	    unsigned int tstId = NONCTLID;
	    
	    tstId = aChunk->item.gid;
	    len = aChunk->item.len;
	    if (tstId == NONCTLID) {
		XRectangle	ink_ret, logic_ret;
		
		xm_xoc->xoc->methods->wc_draw_image_string(display, d,
					   xm_xoc->xoc, gc, x, y,
					   (wchar_t*)aChunk->item.str, len);
		x += xm_xoc->xoc->methods->wc_extents(xm_xoc->xoc,
				      (wchar_t*)aChunk->item.str, len,
				      &ink_ret, &logic_ret);
	    }
	    else {
		XFontStruct	*tmpFs = NULL;
		
		tmpFs = getCtlFont(xm_xoc, tstId);
		XSetFont(display, gc, tmpFs->fid);
		
		if (TWO_BYTE_FONT(tmpFs)) {
		    XDrawImageString16(display, d, gc, x, y, (XChar2b*)aChunk->item.str, len);
		    x += XTextWidth16(tmpFs, (XChar2b*)aChunk->item.str, len);
		}
		else {
		    XDrawImageString(display, d, gc, x, y, (char*)aChunk->item.str, len);
		    x += XTextWidth(tmpFs, (char*)aChunk->item.str, len);
		}
	    }
	    aChunk = aChunk->next;
	}
	freeList(glyphChunk);
    }
    else if (IS2BYTE_FONT(xm_xoc)) {
	int	    fonts_in_fs;
	char	    **font_name_list;
	XFontStruct **font_struct_list;
	
	fonts_in_fs = XFontsOfFontSet(fontset, &font_struct_list, &font_name_list);
	
	if (fonts_in_fs <= 0)
	    XmeWarning(NULL, "ERROR No Fonts in Fontset XmbDrawSting\n");
	else {  
	    Font	  orig_gc_font;
	    XGCValues	  values;
	    unsigned long valuemask;
	    XFontStruct	  *temp_font_struct;
	    Boolean	  is_font_already_set;
	    
	    valuemask = GCFont; /* Get the old GC font id */
	    XGetGCValues(display, gc, valuemask, &values);
	    orig_gc_font = values.font;
	    
	    XSetFont(display, gc, (*font_struct_list)->fid); /* Set new font id */
	    XDrawImageString16(display, d, gc, x, y, (XChar2b*)glyph_text, glyph_bytes/2); 
	    
	    /* restore the font id in the gc if the gc had a proper font earlier */
	    temp_font_struct = XQueryFont(display, orig_gc_font);
	    is_font_already_set = (temp_font_struct != NULL && orig_gc_font !=(*font_struct_list)->fid);
	    if (is_font_already_set) { /* Set the new font id */
		values.font = orig_gc_font;
		valuemask = GCFont;
		XChangeGC(display, gc, valuemask, &values);
		/*XFreeFont(display, temp_font_struct);*/
	    }
	    else ;/* forget about restoring it */
	}
    }
    else 
      xm_xoc->xoc->methods->mb_draw_image_string(display, d, xm_xoc->xoc,
						 gc, x, y,
						 glyph_text, glyph_bytes);
    XmStackFree((char*)oBuf, oBuf_cache);
}

static void
_XwcCTLDrawImageString(Display		*display,
		       Drawable		d,
		       XFontSet		fontset,
		       GC		gc,
		       int		x,
		       int		y,
		       _Xconst wchar_t	*text,
		       int		num_wchars)
{
    size_t	glyph_num = 0;
    wchar_t	*glyph_text = NULL;
    wchar_t	oBuf_cache[CTL_CACHE_SIZE];
    INIT_TRANS_DATA

    if (text[num_wchars - 1] == (wchar_t)'\n')
        num_wchars--;
    if (num_wchars <= 0) return;
    if (!(lo && xm_xoc->layout_active)) return;
    
    glyph_num = num_wchars * xm_xoc->layout_max_expand;
    oBuf = (void*)XmStackAlloc(glyph_num * sizeof(wchar_t), oBuf_cache);
    /* Can query ULE to find out no of bytes to allocate */
    status = _XmXOC_transform_layout(xm_xoc,
                                     lo,
                                     True,
				     text, (size_t)num_wchars,
				     oBuf, &glyph_num,
				     NULL, NULL, NULL, &index);
    if (status == 0) /* OK */
	glyph_text = (wchar_t*)oBuf;
    else {
	XmeWarning(NULL, "XwcCTLDrawImageString-Transform failure\n");
	XmStackFree((char*)oBuf, oBuf_cache);
	return;
    }
    
    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	chunkList	*glyphChunk = (chunkList*)XtMalloc(sizeof(chunkList));
	struct chunk	*aChunk;
	
	glyphChunk->numChunks = 0;
	wcMakeChunk(xm_xoc, glyphChunk, glyph_text, glyph_num);
	
	aChunk = glyphChunk->head;
	while (aChunk != NULL) {
	    int		 len;
	    unsigned int tstId = NONCTLID;
	    
	    tstId = aChunk->item.gid;
	    len = aChunk->item.len;
	    if (tstId == NONCTLID) {
		XRectangle	ink_ret, logic_ret;
		
		xm_xoc->xoc->methods->wc_draw_image_string(display, d,
					   xm_xoc->xoc, gc, x, y,
					   (wchar_t*)aChunk->item.str, len);
		x += xm_xoc->xoc->methods->wc_extents(xm_xoc->xoc,
				      (wchar_t*)aChunk->item.str, len,
				      &ink_ret, &logic_ret);
	    }
	    else {
		XFontStruct	*tmpFs = NULL;
		
		tmpFs = getCtlFont(xm_xoc, tstId);
		XSetFont(display, gc, tmpFs->fid);
		if (TWO_BYTE_FONT(tmpFs)) {
		    XDrawImageString16(display, d, gc, x, y, (XChar2b*)aChunk->item.str, len);
		    x += XTextWidth16(tmpFs, (XChar2b*)aChunk->item.str, len); 
		}
		else {
		    XDrawImageString(display, d, gc, x, y, (char*)aChunk->item.str, len);
		    x += XTextWidth(tmpFs, (char*)aChunk->item.str, len);
		}
	    }
	    aChunk = aChunk->next;
	}
	freeList(glyphChunk);
    }
    else 
      xm_xoc->xoc->methods->wc_draw_image_string(display, d, xm_xoc->xoc,
						 gc, x, y,
						 glyph_text, glyph_num);
    XmStackFree((char*)oBuf, oBuf_cache);
}

#define ZERO_WIDTH_CHAR(xrectangle) ((xrectangle).width == 0)
static Status
_XmbCTLTextPerCharExtents(XFontSet	fontset,
			  _Xconst char	*text,
			  int		bytes_text,
			  XRectangle	*ink_array_return,
			  XRectangle	*logical_array_return,
			  int		array_size,
			  int		*num_chars_return,
			  XRectangle	*overall_ink_return,
			  XRectangle	*overall_logical_return)
{
    int		ret_status, i;
    XSegment	perchar_extents_cache[CTL_CACHE_SIZE];
    XSegment	perink_extents_cache[CTL_CACHE_SIZE];
    XSegment	*perchar_extents_segarray = NULL, *perink_extents_segarray = NULL;
    XSegment	seg_overall_ink_return, seg_overall_logical_return;
    size_t	size = sizeof(XSegment);
    
    if (array_size > 0) {
	perchar_extents_segarray = (XSegment*)XmStackAlloc(array_size * size, perchar_extents_cache);
	perink_extents_segarray = (XSegment*)XmStackAlloc(array_size *	size, perink_extents_cache);
    }
    
    ret_status = _XocCTLTextPerCharExtents_mb((XOC)fontset, text, bytes_text,
					      perink_extents_segarray, 
					      perchar_extents_segarray, 
					      array_size, 
					      num_chars_return,
					      &seg_overall_ink_return, 
					      &seg_overall_logical_return);
    if (ret_status) {
	for (i = 0; i < array_size; i++) {
	    _XSegmentToXRectangle(logical_array_return+i, perchar_extents_segarray+i);
	    _XSegmentToXRectangle(ink_array_return+i, perink_extents_segarray+i);
	}
    }
    
    if (overall_ink_return)
	_XSegmentToXRectangle(overall_ink_return, &seg_overall_ink_return);
    if (overall_logical_return)
	_XSegmentToXRectangle(overall_logical_return, &seg_overall_logical_return);
    
    if (array_size > 0) {
	XmStackFree((char*)perchar_extents_segarray, perchar_extents_cache);
	XmStackFree((char*)perink_extents_segarray, perink_extents_cache);
    }
    return ret_status;
}

static Status
_XwcCTLTextPerCharExtents(XFontSet	        fontset,
			  _Xconst wchar_t	*text,
			  int			num_wchars,
			  XRectangle	        *ink_array_return,
			  XRectangle	        *logical_array_return,
			  int			array_size,
			  int			*num_chars_return,
			  XRectangle		*overall_ink_return,
			  XRectangle		*overall_logical_return)
{
    int		ret_status, i;
    XSegment	perchar_extents_cache[CTL_CACHE_SIZE];
    XSegment	perink_extents_cache[CTL_CACHE_SIZE];
    XSegment	*perchar_extents_segarray = NULL, *perink_extents_segarray = NULL;
    XSegment	seg_overall_ink_return, seg_overall_logical_return;
    size_t	size = sizeof(XSegment);
    
    if (array_size > 0) {
	perchar_extents_segarray = (XSegment*)XmStackAlloc(array_size * size, perchar_extents_cache);
	perink_extents_segarray = (XSegment*)XmStackAlloc(array_size *	size, perink_extents_cache);
    }
    
    ret_status = _XocCTLTextPerCharExtents_wc((XOC)fontset, text,
					      num_wchars, 
					      perink_extents_segarray, 
					      perchar_extents_segarray, 
					      array_size, 
					      num_chars_return,
					      &seg_overall_ink_return, 
					      &seg_overall_logical_return);
    if (ret_status) {
	for (i = 0; i < array_size; i++) {
	    _XSegmentToXRectangle(logical_array_return+i, perchar_extents_segarray+i);
	    _XSegmentToXRectangle(ink_array_return+i, perink_extents_segarray+i);
	}
    }
    
    if (overall_ink_return)
	_XSegmentToXRectangle(overall_ink_return, &seg_overall_ink_return);
    if (overall_logical_return)
	_XSegmentToXRectangle(overall_logical_return, &seg_overall_logical_return);
    
    if (array_size > 0) {
	XmStackFree((char*)perchar_extents_segarray, perchar_extents_cache);
	XmStackFree((char*)perink_extents_segarray, perink_extents_cache);
    }
    return ret_status;
}

void _XRectangleToXSegment(Boolean left_to_right, XSegment *seg, XRectangle *rect)
{
    seg->x1 = rect->x + (left_to_right ? 0 : rect->width);
    seg->y1 = rect->y;
    seg->x2 = rect->x + (left_to_right ? rect->width : 0);
    seg->y2 = rect->y + rect->height;
}

void _XSegmentToXRectangle(XRectangle *rect, XSegment *seg)
{
    Boolean right_to_left = (seg->x1 > seg->x2); /* Treat zero width as LTR */
    
    rect->y      = seg->y1;
    rect->height = seg->y2 - seg->y1; /* Caller warrants that y2 > y1 */
    rect->x      = right_to_left ? seg->x2 : seg->x1;
    rect->width  = abs(seg->x2 - seg->x1);
}

static Status
_XocCTLTextPerCharExtents_mb(XOC	xoc,
			  _Xconst char	*text,
			  int		text_bytes,
			  XSegment	*ink_array_return,
			  XSegment	*logical_array_return,
			  int		array_size,
			  int		*num_chars_return,
			  XSegment	*overall_ink_return,
			  XSegment	*overall_logical_return)
{
    Status	ret_status = 0;
    
    /* All tmp rectangles correspond to glyph rect */
    XRectangle	*glyph_ink    = NULL;
    XRectangle	*glyph_log    = NULL;
    XRectangle	glyph_overall_ink;
    XRectangle	glyph_overall_logical;
    int		in_i, out_i;
    Boolean	l_to_r;
    
    size_t	glyph_bytes;
    int		char_num, glyph_num;
    
    /* char_bytes == test_bytes */
    XFontSet    fontset = (XFontSet)xoc;
        
    int		curr_byte_pos;
    int		curr_glyph_pos;
    int		curr_char_pos;
    size_t	*gpos2cpos = NULL;
    size_t	*cpos2gpos = NULL;
    
    char	*text_passed = (char*)text;
    char	*glyph_str;

    char	oBuf_cache[CTL_CACHE_SIZE];
    size_t	cpos_cache[CTL_CACHE_SIZE];
    size_t	gpos_cache[CTL_CACHE_SIZE];
    XRectangle	glyphink_cache[CTL_CACHE_SIZE];
    XRectangle	glyphlog_cache[CTL_CACHE_SIZE];
    int		sizet = sizeof(size_t);
    wchar_t     tmpwchar_cache[CTL_CACHE_SIZE], *tmpwchar_text;

    INIT_TRANS_DATA
    INIT_CACHE_DATA
    
    *num_chars_return = 0;
    tmpwchar_text= (wchar_t*)XmStackAlloc(text_bytes * sizeof(wchar_t), tmpwchar_cache);
    char_num= mbstowcs(tmpwchar_text, text_passed, text_bytes);
    
    if (char_num <= 0) return 0;
    
    if (!(lo && xm_xoc->layout_active)) return 0;
    
    glyph_bytes = text_bytes * xm_xoc->layout_max_expand * 
		  xm_xoc->layout_shape_charset_size;
    oBuf = (void*)XmStackAlloc(glyph_bytes, oBuf_cache);
    i2o = (size_t*)XmStackAlloc(text_bytes * sizet, i2o_cache);
    o2i = (size_t*)XmStackAlloc(glyph_bytes * sizet, o2i_cache);
    props = (unsigned char*)XmStackAlloc(text_bytes, props_cache);
    
    /* after transformation obuf contains transformed string */
    status = _XmXOC_transform_layout(xm_xoc, lo, False, text_passed, text_bytes,
				     oBuf, &glyph_bytes, i2o, o2i, props, &index);
    if (status == 0) /* OK */
	glyph_str = (char*)oBuf;
    else {
	XmeWarning(NULL, "XocCTLTextPerCharExtents_mb-Transform failure\n");
	FREE_CACHE_DATA
	return 0;
    }
  
    if (IS2BYTE_FONT(xm_xoc) || MULTIFONT(xm_xoc))
	glyph_num = glyph_bytes / xm_xoc->layout_shape_charset_size;
    else
	glyph_num = glyph_bytes;
    
    cpos2gpos = (size_t*)XmStackAlloc(text_bytes * sizet, cpos_cache);
    gpos2cpos = (size_t*)XmStackAlloc(glyph_num * sizet, gpos_cache);
    
    /* Convert glyph byte mapping onto char byte mapping into glyph pos
       mapping on to char pos mapping */
    curr_glyph_pos = 0;
    curr_byte_pos = 0;
    while (curr_byte_pos < glyph_bytes) {
	gpos2cpos[curr_glyph_pos] = o2i[curr_byte_pos];
	if (IS2BYTE_FONT(xm_xoc) || MULTIFONT(xm_xoc))
	    curr_byte_pos += xm_xoc->layout_shape_charset_size;
	else
	    curr_byte_pos++;
	curr_glyph_pos++;
    }
    
    /* Convert char byte mapping onto glyph byte mapping into character pos
       mapping on to glyph pos mapping */
    curr_char_pos = 0;
    curr_byte_pos = 0;
    while (curr_byte_pos < text_bytes) {
	cpos2gpos[curr_char_pos] = i2o[curr_byte_pos];
	curr_byte_pos++;
	curr_char_pos++;
    }
    
    glyph_ink = (XRectangle*)XmStackAlloc(glyph_num * sizeof(XRectangle), glyphink_cache);
    glyph_log = (XRectangle*)XmStackAlloc(glyph_num * sizeof(XRectangle), glyphlog_cache);

    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	unsigned int	tstId = NONCTLID;
	int		ct = 0, tmp_xtent = 0, curr_x = 0;
	int		dir = 0, ascent=0, descent=0;
	wchar_t		to_draw_wc, byte1, byte2, byte3, byte4;
	XRectangle	one_tmp_ink, one_tmp_log;
	XCharStruct	overall;
	XFontStruct	*tmp_fstr = NULL;
	int             fontset_ascent;
	int             fontset_descent;
	XFontSetExtents *fs_extents = NULL;

	/*fill the vertical components of the string with the max values in the fontset */
	fs_extents      = XExtentsOfFontSet((XFontSet)fontset);
	fontset_ascent  = -fs_extents->max_logical_extent.y;
	fontset_descent =  fs_extents->max_logical_extent.height + fs_extents->max_logical_extent.y;	
	
 	for (ct = 0, curr_glyph_pos = 0; ct < glyph_bytes; curr_glyph_pos++) {
	    tstId = ctlChar(xm_xoc, (void*)glyph_str, ct);
	    if (tstId != NONCTLID) {
		tmp_fstr = getCtlFont(xm_xoc, tstId);
		ct += 2;
		
		if (TWO_BYTE_FONT(tmp_fstr))
		  XTextExtents16(tmp_fstr, (XChar2b*)&glyph_str[ct], 1,
				 &dir, &ascent, &descent, &overall);
		else
		  XTextExtents(tmp_fstr, (char*)&glyph_str[ct + 1], 1, &dir,
			       &ascent, &descent, &overall);
		
		/* Fill the current char rectangle manually */
		glyph_log[curr_glyph_pos].x = curr_x;
		glyph_log[curr_glyph_pos].y = -ascent;
		glyph_log[curr_glyph_pos].width = overall.width;
		glyph_log[curr_glyph_pos].height = ascent + descent;
		curr_x += overall.width;
		tmp_xtent += curr_x;
		ct += 2;
	    }
	    else { /* Not a CTL Character */
		byte4 = 0x00FFFFFF | (glyph_str[ct] << 24);
		byte3 = 0xFF00FFFF | (glyph_str[ct + 1] << 16);
		byte2 = 0xFFFF00FF | (glyph_str[ct + 2] << 8);
		byte1 = 0xFFFFFF00 | glyph_str[ct + 3];
		to_draw_wc = byte4 & byte3 & byte2 & byte1;
		xm_xoc->xoc->methods->wc_extents_per_char(xm_xoc->xoc,
				       &to_draw_wc, 1,
				       &one_tmp_ink, &one_tmp_log,
				       1, num_chars_return,
				       &glyph_overall_ink,
				       &glyph_overall_logical);
		
		glyph_log[curr_glyph_pos].x = curr_x;
		glyph_log[curr_glyph_pos].y = one_tmp_log.y;
		glyph_log[curr_glyph_pos].width = one_tmp_log.width;
		glyph_log[curr_glyph_pos].height = one_tmp_log.height;
		curr_x += one_tmp_log.width;
		tmp_xtent += curr_x;
		glyph_overall_logical.width = curr_x;
		ct += 4;
	    }
	}
	/* Fill the overall rectangle */
	glyph_overall_logical.x = 0;
	glyph_overall_logical.y = -fontset_ascent; 
	glyph_overall_logical.width = curr_x;
        glyph_overall_logical.height = fontset_descent+fontset_ascent;
	*num_chars_return = glyph_num;
	ret_status = tmp_xtent;
    }
    else if (IS2BYTE_FONT(xm_xoc)) {  /* 16  bit font support */
	int	 num_fonts_in_fontset;
	char	 **font_name_list;
	XFontStruct **font_struct_list;
	XCharStruct overall;
	int	 dir, ascent, descent;
	
	num_fonts_in_fontset = XFontsOfFontSet(fontset, &font_struct_list, &font_name_list);
	if (num_fonts_in_fontset <= 0)
	    XmeWarning(NULL, "Error XmXOC.c:_XocCTLTextPerCharExtents\n");
	else { /* Note: No support for multipile fonts in fontset */
	    int	i = 0, curr_x = 0;
	    _Xconst char *glyph_text_ptr = glyph_str;
	    
	    dir = 0; ascent = 0; descent = 0;
	    for (i = 0; i < glyph_num; i++) {
		
		/* Unfortunately we don't have a single Xlib Fn to get TextPerCharExtents */
		XTextExtents16(*font_struct_list, (XChar2b*)glyph_text_ptr, 1, &dir,
			       &ascent, &descent, &overall);
		/* Fill the current char rectangle manually */
		glyph_log[i].x = curr_x;
		glyph_log[i].y = -ascent;
		glyph_log[i].width = overall.width;
		glyph_log[i].height = ascent + descent;
		curr_x += overall.width;
		glyph_text_ptr += 2; /* increment the pointer by 2 bytes. For 2 byte font */
	    }
	    /* Fill the overall rectangle */
	    glyph_overall_logical.x = 0;
	    glyph_overall_logical.y = -ascent;
	    glyph_overall_logical.width = curr_x;
	    glyph_overall_logical.height = ascent + descent;
	    *num_chars_return = glyph_num;
	    ret_status =curr_x;
	}
    }
    else /* Single byte font */
	ret_status = xm_xoc->xoc->methods->mb_extents_per_char(xm_xoc->xoc,
			       glyph_str, glyph_bytes, glyph_ink, glyph_log,
			       glyph_bytes, num_chars_return,
			       &glyph_overall_ink, &glyph_overall_logical);
    /* Now we have to convert the XRectangle* return arrays of XmbTextPerCharExtents() 
       into the XSegment* return arrays of XocTextPerCharExtents() 
       */
    if (overall_ink_return)	/* can be NULL */
	_XRectangleToXSegment(True, overall_ink_return, &glyph_overall_ink);
    
    if (overall_ink_return)
	_XRectangleToXSegment(True, overall_logical_return, &glyph_overall_logical);

    /* Follow Xlibrary Convention : Prabhat */
    if (array_size < text_bytes) {
	*num_chars_return = text_bytes;
	FREE_CACHE_DATA
	XmStackFree((char*)gpos2cpos, gpos_cache);
	XmStackFree((char*)cpos2gpos, cpos_cache);
	XmStackFree((char*)glyph_log, glyphlog_cache);
	XmStackFree((char*)glyph_ink, glyphink_cache);
	XmStackFree((char*)tmpwchar_text, tmpwchar_cache);
	return 0;
    }
    else {
	/* this check will fail for languages which has
	   many2one char glyphs and one2many glyph chars */
	if (glyph_num == char_num) {
	    for (out_i = 0; out_i < glyph_num; out_i++) {
		in_i = gpos2cpos[out_i];
		/* nestlevel odd=RTL, even=LTR (see PLS w_transform_layout() Examples) */
		l_to_r = !(Boolean)((props[in_i] & NESTLEVEL_MASK) & 0x01);
		_XRectangleToXSegment(l_to_r, ink_array_return+in_i, glyph_ink+out_i);
		_XRectangleToXSegment(l_to_r, logical_array_return+in_i, glyph_log+out_i);
	    }
	}
	else { /* one2many or many2one*/
	    out_i = 0;
	    while (out_i < glyph_num) {
		int st_pos, end_pos;
		int combo_glyph_char_st, combo_glyph_char_end;
		
		st_pos = end_pos = out_i;
		in_i = gpos2cpos[st_pos];
		
		/* find the character in the output buffer */
		while (end_pos < glyph_num && gpos2cpos[st_pos] == gpos2cpos[end_pos])
		    end_pos++;
		
		l_to_r = !(Boolean)((props[in_i] & NESTLEVEL_MASK) & 0x01);
		/* Determine its extents */
		if (end_pos - st_pos == 1) { /* inp char is mapped onto single glyph */
		    _XRectangleToXSegment(l_to_r, ink_array_return+in_i, glyph_ink+out_i);
		    _XRectangleToXSegment(l_to_r, logical_array_return+in_i, glyph_log+out_i);
		}
		else if (end_pos - st_pos > 1) {
		    XRectangle temp_rect;
		    
		    temp_rect = glyph_log[st_pos];
		    temp_rect.width = (glyph_log[end_pos - 1].x +
				       glyph_log[end_pos - 1].width) -
				      glyph_log[st_pos].x;
		    _XRectangleToXSegment(l_to_r, logical_array_return+in_i, &temp_rect);
		    *num_chars_return -=1; /*two glyphs has been combined: moatazm*/
		}
		else; /* ERROR */
		
		/* Handle the combo character */
		/* Now find out the num of chars which correspond to this glyph i.e
		   many characters mapped onto a single glyph
		   in arabic they are called the combo characters */
		
		combo_glyph_char_st = combo_glyph_char_end = in_i;
		while (combo_glyph_char_end + 1 < char_num && cpos2gpos[in_i] == cpos2gpos[combo_glyph_char_end+1])
		    combo_glyph_char_end++;
		while (combo_glyph_char_st - 1 > 0 && cpos2gpos[in_i] == cpos2gpos[combo_glyph_char_st - 1])
		    combo_glyph_char_st--;
		
		if (combo_glyph_char_end > combo_glyph_char_st) { 
		    /* multiple characters for the same glyphs */
		    int j;
		    /* average_width, total_width of comboglyph accumulated width */
		    int avg_width, total_width, acc_width = 0;
		    /* compute the width of each of character
		       Assumption: We are assigning equal width to each of the
		       characters which belong to the same combo
		       */
		    total_width =  (logical_array_return[in_i].x2 - logical_array_return[in_i].x1);
		    avg_width = total_width / (combo_glyph_char_end - combo_glyph_char_st + 1);
		    /* Fill the first character */
		    logical_array_return[combo_glyph_char_st].x1 = logical_array_return[in_i].x1;
		    logical_array_return[combo_glyph_char_st].x2 = logical_array_return[in_i].x1 + avg_width;
		    logical_array_return[combo_glyph_char_st].y1 = logical_array_return[in_i].y1;
		    logical_array_return[combo_glyph_char_st].y2 = logical_array_return[in_i].y2;
		    acc_width += avg_width;

		    *num_chars_return +=1; /*the combo glyph is divided into two glyps:moatazm */
		    /* Fill rest of the characters per char extents   */
		    /* For two char combo glyph this loop doesn't gets executed */
		    for (j = combo_glyph_char_st + 1; j <= combo_glyph_char_end - 1; j++){
			logical_array_return[j].x1 = logical_array_return[j - 1].x2;
			logical_array_return[j].x2 = logical_array_return[j - 1].x2 + avg_width;
			logical_array_return[j].y1 = logical_array_return[j - 1].y1;
			logical_array_return[j].y2 = logical_array_return[j - 1].y2;
			acc_width += avg_width;
		    }
		    /* Fill the last characters per char extents */
		    /* The last character will get all the remaining width of the combo glyph */
		    
		    logical_array_return[j].x1 = logical_array_return[j - 1].x2;
		    /* assign the remaining width to the last character of the combo glyph */
		    logical_array_return[j].x2 = logical_array_return[j - 1].x2 + (total_width - acc_width);
		    logical_array_return[j].y1 = logical_array_return[j - 1].y1;
		    logical_array_return[j].y2 = logical_array_return[j - 1].y2;
		}
		out_i = end_pos;
	    } /* while .. */
	} /* else one2many == True */
    }
    FREE_CACHE_DATA
    XmStackFree((char*)gpos2cpos, gpos_cache);
    XmStackFree((char*)cpos2gpos, cpos_cache);
    XmStackFree((char*)glyph_log, glyphlog_cache);
    XmStackFree((char*)glyph_ink, glyphink_cache);
    XmStackFree((char*)tmpwchar_text, tmpwchar_cache);
    return ret_status;
}

static Status
_XocCTLTextPerCharExtents_wc(XOC                xoc,
			     _Xconst wchar_t	*text,
			     int	        num_chars,
			     XSegment		*ink_array_return,
			     XSegment		*logical_array_return,
			     int	        array_size,
			     int	        *num_chars_return,
			     XSegment		*overall_ink_return,
			     XSegment		*overall_logical_return)
{
    int		in_i, out_i;
    Status	ret_status = 0;
    XRectangle	*glyph_ink = NULL, *glyph_log = NULL;
    XRectangle	glyph_overall_ink, glyph_overall_logical;
    Boolean	l_to_r;
    
    int		char_num = num_chars;
    size_t	glyph_num;
    XFontSet	fontset = (XFontSet)xoc;
    
    int		sizet = sizeof(size_t);
    wchar_t	*glyph_str;
    wchar_t	oBuf_cache[CTL_CACHE_SIZE];
    XRectangle	glyphink_cache[CTL_CACHE_SIZE];
    XRectangle	glyphlog_cache[CTL_CACHE_SIZE];
    
    /* Buffers used during transformation : Need to be freed after use */
    INIT_TRANS_DATA
    INIT_CACHE_DATA
    
    *num_chars_return = 0;
    
    if (char_num <= 0) return 0;
    if (!(lo && xm_xoc->layout_active)) return 0;
    
    /* after transformation obuf contains transformed string */
    glyph_num = char_num * xm_xoc->layout_max_expand;
    oBuf = (void*)XmStackAlloc(glyph_num * sizeof(wchar_t), oBuf_cache);
    i2o = (size_t*)XmStackAlloc(num_chars * sizet, i2o_cache);
    o2i = (size_t*)XmStackAlloc(glyph_num * sizet, o2i_cache);
    props = (unsigned char*)XmStackAlloc(num_chars, props_cache);
    
    /* after transformation obuf contains transformed string */
    status = _XmXOC_transform_layout(xm_xoc, lo, True, text, num_chars,
				     oBuf, &glyph_num, i2o, o2i, props, &index);
    if (status == 0) /* OK */
	glyph_str = (wchar_t*)oBuf;
    else {
	XmeWarning(NULL, "XocCTLTextPerCharExtents_wc-transform failure\n");
	FREE_CACHE_DATA
	return 0;
    }
    
    glyph_ink = (XRectangle*)XmStackAlloc(glyph_num * sizeof(XRectangle), glyphink_cache);
    glyph_log = (XRectangle*)XmStackAlloc(glyph_num * sizeof(XRectangle), glyphlog_cache);
    
    if (MULTIFONT(xm_xoc)) { /* UTF8 */
	unsigned int	tstId = NONCTLID;
	int		dir = 0, ascent = 0, descent = 0;
	int		currX = 0, curr_glyph_pos = 0, tmp_xtent = 0;
	XRectangle	one_glyph_ink, one_glyph_log;
	XCharStruct	overall;
	XFontStruct	*tmp_fs = NULL;
	unsigned char	scratch[2], id;
	int             fontset_ascent;
	int             fontset_descent;
	XFontSetExtents *fs_extents = NULL;

	/* fill the vertical components of the string with the max values in the fontset */
	fs_extents      = XExtentsOfFontSet((XFontSet)fontset);
	fontset_ascent  = -fs_extents->max_logical_extent.y;
	fontset_descent =  fs_extents->max_logical_extent.height + fs_extents->max_logical_extent.y;	

	for (curr_glyph_pos = 0; curr_glyph_pos < glyph_num; curr_glyph_pos++) {
	    tstId = ctlChar(xm_xoc, (void*)&glyph_str[curr_glyph_pos], WC_MFONT);
	    if (tstId != NONCTLID) {
		tmp_fs = getCtlFont(xm_xoc, tstId);
		
		if (TWO_BYTE_FONT(tmp_fs)) {
		    scratch[0] = (u_int)((glyph_str[curr_glyph_pos] & 0x0000FF00) >> 8);
		    scratch[1] = (u_int)(glyph_str[curr_glyph_pos] & 0x000000FF);
		    XTextExtents16(tmp_fs, 
				   (XChar2b*)scratch, 1,
				   &dir, 
				   &ascent, &descent, 
				   &overall);
		}
		else {
		    id = (u_int)(glyph_str[curr_glyph_pos] & 0x000000FF);
		    XTextExtents(tmp_fs, 
				 (char*)&id, sizeof(id), 
				 &dir,
				 &ascent, &descent, 
				 &overall);
		}
		/* Fill the current char rectangle manually */
		glyph_log[curr_glyph_pos].x = currX;
	        glyph_log[curr_glyph_pos].y = -ascent;
		glyph_log[curr_glyph_pos].width = overall.width;
		glyph_log[curr_glyph_pos].height = ascent + descent;
		currX += overall.width;
		tmp_xtent = currX;
	    }
	    else { /* Not a CTL Character */
		xm_xoc->xoc->methods->wc_extents_per_char(xm_xoc->xoc,
				  (wchar_t*)(&glyph_str[curr_glyph_pos]), 1,
				  &one_glyph_ink, &one_glyph_log,
				  1, num_chars_return,
				  &glyph_overall_ink,
				  &glyph_overall_logical);
		
		glyph_log[curr_glyph_pos].x = currX;
		glyph_log[curr_glyph_pos].y = one_glyph_log.y;
		glyph_log[curr_glyph_pos].width = one_glyph_log.width;
		glyph_log[curr_glyph_pos].height = one_glyph_log.height;
		currX += one_glyph_log.width;
		tmp_xtent = currX;
		glyph_overall_logical.width = currX;
	    }
	} /* curr_glyph_pos < glyph_num */
	
	/* Fill the overall rectangle */
	glyph_overall_logical.x = 0;
	glyph_overall_logical.y = -fontset_ascent;
	glyph_overall_logical.width = currX;
	glyph_overall_logical.height = fontset_ascent + fontset_descent;
	ret_status = tmp_xtent;
    } /* MFont */
    else
	ret_status = xm_xoc->xoc->methods->wc_extents_per_char(xm_xoc->xoc,
				       glyph_str, glyph_num,
				       glyph_ink, glyph_log,
				       array_size, num_chars_return,
				       &glyph_overall_ink,
				       &glyph_overall_logical);
    *num_chars_return = glyph_num;
    
    /* Now we have to convert the XRectangle* return arrays of XmbTextPerCharExtents()
       into the XSegment* return arrays of XocTextPerCharExtents() */
    if (overall_ink_return)     /* can be NULL */
	_XRectangleToXSegment(True, overall_ink_return, &glyph_overall_ink);
    if (overall_ink_return)
	_XRectangleToXSegment(True, overall_logical_return, &glyph_overall_logical);

    /* Follow Xlibrary Convention : Prabhat */
    if (array_size < num_chars) {
	*num_chars_return = num_chars;
	FREE_CACHE_DATA
	XmStackFree((char*)glyph_log, glyphlog_cache);
	XmStackFree((char*)glyph_ink, glyphink_cache);
	return 0;
    }
    else {
	if (glyph_num == char_num) {
	    for (out_i = 0; out_i < glyph_num; out_i++) {
		/* nestlevel odd=RTL, even=LTR (see PLS w_transform_layout() Examples) */
		in_i = o2i[out_i];
		l_to_r = !(Boolean)((props[in_i] & NESTLEVEL_MASK) & 0x01);
		
		_XRectangleToXSegment(l_to_r, ink_array_return+in_i, glyph_ink+out_i);
		_XRectangleToXSegment(l_to_r, logical_array_return+in_i, glyph_log+out_i);
	    }
	}
	else { /* one2many or many2one*/
	    out_i = 0;
	    while (out_i < glyph_num) {
		int st_pos, end_pos;
		int combo_glyph_char_st, combo_glyph_char_end;
		
		st_pos = end_pos = out_i;
		in_i = o2i[st_pos];
		
		/* find the character in the output buffer */
		while (end_pos < glyph_num && o2i[st_pos] == o2i[end_pos])
		    end_pos++;
		
		l_to_r = !(Boolean)((props[in_i] & NESTLEVEL_MASK) & 0x01);
		/* Determine its extents */
		if (end_pos - st_pos == 1) { /* inp char is mapped onto single glyph */
		    _XRectangleToXSegment(l_to_r, ink_array_return+in_i, glyph_ink+out_i);
		    _XRectangleToXSegment(l_to_r, logical_array_return+in_i, glyph_log+out_i);
		}
		else if (end_pos - st_pos > 1) {
		    XRectangle temp_rect;
		    
		    temp_rect = glyph_log[st_pos];
		    temp_rect.width = (glyph_log[end_pos - 1].x + glyph_log[end_pos - 1].width) - glyph_log[st_pos].x;
		    _XRectangleToXSegment(l_to_r, logical_array_return+in_i, &temp_rect);
		    *num_chars_return -=1; /*two glyphs has been combined : moatazm*/
		}
		else; /* ERROR */
		
		/* Handle the combo character */
		/* Now find out the num of chars which correspond to this glyph.
		   i.e many characters mapped onto a single glyph.
		   In arabic they are called the combo characters */
		
		combo_glyph_char_st = combo_glyph_char_end = in_i;
		while (combo_glyph_char_end + 1 < char_num && i2o[in_i] == i2o[combo_glyph_char_end+1])
		    combo_glyph_char_end++;
		while (combo_glyph_char_st - 1 > 0 && i2o[in_i] == i2o[combo_glyph_char_st - 1])
		    combo_glyph_char_st--;
		
		if (combo_glyph_char_end > combo_glyph_char_st) {
		    int j;
		    /* average_width, total_width of coboglyph, accumulated width */
		    int avg_width, total_width, acc_width = 0;
		    /* compute the width of each of character
		       Assumption: We are assinging equal widht to each of the
		       characters which belong to the same combo
		       */
		    total_width =  (logical_array_return[in_i].x2 - logical_array_return[in_i].x1);
		    avg_width = total_width / (combo_glyph_char_end - combo_glyph_char_st + 1);
		    /* Fill the first character */
		    logical_array_return[combo_glyph_char_st].x1 = logical_array_return[in_i].x1;
		    logical_array_return[combo_glyph_char_st].x2 = logical_array_return[in_i].x1 + avg_width;
		    logical_array_return[combo_glyph_char_st].y1 = logical_array_return[in_i].y1;
		    logical_array_return[combo_glyph_char_st].y2 = logical_array_return[in_i].y2;
		    acc_width += avg_width;
		    
		    *num_chars_return +=1; /*the combo glyph is divided into two glyphs: moatazm*/
		    /* Fill rest of the characters per char extents   */
		    /* For two char combo glyph this loop doesn't gets executed */
		    for (j = combo_glyph_char_st + 1; j <= combo_glyph_char_end - 1; j++){
			logical_array_return[j].x1 = logical_array_return[j - 1].x2;
			logical_array_return[j].x2 = logical_array_return[j - 1].x2 + avg_width;
			logical_array_return[j].y1 = logical_array_return[j - 1].y1;
			logical_array_return[j].y2 = logical_array_return[j - 1].y2;
			acc_width += avg_width;
		    }
		    /* Fill the last characters per char extents */
		    /* The last character will get all the remaining width of the combo glyph */
		    
		    logical_array_return[j].x1 = logical_array_return[j - 1].x2;
		    /* assign the remaining width to the last character of the combo glyph */
		    logical_array_return[j].x2 = logical_array_return[j - 1].x2 + (total_width - acc_width);
		    logical_array_return[j].y1 = logical_array_return[j - 1].y1;
		    logical_array_return[j].y2 = logical_array_return[j - 1].y2;
		}
		out_i = end_pos;
	    } /* while .. */
	} /* else one2many == True */
    }
    XmStackFree((char*)glyph_log, glyphlog_cache);
    XmStackFree((char*)glyph_ink, glyphink_cache);
    FREE_CACHE_DATA
    return ret_status;
}

static Status
_XocCTLTextPerCharExtents(XOC           oc,
			  _Xconst void *string,
			  Boolean       is_wchar, 
			  int           num_chars,
			  XSegment     *ink_array_return,
			  XSegment     *logical_array_return,
			  int           array_size,
			  int          *num_chars_return,
			  XSegment     *overall_ink_return,
			  XSegment     *overall_logical_return)
{
    if (is_wchar)
	_XocCTLTextPerCharExtents_wc(oc,
				     (wchar_t*)string,
				     num_chars,
				     ink_array_return,
				     logical_array_return,
				     array_size,
				     num_chars_return,
				     overall_ink_return,
				     overall_logical_return);
    else
	_XocCTLTextPerCharExtents_mb(oc,
				     (char*)string,
				     num_chars,
				     ink_array_return,
				     logical_array_return,
				     array_size,
				     num_chars_return,
				     overall_ink_return,
				     overall_logical_return);
}

#define IS_CELL_START(a) (((a) & DISPLAYCELL_MASK) || ((a) & 020))
Status 
XocCellScan(XFontSet               fontset,
	    char                  *string,
	    Boolean                is_wchar, 
	    int                    num_chars,
	    XmTextPosition         position,
	    XmTextScanDirection    dir,
	    XmTextPosition         *start_pos)
{
    int		glyph_num = num_chars, i = 0;
    Boolean	in_cell;
    size_t	oSize = 0, tmpGlyphs = 0;
    
    wchar_t	oBuf_wc[CTL_CACHE_SIZE];
    char	oBuf_mb[CTL_CACHE_SIZE];
    
    INIT_TRANS_DATA
    INIT_CACHE_DATA
    
    if (position == num_chars && dir == XmsdRight) {
	*start_pos = num_chars;
	return (LINE_END);
    }
    
    if (position == 0 && dir == XmsdLeft) {
	*start_pos = 0;
	return (LINE_START);
    }
    
    if (num_chars < 0) {
	*start_pos = 0;
	return (NO_CELL);
    }
    
    if (num_chars == 0) {
	*start_pos = 0;
	if (dir == XmsdLeft)
	    return (LINE_START);
	else /* dir == XmsdRight */
	    return (LINE_END);
    }

    oSize = num_chars * xm_xoc->layout_max_expand;
    oSize *= (is_wchar ? sizeof(wchar_t) : xm_xoc->layout_shape_charset_size);
    tmpGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);

    oBuf = (void*) (is_wchar ? XmStackAlloc(oSize, oBuf_wc) :
		    	       XmStackAlloc(oSize, oBuf_mb));
    i2o = (size_t*) XmStackAlloc(num_chars * sizeof(size_t), i2o_cache);
    o2i = (size_t*) XmStackAlloc(tmpGlyphs * sizeof(size_t), o2i_cache);
    props = (unsigned char*) XmStackAlloc(num_chars, props_cache);
    oSize = tmpGlyphs;
    
    status = _XmXOC_transform_layout(xm_xoc,
				     lo,
				     is_wchar,
				     string, num_chars,
				     oBuf, &oSize,
				     i2o, o2i, props,
				     &index);
    if (status != 0) {
        XmeWarning(NULL, "XocCellScan-Transform failure\n");
	FREE_DUAL_CACHE_DATA
	*start_pos = 0;
	return (NO_CELL);
    }
    
    if (IS2BYTE_FONT(xm_xoc) || (MULTIFONT(xm_xoc) && (is_wchar == False)))
       glyph_num = oSize / xm_xoc->layout_shape_charset_size;
    else
	glyph_num = oSize;
    
    /* Reminder: This won't work for visual */
    if (dir == XmsdRight) {
	for (i = position + 1; i < glyph_num; i++)
	    if (!props || IS_CELL_START(props[i])) /* bugfix 4133745 leob */
		break;
	*start_pos = i;
    }
    else {  /* dir == XmsdLeft */
	for (i = position - 1; i > 0; i--)
	    if (!props || IS_CELL_START(props[i])) /* bugfix 4133745 leob */
		break;
	*start_pos = i;
    }
    
    FREE_DUAL_CACHE_DATA
    return 0;
}

int
XocFindCell(XFontSet               fontset,
	    char                  *string,
	    Boolean                is_wchar, 
	    int                    num_chars,
	    XmTextPosition         position,
	    XmTextScanDirection    dir,
	    XmTextPosition         *start_pos,
	    XmTextPosition         *end_pos)
{
    int		glyph_num = num_chars, i = 0;
    size_t	oSize = 0, tmpGlyphs = 0;
    Boolean	in_cell;

    wchar_t	oBuf_wc[CTL_CACHE_SIZE];
    char	oBuf_mb[CTL_CACHE_SIZE];

    INIT_TRANS_DATA
    INIT_CACHE_DATA

    if (position == num_chars && dir == XmsdRight)
	return (LINE_END);
    if (position == 0 && dir == XmsdLeft)
	return (LINE_START);
    
    if (num_chars < 0) {
	*start_pos = 0;
	return (NO_CELL);
    }
    
    if (num_chars == 0) {
	*start_pos = 0;
	if (dir == XmsdLeft)
	    return (LINE_START);
	else
	    return (LINE_END);
    }

    oSize = num_chars * xm_xoc->layout_max_expand;
    oSize *= (is_wchar ? sizeof(wchar_t) : xm_xoc->layout_shape_charset_size);
    tmpGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);

    oBuf = (void*) (is_wchar ? XmStackAlloc(oSize, oBuf_wc) :
		    	       XmStackAlloc(oSize, oBuf_mb));
    i2o = (size_t*) XmStackAlloc(num_chars * sizeof(size_t), i2o_cache);
    o2i = (size_t*) XmStackAlloc(tmpGlyphs * sizeof(size_t), o2i_cache);
    props = (unsigned char*) XmStackAlloc(num_chars, props_cache);
    oSize = tmpGlyphs;

    status = _XmXOC_transform_layout(xm_xoc,
				     lo,
				     is_wchar,
				     string, num_chars,
				     oBuf, &oSize,
				     i2o, o2i, props,
				     &index);
    if (status != 0) {
        XmeWarning(NULL, "XocFindCell-Transform failure\n");
	FREE_DUAL_CACHE_DATA
	*start_pos = 0;
	return (NO_CELL);
    }
    
    if (IS2BYTE_FONT(xm_xoc) || (MULTIFONT(xm_xoc) && (is_wchar == False)))
       glyph_num = oSize / xm_xoc->layout_shape_charset_size;
    else
	glyph_num = oSize;

    if (dir == XmsdRight) {
	/* find the starting of the next cell */
	for (i = position; i < glyph_num; i++)
	    if (IS_CELL_START(props[i])) break;
	*start_pos = i;
	if (i == glyph_num) {
	    *end_pos = i;
	    FREE_DUAL_CACHE_DATA
	    return (LINE_END);
	}
	for (i++; i < glyph_num; i++)
	    if (IS_CELL_START(props[i])) break;
	*end_pos = i;
    }
    else { /* dir == XmsdLeft */
	
	if (position == 0) {
	    *end_pos = *start_pos = 0;
	    FREE_DUAL_CACHE_DATA
	    return (LINE_START);
	}
	/* get to start position of left Cell */
	for (i = position - 1; i > 0; i--)
	    if (IS_CELL_START(props[i])) break;
	*start_pos = i;
	/* get to end position of left cell */
	for (i = position;i < glyph_num; i++)
	    if (IS_CELL_START(props[i])) break;
	/* End of previous/left cell is start of current cell */
	*end_pos = i;
    }

    FREE_DUAL_CACHE_DATA
    return (0);
}

Status
XocLogicalScan(XFontSet               fontset,
	      char                  *string,
	      Boolean                is_wchar, 
	      int                    num_chars,
	      int                    position,
	      XocPosSelectionType    pos_sel_type,
	      XmTextScanType         stype,
	      XmTextScanDirection    dir,
	      XmTextPosition        *new_pos)
{
    if (pos_sel_type == XocRELATIVE_POS) {
	switch (stype) {
	    case  XmSELECT_CELL: {
		return 1; /* XocCellScan(fontset, string, is_wchar,
			     num_chars, position, dir, new_pos); */
		break;
	    }
	    default:
		XmeWarning(NULL, "Only Cell is supported in XOC for logicalScan\n");
	}
    }
}

/* Assumption : This routine assumes that cursor is always 
   		aligned to the left of character. All Visual
  		routines are based on this assumption 		*/
Status
XocVisualScan(XFontSet		  fontset,
	      char		  *string,
	      Boolean		  is_wchar, 
	      int		  num_chars,
	      int		  position,
	      XocPosSelectionType pos_sel_type,
	      XmTextScanType	  stype,
	      XmTextScanDirection dir,
	      Boolean		  include_ws,
	      XmTextPosition	  *new_pos)
{
    if (pos_sel_type == XocRELATIVE_POS) {
	
	switch (stype){
	    case  XmSELECT_POSITION:
		return XocVisualCharScan(fontset, string, is_wchar, 
					 num_chars, position, dir, new_pos);
		break;
	    case  XmSELECT_WORD:
		return XocVisualWordScan(fontset, string, is_wchar, 
					 num_chars, position, dir, include_ws, new_pos);
		break;
	    case  XmSELECT_LINE:
		return XocVisualLineScan(fontset, string, is_wchar, 
					 num_chars, position, dir, new_pos);
		break;
	}
    }
    else { /* pos_sel_type == XocCONST_POS */
	
	switch (stype) {
	    case  XmSELECT_POSITION:
		return XocVisualConstCharScan(fontset, string, is_wchar,
					      num_chars, position, new_pos);
		break;
	    case  XmSELECT_WORD:
		return XocVisualConstWordScan(fontset, string, is_wchar,
					      num_chars, position, new_pos);
		break;
	}
    }
}

static Status
XocVisualCharScan(XFontSet               fontset,
		  char                  *string,
		  Boolean                is_wchar, 
		  int                    num_chars,
		  int                    position,
		  XmTextScanDirection    dir,
		  XmTextPosition        *new_pos)
{
  int		glyph_num = num_chars;
  size_t	oSize = 0, tmpGlyphs = 0;
  Status	ret_status;
  Boolean	found = False;

  wchar_t	oBuf_wc[CTL_CACHE_SIZE];
  char		oBuf_mb[CTL_CACHE_SIZE];

  INIT_TRANS_DATA
  INIT_CACHE_DATA
    
  if (num_chars <= 0) {
    *new_pos = 0;
    if (dir == XmsdLeft)
      return (AT_VISUAL_LINE_START);
    else 
      return (AT_VISUAL_LINE_END);
  }
  
  if (position > num_chars) {
    *new_pos = num_chars;
    return 0;
  }
  
  if (position < 0) {
    *new_pos = 0;
    return 0;
  }
  
  oSize = num_chars * xm_xoc->layout_max_expand;
  oSize *= (is_wchar ? sizeof(wchar_t) : xm_xoc->layout_shape_charset_size);
  tmpGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);
  
  oBuf = (void*) (is_wchar ? XmStackAlloc(oSize, oBuf_wc) :
		  XmStackAlloc(oSize, oBuf_mb));
  i2o = (size_t*) XmStackAlloc(num_chars * sizeof(size_t), i2o_cache);
  o2i = (size_t*) XmStackAlloc(tmpGlyphs * sizeof(size_t), o2i_cache);
  props = (unsigned char*) XmStackAlloc(num_chars, props_cache);
  oSize = tmpGlyphs;
  
  status = _XmXOC_transform_layout(xm_xoc,
				   lo,
				   is_wchar,
				   string, num_chars,
				   oBuf, &oSize,
				   i2o, o2i, props,
				   &index);
  if (status != 0) {
    perror("Warning! XmXOC transform_layout failure\n");
    FREE_DUAL_CACHE_DATA
    *new_pos = 0;
    return 0;
  }
  
  if (IS2BYTE_FONT(xm_xoc) || (MULTIFONT(xm_xoc) && (is_wchar == False)))
    glyph_num = oSize / xm_xoc->layout_shape_charset_size;
  else
    glyph_num = oSize;
  
  if (dir == XmsdLeft) {
    if (position == glyph_num) {
      *new_pos = o2i[glyph_num - 1];
      FREE_DUAL_CACHE_DATA
      return 0;
    }
    if (i2o[position] == 0)	{
      *new_pos = position;
      ret_status = AT_VISUAL_LINE_START;
    }
    else
      *new_pos = o2i[i2o[position]-1];
  }
  else if (dir == XmsdRight) {
    /* Note2: Cursor is aligned at the end of text[glyph_num -1]
     * character when position == glyph_num.
     */
    if (position == glyph_num) {
      *new_pos = glyph_num;
      FREE_DUAL_CACHE_DATA
      return (AT_VISUAL_LINE_END);
    }
    
    if (i2o[position] == glyph_num - 1)
      *new_pos = glyph_num;
    else
      *new_pos = o2i[i2o[position] + 1];
  }
  
  FREE_DUAL_CACHE_DATA  
  return ret_status;
}

/* first position */
static int
XocVisualConstCharScan(XFontSet		fontset,
		       char		*string,
		       Boolean		is_wchar, 
		       int		num_chars,
		       XmTextPosition	position,
		       XmTextPosition	*new_pos)
{
  int		glyph_num = num_chars;
  size_t	oSize = 0, tmpGlyphs = 0;

  wchar_t	oBuf_wc[CTL_CACHE_SIZE];
  char		oBuf_mb[CTL_CACHE_SIZE];

  INIT_TRANS_DATA
  INIT_CACHE_DATA
 
  if (num_chars <= 0) {
    *new_pos = 0;
    return 0;
  }
  
  oSize = num_chars * xm_xoc->layout_max_expand;
  oSize *= (is_wchar ? sizeof(wchar_t) : xm_xoc->layout_shape_charset_size);
  tmpGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);
  
  oBuf = (void*) (is_wchar ? XmStackAlloc(oSize, oBuf_wc) :
		  XmStackAlloc(oSize, oBuf_mb));
  i2o = (size_t*) XmStackAlloc(num_chars * sizeof(size_t), i2o_cache);
  o2i = (size_t*) XmStackAlloc(tmpGlyphs * sizeof(size_t), o2i_cache);
  props = (unsigned char*) XmStackAlloc(num_chars, props_cache);
  oSize = tmpGlyphs;

  status = _XmXOC_transform_layout(xm_xoc,
				   lo,
				   is_wchar,
				   string, num_chars,
				   oBuf, &oSize,
				   i2o, o2i, props,
				   &index);
  if (status != 0) {
    perror("Warning! XmXOC transform_layout failure\n");
    FREE_DUAL_CACHE_DATA
      *new_pos = num_chars;
    return 0;
  }
  
  if (IS2BYTE_FONT(xm_xoc) || (MULTIFONT(xm_xoc) && (is_wchar == False)))
    glyph_num = oSize / xm_xoc->layout_shape_charset_size;
  else
    glyph_num = oSize;
  
  if (position == LINE_START)
    *new_pos = o2i[0];
  else if (position == LINE_END || position >= glyph_num)
    *new_pos = glyph_num;
  else
    *new_pos = o2i[position];
  
  FREE_DUAL_CACHE_DATA  
  return 0;
}

/* first position */
static int
XocVisualConstWordScan(XFontSet		fontset,
		       char		*string,
		       Boolean		is_wchar, 
		       int		num_chars,
		       XmTextPosition	position,
		       XmTextPosition	*new_pos)
{
  int		glyph_num = num_chars;
  char*		text = (char*)string;
  size_t	oSize, tmpGlyphs;
  Status	ret_status = 0;

  wchar_t	oBuf_wc[CTL_CACHE_SIZE];
  char		oBuf_mb[CTL_CACHE_SIZE];

  INIT_TRANS_DATA
  INIT_CACHE_DATA
    
  if (num_chars <= 0) {
    *new_pos = 0;
    return (NO_WORD);
  }
  
  oSize = num_chars * xm_xoc->layout_max_expand;
  oSize *= (is_wchar ? sizeof(wchar_t) : xm_xoc->layout_shape_charset_size);
  tmpGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);
  
  oBuf = (void*) (is_wchar ? XmStackAlloc(oSize, oBuf_wc) :
		  XmStackAlloc(oSize, oBuf_mb));
  i2o = (size_t*) XmStackAlloc(num_chars * sizeof(size_t), i2o_cache);
  o2i = (size_t*) XmStackAlloc(tmpGlyphs * sizeof(size_t), o2i_cache);
  props = (unsigned char*) XmStackAlloc(num_chars, props_cache);
  oSize = tmpGlyphs;
  
  status = _XmXOC_transform_layout(xm_xoc,
				   lo,
				   is_wchar,
				   string, num_chars,
				   oBuf, &oSize,
				   i2o, o2i, props,
				   &index);
  if (status != 0) {
    *new_pos = num_chars;
    perror("Warning! XmXOC transform_layout failure\n");
    FREE_DUAL_CACHE_DATA
    return (NO_WORD);
  }
  text = (char*)oBuf;
  
  if (IS2BYTE_FONT(xm_xoc) || (MULTIFONT(xm_xoc) && (is_wchar == False)))
    glyph_num = oSize / xm_xoc->layout_shape_charset_size;
  else
    glyph_num = oSize;
  
  if (position == FIRST_WORD) {
    int i = 0;
    while (i < glyph_num && ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i], is_wchar)))
      i++;
    if (i == glyph_num) {
      *new_pos = glyph_num;
      ret_status = NO_WORD;
    }
    else
      *new_pos = o2i[i];
  }
  else if (position == LAST_WORD) {
    int i = glyph_num - 1;
    
    while (i >= 0 && ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i], is_wchar)))
      i--;
    if (i < 0) {
      *new_pos = o2i[0];
      ret_status = NO_WORD;
    }
    else { /* skip to the starting of the word */
      while (i > 0 && !ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i-1], is_wchar)))
	i--;
      *new_pos = o2i[i];
    }
  }
  
  FREE_DUAL_CACHE_DATA
  return ret_status;
}

static Status
XocVisualLineScan(XFontSet               fontset,
		  char                  *string,
		  Boolean                is_wchar, 
		  int                    num_chars,
		  int                    position,
		  XmTextScanDirection    dir,
		  XmTextPosition        *new_pos)
{
  int		glyph_num = num_chars;
  size_t	oSize = 0, tmpGlyphs = 0;
  Status	ret_status = 0;

  wchar_t	oBuf_wc[CTL_CACHE_SIZE];
  char		oBuf_mb[CTL_CACHE_SIZE];

  INIT_TRANS_DATA
  INIT_CACHE_DATA

  if ((position > num_chars) || (position < 0)) {
    *new_pos = num_chars;
    return (0);
  }

  oSize = num_chars * xm_xoc->layout_max_expand;
  oSize *= (is_wchar ? sizeof(wchar_t) : xm_xoc->layout_shape_charset_size);
  tmpGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);
  
  oBuf = (void*) (is_wchar ? XmStackAlloc(oSize, oBuf_wc) :
		  XmStackAlloc(oSize, oBuf_mb));
  i2o = (size_t*) XmStackAlloc(num_chars * sizeof(size_t), i2o_cache);
  o2i = (size_t*) XmStackAlloc(tmpGlyphs * sizeof(size_t), o2i_cache);
  props = (unsigned char*) XmStackAlloc(num_chars, props_cache);
  oSize = tmpGlyphs;
  
  status = _XmXOC_transform_layout(xm_xoc,
				   lo,
				   is_wchar,
				   string, num_chars,
				   oBuf, &oSize,
				   i2o, o2i, props,
				   &index);
  if (status != 0) {
    perror("Warning! XmXOC transform_layout failure\n");
    *new_pos = num_chars;
    FREE_DUAL_CACHE_DATA
    return 0;
  }
    
  if (IS2BYTE_FONT(xm_xoc) || (MULTIFONT(xm_xoc) && (is_wchar == False)))
    glyph_num = oSize / xm_xoc->layout_shape_charset_size;
  else
    glyph_num = oSize;

  if (dir == XmsdLeft)
    *new_pos = o2i[0];
  else /* dir == XmsdRight */
    *new_pos = glyph_num;
  FREE_DUAL_CACHE_DATA
  return 1;
}

/* Assumption1 : This routine assumes that the cursor is always aligned to *
 *  		 the begining of the character.				   *
 * Assumption2 : The cursor is aligned to the end of the last character	   *
 *  		 when at the end of the line				   */
static Status
XocVisualWordScan(XFontSet               fontset,
		  char                  *string,
		  Boolean                is_wchar, 
		  int                    num_chars,
		  int                    position,
		  XmTextScanDirection    dir,
		  Boolean                include_ws, /* include white space */
		  XmTextPosition        *new_pos)
{
  int		glyph_num = num_chars;
  int		ws_pos = -1; /* white space position */
  char		*text = string;
  size_t	oSize = 0, tmpGlyphs = 0;
  Status	ret_status = 0;
  
  wchar_t	oBuf_wc[CTL_CACHE_SIZE];
  char       	oBuf_mb[CTL_CACHE_SIZE];
  
  INIT_TRANS_DATA
  INIT_CACHE_DATA 

  if ((position > num_chars) || (position < 0)) {
    *new_pos = num_chars;
    return(0);
  }

  oSize = num_chars * xm_xoc->layout_max_expand;
  oSize *= (is_wchar ? sizeof(wchar_t) : xm_xoc->layout_shape_charset_size);
  tmpGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);
  
  oBuf = (void*) (is_wchar ? XmStackAlloc(oSize, oBuf_wc) :
		  XmStackAlloc(oSize, oBuf_mb));
  i2o = (size_t*) XmStackAlloc(num_chars * sizeof(size_t), i2o_cache);
  o2i = (size_t*) XmStackAlloc(tmpGlyphs * sizeof(size_t), o2i_cache);
  props = (unsigned char*) XmStackAlloc(num_chars, props_cache);
  oSize = tmpGlyphs;
  
  status = _XmXOC_transform_layout(xm_xoc,
				   lo,
				   is_wchar,
				   string, num_chars,
				   oBuf, &oSize,
				   i2o, o2i, props,
				   &index);
  if (status != 0) {
    *new_pos = num_chars;
    perror("Warning! XmXOC transform_layout failure\n");
    FREE_DUAL_CACHE_DATA
    return 0;
  }
  text = (char*)oBuf;

  if (IS2BYTE_FONT(xm_xoc) || (MULTIFONT(xm_xoc) && (is_wchar == False)))
    glyph_num = oSize / xm_xoc->layout_shape_charset_size;
  else
    glyph_num = oSize;

  if (dir == XmsdLeft) {
    int i;
    if (position == glyph_num) {
      if (glyph_num == 1) {
	*new_pos = 0;
	if (ISWORD_DELIMITOR(STR_ICHAR(text, o2i[0], is_wchar)))
	  ret_status = NO_WORD;
	FREE_DUAL_CACHE_DATA
	return ret_status;
      }
      i = glyph_num - 1;
    }
    else
      i = i2o[position];
	
    if (i == 0) {
      *new_pos = 0;
      ret_status = AT_VISUAL_LINE_START; /* return at start */
    }
    else {
      /* check whether the position is in the middle of the word */
      Boolean in_white_spaces = ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i-1], is_wchar));
      
      if (in_white_spaces) { /* get back by a char */
	i--; 
	
	/* skip the white spaces */
	while (i > 0 && ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i], is_wchar)) )
	  i--;
	if (i <= glyph_num && i >= 0)
	  ws_pos = o2i[i];
	if (i < 0)
	  ret_status = NO_WORD;
	ws_pos = position;
      }
      /* now skip to the beggining of the word */
      while (i > 0 && !ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i-1], is_wchar)) ) 
	i--;
    }
    *new_pos = o2i[i];
    if (ws_pos == -1) /* whitespace position not found */
      ws_pos = *new_pos;
  }
  else if (dir == XmsdRight) {
    int i;
    Boolean in_word = !ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i], is_wchar));
    
    if (position == glyph_num) {
      *new_pos = glyph_num;
      FREE_DUAL_CACHE_DATA
      return AT_VISUAL_LINE_END;
    }
    else 
      i = i2o[position];
    
    if (in_word) {
      /* skip the characters of the word */
      while (i < glyph_num && !ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i], is_wchar)) ) 
	i++;
      ws_pos = (i < glyph_num ? o2i[i] : glyph_num);
    }
    else
      ws_pos = position;
    
    /* Skip the white spaces and get to the starting of the next word */
    while (i < glyph_num && ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i], is_wchar)) )
      i++;
    if (i == glyph_num) {
      *new_pos = glyph_num;
      ret_status = NO_WORD;
    }
    else
      *new_pos = o2i[i];
  }
  
  if (!include_ws)
    *new_pos = ws_pos;
  FREE_DUAL_CACHE_DATA
  return ret_status;
}

static Boolean
InList(XmTextPosition *pos_list,
       int             list_size,
       XmTextPosition  pos)
{
  int i;
  Boolean found = False;
  
  for (i = 0; i < list_size; i++) {
    found = found || (pos == pos_list[i]);
    if (found)
      break;
  }
  return found;
}

int 
XocFindVisualWord(XFontSet               fontset,
		  char                  *string,
		  Boolean                is_wchar, 
		  int                    num_chars,
		  XocPosSelectionType    pos_sel_type,
		  int                    position,
		  XmTextScanDirection    dir,
		  XmTextPosition        *word_char_list, 
		  int                   *num_chars_in_word,
		  XmTextPosition        *new_pos)
{
    int		glyph_num;
    int		tmp_str_i;
    int		i, new_pos_visual_pos;
    char	*text = string;
    char	tmp_str[CTL_MAX_BUF_SIZE];
    size_t	oSize, tmpWcGlyphs;
    Status	ret_status = 0;
    Boolean	in_white_spaces;
    INIT_TRANS_DATA
    
    if (position > num_chars) {
	*new_pos = num_chars;
	return 0;
    }
    
    if (num_chars <= 0) {
	*new_pos = 0;
	return 0;
    }
    
    *num_chars_in_word = 0;

    if (is_wchar)
	oSize = num_chars * xm_xoc->layout_max_expand * sizeof(wchar_t);
    else
	oSize = num_chars * xm_xoc->layout_max_expand * xm_xoc->layout_shape_charset_size;
    
    tmpWcGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);
    oBuf = (void*)XtMalloc(oSize);
    i2o  = (size_t*)XtMalloc(num_chars * sizeof(size_t));
    o2i  = (size_t*)XtMalloc(tmpWcGlyphs * sizeof(size_t));
    props = (unsigned char*)XtMalloc(num_chars * sizeof(unsigned char));
    status = _XmXOC_transform_layout(xm_xoc,
				     lo,
				     is_wchar,
				     string, num_chars,
				     oBuf, &oSize,
				     i2o, o2i, props,
				     &index);
    if (status != 0) {
        *new_pos = num_chars;
	perror("Warning! XmXOC transform_layout failure\n");
	FREE_TRANS_DATA
	return 0;
    }
    
    text = (char*)oBuf;
    if (IS2BYTE_FONT(xm_xoc) || MULTIFONT(xm_xoc))
	glyph_num = oSize / xm_xoc->layout_shape_charset_size;
    else
	glyph_num = oSize;
    
    if ( pos_sel_type == XocCONST_POS) {
	if (position == LAST_WORD) {
	    position = glyph_num-1;
	    dir = XmsdLeft;
	}
	else if (position == FIRST_WORD) {
	    position = o2i[0];
	    dir = XmsdRight;
	}
    }
    
    if (dir == XmsdLeft) {
	if (position == glyph_num) {
	    if (glyph_num == 1) {
		word_char_list[0] = 0;
		*num_chars_in_word = 1;
		*new_pos = 0;
		FREE_TRANS_DATA
		return ret_status;
	    }
	    i = glyph_num;
	}
	else
	    i = i2o[position];
	
	if (i == 0) {
	    *new_pos = 0;
	    ret_status = AT_VISUAL_LINE_START; /* return at start */
	}
	else {
	    /* check whether the position is in the middle of the word */
	    in_white_spaces = ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i-1], is_wchar))
		      || (i == glyph_num);
	    if (in_white_spaces) {
		/* skip the white spaces */
		while (i > 0 && ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i-1], is_wchar))) {
		    word_char_list[*num_chars_in_word] = o2i[i-1];
		    (*num_chars_in_word)++;
		    i--;
		}
	    }
	    /* now skip to the beggining of the word */
	    while (i > 0 && !ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i-1], is_wchar))) {
		word_char_list[*num_chars_in_word] = o2i[i-1];
		(*num_chars_in_word)++;
		i--;
	    }
	}
	if (position == glyph_num) {
	    *new_pos = glyph_num - *num_chars_in_word;
	    FREE_TRANS_DATA
	    return ret_status;
	}
	else
	    new_pos_visual_pos = i2o[position] - *num_chars_in_word;
    }
    else if (dir == XmsdRight) {
	
	if (position == glyph_num) {
	    *new_pos = glyph_num;
	    FREE_TRANS_DATA
	    return AT_VISUAL_LINE_END;
	}
	else 
	    i = i2o[position];
	
	in_white_spaces = ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i], is_wchar));

	/* Skip the white spaces and get to the starting of the next  word */
	if (in_white_spaces) {
	    while (i < glyph_num && ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i], is_wchar))) {
		word_char_list[*num_chars_in_word] = o2i[i];
		(*num_chars_in_word)++;
		i++;
	    }
	}
	    
	/* skip the characters of the word */
	while (i < glyph_num && !ISWORD_DELIMITOR(STR_ICHAR(text, o2i[i], is_wchar))) {
	    word_char_list[*num_chars_in_word] = o2i[i];
	    (*num_chars_in_word)++;
	    i++;
	}

	if (i == glyph_num) {
	    *new_pos = glyph_num - *num_chars_in_word;
	    FREE_TRANS_DATA;
	    return ret_status;
	}
	else
	    new_pos_visual_pos = i2o[position];
    }
    
    FREE_TRANS_DATA
    memset(tmp_str, '\0', CTL_MAX_BUF_SIZE);
    text = tmp_str;
    for (i = 0, tmp_str_i = 0; i < glyph_num; i++) {
	if (!InList(word_char_list, *num_chars_in_word, i)) {
	    CHAR_CPY(STR_IPTR(tmp_str, tmp_str_i, is_wchar), 
		     STR_IPTR(string, i, is_wchar), is_wchar);
	    tmp_str_i++;
	}
    }

    if (is_wchar)
	oSize = num_chars * xm_xoc->layout_max_expand * sizeof(wchar_t);
    else
	oSize = num_chars * xm_xoc->layout_max_expand * xm_xoc->layout_shape_charset_size;
	
    tmpWcGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);
    oBuf = (void*)XtMalloc(oSize);
    i2o  = (size_t*)XtMalloc(num_chars * sizeof(size_t));
    o2i  = (size_t*)XtMalloc(tmpWcGlyphs * sizeof(size_t));
    props = (unsigned char*)XtMalloc(num_chars * sizeof(unsigned char));
    status = _XmXOC_transform_layout(xm_xoc,
				     lo,
				     is_wchar,
				     string, num_chars,
				     oBuf, &oSize,
				     i2o, o2i, props,
				     &index);
    if (status == 0)
	*new_pos = o2i[new_pos_visual_pos];
    else
	perror("Warning! XmXOC transform_layout failure\n");
    FREE_TRANS_DATA
    return ret_status;
}

static void
destroy_oc(XOC oc)		/* It's really an XmXOC */
{
    XmXOC xm_xoc = (XmXOC)oc;
    
    XDestroyOC(xm_xoc->xoc);
    XmRealDestroyXmXOC(oc);
}

static char *
get_oc_values(XOC oc, XlcArgList args, int num_args)
{
    XmXOC xm_xoc = (XmXOC)oc;
    
    if (oc->core.resources == NULL)
	return NULL;
    return _XlcGetValues((XPointer)oc, oc->core.resources,
			 oc->core.num_resources, args, num_args, XlcGetMask);
}

static char *
set_oc_values(XOC oc, XlcArgList args, int num_args)
{
  XmXOC xm_xoc             = (XmXOC)oc;
  int                      j;
  LayoutValueRec           layout_values[2];
  LayoutTextDescriptorRec  text_desc;

  for ( j = 0; j <num_args ; j++) /*chack the layout modifier*/
    { 	  
      if (strcmp(args[j].name,"layoutModifier") == 0){ /*need to break the layoutModifier into layout values*/
	int index_ret=0;
	if(parseOrientation(xm_xoc, args[j].value, layout_values, &text_desc)){ /*orientation only, for now!!*/
	  if((*xm_xoc->fnRec->m_setvalues_layout)(xm_xoc->layout_object, layout_values, &index_ret))
	    perror("Error:: m_setvalues_layout failure in set_oc_values");	  
	  else { /*clean the cash dummy call to m_tramsform_layout*/
	  void       *oBuf  = NULL;
	  size_t     index = 0;   
	  size_t     glyph_bytes = 2 * xm_xoc->layout_max_expand * xm_xoc->layout_shape_charset_size;
	  oBuf =     (void*)XtMalloc(glyph_bytes+1);
	  _XmXOC_transform_layout(xm_xoc,
				  xm_xoc->layout_object,
				  False,
				  " ", 1,
				  oBuf, &glyph_bytes,
				  NULL, NULL, NULL, &index);
	  if (oBuf)
	    XtFree((char*)oBuf);
	}
	}
	if (layout_values)
	  XtFree((char*)layout_values);
      }
    }
  if (oc->core.resources == NULL)
    return NULL;
  return _XlcSetValues((XPointer)oc, oc->core.resources,
		       oc->core.num_resources, args, num_args, XlcSetMask);
}

static XOCMethodsRec xm_xoc_methods = { 
    destroy_oc,
    set_oc_values,
    get_oc_values,
    _XmbCTLTextEscapement,
    _XmbCTLTextExtents,
    _XmbCTLTextPerCharExtents,
    _XmbCTLDrawString,
    _XmbCTLDrawImageString,
    _XwcCTLTextEscapement,
    _XwcCTLTextExtents,
    _XwcCTLTextPerCharExtents,
    _XwcCTLDrawString,
    _XwcCTLDrawImageString
};

Status
XocTextPerCharExtents(XOC           oc,
		      _Xconst void *string,
		      Boolean       is_wchar, 
		      int           num_chars,
		      XSegment     *ink_array_return,
		      XSegment     *logical_array_return,
		      int           array_size,
		      int          *num_chars_return,
		      XSegment     *overall_ink_return,
		      XSegment     *overall_logical_return)
{
    return (*((XmXOC)oc)->xoc_extents_per_char)(oc, string, is_wchar, num_chars,
						ink_array_return, 
						logical_array_return,
						array_size, num_chars_return, 
						overall_ink_return,
						overall_logical_return);
}

typedef struct _XmXOCList {
  struct _XmXOCList *next;
  XmXOC xm_xoc;
} XmXOCList;

static XmXOCList *g_xmoc_list = 0;

XmXOC XmCreateXmXOC(XOC xoc, String modifier, XmXOCFuncRec *fnRec)
{
    int		   i = 0, index_ret = 0;
    char	   libpath_string[100]; /* We dont expect pathname to be > 100 */
    size_t	   size = 1024;
    XmXOC	   xm_xoc = 0;
    BooleanValue   Active_dir = FALSE, Active_shape = FALSE;
    LayoutValueRec layout_values[5];
    XmXOCList *new_xmoc = 0;

    /* search for freed list first */
    if (g_xmoc_list) {
      XmXOCList *p;
      for (p = g_xmoc_list; p != 0; p = p->next) {
	if (p->xm_xoc->xoc == xoc &&
	    p->xm_xoc->layout_modifier == modifier) {
	  xm_xoc = p->xm_xoc;
	  /* we can share the same XmXOC for the same XOC and modifier,
	     as fnRec always has the same contents */
	  if (xm_xoc->fnRec != fnRec) {
	    free(xm_xoc->fnRec); /* new fnRec should be used*/
	    xm_xoc->fnRec = fnRec;
	  }
	  return xm_xoc;
	}
      }
    }
    if (xm_xoc == 0) {
      xm_xoc = (XmXOC)Xmalloc(sizeof(XmXOCRec));
      memset((char*)xm_xoc, 0, sizeof(XmXOCRec));
    }
    if (xm_xoc == NULL) return (XmXOC)NULL;
    
    /* Initialize XOC part of XmXOC object from the XOC argument passed in */
    *((XOC)xm_xoc) = *xoc;	/* This is the fontset part */
    xm_xoc->fnRec = fnRec;	/* This is the api part */
    
    if (xm_xoc_resources[0].xrm_name == NULLQUARK)
	_XlcCompileResourceList(xm_xoc_resources, XlcNumber(xm_xoc_resources));
    ((XOC)xm_xoc)->core.resources = xm_xoc_resources;
    ((XOC)xm_xoc)->core.num_resources = XlcNumber(xm_xoc_resources);
    ((XOC)xm_xoc)->methods = &xm_xoc_methods;	/* Splice in new methods */
    
    xm_xoc->xoc = xoc;		
    xm_xoc->layout_modifier = modifier;
    
    xm_xoc->layout_object = (LayoutObject)(*xm_xoc->fnRec->m_create_layout)
			    (NULL, xm_xoc->layout_modifier);
    if (!xm_xoc->layout_object) {
	XFree(xm_xoc);
	return (XmXOC)NULL;
    }
    
    xm_xoc->layout_active		= False;
    xm_xoc->ule_active			= False;
    xm_xoc->layout_max_expand		= 2; /* Supplied by Layout Object */
    xm_xoc->xoc_extents_per_char	= _XocCTLTextPerCharExtents;
    xm_xoc->layout_shape_charset_size	= 0; /* Default */
    xm_xoc->layout_cache.char_buf	= NULL;
    xm_xoc->layout_cache.char_buf_index	= 0;
    xm_xoc->layout_cache.ret 		= -1;
    xm_xoc->layout_cache.GlyphBuf	= NULL;
    xm_xoc->layout_cache.BufSize	= 0;
    xm_xoc->layout_cache.CharNum	= 0;
    xm_xoc->layout_cache.GlyphNum	= 0;
    xm_xoc->layout_cache.CPos2GPos	= NULL;
    xm_xoc->layout_cache.GPos2CPos	= NULL;
    xm_xoc->layout_cache.CharProps	= NULL;
    xm_xoc->uleFonts			= NULL;
    
    /* Obtain layout_active, ule_active, uleFonts, layout_shape_charset_size */
    layout_values[i].name = ActiveShapeEditing;
    layout_values[i].value = &Active_dir; i++;
    layout_values[i].name = ActiveDirectional;
    layout_values[i].value = &Active_shape; i++;
    layout_values[i].name = ShapeCharsetSize;
    layout_values[i].value = &xm_xoc->layout_shape_charset_size; i++;
    layout_values[i].name = QueryValueSize | ShapeCharset;
    layout_values[i].value = &size; i++;
    layout_values[i].name = 0;
    
    if ((*xm_xoc->fnRec->m_getvalues_layout)
	(xm_xoc->layout_object, &layout_values, &index_ret)) {
	perror("Error:: m_getvalues_layout failure in create_oc");
    }
    else {
      /* Fetch shape editing and directional values to check if ctl	*
	 * support is needed. This value is cached. size != 0 means that*
	 * the shapecharset has a valid value and that the locale is a	*
	 * ULE locale. Hence use shapecharset to create Multifont table.*/
	if ((Active_dir == TRUE) || (Active_shape == TRUE)) 
	    xm_xoc->layout_active = True;
	
	if (size != (size_t)0) {
	    LayoutValueRec layout_value[2];
	    char 	   *tmp_shape_charset = (char*)XtMalloc(size);
	    int		   index = 0;
	    
	    layout_value[0].name = ShapeCharset;
	    layout_value[0].value = &tmp_shape_charset;
	    layout_value[1].name = 0;
	    
	    if ((*xm_xoc->fnRec->m_getvalues_layout)(xm_xoc->layout_object,
						     &layout_value, &index)) {
		perror("Error:m_getvalues_layout failure in get_shape");
	    }
	    else {
		if ((tmp_shape_charset != NULL) && 
		    (strchr(tmp_shape_charset, ';') != NULL)) {
		    xm_xoc->ule_active = True;
		    parseShape(xm_xoc, tmp_shape_charset);
		}
	    }
	    if (tmp_shape_charset) XtFree(tmp_shape_charset);
	}
    }

    new_xmoc = Xmalloc(sizeof(XmXOCList));
    if (new_xmoc) {
      new_xmoc->xm_xoc = xm_xoc;
      new_xmoc->next = g_xmoc_list;
      g_xmoc_list = new_xmoc;
    }
    return (XmXOC) xm_xoc;
}

static void
XmRealDestroyXmXOC(XOC xoc)
{
  int   i = 0;
  XmXOC xm_xoc = (XmXOC)xoc;

  if (xm_xoc->layout_object) {
    (*xm_xoc->fnRec->m_destroy_layout)(xm_xoc->layout_object);
  }

  _XmXOC_InvalidateCache(xm_xoc);

  if (xm_xoc->fnRec)
    free(xm_xoc->fnRec);

  if (xm_xoc->uleFonts) {
    for (i = 0; i < xm_xoc->uleFonts->num_entries; i++) {
      if (xm_xoc->uleFonts->MFontRec[i].fname)
	free(xm_xoc->uleFonts->MFontRec[i].fname);
    }
    XtFree((char*)xm_xoc->uleFonts);
  }
  /* XFree(xm_xoc); */
}

XOC
XmDestroyXmXOC(XOC xoc)
{ 
  XmXOC xm_xoc = (XmXOC)xoc;

  /* xm_xoc should not be freed as it may have been set to the
     X input context in XmIm.c */
  return xm_xoc->xoc;
}

/********************************************************************
  This is _XmbDefaultTextPerCharExtents with the first arg being a
  XFontStruct* rather than XFontSet.  Note that the parameter names
  have been changed so that we can use the source of the body as is.
  Needs Xlibint.h, which we include in XmXOC.h
 ********************************************************************/
Status
_XFontStructTextPerCharExtents(XFontStruct	*font,
			       char		*text,
			       int		length,
			       XRectangle	*ink_buf,
			       XRectangle	*logical_buf,
			       int		buf_size,
			       int		*num_chars,
			       XRectangle	*overall_ink,
			       XRectangle	*overall_logical)
{
    XCharStruct *def, *cs, overall;
    Bool first = True;
    
    *num_chars = length;
    
    if (buf_size < length)
	return 0;
    
    memset((char*)&overall, 0, sizeof(XCharStruct));
    CI_GET_DEFAULT_INFO_1D(font, def)
    
    while (length-- > 0) {
	CI_GET_CHAR_INFO_1D(font, *text, def, cs)
	text++;
	if (cs == NULL)
	    continue;
	
	ink_buf->x = overall.width + cs->lbearing;
	ink_buf->y = -(cs->ascent);
	ink_buf->width = cs->rbearing - cs->lbearing;
	ink_buf->height = cs->ascent + cs->descent;
	ink_buf++;
	logical_buf->x = overall.width;
	logical_buf->y = -(font->ascent);
	logical_buf->width = cs->width;
	logical_buf->height = font->ascent + font->descent;
	logical_buf++;
	
	if (first) {
	    overall = *cs;
	    first = False;
	} else {
	    overall.ascent = max(overall.ascent, cs->ascent);
	    overall.descent = max(overall.descent, cs->descent);
	    overall.lbearing = min(overall.lbearing, overall.width + cs->lbearing);
	    overall.rbearing = max(overall.rbearing, overall.width + cs->rbearing);
	    overall.width += cs->width;
	}
    }

    if (overall_ink) {
	overall_ink->x = overall.lbearing;
	overall_ink->y = -(overall.ascent);
	overall_ink->width = overall.rbearing - overall.lbearing;
	overall_ink->height = overall.ascent + overall.descent;
    }
    if (overall_logical) {
	overall_logical->x = 0;
	overall_logical->y = -(font->ascent);
	overall_logical->width = overall.width;
	overall_logical->height = font->ascent + font->descent;
    }
    return 1;
}

int
XocTextInfo(XOC			oc,
	    _Xconst void	*string,
	    Boolean		is_wchar, 
	    int			num_chars,
	    unsigned char	*char_props,
	    XmTextPosition	*text_i2o,
	    XmTextPosition	*text_o2i)
{
  int		i = 0;
  size_t	oSize = 0, tmpGlyphs = 0;
  XFontSet	fontset = (XFontSet)oc;

  wchar_t	oBuf_wc[CTL_CACHE_SIZE];
  char		oBuf_mb[CTL_CACHE_SIZE];

  INIT_TRANS_DATA
  INIT_CACHE_DATA

  oSize = num_chars * xm_xoc->layout_max_expand;
  oSize *= (is_wchar ? sizeof(wchar_t) : xm_xoc->layout_shape_charset_size);
  tmpGlyphs = (is_wchar ? (oSize / sizeof(wchar_t)) : oSize);
  
  oBuf = (void*) (is_wchar ? XmStackAlloc(oSize, oBuf_wc) :
		  XmStackAlloc(oSize, oBuf_mb));
  i2o = (size_t*) XmStackAlloc(num_chars * sizeof(size_t), i2o_cache);
  o2i = (size_t*) XmStackAlloc(tmpGlyphs * sizeof(size_t), o2i_cache);
  props = (unsigned char*) XmStackAlloc(num_chars, props_cache);
  oSize = tmpGlyphs;  
  
  status = _XmXOC_transform_layout(xm_xoc,
				   lo,
				   is_wchar,
				   string, num_chars,
				   oBuf, &oSize,
				   i2o, o2i, props,
				   &index);
  if (status != 0) {
    perror("Warning! XmXOC transform_layout failure\n");
    FREE_DUAL_CACHE_DATA
  }
  
  for (i = 0; i < num_chars; i++) {
    if (char_props)
      char_props[i] = props[i];
    if (text_i2o)
      text_i2o[i] = i2o[i];
  }
  
  /* Needs to handle multibytes */
  for (i = 0; i < oSize; i++)
    if (text_o2i)
      text_o2i[i] = o2i[i];
  FREE_DUAL_CACHE_DATA
}

void
XocVisualCharInsertionInfo(XOC                oc,
			   char              *text,
			   Boolean            is_wchar,
			   int                text_len,
			   XmTextPosition     curr_pos,
                           char              *insert_string,
                           int                insert_length,
                           XmTextPosition    *insert_pos,
                           XmTextPosition    *next_pos,
			   XmTextPosition    *new_cursor_pos)
{
    Boolean	    at_boundary;    
    unsigned char   istr_props[CTL_MAX_BUF_SIZE];
    unsigned char   text_props[CTL_MAX_BUF_SIZE];
    XmTextPosition  text_i2o[CTL_MAX_BUF_SIZE], text_o2i[CTL_MAX_BUF_SIZE];
    XmCharDirection cc_dir, lc_dir, ic_dir;
    XmTextPosition  left_pos;

    /* Get the props for the insert string */
    XocTextInfo(oc, insert_string, is_wchar, insert_length, istr_props, NULL, NULL);
    
    /* get the direction of the text */
    ic_dir = GET_DIRECTION(istr_props[0]);
    
    if (text_len == 0) {
	*insert_pos = *next_pos = 0;
	if (ic_dir == XmcdRTL)
	    *new_cursor_pos = 0;
	else 
	    *new_cursor_pos = 1;
	return; 
    }
    
    /* Get the properties info about the text */
    XocTextInfo(oc, text, is_wchar, text_len, text_props, text_i2o, text_o2i);
    if (curr_pos == text_len) { /* Cursor at the end of the text */
	XmCharDirection pc_dir;
	
	pc_dir = GET_DIRECTION(text_props[text_len - 1]);
	if (pc_dir == XmcdRTL)
	    if (ic_dir == XmcdRTL)
		*new_cursor_pos = *insert_pos = *next_pos = text_o2i[text_len - 1];
	    else { /* ic_dir == XmcdLTR */
		*insert_pos = *next_pos = text_len;
		*new_cursor_pos = text_len + 1;
	    }
	else { /* pc_dir == XmcdLTR */
	    if (ic_dir == XmcdLTR) {
		*insert_pos = *next_pos = text_len;
		*new_cursor_pos = text_len + 1;
	    }
	    else /* ic_dir == XmcdRTL */
		*insert_pos = *next_pos = *new_cursor_pos = text_len;
	}
	return;		       
    }
    
    if (text_i2o[curr_pos] == 0) { /* Cursor at visual beginning */
	XmCharDirection nc_dir = GET_DIRECTION(text_props[0]);
	
	if (nc_dir == XmcdRTL)
	    if (ic_dir == XmcdRTL)
		*new_cursor_pos = *insert_pos = *next_pos = text_o2i[0] + 1;
	    else { /* ic_dir == XmcdLTR */
		*insert_pos = *next_pos = 0;
		*new_cursor_pos = text_o2i[0] + 1;
	    }
	else { /* nc_dir == XmcdLTR */
	    if (ic_dir == XmcdLTR) {
		*insert_pos = *next_pos = 0;
		*new_cursor_pos = text_o2i[0] + 1; /* is this 1? always */
	    }
	    else /* ic_dir == XmcdRTL */
		*insert_pos = *next_pos = *new_cursor_pos = 0;
	}
	return;
    }
    
    left_pos = text_o2i[text_i2o[curr_pos] - 1];
    cc_dir = GET_DIRECTION(text_props[curr_pos]);
    lc_dir = GET_DIRECTION(text_props[left_pos]);

    at_boundary = (lc_dir != cc_dir);
    if (at_boundary) {
	if (is_wchar) /* Not yet implemented */
	    XmeWarning(NULL, "NYI Visual Insert Char Info XmXOC.c ");
	else {
	    char tmp_str[CTL_MAX_BUF_SIZE];
	    unsigned char tmp_str_props[CTL_MAX_BUF_SIZE];
	    XmTextPosition tmp_str_i2o[CTL_MAX_BUF_SIZE];
	    XmTextPosition tmp_str_o2i[CTL_MAX_BUF_SIZE];
	    int i, j;
	    
	    if (cc_dir == XmcdRTL && lc_dir == XmcdLTR) 
		if (ic_dir == XmcdLTR) 
		    *insert_pos = *next_pos = left_pos + 1;
		else /* ic_dir == XmcdRTL */
		    *insert_pos = *next_pos = curr_pos + 1;
	    else if (cc_dir == XmcdLTR && lc_dir == XmcdRTL) {
		if (ic_dir == XmcdLTR) 
		    *insert_pos = *next_pos = curr_pos;
		else  /* ic_dir == XmcdRTL */ 
		    *insert_pos = *next_pos = left_pos;
	    }
	    
	    for (i = 0; i < *insert_pos; i++)
		tmp_str[i] = text[i];
	    
	    for (j = 0; j < insert_length; j++)
		tmp_str[i + j] = insert_string[j];
		
	    for (i = *insert_pos; i < text_len; i++)
		tmp_str[j + i] = text[i];
		
	    XocTextInfo(oc, tmp_str, is_wchar, i + j, 
			tmp_str_props, tmp_str_i2o, tmp_str_o2i);
	    if (ic_dir == XmcdLTR)
		*new_cursor_pos = tmp_str_o2i[text_i2o[curr_pos] + 1];
	    else /* ic_dir == XmcdRTL */
		*new_cursor_pos = tmp_str_o2i[text_i2o[curr_pos]];
	}
    }
    else { /* not at boundary */ 
	if (cc_dir == XmcdLTR) 
	    *insert_pos = *next_pos = curr_pos;
	else 
	    *insert_pos = *next_pos = curr_pos + 1;
	*new_cursor_pos = curr_pos + 1;
    }
}

void
XocVisualWordInfo(XOC                oc,
		  char              *text,
		  Boolean            is_wchar,
		  int                text_len,
		  XmTextPosition     pos,
		  XmTextPosition    *start_pos,
		  XmTextPosition    *end_pos)
{
    unsigned char text_props[CTL_MAX_BUF_SIZE];
    XmTextPosition text_i2o[CTL_MAX_BUF_SIZE], text_o2i[CTL_MAX_BUF_SIZE];
    int i;
    
    if (text_len == 0) {
	*start_pos = *end_pos = pos;
	return;
    }
    /* Get the props for the insert string */
    XocTextInfo(oc, text, is_wchar, text_len, NULL, text_i2o, text_o2i);

    if (pos == text_len)
	i = text_i2o[text_len - 1];
    else
	i = text_i2o[pos];
    
    /* get the starting position of the word */
    while (i > 0 && !isspace(STR_ICHAR(text, text_o2i[i - 1], is_wchar)))
	i--;
    *start_pos = text_o2i[i];
    
    if (pos == text_len)
	*end_pos = text_len;
    else {
	i = text_i2o[pos];
	while (i < text_len && !isspace(STR_ICHAR(text, text_o2i[i], is_wchar)))
	    i++;
    }
    
    if (i == text_len)
	*end_pos = text_len;
    else
	*end_pos = text_o2i[i];
    if (*start_pos > *end_pos) {
	XmTextPosition temp;
	
	temp = *start_pos;
	*start_pos = *end_pos;
	*end_pos = temp;
    }
    return;
}

Status
XocVisualCharDelInfo(XFontSet		fontset,
		     char		*string,
		     Boolean		is_wchar,
		     int		num_chars,
		     int		position,
		     XmTextScanDirection dir,
		     XmTextPosition	*del_char_pos,
		     XmTextPosition	*new_pos)
{
    printf("Place Holder\n");
    return 1;
}

/****************************************************************************************
 * Prabhat : 06/12.									*
 * The functions below are used as a base to implement ULE Multifont support. The main  *
 * idea is for motif(XOC) to hold additional font and encoding data based on information*
 * obtained from ULE. This data is later used in drawing and measurement.		*
 ****************************************************************************************/
/* Add an entry into the MultiFont table */
static int
mfontAdd(XmXOC xmxoc, mfontRec mfRec)
{
    int		i, j, found, num_fonts = 0;
    char	**fname_list, *substr = NULL;
    XFontStruct	**fstruct_list;
    XFontSet	fontset	= (XFontSet)xmxoc->xoc;
    
    if (!mfRec.fname || (mfRec.fname[0] == '\0') ||
	(xmxoc->uleFonts->num_entries == (MAX_PLANES-1)))
        return 0;
    else {
	for (i = 0; i < xmxoc->uleFonts->num_entries; i++)
	    if (xmxoc->uleFonts->MFontRec[i].fid == mfRec.fid) 
		return 1;
	xmxoc->uleFonts->MFontRec[i].fid = mfRec.fid;
	xmxoc->uleFonts->MFontRec[i].fname = mfRec.fname;
	/* Get the multifont (XFontStruct) */
	num_fonts = XFontsOfFontSet(fontset, &fstruct_list, &fname_list);
	if (num_fonts <= 0) {
	    XmeWarning(NULL, "Error No MultiFont Support\n");
	    return 0;
	}
	else {
	    j = 0; found = 0;
	    while ((j < num_fonts) && !found) {
		substr = strstr(fname_list[j], mfRec.fname);
		if (substr != NULL) {
		    xmxoc->uleFonts->MFontRec[i].mfont = fstruct_list[j];
		    found = 1;
		}
		j++;
	    }
	    /* Default - First Font in FS */
	    if (!found)
		xmxoc->uleFonts->MFontRec[i].mfont = fstruct_list[0];
        }
	xmxoc->uleFonts->num_entries++;
    }
    return 1;
}

/* Parse shapecharset(assumed to be error free) & create a multifont table */
static int
parseShape(XmXOC xmxoc, char *modifier)
{
#define MAX_BUF_LEN 64
    char	str_buf[MAX_BUF_LEN];
    int		i = 0, result = True;
    char	*tmpStr = modifier;
    Boolean	bufhasequal = True;
    mfontRec	mfRec;
    
    if (!tmpStr) return 0;
    if (xmxoc->uleFonts == NULL) {
	xmxoc->uleFonts = (MultiFont*)XtMalloc(sizeof(MultiFont));
	memset((void*)xmxoc->uleFonts, 0, sizeof(MultiFont));
    }
    
    i = 0;
    while ((tmpStr) && (*tmpStr != '\0')) {
	if (!(isspace(*tmpStr) && (*tmpStr == '"'))) {
	    if ((*tmpStr == '=') || (*tmpStr == ';')) {
		str_buf[i] = '\0';
		if (*tmpStr == '=') /* Store string as encoding (name) */
		    mfRec.fname = strdup(str_buf);
		else { /* Store string as fontid */
		    sscanf(str_buf, "%X", &mfRec.fid);
		    result = mfontAdd(xmxoc, mfRec);
		}
		i = 0;
	    }
	    else {
		str_buf[i] = *tmpStr;
		i++;
	    }
	}
	tmpStr++;
    }
    return result;
}

/*
 * Functions below check if or not a given mb or a wc character is 
 * from one of CTL locales based on FID bytes. Presume valid text.
 */
static unsigned int
ctlChar(XmXOC xmxoc, void* text, int pos)
{
    int		 i = 0;
    unsigned int tst_id = 0;
    char	 *glyph;
    wchar_t	 *wc_glyph;

    if (pos == -1) {
	wc_glyph = (wchar_t*)text;
	tst_id = (unsigned int)((*wc_glyph & 0xFF000000) >> 24);
    }
    else {
	glyph = (char*)text;
	tst_id = (unsigned int)(glyph[pos] & 0xFF);
    }
    
    if (tst_id <= 0) 
	return NONCTLID;
    for (i = 0; i < xmxoc->uleFonts->num_entries; i++)
	if (tst_id == xmxoc->uleFonts->MFontRec[i].fid) 
	    return tst_id;
    return NONCTLID;
}

/* This function obtains the multifont XFontStruct from multifont table */
static XFontStruct*
getCtlFont(XmXOC xmxoc, unsigned int tst_id)
{
    int		i;

    for (i = 0; i < xmxoc->uleFonts->num_entries; i++)
        if (tst_id == xmxoc->uleFonts->MFontRec[i].fid)
	    return ((XFontStruct*)xmxoc->uleFonts->MFontRec[i].mfont);
    return((XFontStruct*)NULL);
}

/* Parse the layout modifier and return the layout values for the orientation */
static int
parseOrientation(XmXOC xmxoc, char *modifier, LayoutValues layout_values, LayoutTextDescriptor text_desc)
{
 
#define MAX_BUF_LEN 64
    char	             inp_buf[MAX_BUF_LEN];
    char	             out_buf[MAX_BUF_LEN]; 
    int		             i = 0, result = False;
    int		             has_out_value = False, has_inp_value=False ;
    char	             *str1 = modifier;
    LayoutValues       	     temp_ptr=layout_values;

    typedef struct {
      char	        	*str;
      int			value;
    } MappingRec;
    
    MappingRec   	orient_tbl[] = {
      "ltr",		ORIENTATION_LTR,
      "rtl",		ORIENTATION_RTL,
      "ttblr",		ORIENTATION_TTBRL,
      "ttbrl",		ORIENTATION_TTBLR,
      "contextual",	ORIENTATION_CONTEXTUAL,
    };
    
    if (!str1)
      return 0;   

    i = 0;
    text_desc->inp = text_desc->out = 0;
    for (i= 0; i < 2 ; i++) {
      temp_ptr[i].name = 0;
      temp_ptr[i].value = 0;
    }

    str1= strstr(str1,"orientation"); /*we assume that orientatin does exist in the modifier*/    
    str1= strstr(str1,"="); 
    str1++;
    while (str1 && (isspace(*str1))) /*skip space*/
      str1++;
    /*parse for inp value*/
    for (i = 0; (str1 && (i < MAX_BUF_LEN)); i++) {
      if (isspace(*str1)) {
	inp_buf[i] = NULL;
      } else if (*str1 == ',') {
	inp_buf[i] = NULL;
	break;
      } else if (*str1 == ':') {
	inp_buf[i] = NULL;
	str1++;
	has_out_value = True;
	break;
      } else{
	inp_buf[i] = *str1;
	has_inp_value = True;
      }
      str1++;
    }
        
/* If outvalue is specified using a colon, parse it and obtain the value */
    if (has_out_value) {
      while (str1 && (isspace(*str1))) /*skip space*/
	str1++;     
      for (i = 0; (str1 && (i < MAX_BUF_LEN)); i++) {
	if (isspace(*str1)) {
	  out_buf[i] = NULL;
	  break;
	} else if (*str1 == ',') {
	  out_buf[i] = NULL;
	  break;
	} else
	  out_buf[i] = *str1;
	str1++;
      }
    }

/* parse thru the table, match the string and obtain the corresponding value
 * from the global table referred to by value_tbl
 */
    if (has_inp_value) {
      for (i = 0; i < 4 ; i++) {
	if (strcmp(inp_buf, orient_tbl[i].str) == 0) {
	  text_desc->inp |= orient_tbl[i].value;
	  if ((!has_out_value) || out_buf[0] == NULL)
	    text_desc->out |= orient_tbl[i].value;
	  break;   
	}
      }
    }    
    if (has_out_value) {
      for (i = 0; i < 4 ; i++) {
	if (strcmp(out_buf, orient_tbl[i].str) == 0) {
	  text_desc->out |= orient_tbl[i].value; 
	  break;
	}
      }
    } 

    if (text_desc->out && text_desc->inp)
      temp_ptr->name = Orientation |InOutTextDescrMask;	
    else if (text_desc->inp)
      temp_ptr->name = Orientation |InOnlyTextDescr;
    else if (text_desc->out)
      temp_ptr->name = Orientation |OutOnlyTextDescr;
    else
      return False;
    temp_ptr->value=(LayoutValue)text_desc;
    temp_ptr++;
    temp_ptr->name = 0;
    temp_ptr->value = 0;

return True;
}

/*fill the transformation bufferes with the input values, 
  this function will be called if the transformation be language engine is failed
  */
int
_dummy_transform_layout( XmXOC         xm_xoc,
			 Boolean        is_wChar,
			 _Xconst void   *inpBuffer, 
			 size_t        inpSize, 
			 void          *outBuf, 
			 size_t        *outSize,
			 size_t        *i2o,
			 size_t        *o2i, 
			 unsigned char *props, 
			 size_t        *in_buf_index)
{
  int i,j;
  int startIndex = (in_buf_index && (*in_buf_index < inpSize)) ? *in_buf_index : 0;
  for( i=0; i+startIndex < inpSize ; i++){
    if (is_wChar){
      if (outBuf)      
	((wchar_t*)outBuf)[i] = ((wchar_t*)inpBuffer)[i+startIndex];
      if (i2o)
	i2o[i]= i+startIndex;
      if (o2i)
	o2i[i]= i+startIndex;
      if (props)
	props[i]= DISPLAYCELL_MASK;
    }
    else{     
      if (i2o)
	i2o[i]= i+startIndex;
      if (props)
	props[i]= DISPLAYCELL_MASK;
      for ( j=0; j< xm_xoc->layout_shape_charset_size-1; j++){
	if (outBuf)
	  ((char*)outBuf)[(i*xm_xoc->layout_shape_charset_size)+j]=0x00;
	if (o2i)
	  o2i[(i*xm_xoc->layout_shape_charset_size)+j]=i;
      }
      if (outBuf)
	((char*)outBuf)[(i*xm_xoc->layout_shape_charset_size)+j]=((char*)inpBuffer)[i+startIndex];
      if (o2i)
	o2i[(i*xm_xoc->layout_shape_charset_size)+j]=i;
    }    
  }    
  *outSize= (is_wChar)? inpSize-startIndex : (inpSize-startIndex) * xm_xoc->layout_shape_charset_size;
  if (in_buf_index)
    *in_buf_index += inpSize;
  return 0;
}

#endif /* CTL */

