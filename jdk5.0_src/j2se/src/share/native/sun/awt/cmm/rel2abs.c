/*
 * @(#)rel2abs.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)rel2abs.c	1.15 99/01/24

	Contains:	PTGetRelToAbsPT

	COPYRIGHT (c) 1997-1999 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */


#include "kcms_sys.h"
#include "kcmptlib.h"
#include "kcptmgrd.h"
#include "attrib.h"
#include "attrcipg.h"
#include "fut.h"
#include "makefuts.h"
#include "kcptmgr.h"

typedef struct FloatXYZColor_s {
	KpFloat32_t	X;
	KpFloat32_t	Y;
	KpFloat32_t	Z;
} FloatXYZColor_t;

/*------------------------------------------------------------------------------
 *  PTGetRelToAbsPT -- generate a PT which
 					converts from ICC relative colorimetry to ICC absolute colorimetry
 					
 	(absolute color) = ((media white point) / (profile white point)) * (relative color)

 	so
 	(source absolute color) = ((source media white point) / (source profile white point)) * (source relative color)
 	and
 	(dest absolute color) = ((dest media white point) / (dest profile white point)) * (dest relative color)

	equating source and dest absolute colors
	 	(dest relative color) = ((source media white point) / (dest media white point))
	 						  * ((dest profile white point) / (source profile white point))
	 						  * (source relative color)

 *------------------------------------------------------------------------------
 */
PTErr_t
	 PTGetRelToAbsPT(	KpUInt32_t		RelToAbsMode,
						PTRelToAbs_p	PTRelToAbs,
						PTRefNum_p		PTRefNumPtr)
{
PTErr_t				PTErr;
KpInt32_t			status;
FloatXYZColor_t		sMWP, dMWP, sPWP, dPWP;
Fixed_t				matrix[MF_MATRIX_DIM * MF_MATRIX_DIM];
fut_p				theFutFromMatrix = NULL;

		/* just one mode now.  allows for future expansion of the function */
	if (RelToAbsMode != 0)		return KCP_NOT_IMPLEMENTED;
	if (PTRefNumPtr == NULL)	return KCP_BAD_ARG;

	*PTRefNumPtr = 0;

	sMWP.X = PTRelToAbs->srcMediaWhitePoint.X / (KpFloat32_t)KpF15d16Scale;	/* convert fixed point XYZ to floating point */
	sMWP.Y = PTRelToAbs->srcMediaWhitePoint.Y / (KpFloat32_t)KpF15d16Scale;
	sMWP.Z = PTRelToAbs->srcMediaWhitePoint.Z / (KpFloat32_t)KpF15d16Scale;
	dMWP.X = PTRelToAbs->dstMediaWhitePoint.X / (KpFloat32_t)KpF15d16Scale;
	dMWP.Y = PTRelToAbs->dstMediaWhitePoint.Y / (KpFloat32_t)KpF15d16Scale;
	dMWP.Z = PTRelToAbs->dstMediaWhitePoint.Z / (KpFloat32_t)KpF15d16Scale;
	sPWP.X = PTRelToAbs->srcProfileWhitePoint.X / (KpFloat32_t)KpF15d16Scale;
	sPWP.Y = PTRelToAbs->srcProfileWhitePoint.Y / (KpFloat32_t)KpF15d16Scale;
	sPWP.Z = PTRelToAbs->srcProfileWhitePoint.Z / (KpFloat32_t)KpF15d16Scale;
	dPWP.X = PTRelToAbs->dstProfileWhitePoint.X / (KpFloat32_t)KpF15d16Scale;
	dPWP.Y = PTRelToAbs->dstProfileWhitePoint.Y / (KpFloat32_t)KpF15d16Scale;
	dPWP.Z = PTRelToAbs->dstProfileWhitePoint.Z / (KpFloat32_t)KpF15d16Scale;
		
	matrix[0] = (Fixed_t)(((sMWP.X * dPWP.X) / (dMWP.X * sPWP.X) * KpF15d16Scale) + 0.5);	/* fill in the matrix */
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;
	matrix[4] = (Fixed_t)(((sMWP.Y * dPWP.Y) / (dMWP.Y * sPWP.Y) * KpF15d16Scale) + 0.5);
	matrix[5] = 0;
	matrix[6] = 0;
	matrix[7] = 0;
	matrix[8] = (Fixed_t)(((sMWP.Z * dPWP.Z) / (dMWP.Z * sPWP.Z) * KpF15d16Scale) + 0.5);

	status = makeOutputMatrixXform ((Fixed_p)&matrix, PTRelToAbs->gridSize, &theFutFromMatrix);
	if (status != 1) {
		goto ErrOut1;
	}

	if (fut_to_mft (theFutFromMatrix) != 1) {		/* convert to reference tables */
		goto ErrOut3;
	}

	PTErr = fut2PT (&theFutFromMatrix, KCM_CIE_XYZ, KCM_CIE_XYZ, PTTYPE_CALCULATED, PTRefNumPtr);	/* make into PT */
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut0;
	}

GetOut:
	return (PTErr);


ErrOut3:
	PTErr = KCP_INCON_PT;
	goto ErrOut0;

ErrOut1:
	PTErr = KCP_BAD_ARG;

ErrOut0:
	if (theFutFromMatrix != NULL) fut_free (theFutFromMatrix);
	if (*PTRefNumPtr != 0) PTCheckOut (*PTRefNumPtr);
	goto GetOut;
}


