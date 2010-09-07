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
/* $XConsortium: ResIndI.h /main/5 1995/07/13 17:50:09 drk $ */
#ifndef _XmResIndI_h
#define _XmResIndI_h

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

/********    Private Function Declarations for ResInd.c    ********/
  

extern int _XmConvertUnits( 
                        Screen *screen,
                        int dimension,
                        register int from_type,
                        register int from_val,
                        register int to_type) ;
extern void _XmUnitTypeDefault( 
                        Widget widget,
                        int offset,
                        XrmValue *value) ;
extern unsigned char _XmGetUnitType( 
                        Widget widget) ;
extern int _XmConvertFloatUnitsToIntUnits(
  			int unitType,
  			float unitValue,
  			int *intUnitType,
			float *intUnitValue,
  			int default_from_type) ;
extern int _XmConvertStringToUnits(
			Screen *screen,
		        String spec,
		        int default_from_type,
		        int orientation,
		        int to_type,
		        XtEnum *parse_error);
extern void _XmStringDirectionDefault(Widget widget,
				      int offset,
				      XrmValue *value );
 
/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmResIndI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
