/* $XConsortium: ScalTics.c /main/7 1995/12/06 21:39:58 cde-sun $ */
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

#include <Xm/Xm.h>
#include <Xm/XmosP.h>  /* for allocate local */
#include <Xm/Scale.h>
#include <Xm/SeparatoG.h>
#include "XmI.h"

/************************************************************************
 *
 *  XmScaleSetTicks
 *
 ************************************************************************/
void
XmScaleSetTicks(
        Widget scale,
        int big_every,
        Cardinal num_med,
        Cardinal num_small, 
	Dimension  size_big,
	Dimension  size_med,
	Dimension  size_small)
{
    Widget *sep ;
    Cardinal n, i, j, k, sep_num;
    int real_num_big, real_num_med, real_num_small;
    Arg args[5];
    int max, min ;
    unsigned char orient ;
    char * dim_res ;

    _XmWidgetToAppContext(scale);

    _XmAppLock(app);

    /* Some checking first */
    if (size_big == 0) { _XmAppUnlock(app); return ; }
    if (size_med == 0) num_med = 0 ;
    if (size_small == 0) num_small = 0 ;

    /* big_every is the number of values between big tics, while
       num_med and num_small are the number of tics between resp.
       big and med values */

    /* compute num_big first */
    n = 0 ;
    XtSetArg(args[n], XmNmaximum, &max); n++;
    XtSetArg(args[n], XmNminimum, &min); n++;
    XtSetArg(args[n], XmNorientation, &orient); n++;
    XtGetValues(scale, args, n);
    
    real_num_big = ((max - min) / big_every) + 1 ;
    if (real_num_big < 2) { _XmAppUnlock(app); return ; }

    real_num_med = (real_num_big - 1) * num_med;
    real_num_small = (real_num_big + real_num_med - 1) * num_small;
    sep_num = real_num_big + real_num_med + real_num_small;

    sep = (Widget*) ALLOCATE_LOCAL(sep_num * sizeof(Widget));

    if (orient == XmHORIZONTAL) {
	dim_res = XmNheight;
	orient = XmVERTICAL;
    } else {
	dim_res = XmNwidth ;
	orient = XmHORIZONTAL;
    }
    XtSetArg(args[0], XmNmargin, 0); 
    XtSetArg(args[1], XmNorientation, orient); 

    sep_num = 0 ;
    for (i=0; i < real_num_big; i++) {
	n = 2 ;
	XtSetArg(args[n], dim_res, size_big); n++ ;
	sep[sep_num] = XmCreateSeparatorGadget(scale, "BigTic", args, n); 
	sep_num++;

	if (i == real_num_big - 1) break ;

	for (k=0; k < num_small; k++) {
	    n = 2 ;
	    XtSetArg(args[n], dim_res, size_small); n++;
	    XtSetArg(args[n], XmNseparatorType, XmSINGLE_LINE); n++;
	    sep[sep_num] = XmCreateSeparatorGadget(scale, "SmallTic", 
						   args, n); 
	    sep_num++;
	}

	for (j=0; j < num_med; j++) {
	    n = 2 ;
	    XtSetArg(args[n], dim_res, size_med); n++;
	    sep[sep_num] = XmCreateSeparatorGadget(scale, "MedTic", args, n); 
	    sep_num++;
	    for (k=0; k < num_small; k++) {
		n = 2 ;
		XtSetArg(args[n], dim_res, size_small); n++;
		XtSetArg(args[n], XmNseparatorType, XmSINGLE_LINE); n++;
		sep[sep_num] = XmCreateSeparatorGadget(scale, "SmallTic", 
						       args, n); 
		sep_num++;
	    }
	    
	}
    }

    XtManageChildren(sep, sep_num);			
    DEALLOCATE_LOCAL((char*)sep);

    _XmAppUnlock(app);
}


