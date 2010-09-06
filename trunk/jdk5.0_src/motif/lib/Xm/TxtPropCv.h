/* $XConsortium: TxtPropCv.h /main/5 1995/07/15 20:56:52 drk $ */
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

#ifndef _XmTxtPropCvP_h
#define _XmTxtPropCvP_h

#ifdef __cplusplus
extern "C" {
#endif

/********    Public Function Declarations    ********/

extern int XmCvtXmStringTableToTextProperty(Display *display,
				 XmStringTable string_table,
				 int count,
				 XmICCEncodingStyle style,
				 XTextProperty *text_prop_return);

extern int XmCvtTextPropertyToXmStringTable(Display *display,
				 XTextProperty *text_prop,
				 XmStringTable *string_table_return,
				 int *count_return);

/********    End Public Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif  /* _XmTxtPropCvP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
