/* $XConsortium: ColObjFunc.c /main/7 1995/10/25 19:56:10 cde-sun $ */
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


/**********************************************************************/
/** XmeUseColorObj()                                                 **/
/**           Return False if color is not working for some reason.  **/
/**                                                                  **/
/**           Could be due to useColorObj resource == False, or any  **/
/**           problem with the color server or color object.         **/
/**                                                                  **/
/**********************************************************************/
Boolean 
XmeUseColorObj( void )
{
    XmColorObj tmpColorObj = _XmDefaultColorObj;

    _XmProcessLock();
    if (!tmpColorObj ||
      !tmpColorObj->color_obj.colorIsRunning ||
      !tmpColorObj->color_obj.useColorObj) {
      _XmProcessUnlock();
      return False;
    }
    else {
      _XmProcessUnlock();
      return True;
    }
}



/**********************************************************************/
/** Following entries kept for bc with CDE (they'll be moved to
 **       obsolete module later                                      **/
/**                                                                  **/
/**********************************************************************/
Boolean 
_XmGetPixelData(
        int screen,
        int *colorUse,
        XmPixelSet *pixelSet,
        short *a,
        short *i,
        short *p,
        short *s )
{
    return XmeGetPixelData( screen, colorUse, pixelSet, a, i, p, s );
}

Boolean 
_XmGetIconControlInfo(
        Screen  *screen,
	Boolean *useMaskRtn,
        Boolean *useMultiColorIconsRtn,
        Boolean *useIconFileCacheRtn)
{
    return XmeGetIconControlInfo(screen, useMaskRtn, 
			  useMultiColorIconsRtn, useIconFileCacheRtn);
}

Boolean 
_XmUseColorObj( void )
{
    return XmeUseColorObj();
}

