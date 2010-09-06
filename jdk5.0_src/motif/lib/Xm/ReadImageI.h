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
/* $XConsortium: ReadImageI.h /main/5 1995/07/13 17:46:29 drk $ */
#ifndef _XmReadImageI_h
#define _XmReadImageI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for ReadImage.c    ********/

extern XImage * _XmReadImageAndHotSpotFromFile( 
                        Display * display,
			char *filename,
                        int *hot_x,
                        int *hot_y) ;

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmReadImageI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
