/*
 * @(#)matrix.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*

	File:		matrix.h	@(#)matrix.h	1.2 02/02/95

	Contains:	Matrix operations.

   Written by:	Drivin' Team

   Copyright:	(C) 1995 by Eastman Kodak Company, all rights reserved.

   Change History (most recent first):

*/

/*********************************************************************
 *********************************************************************
 * COPYRIGHT (c) 1995 Eastman Kodak Company
 * As  an unpublished  work pursuant to Title 17 of the United
 * States Code.  All rights reserved.
 *********************************************************************
 *********************************************************************
 */

#ifndef KP_MATRIX_H
#define KP_MATRIX_H

/*#include <Math.h>
#include <StdLib.h>
*/

#include "kcms_sys.h"

#define KP_MATRIX_MAX_DIM 3

/* define a matrix */
typedef struct KpMatrix_s {
	KpInt32_t	nRows;			/* number of rows in this matrix */
	KpInt32_t	nCols;			/* number of columns in this matrix */
	double		coef[KP_MATRIX_MAX_DIM][KP_MATRIX_MAX_DIM];	/* the matrix coefficients */

} KpMatrix_t, FAR* KpMatrix_p, FAR* FAR* KpMatrix_h;

/* function prototypes */
KpInt32_t	KpMatMul (KpMatrix_p, KpMatrix_p, KpMatrix_p);
KpInt32_t	KpMatDotMul (KpMatrix_p, KpMatrix_p, KpMatrix_p);
KpInt32_t	KpMatDotDiv (KpMatrix_p, KpMatrix_p, KpMatrix_p);
KpInt32_t	KpMatCopy (	KpMatrix_p, KpMatrix_p);
KpInt32_t	KpMatZero (	KpMatrix_p);


#endif	/* KP_MATRIX_H */
