/* $XConsortium: IconFile.h /main/5 1995/07/15 20:51:44 drk $ */
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
#ifndef _XmIconFile_h
#define _XmIconFile_h

#include <Xm/Xm.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
 * values for icon magnitude
 */
#define XmUNSPECIFIED_ICON_SIZE	0
#define XmLARGE_ICON_SIZE	1
#define XmMEDIUM_ICON_SIZE	2
#define XmSMALL_ICON_SIZE	3
#define XmTINY_ICON_SIZE	4


/********    Public Function Declarations for IconFile.c    ********/

extern String XmGetIconFileName( 
                        Screen *screen,
                        String imageInstanceName,
                        String imageClassName,
                        String hostPrefix,
                        unsigned int size) ;

/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmIconFile_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */



