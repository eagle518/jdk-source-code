/* $XConsortium: GetRes.c /main/5 1995/07/15 20:51:12 drk $ */
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

/********    Static Function Declarations    ********/

static Cardinal GetSecResData(WidgetClass w_class,
			      XmSecondaryResourceData **secResDataRtn);

/********    End Static Function Declarations    ********/


Cardinal 
XmGetSecondaryResourceData(
        WidgetClass w_class,
        XmSecondaryResourceData **secondaryDataRtn )
{
  int num = GetSecResData(w_class, secondaryDataRtn);

  return num;
}

/*
 * GetSecResData()
 *  - Called from : XmGetSecondaryResourceData ().
 */
static Cardinal 
GetSecResData(
        WidgetClass w_class,
        XmSecondaryResourceData **secResDataRtn )
{
  XmBaseClassExt  *bcePtr;	/* bcePtr is really **XmBaseClassExtRec */
  Cardinal count = 0;
  
  bcePtr = _XmGetBaseClassExtPtr( w_class, XmQmotif); 
  if ((bcePtr) && (*bcePtr) && ((*bcePtr)->getSecResData))
    count = ( (*bcePtr)->getSecResData)( w_class, secResDataRtn);

  return count;
}


