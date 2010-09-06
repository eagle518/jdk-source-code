/* $XConsortium: CvtUnit.c /main/5 1995/07/15 20:50:25 drk $ */
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
#include "RepTypeI.h"

/************************************************************************
 *
 *  XmCvtStringToUnitType
 *	Convert a string to resolution independent unit type.
 *  This routine is obsolete, the converter is now available
 *  by default. We just call it here.
 *  
 ************************************************************************/
/*ARGSUSED*/
void 
XmCvtStringToUnitType(
        XrmValuePtr args,	/* unused */
        Cardinal *num_args,	/* unused */
        XrmValue *from_val,
        XrmValue *to_val )
{
   Display * dpy = _XmGetDefaultDisplay();

   /* we cannot call XmRUnitType directly, since it would loop
      if a program registers this function with to_type = XmRUnitType (which
      is very likely).
      So we use REAL_UNIT_TYPE_NAME, which has been registered with the
      same semantics as the original XmRUnitType in RepType.c */
   XtConvertAndStore(XmGetXmDisplay(dpy), XmRString, from_val,
		     REAL_UNIT_TYPE_NAME, to_val);

}
