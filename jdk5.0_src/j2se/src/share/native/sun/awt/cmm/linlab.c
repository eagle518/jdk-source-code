/*
 * @(#)linlab.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	linlab.c
 
	Contains:	functions to create the LinearLab FuT.

	Created by msm, November 7, 1997

	COPYRIGHT (c) 1997-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <stdio.h>
#include "kcptmgr.h"
#include "makefuts.h"


typedef struct fData_s {
	fut_calcData_t	std;
	KpInt32_t		iClass;
	KpInt32_t		oClass;
	double			neutralgrid;
	double			neutralInput;
} fData_t, FAR* fData_p;


/* Lifun -- input mappings:  Convert L* to unsigned
 *---------------------------------------------------------------------
 */
static double
	Lifun (double L, fut_calcData_p dataP)
{
	if (((fData_p) dataP)->iClass == KCP_VARIABLE_RANGE) {
		L *= KCP_16_TO_8_ENCODING;
	}

	return (RESTRICT (L, 0.0, 1.0));
}


/*  abifun -- piecewise-linear input mappings, with gridpoint at
 *	neutral (a* = b* = 0; 8-bit encoding = 128)
 */
static double
	abifun (double y, fut_calcData_p dataP)
{
double	delta, neutralgrid;

	neutralgrid = ((fData_p) dataP)->neutralgrid;

	if (((fData_p) dataP)->iClass == KCP_VARIABLE_RANGE) {
		y *= KCP_16_TO_8_ENCODING;
	}

	delta = y - ((fData_p) dataP)->neutralInput;
	if (delta < 0.0) {
		y = neutralgrid * (y / ((fData_p) dataP)->neutralInput);
	}
	else {
		y = 1.0 - (1.0 - neutralgrid) * ((1.0 - y) / (1.0 - ((fData_p) dataP)->neutralInput));
	}

	return RESTRICT (y, 0.0, 1.0);
}


/*  ofun -- piecewise-linear output-table mappings; inverse of xfun, yfun, zfun
 */
static double
	ofun (double q, fut_calcData_p dataP)
{
double	p, delta, neutralgrid;

	neutralgrid = ((fData_p) dataP)->neutralgrid;

	p = q;

	switch (((fData_p) dataP)->std.chan)
	{
	   case 0:   /* L* */
			break;

	   case 1:   /* a */
	   case 2:   /* b */
			delta = p - neutralgrid;
			if (delta < 0.0)
			   p = ((fData_p) dataP)->neutralInput * (p / neutralgrid);
			else
			   p = 1.0 - (1.0 - ((fData_p) dataP)->neutralInput) * ((1.0 - p) / (1.0 - neutralgrid));
			break;
	}

	if (((fData_p) dataP)->oClass == KCP_VARIABLE_RANGE) {
		p *= KCP_8_TO_16_ENCODING;
	}
	p = RESTRICT (p, 0.0, 1.0);
	return p;
}


/*  get_linlab_fut --	construct an Linear Lab FuT
 *						to establish a grid for composition
	input and output data class is set to use MP2 scaling
 */

#define KCP_THIS_FUT_CHANS (3)

fut_p
	get_linlab_fut (KpInt32_t		size,
					PTDataClass_t	iClass,
					PTDataClass_t	oClass)
{
fut_p		futp;
KpInt32_t	iomask, sizeArray[KCP_THIS_FUT_CHANS];
fut_ifunc_t	ifunArray[KCP_THIS_FUT_CHANS] = {Lifun, abifun, abifun};
fut_ofunc_t	ofunArray[KCP_THIS_FUT_CHANS] = {ofun, ofun, ofun};
fData_t		fData;

	if (iClass) {}	if (oClass) {}

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "get_linlab_fut\n size %d\n", size);
	kcpDiagLog (string); }
	#endif

	iomask = FUT_IN(FUT_XYZ) | FUT_OUT(FUT_XYZ);

	/* define neutral input position and grid point */
	fData.iClass = iClass;
	fData.oClass = oClass;
	fData.neutralInput = KCP_LAB8_NEUTRAL;
	fData.neutralgrid = (double)(size / 2) / (double)(size - 1);

	/* assume all dimensions are the same */
	sizeArray[0] = size;
	sizeArray[1] = size;
	sizeArray[2] = size;

	futp = constructfut (iomask, sizeArray, &fData.std, ifunArray, NULL, ofunArray,
						iClass, oClass);

	#if defined KCP_DIAG_LOG
	saveFut (futp, "CP24.fut");
	#endif
				
	return (futp);
}
