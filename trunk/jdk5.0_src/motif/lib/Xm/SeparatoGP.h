/* 
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
*/ 
/* 
 * HISTORY
*/ 
/*   $XConsortium: SeparatoGP.h /main/11 1995/07/13 17:59:16 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmSeparatorGadgetP_h
#define _XmSeparatorGadgetP_h

#ifndef MOTIF12_HEADERS

#include <Xm/SeparatoG.h>
#include <Xm/GadgetP.h>
#include <Xm/ExtObjectP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************/
/* The Separator Gadget Cache Object's class and instance records*/
/*****************************************************************/

typedef struct _XmSeparatorGCacheObjClassPart
{
    int foo;
} XmSeparatorGCacheObjClassPart;


/* separator cache class record */
typedef struct _XmSeparatorGCacheObjClassRec
{
    ObjectClassPart                     object_class;
    XmExtClassPart                      ext_class;
    XmSeparatorGCacheObjClassPart       separator_class_cache;
} XmSeparatorGCacheObjClassRec;

externalref XmSeparatorGCacheObjClassRec xmSeparatorGCacheObjClassRec;


/*  The Separator Gadget Cache instance record  */

typedef struct _XmSeparatorGCacheObjPart
{
   Dimension	  margin;
   unsigned char  orientation;
   unsigned char  separator_type;
   GC             separator_GC;
   
   GC               background_GC;
   GC               top_shadow_GC;
   GC               bottom_shadow_GC;
   
   Pixel            foreground;
   Pixel            background;
   
   Pixel            top_shadow_color;
   Pixmap           top_shadow_pixmap;
   
   Pixel            bottom_shadow_color;
   Pixmap           bottom_shadow_pixmap;

   Boolean          colors_inherited;
} XmSeparatorGCacheObjPart;

typedef struct _XmSeparatorGCacheObjRec
{
  ObjectPart                object;
  XmExtPart		    ext;
  XmSeparatorGCacheObjPart  separator_cache;
} XmSeparatorGCacheObjRec;


/*****************************************************/
/*  The Separator Widget Class and instance records  */
/*****************************************************/

typedef struct _XmSeparatorGadgetClassPart
{
   XtPointer               extension;
} XmSeparatorGadgetClassPart;


/*  Full class record declaration for Separator class  */

typedef struct _XmSeparatorGadgetClassRec
{
   RectObjClassPart            rect_class;
   XmGadgetClassPart           gadget_class;
   XmSeparatorGadgetClassPart  separator_class;
} XmSeparatorGadgetClassRec;

externalref XmSeparatorGadgetClassRec xmSeparatorGadgetClassRec;

typedef struct _XmSeparatorGadgetPart
{
  XmSeparatorGCacheObjPart  *cache;
  Boolean fill_bg_box;
} XmSeparatorGadgetPart;

/*  Full instance record declaration  */

typedef struct _XmSeparatorGadgetRec
{
   ObjectPart             object;
   RectObjPart            rectangle;
   XmGadgetPart           gadget;
   XmSeparatorGadgetPart  separator;
} XmSeparatorGadgetRec;

#define SEPG_INHERIT_NONE   		0
#define SEPG_INHERIT_BACKGROUND		(1 << 0)
#define SEPG_INHERIT_FOREGROUND		(1 << 1)
#define SEPG_INHERIT_TOP_SHADOW		(1 << 2)
#define SEPG_INHERIT_BOTTOM_SHADOW	(1 << 3)

#define SEPG_SetColorsInherited(byte, bit, v)    byte = (byte & (~bit)) | (v ? bit : 0)
#define SEPG_InheritBackground(w) (((XmSeparatorGadget)(w))-> \
                                    separator.cache->colors_inherited & SEPG_INHERIT_BACKGROUND)
#define SEPG_InheritForeground(w) (((XmSeparatorGadget)(w))-> \
                                    separator.cache->colors_inherited & SEPG_INHERIT_FOREGROUND)
#define SEPG_InheritTopShadow(w) (((XmSeparatorGadget)(w))-> \
                                    separator.cache->colors_inherited & SEPG_INHERIT_TOP_SHADOW)
#define SEPG_InheritBottomShadow(w) (((XmSeparatorGadget)(w))-> \
                                    separator.cache->colors_inherited & SEPG_INHERIT_BOTTOM_SHADOW)


/* MACROS for accessing instance fields*/
#define SEPG_Margin(w)		\
	(((XmSeparatorGadget)(w))->separator.cache->margin)
#define SEPG_Orientation(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->orientation)
#define SEPG_SeparatorType(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->separator_type)
#define SEPG_SeparatorGC(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->separator_GC)
#define SEPG_BackgroundGC(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->background_GC)
#define SEPG_TopShadowGC(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->top_shadow_GC)
#define SEPG_BottomShadowGC(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->bottom_shadow_GC)
#define SEPG_Foreground(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->foreground)
#define SEPG_Background(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->background)
#define SEPG_TopShadowColor(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->top_shadow_color)
#define SEPG_TopShadowPixmap(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->top_shadow_pixmap)
#define SEPG_BottomShadowColor(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->bottom_shadow_color)
#define SEPG_BottomShadowPixmap(w)	\
	(((XmSeparatorGadget)(w))->separator.cache->bottom_shadow_pixmap)
#define SEPG_ColorsInherited(w) \
	(((XmSeparatorGadget)(w))->separator.cache->colors_inherited)

/* Convenience Macros */
#define SEPG_Cache(w)		\
	(((XmSeparatorGadget)(w))->separator.cache)
#define SEPG_ClassCachePart(w) \
        (((XmSeparatorGadgetClass)xmSeparatorGadgetClass)->gadget_class.cache_part)


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */

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
/*   $XConsortium: SeparatoGP.h /main/cde1_maint/2 1995/08/18 19:22:32 drk $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/SeparatoG.h>
#include <Xm/GadgetP.h>
#include <Xm/ExtObjectP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************/
/* The Separator Gadget Cache Object's class and instance records*/
/*****************************************************************/

typedef struct _XmSeparatorGCacheObjClassPart
{
    int foo;
} XmSeparatorGCacheObjClassPart;


typedef struct _XmSeparatorGCacheObjClassRec  /* separator cache class record */
{
    ObjectClassPart                     object_class;
    XmExtClassPart                      ext_class;
    XmSeparatorGCacheObjClassPart       separator_class_cache;
} XmSeparatorGCacheObjClassRec;

externalref XmSeparatorGCacheObjClassRec xmSeparatorGCacheObjClassRec;


/*  The Separator Gadget Cache instance record  */

typedef struct _XmSeparatorGCacheObjPart
{
   Dimension	  margin;
   unsigned char  orientation;
   unsigned char  separator_type;
   GC             separator_GC;
} XmSeparatorGCacheObjPart;

typedef struct _XmSeparatorGCacheObjRec
{
  ObjectPart                object;
  XmExtPart		    ext;
  XmSeparatorGCacheObjPart  separator_cache;
} XmSeparatorGCacheObjRec;


/*****************************************************/
/*  The Separator Widget Class and instance records  */
/*****************************************************/

typedef struct _XmSeparatorGadgetClassPart
{
   XtPointer               extension;
} XmSeparatorGadgetClassPart;


/*  Full class record declaration for Separator class  */

typedef struct _XmSeparatorGadgetClassRec
{
   RectObjClassPart            rect_class;
   XmGadgetClassPart           gadget_class;
   XmSeparatorGadgetClassPart  separator_class;
} XmSeparatorGadgetClassRec;

externalref XmSeparatorGadgetClassRec xmSeparatorGadgetClassRec;

typedef struct _XmSeparatorGadgetPart
{
  XmSeparatorGCacheObjPart  *cache;
} XmSeparatorGadgetPart;

/*  Full instance record declaration  */

typedef struct _XmSeparatorGadgetRec
{
   ObjectPart             object;
   RectObjPart            rectangle;
   XmGadgetPart           gadget;
   XmSeparatorGadgetPart  separator;
} XmSeparatorGadgetRec;

/* MACROS for accessing instance fields*/
#define SEPG_Margin(w)			(((XmSeparatorGadget)(w))->   \
					   separator.cache->margin)
#define SEPG_Orientation(w)		(((XmSeparatorGadget)(w))->   \
					   separator.cache->orientation)
#define SEPG_SeparatorType(w)		(((XmSeparatorGadget)(w))->   \
					   separator.cache->separator_type)
#define SEPG_SeparatorGC(w)		(((XmSeparatorGadget)(w))->   \
					   separator.cache->separator_GC)

/* Convenience Macros */
#define SEPG_Cache(w)                    (((XmSeparatorGadget)(w))->\
					   separator.cache)
#define SEPG_ClassCachePart(w) \
        (((XmSeparatorGadgetClass)xmSeparatorGadgetClass)->gadget_class.cache_part)


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern int _XmSeparatorCacheCompare() ;

#else

extern int _XmSeparatorCacheCompare( 
                        XtPointer A,
                        XtPointer B) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _XmSeparatorGadgetP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
