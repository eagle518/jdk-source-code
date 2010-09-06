/* $XConsortium: RCHook.c /main/10 1996/12/16 18:32:29 drk $ */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF
 * for the full copyright text.
 * 
 */
/*
 * HISTORY
 */

#include "XmI.h"
#include "ColorObjI.h"
#include "RCHookI.h"

/**********************************************************************/
/** RCHook                                                           **/
/**    With new "per-display" color objects, we need to check to see **/
/**    if this ColorObj is using the color server before we set up   **/
/**    any special colors.                                           **/
/**                                                                  **/
/**********************************************************************/

/*ARGSUSED*/
void 
_XmRCColorHook(
        Widget w,
        ArgList alIn,		/* unused */
        Cardinal *acPtrIn )	/* unused */
{
    Arg al[10];
    int ac;
    unsigned char rcType;
    static int mono, color, colorPrim, init=0;
    static Screen *screen;
    Pixmap  ditherPix, solidPix;
    XmColorObj tmpColorObj=NULL;
    Pixel  defaultBackground;
    int depth = w->core.depth ;
    Display *ColorObjCacheDisplay;
    XContext ColorObjCache;
    XmColorObj DefaultColorObj;

    _XmProcessLock();
    ColorObjCacheDisplay = _XmColorObjCacheDisplay;
    ColorObjCache = _XmColorObjCache;
    DefaultColorObj = _XmDefaultColorObj;
    _XmProcessUnlock();

    /** get the colorObj for this display connection **/
    if (XFindContext(ColorObjCacheDisplay, (XID)XtDisplay(w), 
		     ColorObjCache, (XPointer *)&tmpColorObj) != 0)
    {   /* none found, use default */
	if (DefaultColorObj)
	    tmpColorObj = DefaultColorObj;
	else  /* this should NEVER happen... RowColInitHook won't get called */
	    return;
    }

    /** don't set colors if this display isn't using the color server **/
    if (!tmpColorObj->color_obj.colorIsRunning)	return;

    ac = 0;
    XtSetArg (al[ac], XmNrowColumnType, &rcType);                ac++;
    XtSetArg (al[ac], XmNbackground, &defaultBackground);        ac++;
    XtGetValues (w, al, ac);

    if (rcType == XmMENU_BAR)  /* set to secondary, rather than primary */
    {
	_XmProcessLock();
        if (!init)
        {
            if (tmpColorObj->color_obj.
		colorUse[tmpColorObj->color_obj.myScreen] == XmCO_BLACK_WHITE)
                mono = 1;
            else 
                mono = 0;

            color = tmpColorObj->color_obj.secondary;
            colorPrim = tmpColorObj->color_obj.primary;
            screen = XtScreen(tmpColorObj);
            init = 1;
        }
	_XmProcessUnlock();

	/** if background didn't default to ColorObj, 
	  don't overwrite colors **/
	if (defaultBackground != tmpColorObj->color_obj.myColors[colorPrim].bg)
	    return;

        ac = 0;
        XtSetArg (al[ac], XmNbackground, 
                  tmpColorObj->color_obj.myColors[color].bg);        ac++;
        XtSetArg (al[ac], XmNforeground, 
                  tmpColorObj->color_obj.myColors[color].fg);        ac++;
        XtSetArg (al[ac], XmNtopShadowColor, 
                  tmpColorObj->color_obj.myColors[color].ts);        ac++;
        XtSetArg (al[ac], XmNbottomShadowColor, 
                  tmpColorObj->color_obj.myColors[color].bs);        ac++;

        /** put dithers for top shadows if needed **/
        if (XmCO_DitherTopShadow(tmpColorObj->color_obj.display, 
				 tmpColorObj->color_obj.myScreen,
				 &tmpColorObj->color_obj.myColors[color]))
        {
/* Solaris 2.6 Motif diff bug 4085003 10 lines */

            if (mono)
                ditherPix = Xm21GetPixmapByDepth (screen, XmS50_foreground,
                                         BlackPixelOfScreen(screen),
                                         WhitePixelOfScreen(screen),
					 depth);
            else
                ditherPix = Xm21GetPixmapByDepth (screen, XmS50_foreground,
                                    tmpColorObj->color_obj.myColors[color].bg,
                                         WhitePixelOfScreen(screen),
					 depth);

            XtSetArg (al[ac], XmNtopShadowPixmap, ditherPix);         ac++;
        }
        else      /** see if we need to "undo" primary dither **/
        if (XmCO_DitherTopShadow(tmpColorObj->color_obj.display, 
				 tmpColorObj->color_obj.myScreen,
				 &tmpColorObj->color_obj.myColors[colorPrim]))
        {   /* simulate solid white (will happen for B_W case only)*/
/* Solaris 2.6 Motif diff bug 4085003 1 line */

            solidPix = Xm21GetPixmapByDepth (screen, "background",
					   WhitePixelOfScreen(screen),
					   WhitePixelOfScreen(screen),
					   depth);
            XtSetArg (al[ac], XmNtopShadowPixmap, solidPix);          ac++;
        }

        /** put dithers for bottom shadows if needed **/
        if (XmCO_DitherBottomShadow(tmpColorObj->color_obj.display, 
				    tmpColorObj->color_obj.myScreen,
				    &tmpColorObj->color_obj.myColors[color]))
        {
/* Solaris 2.6 Motif diff bug 4085003 1 line */

            if (mono)
                ditherPix = Xm21GetPixmapByDepth (screen, XmS50_foreground,
						BlackPixelOfScreen(screen),
						WhitePixelOfScreen(screen),
						depth);
            else
                ditherPix = Xm21GetPixmapByDepth (screen, XmS50_foreground,
                                   tmpColorObj->color_obj.myColors[color].bg,
						BlackPixelOfScreen(screen),
						depth);
            XtSetArg (al[ac], XmNbottomShadowPixmap, ditherPix);      ac++;
        }
        else      /** see if we need to "undo" primary dither **/
        if (XmCO_DitherBottomShadow(tmpColorObj->color_obj.display, 
				    tmpColorObj->color_obj.myScreen,
				    &tmpColorObj->color_obj.myColors[colorPrim]))
        {   /* simulate solid black (will happen for B_W case only)*/
/* Solaris 2.6 Motif diff bug 4085003 1 line */

            solidPix = Xm21GetPixmapByDepth (screen, "background",
					   BlackPixelOfScreen(screen),
					   BlackPixelOfScreen(screen),
					   depth);
            XtSetArg (al[ac], XmNbottomShadowPixmap, solidPix);       ac++;
        }

        XtSetValues (w, al, ac);
    }
}


