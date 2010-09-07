/* $XConsortium: GetPixData.c /main/6 1995/10/25 20:05:46 cde-sun $ */
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

#include <Xm/XmP.h>
#include "ImageCachI.h"
#include "XmI.h"

/*******************************************************************
 *
 * XmeGetPixmapData.
 *   see if this pixmap is in the cache. If it is then return all the
 *   gory details about it. If not put it in the cache first.
 *   The RETURN pointers can be NULL.
 *   This function never fails (unless pixmap bad id) and always returns
 *   valid information for depth, with, height, etc .
 *   It returns True if the pixmap was in the cache to start with
 *   and True if it had to fetch it to the server first.
 *
 *******************************************************************/
Boolean 
XmeGetPixmapData(
    Screen *screen,
    Pixmap pixmap,
    char **image_name,
    int *depth,
    Pixel *foreground,
    Pixel *background,
    int *hot_x,
    int *hot_y,
    unsigned int *width,
    unsigned int *height)
{
    char *loc_image_name;
    int loc_depth;
    Pixel loc_foreground;
    Pixel loc_background;
    int loc_hot_x;
    int loc_hot_y;
    unsigned int loc_width = 0;
    unsigned int loc_height;
    XtAppContext app;
    
    app = XtDisplayToApplicationContext(DisplayOfScreen(screen));
    
    _XmAppLock(app);
    
    /* support passed NULL argument */
    if (!image_name) image_name = &loc_image_name ;
    if (!depth) depth = &loc_depth ;
    if (!background) background = &loc_background ;
    if (!foreground) foreground = &loc_foreground ;
    if (!hot_x) hot_x = &loc_hot_x ;
    if (!hot_y) hot_y = &loc_hot_y ;
    if (!width) width = &loc_width ;
    if (!height) height = &loc_height ;

/* Solaris 2.6 Motif diff bug 4085003 1 line */

    if (_Xm21GetPixmapData(screen, pixmap, image_name, depth, foreground,
			 background, hot_x, hot_y, width, height)) {
	_XmAppUnlock(app);
	return True ;
    }

    
    /* not in the cache, generate an incomplete entry in the pixmap cache */
    /* Use a magic name, which will have _XmCachePixmap not cache this
       one in the pixmap_data name based cache */

/* Solaris 2.6 Motif diff bug 4085003 1 line */

    if (_XmCachePixmap(pixmap, screen, DIRECT_PIXMAP_CACHED, 1, 0, 0, 0, 0)) {
	/* and query again */
	_Xm21GetPixmapData(screen, pixmap, image_name, depth, foreground,
			 background, hot_x, hot_y, width, height);
    }

    _XmAppUnlock(app);

    return (False);
}

