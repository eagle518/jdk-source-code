/* $XConsortium: ColorObj.h /main/cde1_maint/3 1995/10/05 12:05:31 lehors $ */
/* 
 * @OSF_COPYRIGHT@
 * (c) Copyright 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *  
*/ 
/*
 * HISTORY
 * Motif Release 1.2.5
 * Included for binary compatibility with 1.2 Motif
*/

#ifndef _ColorObj_h
#define _ColorObj_h

#ifndef MOTIF12_HEADERS

#include <Xm/Xm.h>
#include <Xm/ColorObjP.h>

/* Adding these to make them refer to 2.1 stuff */
#define PixelSet		XmPixelSet
#define _ColorObjClassRec	_XmColorObjClassRec
#define _ColorObjRec		_XmColorObjRec
#define _xmColorObjClass	xmColorObjClass


#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern WidgetClass  _xmColorObjClass;

typedef struct _ColorObjClassRec *ColorObjClass;
typedef struct _ColorObjRec      *ColorObj;

/** misc structures, defines, and functions for using ColorObj **/

#define  DitherTopShadow(display, screen, pixelSet) \
                        ((pixelSet)->bs == BlackPixel((display), (screen)))

#define  DitherBottomShadow(display, screen, pixelSet) \
                        ((pixelSet)->ts == WhitePixel((display), (screen)))

#define  DITHER		XmCO_DITHER
#define  NO_DITHER	XmCO_NO_DITHER

/* defines for color usage */
#define B_W			0
#define LOW_COLOR		1
#define MEDIUM_COLOR		2
#define HIGH_COLOR		3

#define COLOR_SRV_NAME "ColorServer"

/* defines for palette.c */
#define VALUE_THRESHOLD 225

/* defines for Atom strings */
#define PIXEL_SET        "Pixel Sets"

#ifdef SUN_SDT_COLOR_OBJECT_CACHE
#define _SUN_SDT_COLOR_OBJECT_CACHE        "SDT Pixel Set"
#define SUN_SDT_COLOR_OBJECT_VERSION        '1'
#endif /* SUN_SDT_COLOR_OBJECT_CACHE */

#define PALETTE_NAME     "DefaultPalette Name"
#define TYPE_OF_MONITOR  "Type Of Monitor"
#define UPDATE_FILE      "Update Default File"
#define CUST_DATA        "Customize Data:"

#define MAX_NUM_COLORS	XmCO_MAX_NUM_COLORS
#define NUM_COLORS	XmCO_NUM_COLORS

#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#else /* MOTIF12_HEADERS */

/* $XConsortium: ColorObj.h /main/cde1_maint/3 1995/10/05 12:05:31 lehors $ */
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
/*** ColorObj.h ***/


#include <Xm/Xm.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern WidgetClass  _xmColorObjClass;

typedef struct _ColorObjClassRec *ColorObjClass;
typedef struct _ColorObjRec      *ColorObj;

#define XmNprimaryColorSetId          "primaryColorSetId"
#define XmCPrimaryColorSetId          "PrimaryColorSetId"
#define XmNsecondaryColorSetId        "secondaryColorSetId"
#define XmCSecondaryColorSetId        "SecondaryColorSetId"
#define XmNactiveColorSetId           "activeColorSetId"
#define XmCActiveColorSetId           "ActiveColorSetId"
#define XmNinactiveColorSetId         "inactiveColorSetId"
#define XmCInactiveColorSetId         "InactiveColorSetId"
#define XmNuseColorObj                "useColorObj"
#define XmCUseColorObj                "UseColorObj"

#define XmNtextColorSetId             "textColorSetId"
#define XmCTextColorSetId             "TextColorSetId"
#define XmNuseTextColor               "useTextColor"
#define XmCUseTextColor               "UseTextColor"
#define XmNuseTextColorForList        "useTextColorForList"
#define XmCUseTextColorForList        "UseTextColorForList"

#define XmNuseMask		"useMask"
#define XmCUseMask		"UseMask"
#define XmNuseMultiColorIcons	"useMultiColorIcons"
#define XmCUseMultiColorIcons	"UseMultiColorIcons"

/** misc structures, defines, and functions for using ColorObj **/

typedef struct {
    Pixel fg;
    Pixel bg;
    Pixel ts;
    Pixel bs;
    Pixel sc;
} PixelSet;

#define  DitherTopShadow(display, screen, pixelSet) \
                        ((pixelSet)->bs == BlackPixel((display), (screen)))

#define  DitherBottomShadow(display, screen, pixelSet) \
                        ((pixelSet)->ts == WhitePixel((display), (screen)))

#define  DITHER     "50_foreground"
#define  NO_DITHER  "unspecified_pixmap"

/* defines for color usage */
#define B_W           0
#define LOW_COLOR     1
#define MEDIUM_COLOR  2
#define HIGH_COLOR    3

#define COLOR_SRV_NAME "ColorServer"

/* defines for palette.c */
#define VALUE_THRESHOLD 225

/* defines for Atom strings */
#define PIXEL_SET        "Pixel Sets"

#ifdef SUN_SDT_COLOR_OBJECT_CACHE
#define _SUN_SDT_COLOR_OBJECT_CACHE        "SDT Pixel Set"
#define SUN_SDT_COLOR_OBJECT_VERSION        '1'
#endif /* SUN_SDT_COLOR_OBJECT_CACHE */

#define PALETTE_NAME     "DefaultPalette Name"
#define TYPE_OF_MONITOR  "Type Of Monitor"
#define UPDATE_FILE      "Update Default File"
#define CUST_DATA        "Customize Data:"

#define  MAX_NUM_COLORS  8
#define  NUM_COLORS  MAX_NUM_COLORS

#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* MOTIF12_HEADERS */

#endif /* _ColorObj_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
