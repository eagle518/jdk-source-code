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

#ifndef _ImageCachIObso_h
#define _ImageCachIObso_h

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


#endif /* _ImageCachIObso_h */
