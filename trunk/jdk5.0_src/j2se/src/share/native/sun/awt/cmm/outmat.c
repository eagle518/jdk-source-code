/*
 * @(#)outmat.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)outmat.c	1.17 98/11/16

	Contains:	makeOutputMatrixXform

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "kcms_sys.h"
#include <math.h>
#include "makefuts.h"

/*-------------------------------------------------------------------------------
 *  makeOutputMatrixXform -- given a 3 x 3 matrix as part of a Lut8Bit or Lut16Bit
 *		profile for an output device, compute and return the equivalent PT
 *-------------------------------------------------------------------------------
 */

PTErr_t
	makeOutputMatrixXform (	KpF15d16_p	matrix,
							KpUInt32_t	gridsize,
							fut_p FAR*	theFut)
{
PTErr_t				PTErr;
ResponseRecord_t	rTRC, gTRC, bTRC;
ResponseRecord_p	RR[3];
double				row0[3], row1[3], row2[3];
double_p			rowp[3];
MATRIXDATA			mdata;
KpInt32_t			dim[3];

    /* Initialize ResponseRecords to do nothing (identity mapping):  */    
	rTRC.TagSig = gTRC.TagSig = bTRC.TagSig = CURVE_TYPE_SIG;
	rTRC.CurveCount = gTRC.CurveCount = bTRC.CurveCount = 0;
	RR[0] = &rTRC;					/* set record pointers */
	RR[1] = &gTRC;
	RR[2] = &bTRC;

    /* Form matrix (XYZ -> ABC):  */
	row0[0]	= (double)matrix[0] / SCALEFIXED;
	row0[1] = (double)matrix[1] / SCALEFIXED;
	row0[2] = (double)matrix[2] / SCALEFIXED;
	row1[0] = (double)matrix[3] / SCALEFIXED;
	row1[1] = (double)matrix[4] / SCALEFIXED;
	row1[2] = (double)matrix[5] / SCALEFIXED;
	row2[0] = (double)matrix[6] / SCALEFIXED;
	row2[1] = (double)matrix[7] / SCALEFIXED;
	row2[2] = (double)matrix[8] / SCALEFIXED;

	rowp[0] = row0;					/* set row pointers */
	rowp[1] = row1;
	rowp[2] = row2;

    /* Construct matrix-data object:  */
	mdata.dim = 3;					/* always! */
	mdata.matrix = rowp;			/* set pointers */
	mdata.inResponse = RR;

	dim[0] = dim[1] = dim[2] = (KpInt32_t)gridsize;
	
	*theFut = fut_new_empty (3, dim, 3, KCP_XYZ_PCS, KCP_XYZ_PCS);	
	if (*theFut == NULL) {
	   return KCP_NO_MEMORY;
	}

    /* Make and return forward fut (XYZ -> ABC):  */
	PTErr = makeForwardXformFromMatrix (&mdata, KCP_TRC_LAGRANGE4_INTERP, dim, *theFut);
	
	return PTErr;
}
