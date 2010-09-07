/*
 * @(#)spxfgpr.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains conversion routines
			not needed by SUN nor Java Libraries.

			Pulled from spxffpu.c, spxf_gen.c and
				spxfblob.c 7/2/99

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2000 by Eastman Kodak Company, 
	            all rights reserved.

	Changes:
	
	SCCS Revision:
		@(#)spxfgpr.c	1.2	12/19/03

	Revision History:
		$Workfile: spxfgpr.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spxfgpr.c $
		$Revision: 6 $
		$Date: 1/25/02 4:02p $
		$Author: Doro $

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** PROPRIETARY NOTICE:     The  software  information   contained ***
 *** herein is the  sole property of  Eastman Kodak Company  and is ***
 *** provided to Eastman Kodak users under license for use on their ***
 *** designated  equipment  only.  Reproduction of  this matter  in ***
 *** whole  or in part  is forbidden  without the  express  written ***
 *** consent of Eastman Kodak Company.                              ***
 ***                                                                ***
 *** COPYRIGHT (c) Eastman Kodak Company, 1993-2000                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"
#include "fut.h"
#include "kcmptlib.h"
#include "attrcipg.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#define sppow( x, y ) ( pow ( x , y) )
#define splog( x ) ( log ( x ) )
#define spexp( x ) ( exp ( x ) )

#define POW(x, power)   ( ((x) > 0.0) ? spexp ((power) * splog ((x))) \
			: sppow ((x), (power)) )

#if defined (KPMAC68K)
#include <Gestalt.h>

/************************************************************/
/*	Return gestalt status for whether or not FPU is present */
/************************************************************/
static KpInt32_t SpIsFPUpresent ()
{
KpInt32_t	SpFPUType = SpFoundFPU;
OSErr		myErr;
KpInt32_t	myFPUType;
	
	myErr = Gestalt(gestaltFPUType, &myFPUType);
	if (myFPUType == gestaltNoFPU) {
		SpFPUType = SpNoFPU;
	}

	return (SpFPUType);
}
#endif


/*------------------------------------------------------------------------*/
static double HCIE (double t)
{
	const double CIE_exp = 0.333333333333;
	
	/* CIE visual-response function from pre-normalized XYZ */

	if (t <= 0.008856)
		return 9.033 * t;
	else
		return (1.16 * POW (t, CIE_exp)) - 0.16;
}
/* ---------------------------------------------------------------------- */
double MakeGamma (double g, double x)
{
	return POW (x, g);
}

/* ---------------------------------------------------------------------- */
static void BuBvBL2XYZ (
				KpUInt8_t	Bu,
				KpUInt8_t	Bv,
				KpUInt8_t	BL,
				double		FAR *XPtr,
				double		FAR *YPtr,
				double		FAR *ZPtr)
{
	double	u, v, L;
	double	X, Y, Z;

	u = 0.41 * (double)Bu / 255.0 + .07;
	v = 0.42 * (double)Bv / 255.0 + .165;
	L = (double)BL / 2.55;

/* convert u',v', L* to X, Y, Z */
	Y = L / 100.0;
	if (Y <= 0.08)
		Y = Y / 9.033;
	else {
		Y = (Y + 0.16) / 1.16;
		Y = Y * Y * Y;
	}

	Y *= 100;		/* official normalization */
	X = (9 * u * Y) / (4 * v);
	Z = ((3 - 0.75 * u) / v - 5) * Y;

	*XPtr = X;
	*YPtr = Y;
	*ZPtr = Z;
}
/* ---------------------------------------------------------------------- */
static void NormXYZtoLab (
				double	X,
				double	Y,
				double	Z,
				double	FAR *L,
				double	FAR *a,
				double	FAR *b)
{
/* Converts normalized XYZ to Lab */
	double	fx, fy, fz;

	fx = HCIE (X);
	fy = HCIE (Y);
	fz = HCIE (Z);
	*a = 431.0 * (fx-fy);
	*b = 172.4 * (fy-fz);
	*L = 100 * fy;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function will convert byte encoded XYZ into byte encoded Lab
 *
 * INPUT VARIABLES
 *      KpInt32_t numPels       - number of pels to convert 
 *				  (# of XYZ triplets)
 *      KpUInt8_t *bXYZ		- pointer to byte encoded XYZ data 
 *				  (interleave format)
 *      KpUInt8_t *bLab		- pointer to bytes encoded Lab buffer 
 *				  (interleave format)
 *
 * OUTPUT VARIABLES
 *      KpUInt8_t       *bLab   - fills buffer with byte encoded Lab data
 *
 *
 * AUTHOR
 * stanp
 *
 *------------------------------------------------------------------*/
static void BXYZ2BLab(
		KpInt32_t numPels, KpUInt8_t  *bXYZ, KpUInt8_t *bLab)
{
        int i;
        double  X,Y,Z,L,a,b;
        KpUInt8_t       *input, *output;
 
        input = bXYZ;
        output = bLab;
 
        for (i = 0; i < numPels; i++)
        {
                /* convert byte XYZ to doubles */
                X = (double)*input++;
                Y = (double)*input++;
                Z = (double)*input++;
 
                X = X/255.0;
                Y = Y/255.0;
                Z = Z/255.0;
                /* convert XYZ to Lab */
                NormXYZtoLab(X, Y, Z, &L, &a, &b);
 
                /* convert double Lab to byte Lab */
 
                *output++ = (KpUInt8_t)(L * 2.55 + 0.5);
                *output++ = (KpUInt8_t)(a + 128.0 + 0.5);
                *output++ = (KpUInt8_t)(b + 128.0 + 0.5);
        }
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function will convert UShort12 encoded XYZ into UShort12 encoded Lab
 *
 * INPUT VARIABLES
 *      KpInt32_t  numPels	- number of pels to convert 
 *				  (# of XYZ triplets)
 *      KpUInt16_t *pXYZ	- pointer to UShort12 encoded XYZ data 
 *				  (interleave format)
 *      KpUInt16_t *pLab	- pointer to UShort12 encoded Lab buffer 
 *				  (interleave format)
 *
 * OUTPUT VARIABLES
 *      KpUInt16_t      *pLab   - fills buffer with byte encoded Lab data
 *
 *
 * AUTHOR
 * stanp
 *
 *------------------------------------------------------------------*/
static void US12XYZ2US12Lab (
				KpInt32_t	numPels, 
			    KpUInt16_t	*pXYZ, 
			    KpUInt16_t	*pLab)
{
	int i;
	double  X,Y,Z,L,a,b;
	KpUInt16_t      *input, *output;
 
	input = pXYZ;
	output = pLab;
 
	for (i = 0; i < numPels; i++)
	{
		/* convert UShort12 XYZ to doubles */
		X = (double)*input++;
		Y = (double)*input++;
		Z = (double)*input++;
 
		X = X/4080.0;
		Y = Y/4080.0;
		Z = Z/4080.0;
 
		/* convert XYZ to Lab */
		NormXYZtoLab(X, Y, Z, &L, &a, &b);
  
		/* convert double Lab to UShort12 Lab */
 
		*output++ = (KpUInt16_t)(L * 40.80 + 0.5);
		*output++ = (KpUInt16_t)((a + 128.0) * 16.0 + 0.5);
		*output++ = (KpUInt16_t)((b + 128.0) * 16.0 + 0.5);
	}
}

static const double  aStarFactor = (500.0 / 1.16);
static const double  bStarFactor = (200.0 / 1.16);
static const double y2lStarSlope     = 9.03296296296296;
static const double     minXYZ  = -0.5;
static const double     maxXYZ  = 200.0;

static double KdsColorLstar2NormY(double val)
{
        double temp;
 
        if (val <= 0.08) {
                return(val / y2lStarSlope);
        } else {
                temp = (val + 0.16) / 1.16;
                return(temp * temp * temp);
        }
}

static double ClipXYZ(double xyz)
{
        if (xyz > maxXYZ) {
                xyz = maxXYZ;
        } else if (xyz < minXYZ) {
                xyz = minXYZ;
        }
        return(xyz);
}

/* ---------------------------------------------------------------------- */
static void SuSvSL2XYZ (
				KpUInt16_t	Bu,
				KpUInt16_t	Bv,
				KpUInt16_t	BL,
				double		FAR *XPtr,
				double		FAR *YPtr,
				double		FAR *ZPtr)
{
	double	u, v, L;
	double	X, Y, Z;

	u = 0.41 * (double)Bu / 4080.0 + .07;
	v = 0.42 * (double)Bv / 4080.0 + .165;
	L = (double)BL / 40.80;

/* convert u',v', L* to X, Y, Z */
	Y = L / 100.0;
	if (Y <= 0.08)
		Y = Y / 9.033;
	else {
		Y = (Y + 0.16) / 1.16;
		Y = Y * Y * Y;
	}

	Y *= 100;		/* official normalization */
	X = (9 * u * Y) / (4 * v);
	Z = ((3 - 0.75 * u) / v - 5) * Y;

	*XPtr = X;
	*YPtr = Y;
	*ZPtr = Z;
}

/* ---------------------------------------------------------------------- */

static void Lab2NormXYZ(
			double  L,
			double  a,
			double  b,
			double  FAR *X,
			double  FAR *Y,
			double  FAR *Z)
{
double  fx, fy, fz;
 
	fy = L / 100.0;
	fx = a / aStarFactor + fy;
	fz = b / (-bStarFactor) + fy;
 
	*X = ClipXYZ(KdsColorLstar2NormY(fx));
	*Y = ClipXYZ(KdsColorLstar2NormY(fy));
	*Z = ClipXYZ(KdsColorLstar2NormY(fz));
}


/* ---------------------------------------------------------------------- */
static void ComputeLab (
				double	A[6],
				double	R,
				double	G,
				double	B,
				double	FAR *L,
				double	FAR *a,
				double	FAR *b)
{
/*
 * computes LAB from the parameter matrix
 * The matrix is normalized so that the resulting XYZ's
 * are each normalized to 1
 */
	
	double	ColorMatrix [3] [3], RGB [3], XYZ [3];
	int		i,j;
	
/* reconstruct the matrix */
	ColorMatrix [0] [0] = 1.0 - A [0] - A [1];	/* X */
	ColorMatrix [1] [0] = A [0];
	ColorMatrix [2] [0] = A [1];
	ColorMatrix [0] [1] = A [2];				/* Y */
	ColorMatrix [1] [1] = 1.0 - A [2] - A [3];
	ColorMatrix [2] [1] = A [3];
	ColorMatrix [0] [2] = A [4];				/* Z */
	ColorMatrix [1] [2] = A [5];
	ColorMatrix [2] [2] = 1.0 - A [4] - A [5];
	
/* Compute */
	RGB [0] = R;		RGB [1] = G;		RGB [2] = B;

/* Compute normalized XYZ */
	for (i = 0; i < 3; i++) {	/* loop over XYZ */
		XYZ [i] = 0;
		for (j = 0; j < 3; j++)
			XYZ [i] += RGB [j] * ColorMatrix [j] [i];
	}

/* Compute Lab */
	NormXYZtoLab (XYZ [0], XYZ [1], XYZ [2], L, a, b); 
}	

/* ---------------------------------------------------------------------- */
SpStatus_t SolveMat (
				double	FAR *FAR *mat,
				int		DimR,
				int		DimC)
{
	int		i, r, c;
	double	pivot;
	double	factor;
	const	double limit = 1.e-6;

	for (i = 0; i < DimR; i++) {
		pivot = mat [i] [i];
		/* check for singularity */
		if ((pivot > -limit) && (pivot < limit))
			return SpStatOutOfRange;

	/* set the pivot point to 1.0 */
		for (c = 0; c < DimC; c++)
			mat [i] [c] /= pivot;

	/* set the zeros in the columns of the non-pivot rows */
		for (r = 0; r < DimR; r++) {
			if (r == i)
				continue;

			factor = mat [r] [i];
			for (c = 0; c < DimC; c++)
				mat [r] [c] -= factor * mat [i] [c];
		}
	}
	return SpStatSuccess;
}


/* ------------------------------------------------------------------------ */
static SpStatus_t ComputeShaper (
				PTRefNum_t	pt,
				double		FAR *shaper [3],
				double		wht [3])
{
	KpUInt16_t	*Pels, *p, avg_u, avg_v;
	int			i,j;
	SpStatus_t	spErr = SpStatSuccess;
	int			center = SHAPERCURVESIZE / 2;
	int			ml = center, mh = center;
	int32		sum [2], N;
	KpUInt16_t  avgVal;
/* Allocate memory */
	Pels = SpMalloc (3*SHAPERCURVESIZE * sizeof(*Pels));
	if (NULL == Pels)
		return SpStatMemory;
	
/* Compute ramp */
	for (i = 0; i < 3; i++)			/* Loop over colors */
		for (j = 0, p = Pels+i; j < SHAPERCURVESIZE; j++, p += 3)
			*p = (KpUInt16_t)(j*16);

/* Transform colors */
	spErr = Transform12BPels (pt, Pels, SHAPERCURVESIZE);
	if (spErr != SpStatSuccess) {
		SpFree (Pels);
		return spErr;
	}
	
/*
 * Find limits of monotonic region
 * (Note - because of truncation, constancy does not mean clipping)
 */

 	avgVal = (KpUInt16_t)(Pels[2] + Pels[(SHAPERCURVESIZE-1)*3+2]) / 2;
 
	for (i = SHAPERCURVESIZE-1; 
	     ((i > 0) && (*(Pels+2+3*i) > avgVal)); i--) {
		center = i;
	}

/* go down until finding a reversal */
	for (i = center-1, p = Pels+2+3*i;
			(i >=0) && (*p <= *(p+3));
					i--, p -= 3)
		ml = i;			/* no reversal yet */

/* head back up, until function starts rising */
	for (i = ml+1,p = Pels+2+3*i;
			(i < center) && (*p == *(p-3));
					i++, p += 3)
		ml = i;			/* is still constant */
		
/* go up until finding a reversal */
	for (i = center, p = Pels+2+3*i;
			i < SHAPERCURVESIZE && *p >= *(p-3);
					i++, p += 3)
		mh = i;			/* is still going up */

/* head back down, until function starts falling */
	for (i = mh-1, p = Pels+2+3*i;
			i >= center && *p == *(p+3);
					i--, p -= 3)
		mh = i;			/* is still constant */
	
/* Compute effective white value - avg u,v and max L */
	sum[0] = 0;
	sum[1] = 0;
	for (i = center, p = Pels+3*i; i <= mh; i++, p += 3) {
		sum [0] += *p;		/* sum u */
		sum [1] += *(p+1);	/* sum v */
	}
	N = mh-center+1;
			
	avg_u = (KpUInt16_t) ((sum [0] + N/2) / N);
	avg_v = (KpUInt16_t) ((sum [1] + N/2) / N);
	SuSvSL2XYZ (avg_u, avg_v, Pels [2 + mh*3], &wht [0], 
		    &wht [1], &wht [2]);

	for (i = 0; i < 3; i++)	{		/* better safe than sorry */
		if (wht [i] <= 0) {
			SpFree (Pels);		/* clean up */
			return SpStatOutOfRange;	/* can't go on */
		}
	}
	
/* Within monotonic region, shaper = XYZ value/white */
/* loop over monotonic region */
	for (i = ml, p = Pels+3*ml;
			i <= mh;
					i++, p += 3) {
		SuSvSL2XYZ (p [0], p [1], p [2],
			    &shaper [0][i], &shaper [1][i], &shaper [2][i]);

		for (j = 0; j < 3; j++)		/* loop over color */
			shaper [j] [i] /= wht [j];	/* normalize */
	}
	
/* Extend the edges */
	for (i = 0; i < ml; i++)
		for (j = 0; j < 3; j++)
			/* extend to left */
			shaper [j] [i] = shaper [j] [ml];
	for (i = mh+1; i < SHAPERCURVESIZE; i++)
		for (j = 0; j < 3; j++)
			/* extend to right */
			shaper [j] [i] = shaper [j] [mh];
			
/* Clean up */
	SpFree (Pels);
	return spErr;
}

/* ---------------------------------------------------------------------- */
static double ComputeLabError (
				double	A [6],
				double	FAR *shaped_rgb [3],
				double	FAR *Lab_grid [3],
				int		totalGrid)
{
/* Compute value and error */
	
	int		i, j;
	double	Lab [3], delta, error;
	
/* Compute error */
	error = 0;
	for (i = 0; i < totalGrid; i++)	{	/* Loop over data points */

	/* Compute the Lab value */
		ComputeLab (A, shaped_rgb [0][i], shaped_rgb [1][i],
			    shaped_rgb [2][i], &Lab [0], &Lab [1], &Lab [2]);
		for (j = 0; j < 3 ; j++)	{
			delta = Lab_grid [j] [i] - Lab [j];
			error += delta*delta;
		}
	}		/* Next data value */

/* Average error */
	error /= (double)(3 * totalGrid);
	return error;
}

/* ---------------------------------------------------------------------- */
/*   Use Levenberg-Marquard method for determining a new search direction */
static KpBool_t NewSearchDirection (
				double	A [6],
				double	FAR *shaped_rgb [3],
				double	FAR *Lab_grid [3],
				int		Npts,
				double	deltaA [6])
{
	double	*JJ[6], jjBuf[42], J[6][3], maxDiag, diagLimit;
	int		ia, ja, kPt, i;
	double	Lab[3], newLab[3];
	double	newA[6], delta[6];
	
	const double delLimit = 0.00001;
	const double LM = 0.025;
	
/* Assign pointer array */
	for (ia = 0; ia < 6; ia++)
		JJ [ia] = jjBuf + ia*7;

	for (i =0; i < 42; i++)
		jjBuf [i] = 0;
	
/* Set up delta values for dirivative */
	for (ia = 0; ia < 6; ia++) {
		delta [ia] = .001 * A [ia];
		if (delta [ia] < 0)
			delta [ia] = -delta [ia];
		if (delta [ia] < delLimit)
			delta [ia] = delLimit;
	}
		
/* Compute the Jacobian arrays */
	for (kPt = 0; kPt < Npts; kPt++) {	/* Loop over points */
		ComputeLab (A, shaped_rgb [0] [kPt], shaped_rgb [1] [kPt],
			shaped_rgb [2] [kPt], &Lab [0], &Lab [1], &Lab [2]);

	/* Compute the dirivatives at this point */
		for (ia = 0; ia < 6; ia++)	{
			/* Set up incremental A */
			for (ja = 0; ja < 6; ja++)
				newA [ja] = A [ja];

			newA [ia] += delta [ia];
			ComputeLab (newA,
					shaped_rgb [0] [kPt],
					shaped_rgb [1] [kPt],
					shaped_rgb [2] [kPt],
					&newLab [0], &newLab [1], 
					&newLab [2]);

			for (i = 0; i < 3; i++) {
			/* dirivative at this point */
				J [ia] [i] = (newLab [i]-Lab [i]) / delta [ia];
			}
		}

	/* Add contribution to the J2 array */
		for (ia = 0; ia < 6; ia++) {	/* Loop over rows of JJ */

		/* dirivative squared */
			for (ja = 0; ja < 6; ja++)
				for (i = 0; i < 3; i++)
					JJ [ia][ja] += J [ia][i] * J [ja][i];

		/* error in function */
			for (i = 0; i < 3; i++)
				JJ [ia][6] += J [ia][i] * 
					(Lab_grid [i] [kPt]-Lab [i]);

		}		/* Next row of J2 array */	

	}		/* next grid point */
	
/* Normalize */
	for (ia = 0; ia < 6; ia++)
		for (ja = 0; ja < 7; ja++)
			JJ [ia] [ja] /= (double)(3*Npts);
			
/* Adjust diagonal value */
	maxDiag = 0;
	for (ia = 0; ia < 6; ia++)
		if (JJ [ia] [ia]>maxDiag)
			maxDiag = JJ [ia] [ia];

	if (maxDiag <= 1.e-6)
		return KPFALSE;

	diagLimit = maxDiag * 0.01;
	for (ia = 0; ia < 6; ia++)
		JJ [ia] [ia] += LM * (JJ [ia] [ia] > diagLimit
												? JJ [ia] [ia] : diagLimit);
	
/* invert J2 to determine new search direction */
	if (SolveMat (JJ, 6, 7) != SpStatSuccess)
		return KPFALSE;

	for (ia = 0; ia < 6; ia++)
		deltaA [ia] = (1+LM)*JJ [ia] [6];
	
	return KPTRUE;
}

/* ---------------------------------------------------------------------- */
static SpStatus_t SearchLab (
				double	A [6],
				double	FAR *shaped_rgb [3],
				double	FAR *Lab_grid [3],
				int		totalGrid)
{
	int			i, istep;
	double		labError, labError0, newError;
	KpBool_t	notDone = KPTRUE, goingDown;
	double		newA [6], deltaA [6], scale, lastScale = 1.0;

	const int	NSteps = 5;
	const double tolerance = 1.0;

/* Compute initial error */
	labError0 = ComputeLabError (A, shaped_rgb, Lab_grid, totalGrid);
	if (labError0 < tolerance)
		return SpStatSuccess;
	
	labError = labError0;

/* keep looking until error stops going down */
	do {
		/* Get a new search direction */
		notDone = NewSearchDirection (A, shaped_rgb, Lab_grid,
											totalGrid, deltaA);
		if (notDone) {
		/*
		 * Head off in the new direction taking smaller steps,
		 * until error goes down
		 */

		/*keep trying, with smaller steps*/
			for (istep = 1, goingDown = KPFALSE, scale=1.0;
				(istep <= NSteps) && !goingDown;
				istep++, scale *= 0.5) {
				for (i = 0; i < 6; i++)
					/* try this one */
					newA [i] = A [i] + scale * deltaA [i];
				newError = ComputeLabError (newA, shaped_rgb,
							Lab_grid, totalGrid);
				goingDown = (KpBool_t) (labError-newError > tolerance);
				if (goingDown) {	/* it got better */
					labError = newError;
					lastScale = scale;
				}
			}		/* end of steps along search path */

		/* it did not get much better */
			if (labError0 - labError < tolerance)
				notDone = KPFALSE;	/* no more to do */
			else {
				labError0 = labError;
				for (i = 0; i < 6; i++)
					/* update A */
					A [i] += lastScale * deltaA [i];

				notDone = (KpBool_t) (labError0 > tolerance);
			}		/* End of linear search */
		}
	} while (notDone);		/*try another direction*/

	return SpStatSuccess;
}

/* ------------------------------------------------------------------------ */
static SpStatus_t ComputeMatrix (
				PTRefNum_t	pt,
				double		FAR *shaper [3],
				double		wht [3],
				double		ColorMatrix [3] [3])
{
	int			ml, mh;
	KpUInt8_t	*rgbgrid, *rgbValue, *rcsValue, rv,gv, bv;
	double		*shaped_rgb[3];
	double		*xyz_grid[3];
	const int	gridSize = 5, totalGrid = gridSize * 
						gridSize * gridSize;
	int		i, j, k;
	KpBool_t	memOK;
	SpStatus_t	spErr;
	double		cBuf[18], *correlation[3], norm;
	double		A[6];

	xyz_grid[0] = 0;
	xyz_grid[1] = 0;
	xyz_grid[2] = 0;
	shaped_rgb[0] = 0;
	shaped_rgb[1] = 0;
	shaped_rgb[2] = 0;

	correlation [0] = cBuf;
	correlation [1] = cBuf+6;
	correlation [2] = cBuf+12;
	
/* find limits on shaper for monotonic behavior in Y  (ml & mh)  */
	for (ml = 0;
		(ml < SHAPERCURVESIZE-1) && 
		(shaper [2] [ml] == shaper [2] [ml+1]);
		ml++)
		;
	for (mh = SHAPERCURVESIZE-1;
		(mh > 1) && (shaper [2] [mh] == shaper [2] [mh-1]);
		mh--)
		;
	
/* Allocate memory */
	rgbgrid = SpMalloc (3 * totalGrid * sizeof (*rgbgrid));
	if (NULL == rgbgrid)
		return SpStatMemory;

	for (i = 0, memOK = KPTRUE;
			(i < 3) && memOK;
					i++) {		
		shaped_rgb [i] = SpMalloc (totalGrid * sizeof (double));
		memOK &= (NULL != shaped_rgb[i]);
		if (memOK) {
			xyz_grid [i] = SpMalloc (totalGrid * sizeof (double));
			memOK &= (NULL != xyz_grid [i]);
		}
	}	/* next color */

/* check for memory failure */
	if (!memOK) {
		SpFree (rgbgrid);
		for (i = 0; i < 3; i++) {
			SpFree (shaped_rgb [i]);
			SpFree (xyz_grid [i]);
		}
		return SpStatMemory;
	}
	
/* Fill rgb grid */
	rgbValue = rgbgrid;
	for (i = 0; i < gridSize; i++) {						/* r values */
		rv = (KpUInt8_t) ( (ml * gridSize + i * (mh-ml)) / gridSize );	
		for (j = 0; j < gridSize; j++)	{	/* g values */
			gv = (KpUInt8_t) ( (ml * gridSize + j * (mh-ml)) / 
						gridSize );	
			for (k = 0; k < gridSize; k++) {	/* b values */
				bv = (KpUInt8_t) ( (ml * gridSize + k * 
						(mh-ml)) / gridSize );
				*rgbValue++ = rv;
				*rgbValue++ = gv;
				*rgbValue++ = bv;
			}
		}
	}
	
/* Compute shaped values */
	rgbValue = rgbgrid;
	for (i = 0; i < totalGrid; i++) {	/* loop over gridpoints */
		for (j = 0; j < 3; j++) {	/* loop over colors */

		/* look up color in the shaper */
			shaped_rgb [j] [i] = shaper [j] [*rgbValue++];
		}
	}

/* Transform the grid */
	spErr = TransformPels (pt, rgbgrid, totalGrid);
	if (spErr != SpStatSuccess) {
		SpFree (rgbgrid);
		for (i = 0; i < 3; i++)	{
			SpFree (shaped_rgb [i]);
			SpFree (xyz_grid [i]);
		}
		return spErr;
	}
	
/*--------------------------------*/
/* Compute transformed XYZ values */
/*--------------------------------*/
	for (i = 0, rcsValue = rgbgrid;
		i < totalGrid;
		i++, rcsValue += 3) {	/* loop over gridpoints */
		BuBvBL2XYZ (rcsValue [0], rcsValue [1], rcsValue [2],
				&xyz_grid [0] [i], &xyz_grid [1] [i], 
				&xyz_grid [2] [i]);
	}

/* Release initial grid values */
	SpFree (rgbgrid);
	
/* Compute initial estimate */
	
/* Correlation matrix's - pack into 3x6 array */
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)	{
			correlation [i] [j] = 0;
			correlation [i] [j+3] = 0;
			/* loop over grid points */
			for (k = 0; k < totalGrid; k++)	{
				correlation [i] [j] += shaped_rgb [i] [k] * 
							shaped_rgb [j] [k];
				correlation [i] [j+3] += shaped_rgb [i] [k] * 
							xyz_grid [j] [k];
			}
		}
	}

/* solve RGB_RGB * ColorMatrix = RGB_XYZ */
	spErr = SolveMat (correlation, 3,6);		
	if (spErr != SpStatSuccess) {
		for (i = 0; i < 3; i++) {
			SpFree (shaped_rgb [i]);
			SpFree (xyz_grid [i]);
		}
		return spErr;
	}	

	for (i = 0; i < 3; i++)				/* loop over rgb */
		for (j = 0; j < 3; j++)			/* loop over XYZ */
			ColorMatrix [i] [j] = correlation [i] [j+3];

/* Normalize matrix */
	for (i = 0; i < 3; i++)	{		/* loop over XYZ */
		norm = 0;
		for (j = 0; j < 3; j++)
			norm += ColorMatrix [j] [i];	/* sum over RGB */
		for (j = 0; j < 3; j++)			/* loop over RGB */
			/* normalize to unit sum */
			ColorMatrix [j] [i] /= norm;	
	}

/*-----------------------*/
/* Perform search in Lab */
/*-----------------------*/
	
/* Convert result vestor to Lab */
	for (k = 0; k < totalGrid; k++)		/* loop over grid points */
		NormXYZtoLab (xyz_grid [0] [k] / wht [0],
				xyz_grid [1] [k] / wht [1],
				xyz_grid [2] [k] / wht [2],
				&xyz_grid[0][k], &xyz_grid[1][k], 
				&xyz_grid[2][k]);
	
/* Encode normalized matrix	 */
	A [0] = ColorMatrix [1] [0];
	A [1] = ColorMatrix [2] [0];
	A [2] = ColorMatrix [0] [1];
	A [3] = ColorMatrix [2] [1];
	A [4] = ColorMatrix [0] [2];
	A [5] = ColorMatrix [1] [2];
	
/* Call non-linear search routine */
	spErr = SearchLab (A, shaped_rgb, xyz_grid, totalGrid);
	
	if (spErr == SpStatSuccess) {		/* it turned out OK */
		/* reconstruct and normalize the matrix */
		ColorMatrix [0] [0] = wht [0] * (1.0 - A [0] - A [1]);	/* X */
		ColorMatrix [1] [0] = wht [0] * A [0];
		ColorMatrix [2] [0] = wht [0] * A [1];
		ColorMatrix [0] [1] = wht [1] * A [2];					/* Y */
		ColorMatrix [1] [1] = wht [1] * (1.0 - A [2] - A [3]);
		ColorMatrix [2] [1] = wht [1] * A [3];
		ColorMatrix [0] [2] = wht [2] * A [4];					/* Z */
		ColorMatrix [1] [2] = wht [2] * A [5];
		ColorMatrix [2] [2] = wht [2] * (1.0 - A [4] - A [5]);
	}
		
/* Clean up */
	for (i = 0; i < 3; i++)	{
		SpFree (shaped_rgb [i]);
		SpFree (xyz_grid [i]);
	}
	
	return spErr;
}
/*------------------------------------------------------------------------*/
static SpStatus_t Transform12BitPelsEx (
				SpXform_t	Xform,
				KpUInt16_t	FAR *Pels,
				int			nPels)
{
/*
 * Transform the pel array using the xform
 * The array is assumes to be in plane-sequential order
 * RGB input, Lab or XYZ output
 */

	SpPixelLayout_t	layout;
	SpStatus_t		status;
	int			i;

	layout.SampleType = SpSampleType_UShort12;
	layout.NumCols = nPels;
	layout.NumRows = 1;
	layout.OffsetColumn = 3 * sizeof(*Pels);
	layout.OffsetRow = nPels * 3 * sizeof (*Pels);
	layout.NumChannels = 3;

	for (i = 0; i < 3; i++) {
		layout.BaseAddrs [i] = (void FAR *) (Pels+i);
	}

	status = SpEvaluate(Xform, &layout, &layout, NULL, NULL);
	return status;
}

/* ------------------------------------------------------------------------ */
static SpStatus_t PostNormalize (
		double		FAR *shaper [3],
		double		ColorMatrix [3] [3])
{
	KpInt16_t i, j;
	double maxShaper, YWhite = 0; 


	/* for each of the 3 shapers, find the largest value */
	/* if it is <= 1.0 - everything is ok */
	/* if not, divide shaper by this value, and */
	/* multiply the corresponding matrix row by the same value */
	/* i = r,g, or b shapers */
	for (i=0; i<3; i++) {
		maxShaper = 0;
		/* find the largest value in the shaper */
		for (j=0; j<256; j++) {
			if (maxShaper < shaper[i][j])
				maxShaper = shaper[i][j];
		}

		/* all shaper values must be less than 1.0 to fit in a 
		   16-bit integer ! */
	
		/* divide each shaper element by the largest value to 
		   normalize */
		for (j=0; j<256; j++) {
			shaper[i][j] /= maxShaper;
			if (shaper[i][j] >= 1.0) {
				/* this must be less than 1 !!! */
				shaper[i][j] = 0.99999999; 
			}
		}

		/* multiply the corresponding matrix row by this value */
		for (j=0; j<3; j++)	{
			ColorMatrix[i][j] *= maxShaper;
		}
	}
	
	/* normalize the matrix so that the Y column sums to 1.0 */
	for (i = 0; i < 3; i++) {
		YWhite += ColorMatrix[i][1];
	}

	YWhite = 1.0/YWhite;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			ColorMatrix[i][j] *= YWhite;
		}
	}

	return SpStatSuccess;
}

/* ---------------------------------------------------------------------- */
static SpStatus_t TransformPelsEx (
				SpXform_t	Xform,
				KpUInt8_t	FAR *Pels,
				int			nPels)
{
/*
 * Transform the pel array using the xform
 * The array is assumes to be in plane-sequential order
 * RGB input, Lab or XYZ output
 */

	SpPixelLayout_t	layout;
	SpStatus_t		status;
	int			i;

	layout.SampleType = SpSampleType_UByte;
	layout.NumCols = nPels;
	layout.NumRows = 1;
	layout.OffsetColumn = 3 * sizeof(*Pels);
	layout.OffsetRow = nPels * 3 * sizeof (*Pels);
	layout.NumChannels = 3;

	for (i = 0; i < 3; i++) {
		layout.BaseAddrs [i] = (void FAR *) (Pels+i);
	}

	status = SpEvaluate(Xform, &layout, &layout, NULL, NULL);
	return status;
}
/*-----------------------------------------------------------------------*/
static SpStatus_t ComputeMatrixEx (
				SpXform_t	xform,
				double		FAR *shaper [3],
				double		wht [3],
				double		ColorMatrix [3] [3])
{
	int			ml, mh;
	KpUInt8_t	*rgbgrid, *rgbValue, *rcsValue, rv,gv, bv;
	double		*shaped_rgb[3];
	double		*xyz_grid[3];
	const int	gridSize = 5, totalGrid = gridSize * 
						gridSize * gridSize;
	int			i, j, k;
	KpBool_t	memOK;
	SpStatus_t	spErr;
	SpXformDesc_t	Desc;
	double		cBuf[18], *correlation[3], norm;
	double		A[6];
	double		temp_L, temp_a, temp_b;

	xyz_grid[0] = 0;
	xyz_grid[1] = 0;
	xyz_grid[2] = 0;
	shaped_rgb[0] = 0;
	shaped_rgb[1] = 0;
	shaped_rgb[2] = 0;

	correlation [0] = cBuf;
	correlation [1] = cBuf+6;
	correlation [2] = cBuf+12;
	
/* find limits on shaper for monotonic behavior in Y */
	for (ml = 0;
		(ml < SHAPERCURVESIZE-1) && 
		(shaper [2] [ml] == shaper [2] [ml+1]);
		ml++)
		;
	for (mh = SHAPERCURVESIZE-1;
		(mh > 1) && (shaper [2] [mh] == shaper [2] [mh-1]);
		mh--)
		;
	
/* Allocate memory */
	rgbgrid = SpMalloc (3 * totalGrid * sizeof (*rgbgrid));

	if (NULL == rgbgrid)
		return SpStatMemory;

	for (i = 0, memOK = KPTRUE;
			(i < 3) && memOK;
					i++) {		
		shaped_rgb [i] = SpMalloc (totalGrid * sizeof (double));
		memOK &= (NULL != shaped_rgb[i]);
		if (memOK) {
			xyz_grid [i] = SpMalloc (totalGrid * sizeof (double));
			memOK &= (NULL != xyz_grid [i]);
		}
	}	/* next color */

/* check for memory failure */
	if (!memOK) {
		SpFree (rgbgrid);
		for (i = 0; i < 3; i++) {
			SpFree (shaped_rgb [i]);
			SpFree (xyz_grid [i]);
		}

		return SpStatMemory;
	}
	
/* Fill rgb grid */
	rgbValue = rgbgrid;
	for (i = 0; i < gridSize; i++) {						/* r values */
		rv = (KpUInt8_t) ( (ml * gridSize + i * (mh-ml)) / gridSize );	
		for (j = 0; j < gridSize; j++)	{					/* g values */
			gv = (KpUInt8_t) ( (ml * gridSize + j * (mh-ml)) / 
						gridSize );	
			for (k = 0; k < gridSize; k++) {	/* b values */
				bv = (KpUInt8_t) ( (ml * gridSize + 
					k * (mh-ml)) / gridSize );
				*rgbValue++ = rv;
				*rgbValue++ = gv;
				*rgbValue++ = bv;
			}
		}
	}
	
/* Compute shaped values */
	rgbValue = rgbgrid;
	for (i = 0; i < totalGrid; i++) {	/* loop over gridpoints */
		for (j = 0; j < 3; j++) {		/* loop over colors */

		/* look up color in the shaper */
			shaped_rgb [j] [i] = shaper [j] [*rgbValue++];
		}
	}

/* Transform the grid */
	spErr = TransformPelsEx (xform, rgbgrid, totalGrid);
	if (spErr != SpStatSuccess) {
		SpFree (rgbgrid);
		for (i = 0; i < 3; i++)	{
			SpFree (shaped_rgb [i]);
			SpFree (xyz_grid [i]);
		}
		return spErr;
	}
	
	/* if output color space is XYZ, convert pels to Lab */
	spErr = SpXformGetDesc( xform, &Desc);
	if (spErr != SpStatSuccess)
	{
			SpFree (rgbgrid);
		for (i = 0; i < 3; i++)	{
			SpFree (shaped_rgb [i]);
			SpFree (xyz_grid [i]);
		}
		return spErr;
	}

	if (Desc.SpaceOut == SpSpaceXYZ)
	{
		BXYZ2BLab(totalGrid, rgbgrid, rgbgrid);
	}
	
/*--------------------------------*/
/* Compute transformed XYZ values */
/*--------------------------------*/
	for (i = 0, rcsValue = rgbgrid;i < totalGrid;i++, rcsValue += 3) 
	{	/* loop over gridpoints */
		temp_L = (double) (rcsValue[0]) / 2.55;
		temp_a = (double) (rcsValue[1]) - 128.0;
		temp_b = (double) (rcsValue[2]) - 128.0;
		Lab2NormXYZ(temp_L, temp_a, temp_b, 
			&xyz_grid [0] [i], &xyz_grid [1] [i], 
			&xyz_grid [2] [i]);
	}

/* Release initial grid values */
	SpFree (rgbgrid);
	
/* Compute initial estimate */
	
/* Correlation matrix's - pack into 3x6 array */
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)	{
			correlation [i] [j] = 0;
			correlation [i] [j+3] = 0;
			/* loop over grid points */
			for (k = 0; k < totalGrid; k++)	{	
				correlation [i] [j] += shaped_rgb [i] [k] * 
							shaped_rgb [j] [k];
				correlation [i] [j+3] += shaped_rgb [i] [k] * 
							xyz_grid [j] [k];
			}
		}
	}

/* solve RGB_RGB * ColorMatrix = RGB_XYZ */
	spErr = SolveMat (correlation, 3,6);		
	if (spErr != SpStatSuccess) {
		for (i = 0; i < 3; i++) {
			SpFree (shaped_rgb [i]);
			SpFree (xyz_grid [i]);
		}
		return spErr;
	}	

	for (i = 0; i < 3; i++)				/* loop over rgb */
		for (j = 0; j < 3; j++)			/* loop over XYZ */
			ColorMatrix [i] [j] = correlation [i] [j+3];

/* Normalize matrix */
	for (i = 0; i < 3; i++)	{		/* loop over XYZ */
		norm = 0;
		for (j = 0; j < 3; j++)
			norm += ColorMatrix [j] [i];	/* sum over RGB */
		for (j = 0; j < 3; j++)			/* loop over RGB */
			/* normalize to unit sum */
			ColorMatrix [j] [i] /= norm;	
	}

/*-----------------------*/
/* Perform search in Lab */
/*-----------------------*/
	
/* Convert result vestor to Lab */
	for (k = 0; k < totalGrid; k++)		/* loop over grid points */
		NormXYZtoLab (xyz_grid [0] [k] / wht [0],
				xyz_grid [1] [k] / wht [1],
				xyz_grid [2] [k] / wht [2],
				&xyz_grid[0][k], &xyz_grid[1][k], 
				&xyz_grid[2][k]);
	
/* Encode normalized matrix	 */
	A [0] = ColorMatrix [1] [0];
	A [1] = ColorMatrix [2] [0];
	A [2] = ColorMatrix [0] [1];
	A [3] = ColorMatrix [2] [1];
	A [4] = ColorMatrix [0] [2];
	A [5] = ColorMatrix [1] [2];
	
/* Call non-linear search routine */
	spErr = SearchLab (A, shaped_rgb, xyz_grid, totalGrid);
	
	if (spErr == SpStatSuccess) {		/* it turned out OK */
		/* reconstruct and normalize the matrix */
		ColorMatrix [0] [0] = wht [0] * (1.0 - A [0] - A [1]);	/* X */
		ColorMatrix [1] [0] = wht [0] * A [0];
		ColorMatrix [2] [0] = wht [0] * A [1];
		ColorMatrix [0] [1] = wht [1] * A [2];					/* Y */
		ColorMatrix [1] [1] = wht [1] * (1.0 - A [2] - A [3]);
		ColorMatrix [2] [1] = wht [1] * A [3];
		ColorMatrix [0] [2] = wht [2] * A [4];					/* Z */
		ColorMatrix [1] [2] = wht [2] * A [5];
		ColorMatrix [2] [2] = wht [2] * (1.0 - A [4] - A [5]);
	}
		
/* Clean up */
	for (i = 0; i < 3; i++)	{
		SpFree (shaped_rgb [i]);
		SpFree (xyz_grid [i]);
	}
	
	return spErr;
}
/* ------------------------------------------------------------------------ */
static SpStatus_t ComputeShaperMatrix (
				PTRefNum_t	pt,
				double		FAR *shaper [3],
				double		ColorMatrix [3] [3])
{
	double	wht [3];
	SpStatus_t	error;
		
	error = ComputeShaper (pt, shaper, wht);
	if (error == SpStatSuccess)
	{
		error = ComputeMatrix (pt, shaper, wht, ColorMatrix);
		if (error == SpStatSuccess)
			error = PostNormalize(shaper, ColorMatrix);
	}
	return error;
}
/*------------------------------------------------------------------------*/
static SpStatus_t ComputeShaperEx (
				SpXform_t	xform,
				double		FAR *shaper [3],
				double		wht [3])
{
	KpUInt16_t	*Pels, *p, avg_a, avg_b;
	int			i,j;
	SpStatus_t	spErr = SpStatSuccess;
	SpXformDesc_t	Desc;
	int			center = SHAPERCURVESIZE / 2;
	int			ml = center, mh = center;
	int32		sum [2], N;
	double		temp_a, temp_b, temp_L;
	KpUInt16_t  avgVal;
	
/* Allocate memory */
	Pels = SpMalloc (3*SHAPERCURVESIZE * sizeof(*Pels));
	if (NULL == Pels)
		return SpStatMemory;
	
/* Compute ramp */
	for (i = 0; i < 3; i++)			/* Loop over colors */
		for (j = 0, p = Pels+i; j < SHAPERCURVESIZE ; j++, p += 3)
	{
			*p = (KpUInt16_t)(j * 16);
	}

/* Transform colors */
	spErr = Transform12BitPelsEx (xform, Pels, SHAPERCURVESIZE);
	if (spErr != SpStatSuccess) {
		SpFree (Pels);
		return spErr;
	}
	
/* if output color space is XYZ, convert pels to Lab */
	spErr = SpXformGetDesc( xform, &Desc);
	if (spErr != SpStatSuccess)
	{	
		SpFree (Pels);
		return spErr;
	}

	if (Desc.SpaceOut == SpSpaceXYZ)
	{
		US12XYZ2US12Lab(SHAPERCURVESIZE, Pels, Pels);
	}
	
/*
 * Find limits of monotonic region
 * (Note - because of truncation, constancy does not mean clipping)
 */
 
 
 	avgVal = (KpUInt16_t)(Pels[0] + Pels[(SHAPERCURVESIZE-1) *3]) /2;
 
	for (i = SHAPERCURVESIZE; ((i < 0) && (*(Pels+3*i) > avgVal)); i--) {
		center = i;
	}
	
	
/* go down until finding a reversal */
	for (i = center-1, p = Pels+3*i;
			(i >=0) && (*p <= *(p+3));
					i--, p -= 3)
		ml = i;			/* no reversal yet */

/* head back up, until function starts rising */
	for (i = ml+1,p = Pels+3*i;
			(i < center) && (*p == *(p-3));
					i++, p += 3)
		ml = i;			/* is still constant */
		
/* go up until finding a reversal */
	for (i = center, p = Pels+3*i;
			i < SHAPERCURVESIZE && *p >= *(p-3);
					i++, p += 3)
		mh = i;			/* is still going up */

/* head back down, until function starts falling */
	for (i = mh-1, p = Pels+3*i;
			i >= center && (*p == *(p+3));
					i--, p -= 3)
		mh = i;			/* is still constant */
	
/* Compute effective white value - max L and avg a,b */
	sum[0] = 0;
	sum[1] = 0;
	for (i = center, p = Pels+1+3*i; i <= mh; i++, p += 3) {
		sum [0] += *p;		/* sum a */
		sum [1] += *(p+1);	/* sum b */
	}
	N = mh-center+1;
			
	avg_a = (KpUInt16_t) ((sum [0] + N/2) / N);
	avg_b = (KpUInt16_t) ((sum [1] + N/2) / N);
	temp_a = (double)(avg_a)/16.0 - 128.0;
	temp_b = (double)(avg_b)/16.0 - 128.0;
	temp_L = (double)(Pels [mh*3])/40.80;

	Lab2NormXYZ(temp_L, temp_a, temp_b, &wht [0], &wht [1], &wht [2]);

	for (i = 0; i < 3; i++)	{		/* better safe than sorry */
		if (wht [i] <= 0) {
			SpFree (Pels);		/* clean up */
			return SpStatOutOfRange;	/* can't go on */
		}
	}
	
/* Within monotonic region, shaper = XYZ value/white */
/* loop over monotonic region */
	for (i = ml, p = Pels+3*ml;i <= mh;i++, p += 3)
	{
		temp_a = (double)(p[1])/16.0 - 128.0;
		temp_b = (double)(p[2])/16.0 - 128.0;
		temp_L = (double)(p[0]) / 40.80;

		Lab2NormXYZ (temp_L, temp_a, temp_b,
				&shaper [0] [i], &shaper [1] [i], 
				&shaper [2] [i]);

		for (j = 0; j < 3; j++)	/* loop over color */
		{
			shaper [j] [i] /= wht [j];	/* normalize */
			if (shaper [j] [i] < 0.0)
				shaper [j] [i] = 0.0;	/* don't allow < 0.0 */

		}
	}
	
/* Extend the edges */
	for (i = 0; i < ml; i++)
		for (j = 0; j < 3; j++)
			shaper [j] [i] = shaper [j] [ml];	/* extend to left */
	for (i = mh+1; i < SHAPERCURVESIZE; i++)
		for (j = 0; j < 3; j++)
			shaper [j] [i] = shaper [j] [mh];	/* extend to right */
			
/* Clean up */
	SpFree (Pels);
	return spErr;
}
/*----------------------------------------------------------------------*/
/*  ComputeShaperMatrixEx is called by SpXformCreateMatTagsFromXform    */
static SpStatus_t ComputeShaperMatrixEx (
				SpXform_t	xform,
				double		FAR *shaper [3],
				double		ColorMatrix [3] [3])
{
	double	wht [3];
	SpStatus_t	error;
		
	error = ComputeShaperEx (xform, shaper, wht);
	if (error == SpStatSuccess)
	{
		error = ComputeMatrixEx (xform, shaper, wht, ColorMatrix);
		if (error == SpStatSuccess)
			error = PostNormalize(shaper, ColorMatrix);
	}
	return error;
}



/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Generate Colorant and Response Curve tags from the specified PT.
 *
 * AUTHOR
 * 	lsh (modified by doro)
 *
 * DATE CREATED
 *	August 5, 1997
 *------------------------------------------------------------------*/
SpStatus_t	KSPAPI SpXformCreateMatTagsFromPT (
				SpProfile_t		Profile,
				PTRefNum_t		RefNum)
{
	SpStatus_t		Status;
	SpTagValue_t	Tag;
	KpInt32_t		SpaceIn, SpaceOut;
	KpInt32_t		SenseIn;
	SpStatus_t		spStat;
	double			*Shaper [3];
	double			r [SHAPERCURVESIZE];
	double			g [SHAPERCURVESIZE];
	double			b [SHAPERCURVESIZE];
	double			ColorMatrix [3] [3];
	SpCurve_t		TRC;
	KpUInt16_t		TRCData [SHAPERCURVESIZE];
	int				i;

#if defined (KPMAC68K)
	if (SpNoFPU == SpIsFPUpresent()) {
		return SpStatFailure;
	}
#endif

/* check that we are dealing with an RGB to RCS PT */
	SpaceIn = SpGetKcmAttrInt (RefNum, KCM_SPACE_IN);
	SpaceOut = SpGetKcmAttrInt (RefNum, KCM_SPACE_OUT);
	SenseIn = SpGetKcmAttrInt (RefNum, KCM_MEDIUM_SENSE_IN);
	if ((SpaceIn != KCM_RGB)
			|| (SpaceOut != KCM_RCS)
			|| (SenseIn == KCM_NEGATIVE)) {
		return SpStatOutOfRange;
	}

/* compute tag data */
	Shaper [0] = r;
	Shaper [1] = g;
	Shaper [2] = b;

	spStat = ComputeShaperMatrix (RefNum, Shaper, ColorMatrix);
	if (spStat != SpStatSuccess)
		return spStat;

/* save the colorant tags */
	Tag.TagType = Sp_AT_XYZ;

	Tag.TagId = SpTagRedColorant;
	Tag.Data.XYZ.X = KpF15d16FromDouble ((ColorMatrix [0] [0]));
	Tag.Data.XYZ.Y = KpF15d16FromDouble ((ColorMatrix [0] [1]));
	Tag.Data.XYZ.Z = KpF15d16FromDouble ((ColorMatrix [0] [2]));
	Status = SpTagSet (Profile, &Tag);
	if (SpStatSuccess == Status) {
		Tag.TagId = SpTagGreenColorant;
		Tag.Data.XYZ.X = KpF15d16FromDouble ((ColorMatrix [1] [0]));
		Tag.Data.XYZ.Y = KpF15d16FromDouble ((ColorMatrix [1] [1]));
		Tag.Data.XYZ.Z = KpF15d16FromDouble ((ColorMatrix [1] [2]));
		Status = SpTagSet (Profile, &Tag);
	}
	if (SpStatSuccess == Status) {
		Tag.TagId = SpTagBlueColorant;
		Tag.Data.XYZ.X = KpF15d16FromDouble ((ColorMatrix [2] [0]));
		Tag.Data.XYZ.Y = KpF15d16FromDouble ((ColorMatrix [2] [1]));
		Tag.Data.XYZ.Z = KpF15d16FromDouble ((ColorMatrix [2] [2]));
		Status = SpTagSet (Profile, &Tag);
	}

/* save reponse curve tags */
	TRC.Count = SHAPERCURVESIZE;
	TRC.Data = TRCData;
	Tag.Data.Curve = TRC;

	Tag.TagType = Sp_AT_Curve;
	if (SpStatSuccess == Status) {
		Tag.TagId = SpTagRedTRC;
		for (i = 0; i < SHAPERCURVESIZE; i++)
			TRCData [i] = (KpUInt16_t)KpF15d16FromDouble (r [i]);
		Status = SpTagSet (Profile, &Tag);
	}
	if (SpStatSuccess == Status) {
		Tag.TagId = SpTagGreenTRC;
		for (i = 0; i < SHAPERCURVESIZE; i++)
			TRCData [i] = (KpUInt16_t)KpF15d16FromDouble (g [i]);
		Status = SpTagSet (Profile, &Tag);
	}
	if (SpStatSuccess == Status) {
		Tag.TagId = SpTagBlueTRC;
		for (i = 0; i < SHAPERCURVESIZE; i++)
			TRCData [i] = (KpUInt16_t)KpF15d16FromDouble (b [i]);
		Status = SpTagSet (Profile, &Tag);
	}

	return Status;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 * This function will generate TRC curve tags and colorant tags from 
 * a given xform and insert them into a given profile.
 *
 * INPUT VARIABLES
 *	SpProfile_t	Profile	- The profile to receive the tags
 *	SpXform_t	Xform	- The xform used to generate the tags
 *
 * OUTPUT VARIABLES
 *	None
 *
 * RETURNS
 *	SpStatus_t - SpStatSuccess or a sprofile error
 *
 * AUTHOR
 * stanp
 *
 *------------------------------------------------------------------*/
 
SpStatus_t	KSPAPI SpXformCreateMatTagsFromXform (
		SpProfile_t	Profile,
		SpXform_t	Xform)
{
	SpStatus_t	Status;
	SpTagValue_t	Tag;
	SpStatus_t	spStat;
	SpXformDesc_t	Desc;
	double		*Shaper [3];
	double		r [SHAPERCURVESIZE];
	double		g [SHAPERCURVESIZE];
	double		b [SHAPERCURVESIZE];
	double		ColorMatrix [3] [3];
	SpCurve_t	TRC;
	KpUInt16_t	TRCData [SHAPERCURVESIZE];
	int		i;

#if defined (KPMAC68K)
	if (SpNoFPU == SpIsFPUpresent()) {
		return SpStatFailure;
	}
#endif

	/* check that we are dealing with an RGB to PCS xform */
	spStat = SpXformGetDesc( Xform, &Desc);
	if (spStat != SpStatSuccess)
		return spStat;
 
	if ((Desc.SpaceIn != SpSpaceRGB) || 
	    (Desc.SpaceOut != SpSpaceXYZ && 
	     Desc.SpaceOut != SpSpaceLAB )) 
	{
		return SpStatOutOfRange;
	}
 
	/* compute tag data */
	Shaper [0] = r;
	Shaper [1] = g;
	Shaper [2] = b;
/*	spStat = SpXformGetRefNum (Xform, &refNum);
	if (spStat != SpStatSuccess)
		return spStat;
*/
	spStat = ComputeShaperMatrixEx (/*refNum*/ Xform, Shaper, ColorMatrix);
	if (spStat != SpStatSuccess)
		return spStat;
 
	/* save the colorant tags */
	Tag.TagType = Sp_AT_XYZ;
 
	Tag.TagId = SpTagRedColorant;
	Tag.Data.XYZ.X = KpF15d16FromDouble (ColorMatrix [0] [0]);
	Tag.Data.XYZ.Y = KpF15d16FromDouble (ColorMatrix [0] [1]);
	Tag.Data.XYZ.Z = KpF15d16FromDouble (ColorMatrix [0] [2]);
	Status = SpTagSet (Profile, &Tag);

	if (SpStatSuccess == Status) 
	{
		Tag.TagId = SpTagGreenColorant;
		Tag.Data.XYZ.X = KpF15d16FromDouble (ColorMatrix [1] [0]);
		Tag.Data.XYZ.Y = KpF15d16FromDouble (ColorMatrix [1] [1]);
		Tag.Data.XYZ.Z = KpF15d16FromDouble (ColorMatrix [1] [2]);
		Status = SpTagSet (Profile, &Tag);
	}

	if (SpStatSuccess == Status) 
	{
		Tag.TagId = SpTagBlueColorant;
		Tag.Data.XYZ.X = KpF15d16FromDouble (ColorMatrix [2] [0]);
		Tag.Data.XYZ.Y = KpF15d16FromDouble (ColorMatrix [2] [1]);
		Tag.Data.XYZ.Z = KpF15d16FromDouble (ColorMatrix [2] [2]);
		Status = SpTagSet (Profile, &Tag);
	}
 
	/* save reponse curve tags */
	TRC.Count = SHAPERCURVESIZE;
	TRC.Data = TRCData;
	Tag.Data.Curve = TRC;
 
	Tag.TagType = Sp_AT_Curve;
	if (SpStatSuccess == Status) 
	{
		Tag.TagId = SpTagRedTRC;
		for (i = 0; i < SHAPERCURVESIZE; i++)
			TRCData [i] = (KpUInt16_t)KpF15d16FromDouble (r [i]);

		Status = SpTagSet (Profile, &Tag);
	}

	if (SpStatSuccess == Status) 
	{
		Tag.TagId = SpTagGreenTRC;
		for (i = 0; i < SHAPERCURVESIZE; i++)
			TRCData [i] = (KpUInt16_t)KpF15d16FromDouble (g [i]);

		Status = SpTagSet (Profile, &Tag);
	}

	if (SpStatSuccess == Status) 
	{
		Tag.TagId = SpTagBlueTRC;
		for (i = 0; i < SHAPERCURVESIZE; i++)
			TRCData [i] = (KpUInt16_t)KpF15d16FromDouble (b [i]);

		Status = SpTagSet (Profile, &Tag);
	}
 
	return Status;
}



/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create transform given XYZ logical colorspace definition
 *
 * AUTHOR
 * 	lcc
 *
 * DATE CREATED
 *	November 24, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformLCSCreate (
				KpF15d16XYZ_t	FAR *rXYZ,
				KpF15d16XYZ_t	FAR *gXYZ,
				KpF15d16XYZ_t	FAR *bXYZ,
				KpResponse_t	FAR *rTRC,
				KpResponse_t	FAR *gTRC,
				KpResponse_t	FAR *bTRC,
				KpUInt32_t		gridsize, 
				KpBool_t			invert, 
				SpXform_t		FAR *Xform)
{
	PTRefNum_t		PTRefNum;
	PTErr_t			PTStat;
	SpStatus_t		Status;
	ResponseRecord_t	rRespRec, gRespRec, bRespRec;

	*Xform = NULL;

/* Make the fut...  */
	SpCurveToResponseRec(rTRC, &rRespRec);
	SpCurveToResponseRec(gTRC, &gRespRec);
	SpCurveToResponseRec(bTRC, &bRespRec);
	PTStat = PTNewMatGamPT ((FixedXYZColor FAR *) rXYZ,
				(FixedXYZColor FAR *) gXYZ,
				(FixedXYZColor FAR *) bXYZ,
				&rRespRec,
				&gRespRec,
				&bRespRec, 
				gridsize, invert, &PTRefNum);
	if (KCP_SUCCESS != PTStat)
		return SpStatusFromPTErr(PTStat);

/* set the color space */
	if (invert) {
		Status = SpSetKcmAttrInt (PTRefNum, KCM_SPACE_IN, KCM_CIE_XYZ);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_SPACE_OUT, KCM_RGB);

		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_IN_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_XYZ);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_OUT_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_MON_RGB);
	}
	else {
		Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_SPACE_IN, KCM_RGB);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_SPACE_OUT, KCM_CIE_XYZ);

		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_IN_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_MON_RGB);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_OUT_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_XYZ);
	}
	if (SpStatSuccess != Status)
		return Status;

/* build a profile transform structure */
	return SpXformFromPTRefNumImp (PTRefNum, Xform);
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create transform given XYZ logical colorspace definition
 *
 * AUTHOR
 * 	lcc
 *
 * DATE CREATED
 *	January 6, 1995
 *------------------------------------------------------------------*/

SpStatus_t KSPAPI SpXformLCSAdaptCreate (
				KpF15d16XYZ_t	FAR *rXYZ,
				KpF15d16XYZ_t	FAR *gXYZ,
				KpF15d16XYZ_t	FAR *bXYZ,
				KpResponse_t	FAR *rTRC,
				KpResponse_t	FAR *gTRC,
				KpResponse_t	FAR *bTRC,
				KpUInt32_t		gridsize, 
				KpBool_t		invert, 
				KpBool_t		adapt, 
				KpBool_t		lagrange, 
				SpXform_t		FAR *Xform)
{
	PTRefNum_t		PTRefNum;
	PTErr_t			PTStat;
	SpStatus_t		Status;
	newMGmode_t		PTmodes;
	ResponseRecord_t	rRespRec, gRespRec, bRespRec;

	*Xform = NULL;

/* Make the fut - set the modes  */
	/* Bradford adapation for LCS */
	if (adapt) PTmodes.adaptMode = KCP_BRADFORDD50_ADAPTATION; 

	/* no adaptation for pt2pf display pf generation */
	else PTmodes.adaptMode = KCP_NO_ADAPTATION; 
	if (lagrange) PTmodes.interpMode = KCP_TRC_LAGRANGE4_INTERP;
	else PTmodes.interpMode = KCP_TRC_LINEAR_INTERP;

	SpCurveToResponseRec(rTRC, &rRespRec);
	SpCurveToResponseRec(gTRC, &gRespRec);
	SpCurveToResponseRec(bTRC, &bRespRec);
	PTStat = PTNewMatGamAIPT ((FixedXYZColor FAR *) rXYZ,
				(FixedXYZColor FAR *) gXYZ,
				(FixedXYZColor FAR *) bXYZ,
				&rRespRec,
				&gRespRec,
				&bRespRec, 
				gridsize, invert, &PTmodes, &PTRefNum);
	if (KCP_SUCCESS != PTStat)
		return SpStatusFromPTErr(PTStat);

/* set the color space */
	if (invert) {
		Status = SpSetKcmAttrInt (PTRefNum, 
				KCM_SPACE_IN, KCM_CIE_XYZ);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_SPACE_OUT, KCM_RGB);

		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_IN_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_XYZ);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_OUT_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_MON_RGB);
	}
	else {
		Status = SpSetKcmAttrInt (PTRefNum, KCM_SPACE_IN, KCM_RGB);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_SPACE_OUT, KCM_CIE_XYZ);

		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_IN_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_MON_RGB);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_OUT_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_XYZ);
	}
	if (SpStatSuccess != Status)
		return Status;

/* build a profile transform structure */
	return SpXformFromPTRefNumImp (PTRefNum, Xform);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Generate the display transform from profile tag data.
 *
 * AUTHOR
 * 	lcc
 *
 * DATE CREATED
 *	January 24, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformGenerateDisplay (
				SpProfile_t		Profile,
				KpUInt32_t		GridSize,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTrans,
				SpXform_t		FAR *Xform)
{
	SpStatus_t		Status;
	KpBool_t		Invert;
	KpBool_t		Linear = KPTRUE, Adapt = KPFALSE;
	KpF15d16XYZ_t	rXYZ, gXYZ, bXYZ;
	SpTagValue_t	Value, rTRC, gTRC, bTRC;
	
	SpXformData_t	*XformData;

	SPArgUsed (WhichRender);

	*Xform = NULL;

/* Determine the direction of the transform */
	switch (WhichTrans) {
	case SpTransTypeIn:
		Invert = KPFALSE;
		break;

	case SpTransTypeOut:
		Invert = KPTRUE;
		break;

	case SpTransTypeSim:
	case SpTransTypeGamut:
		return SpStatUnsupported;

	default:
		return SpStatOutOfRange;
	}
	
/* get the colorant tags */
	Status = SpTagGetById (Profile, SpTagRedColorant, &Value);
	if (SpStatSuccess != Status)
		return Status;

	rXYZ = Value.Data.XYZ;
	SpTagFree (&Value);

	Status = SpTagGetById (Profile, SpTagGreenColorant, &Value);
	if (SpStatSuccess != Status)
		return Status;

	gXYZ = Value.Data.XYZ;
	SpTagFree (&Value);

	Status = SpTagGetById (Profile, SpTagBlueColorant, &Value);
	if (SpStatSuccess != Status)
		return Status;

	bXYZ = Value.Data.XYZ;
	SpTagFree (&Value);

/* get the responce data */
	Status = SpTagGetById (Profile, SpTagRedTRC, &rTRC);
	if (SpStatSuccess != Status)
		return Status;

	Status = SpTagGetById (Profile, SpTagGreenTRC, &gTRC);
	if (SpStatSuccess != Status) {
		SpTagFree (&rTRC);
		return Status;
	}

	Status = SpTagGetById (Profile, SpTagBlueTRC, &bTRC);
	if (SpStatSuccess != Status) {
		SpTagFree (&rTRC);
		SpTagFree (&gTRC);
		return Status;
	}

/* call the LCSCreate function to generate the fut... */
	Status = SpXformCreate (&rXYZ, &gXYZ, &bXYZ,
				&rTRC.Data.Curve,
				&gTRC.Data.Curve,
				&bTRC.Data.Curve,
				GridSize, Invert, Adapt,
				Linear, Xform);

/* free the Response tags */
	SpTagFree (&rTRC);
	SpTagFree (&gTRC);
	SpTagFree (&bTRC);
	
	if (SpStatSuccess == Status) {
		XformData = SpXformLock (*Xform);
		if (NULL == XformData) {
			return SpStatBadXform;
		}

		XformData->WhichRender = WhichRender;

		SetWtPt(Profile, XformData);

		SpXformUnlock (*Xform);
	}

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get size of data block needed to hold a transform.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	Jne 20, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformToBlobGetDataSize (
				SpXform_t	Xform,
				KpInt32_t	FAR *BufferSize)
{
	SpXformData_t	FAR *XformData;
	KpInt32_t		DataSize;
	PTErr_t			PTStat;

	XformData = SpXformLock (Xform);
	if (NULL == XformData)
		return SpStatBadXform;

	PTStat = PTGetSizeF (XformData->PTRefNum, PTTYPE_FUTF, &DataSize);
	if (KCP_SUCCESS != PTStat) {
		SpXformUnlock (Xform);
		return SpStatusFromPTErr(PTStat);
	}

	*BufferSize = (KpInt32_t)sizeof (*XformData) + DataSize;

	SpXformUnlock (Xform);
	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get transform into a block of data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	Jne 20, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformToBlobGetData (
				SpXform_t		Xform,
				KpInt32_t		BufferSize,
				SpHugeBuffer_t	Buffer)
{
	SpStatus_t		Status;
	SpXformData_t	FAR *XformData;
	KpInt32_t		CheckSize;
	PTErr_t			PTStat;

	Status = SpXformToBlobGetDataSize (Xform, &CheckSize);
	if (SpStatSuccess != Status)
		return Status;

	if (BufferSize < CheckSize)
		return SpStatBufferTooSmall;

	XformData = SpXformLock (Xform);
	if (NULL == XformData)
		return SpStatBadXform;

	KpMemCpy (Buffer, XformData, sizeof (*XformData));
	Buffer = ((char KPHUGE *) Buffer) + sizeof (*XformData);
	BufferSize -= sizeof (*XformData);

	PTStat = PTGetPTF (XformData->PTRefNum, PTTYPE_FUTF,
							BufferSize, (PTAddr_t) Buffer);
	if (KCP_SUCCESS != PTStat) {
		SpXformUnlock (Xform);
		return SpStatusFromPTErr(PTStat);
	}

	SpXformUnlock (Xform);
	return SpStatSuccess;
}

void SpCurveToResponseRec ( KpResponse_t *Curve, ResponseRecord_p ResRec)
{
	SpParaCurve_t	*ParaCurve;

	ParaCurve = (SpParaCurve_t *)Curve;
	if (ParaCurve->TagType == Sp_AT_ParametricCurve)
	{
		ResRec->TagSig = PARA_TYPE_SIG;
		ResRec->ParaFunction = ParaCurve->ParaData.FuncType;
		ResRec->ParaParams =  ParaCurve->ParaData.Parameters;
	} else
	{
		ResRec->TagSig = CURVE_TYPE_SIG;
		ResRec->CurveCount = Curve->Count;
		ResRec->CurveData = Curve->Data;
	}

}

     
