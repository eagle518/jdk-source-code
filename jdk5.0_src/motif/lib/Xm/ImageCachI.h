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
/* $XConsortium: ImageCachI.h /main/7 1996/01/29 13:19:43 daniel $ */
#ifndef _XmImageCacheI_h
#define _XmImageCacheI_h

#ifndef MOTIF12_HEADERS 

#include <Xm/XmP.h>

/* this name is used by XmeGetPixmapData to cache a pixmap in the
   pixmap cache with no associated name. _XmCachePixmap knows about it
   and will not add this one in the pixmap_data cache because it is a
   _name_ based pixmap cache used during conversion */
#define DIRECT_PIXMAP_CACHED ""

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for ImageCache.c    ********/

extern Boolean _Xm21InstallImage( 
                        XImage *image,
                        char *image_name,
                        int hot_x,
                        int hot_y) ;
extern Boolean _Xm21GetImage( 
                        Screen *screen,
                        char *image_name,
                        XImage **image) ;
extern Boolean _XmCachePixmap( 
			      Pixmap pixmap,
			      Screen *screen,
			      char *image_name,
			      Pixel foreground,
			      Pixel background,
			      int depth,
			      Dimension width,
			      Dimension height) ;
extern Pixmap _XmGetColoredPixmap(Screen *screen,
				  char *image_name,
				  XmAccessColorData acc_color,
				  int depth,
				  Boolean only_if_exists) ;

extern Boolean _Xm21GetPixmapData(
		   Screen *screen,
		   Pixmap pixmap,
		   char **image_name,
		   int *depth,
		   Pixel *foreground,
		   Pixel *background,
		   int *hot_x,
		   int *hot_y,
		   unsigned int *width,
		   unsigned int *height) ;
extern Boolean _Xm21InImageCache(
			       String image_name);

extern Pixmap _XmGetScaledPixmap(
    Screen *screen,
    Widget widget,
    char *image_name,
    XmAccessColorData acc_color,
    int depth,
    Boolean only_if_exists,
    double scaling_ratio);

extern void _XmPutScaledImage (    
    Display*		 display ,
    Drawable		 d ,
    GC			 gc ,
    XImage*		 src_image ,
    int			 src_x ,
    int			 src_y ,
    int			 dest_x ,
    int			 dest_y ,
    unsigned int	 src_width ,
    unsigned int	 src_height, 
    unsigned int	 dest_width ,
    unsigned int	 dest_height);

extern void _XmCleanPixmapCache(Screen * screen, Widget shell);

/* for Xm.h */
extern Pixmap XmGetScaledPixmap(
    Widget widget,
    char *image_name,
    Pixel foreground,
    Pixel background,
    int depth,
    double scaling_ratio);

/********    End Private Function Declarations    ********/

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
/************************************************************************* 
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 2002 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 *************************************************************************/
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: ImageCachI.h /main/cde1_maint/2 1995/08/18 19:07:38 drk $"
#endif
#endif

#include <Xm/XmP.h>
#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for Pixmap.c   ********/
#ifdef _NO_PROTO
extern Boolean _XmGetPixmapData() ;
extern Boolean _XmInstallImage();
extern Boolean _XmGetImage();
extern Boolean _XmInImageCache() ;
extern Pixmap _XmGetPixmap() ;
extern Boolean _XmInstallPixmap() ;
extern Boolean _XmInstallPixmapByDepth();

#else
extern Boolean _XmGetPixmapData( 
                        Screen *screen,
                        Pixmap pixmap,
                        char **image_name,
                        int *depth,
                        Pixel *foreground,
                        Pixel *background,
                        int *hot_x,
                        int *hot_y,
                        unsigned int *width,
                        unsigned int *height) ;
extern Boolean _XmInstallImage(
		        XImage *image,
		        char *image_name,
		        int hot_x,
		        int hot_y);
extern Boolean _XmGetImage(
		        Screen *screen,
		        char *image_name,
			XImage **imageRtn);
extern Boolean _XmInImageCache( 
                        char *image_name) ;
extern Pixmap _XmGetPixmap( 
                        Screen *screen,
                        char *image_name,
                        int depth,
                        Pixel foreground,
                        Pixel background);
extern Boolean _XmInstallPixmap( 
                        Pixmap pixmap,
                        Screen *screen,
                        char *image_name,
                        Pixel foreground,
                        Pixel background) ;
extern Boolean _XmInstallPixmapByDepth( Pixmap pixmap,
                                  Screen *screen,
				  char *image_name,
				  Pixel foreground,
				  Pixel background,
				  int depth);

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif


#endif /* MOTIF12_HEADERS */

#endif /* _ImageCachI_h */


