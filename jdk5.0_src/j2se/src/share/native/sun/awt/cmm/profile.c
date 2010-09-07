/*
 * @(#)profile.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)profile.c	1.31 99/01/24

	Contains:	PTNewMatGamPT, makeProfileXform

	COPYRIGHT (c) 1993-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "kcms_sys.h"
#include <math.h>
#include "attrib.h"
#include "makefuts.h"
#include "matrix.h"
#include "kcptmgr.h"


static KpInt32_t xyz2cone (	KpMatrix_p src, KpMatrix_p dest);
static KpInt32_t cone2xyz (	KpMatrix_p src, KpMatrix_p dest);


/*------------------------------------------------------------------------------
 *  PTNewMatGamPT -- Make a PT from a matrix and a set of 1D gamma tables, without white point adaptation.
 *------------------------------------------------------------------------------
 */
PTErr_t
	 PTNewMatGamPT(	FixedXYZColor_p		rXYZ,
					FixedXYZColor_p		gXYZ,
					FixedXYZColor_p		bXYZ,
					ResponseRecord_p	rTRC,
					ResponseRecord_p	gTRC,
					ResponseRecord_p	bTRC,
					KpUInt32_t			gridsize,
					KpBool_t			invert,
					PTRefNum_p			thePTRefNumP)
{
PTErr_t		PTErr;
newMGmode_t	newMGmode;

	newMGmode.adaptMode = KCP_NO_ADAPTATION;
	newMGmode.interpMode = KCP_TRC_LINEAR_INTERP;

	PTErr = PTNewMatGamAIPT (rXYZ, gXYZ, bXYZ, rTRC, gTRC, bTRC,
							gridsize, invert, &newMGmode, thePTRefNumP);

	return (PTErr);
}


/*------------------------------------------------------------------------------
 *  PTNewMatGamAIPT -- Make a PT from a matrix and a set of 1D gamma tables
 *      given a set of tags defining a profile for a monitor or scanner,
 *		not having a 3D LUT, compute and return a PT that will transform 
 *		RGB to XYZ or vice versa
 *------------------------------------------------------------------------------
 */
PTErr_t
	 PTNewMatGamAIPT(	FixedXYZColor_p		rXYZ,
						FixedXYZColor_p		gXYZ,
						FixedXYZColor_p		bXYZ,
						ResponseRecord_p	rTRC,
						ResponseRecord_p	gTRC,
						ResponseRecord_p	bTRC,
						KpUInt32_t			gridsize,
						KpBool_t			invert,
						newMGmode_p			newMGmodeP,
						PTRefNum_p			thePTRefNumP)
{
fut_p		theFut = NULL;
PTErr_t		PTErr;
ResponseRecord_p	RR[3];
double		row0[3], row1[3], row2[3], xfactor, yfactor, zfactor;
double_p	rowp[3];
MATRIXDATA	mdata;
KpMatrix_t	inputXYZ, curWP, adaptedXYZ, newWP, scale, newWPcone, curWPcone, inputCone, adaptedCone;
KpUInt32_t	row, col;
KpInt32_t	error1, error2, dim[3], inSpace, outSpace;

/* Check for valid ptrs */
	PTErr = KCP_BAD_ARG;
	if (rXYZ == NULL)	goto GetOut;
	if (gXYZ == NULL)	goto GetOut;
	if (bXYZ == NULL)	goto GetOut;
	if (rTRC == NULL)	goto GetOut;
	if (gTRC == NULL)	goto GetOut;
	if (bTRC == NULL)	goto GetOut;
	if (thePTRefNumP == NULL)	goto GetOut;
	if (newMGmodeP == NULL)	goto GetOut;

	if ((newMGmodeP->interpMode != KCP_TRC_LINEAR_INTERP) &&
		(newMGmodeP->interpMode != KCP_TRC_LAGRANGE4_INTERP)) 	goto GetOut;
		
	if (gridsize < 2)	goto GetOut;

	*thePTRefNumP = 0;

	/* Create matrix from Profile tags:  */
	inputXYZ.nRows = 3;
	inputXYZ.nCols = 3;
	inputXYZ.coef[0][0] = (double)rXYZ->X / SCALEFIXED;	/* convert to double */
	inputXYZ.coef[0][1] = (double)gXYZ->X / SCALEFIXED;
	inputXYZ.coef[0][2] = (double)bXYZ->X / SCALEFIXED;
	inputXYZ.coef[1][0] = (double)rXYZ->Y / SCALEFIXED;
	inputXYZ.coef[1][1] = (double)gXYZ->Y / SCALEFIXED;
	inputXYZ.coef[1][2] = (double)bXYZ->Y / SCALEFIXED;
	inputXYZ.coef[2][0] = (double)rXYZ->Z / SCALEFIXED;
	inputXYZ.coef[2][1] = (double)gXYZ->Z / SCALEFIXED;
	inputXYZ.coef[2][2] = (double)bXYZ->Z / SCALEFIXED;

	/* Adjust given matrix by applying chromatic adaptation from current white point to D50  */
	adaptedXYZ.nRows = 3;	/* set up resultant matrix */
	adaptedXYZ.nCols = 3;

	curWP.nRows = 3;		/* set up vector for current white point */
	curWP.nCols = 1;
	KpMatZero (&curWP);

	curWP.coef[0][0] = inputXYZ.coef[0][0] + inputXYZ.coef[0][1] + inputXYZ.coef[0][2];	/* = X_RGB */
	if (curWP.coef[0][0] <= 0.0) {
		goto GetOut;
	}

	curWP.coef[1][0] = inputXYZ.coef[1][0] + inputXYZ.coef[1][1] + inputXYZ.coef[1][2];	/* = Y_RGB */
	if (curWP.coef[1][0] <= 0.0) {
		goto GetOut;
	}

	curWP.coef[2][0] = inputXYZ.coef[2][0] + inputXYZ.coef[2][1] + inputXYZ.coef[2][2];	/* = Z_RGB */
	if (curWP.coef[2][0] <= 0.0) {
		goto GetOut;
	}

	switch (newMGmodeP->adaptMode) {
	
	case KCP_NO_ADAPTATION:
	
		error1 = KpMatCopy (&inputXYZ, &adaptedXYZ);	/* use input XYZs without modification */
		if (error1 != 1) {
			goto GetOut;
		}

		break;
		
	case KCP_XYZD50_ADAPTATION:
	
		/* calculate X, Y, and Z adaptation factors */
		xfactor = KCP_D50_X / curWP.coef[0][0];
		yfactor = KCP_D50_Y / curWP.coef[1][0];
		zfactor = KCP_D50_Z / curWP.coef[2][0];

		/* apply adaptation to matrix */
		for (col = 0; col < 3; col++) {
			adaptedXYZ.coef[0][col] = inputXYZ.coef[0][col] * xfactor;
			adaptedXYZ.coef[1][col] = inputXYZ.coef[1][col] * yfactor;
			adaptedXYZ.coef[2][col] = inputXYZ.coef[2][col] * zfactor;
		}
		
		break;
		
	case KCP_BRADFORDD50_ADAPTATION:
	
		newWP.nRows = 3;				/* set up new white point vector */
		newWP.nCols = 1;

		newWP.coef[0][0] = KCP_D50_X;	/* which is D50 */
		newWP.coef[1][0] = KCP_D50_Y;
		newWP.coef[2][0] = KCP_D50_Z;

		scale.nRows = 3;				/* set up scaling vector */
		scale.nCols = 1;

		error1 = xyz2cone (&newWP, &newWPcone);	/* convert new WP to cone primaries */
		error2 = xyz2cone (&curWP, &curWPcone);	/* convert current WP to cone primaries */
		if ((error1 != 1) || (error2 != 1)) {
			goto GetOut;
		}

		error1 = KpMatDotDiv (&newWPcone, &curWPcone, &scale);	/* calculate ratios */
		if (error1 != 1) {
			goto GetOut;
		}

		for (col = 1; col < 3; col++) {	/* replicate scale vector to 2nd and 3rd columns */
			for (row = 0; row < 3; row++) {
				scale.coef[row][col] = scale.coef[row][0];
			}
		}

		scale.nCols = 3;				/* has 3 columns now */
		
		error1 = xyz2cone (&inputXYZ, &inputCone);	/* convert input XYZs to cone primaries */
		if (error1 != 1) {
			goto GetOut;
		}
		
		error1 = KpMatDotMul (&inputCone, &scale, &adaptedCone);	/* adapt cone primaries */
		if (error1 != 1) {
			goto GetOut;
		}

		error1 = cone2xyz (&adaptedCone, &adaptedXYZ);	/* convert input XYZs to cone primaries */
		if (error1 != 1) {
			goto GetOut;
		}
		
		break;
		
	default:
		goto GetOut;
	}

	/* Create matrix-data structure:  */
	rowp[0] = row0;				/* set row pointers */
	rowp[1] = row1;
	rowp[2] = row2;

	RR[0] = rTRC;				/* set record pointers */
	RR[1] = gTRC;
	RR[2] = bTRC;

	mdata.dim = 3;				/* always! */
	mdata.matrix = rowp;		/* set pointers */

	for (col = 0; col < 3; col++) {
		row0[col] = adaptedXYZ.coef[0][col] * XYZSCALE;	/* load matrix coefficients */
		row1[col] = adaptedXYZ.coef[1][col] * XYZSCALE;	/* and scale for ICC */
		row2[col] = adaptedXYZ.coef[2][col] * XYZSCALE;
	}

	dim[0] = dim[1] = dim[2] = (KpInt32_t) gridsize;

	/* Create import or export PT, according to user choice: */
	if (invert == KPFALSE) {	
		theFut = fut_new_empty (3, dim, 3, KCP_FIXED_RANGE, KCP_XYZ_PCS);	
		if (theFut == NULL) {
			goto ErrOut4;
		}

		mdata.inResponse = RR;
		PTErr = makeForwardXformFromMatrix (&mdata, newMGmodeP->interpMode, dim, theFut);

		inSpace = KCM_RGB;			/* setup the foward color space */
		outSpace = KCM_CIE_XYZ; 
	}
	else {	
		theFut = fut_new_empty (3, dim, 3, KCP_XYZ_PCS, KCP_FIXED_RANGE);	
		if (theFut == NULL) {
			goto ErrOut4;
		}

		mdata.outResponse = RR;
		PTErr = makeInverseXformFromMatrix (&mdata, newMGmodeP->interpMode, dim, theFut);

		inSpace = KCM_CIE_XYZ;		/* setup the inverse color space */
		outSpace = KCM_RGB; 
	}
	
	if (PTErr != KCP_SUCCESS) {
	   goto ErrOut1;
	}

	PTErr = fut2PT (&theFut, inSpace, outSpace, PTTYPE_CALCULATED, thePTRefNumP);	/* make into PT */	
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut0;
	}

GetOut:
	return (PTErr);


ErrOut4:
	PTErr = KCP_NO_MEMORY;
	goto ErrOut0;

ErrOut1:
	PTErr = KCP_BAD_ARG;

ErrOut0:
	if (theFut != NULL) fut_free (theFut);
	if (*thePTRefNumP != 0) PTCheckOut (*thePTRefNumP);
	goto GetOut;
}


/* Convert tristimulus values to the rho, gamma, beta
 * cone promaries.  See Hunt, Measuring Colour pg. 71.
 */

static KpInt32_t
	xyz2cone (	KpMatrix_p src,
				KpMatrix_p dest)
{
static KpMatrix_t toCone = {3, 3,
					 0.8951,	0.2664,		-0.1614,
					-0.7502,	1.7135,	 	 0.0367,
					 0.0389,   -0.0685,		 1.0296};
KpInt32_t	error;

	error = KpMatMul (&toCone, src, dest);
	
	return error;
}


/* Convert tristimulus values to the rho, gamma, beta
 * cone promaries.  See Hunt, Measuring Colour pg. 71.
 */

static KpInt32_t
	cone2xyz (	KpMatrix_p src,
				KpMatrix_p dest)
{
static KpMatrix_t fromCone = {3, 3,
					 0.9870,   -0.1471,   	0.1600,
					 0.4323,	0.5184,   	0.0493,
					-0.0085,	0.0400,		0.9685};
KpInt32_t	error;

	error = KpMatMul (&fromCone, src, dest);
	
	return error;
}
