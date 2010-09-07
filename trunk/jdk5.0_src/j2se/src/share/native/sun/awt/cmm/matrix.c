/*
 * @(#)matrix.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)matrix.c	1.5 98/12/03
 *
   Contains:	Matrix operations.

   Written by:	Drivin' Team

   Copyright:	(C) 1995-1998 by Eastman Kodak Company, all rights reserved.

   Change History (most recent first):

*/

#include "matrix.h"

static KpInt32_t isValidMatrix (KpMatrix_p);
static KpInt32_t getMatrixMinDim (KpMatrix_p, KpMatrix_p, KpMatrix_p);


/* This routine multiplies the two matricies specified by src1 and
 * src2 and puts the result in the  matrix dest
 * return:
 * -1	matrix dimension out of range
 * -2	matrix dimensions do not match properly
 */

KpInt32_t
	KpMatMul (	KpMatrix_p src1,
				KpMatrix_p src2,
				KpMatrix_p dest)
{
KpInt32_t	row, col, i;

	if ((isValidMatrix (src1) != 1) ||	/* matrices must be valid */
		(isValidMatrix (src2) != 1) ||
		(dest == NULL)) {
		return -1;
	}
	
	if (src1->nCols != src2->nRows) {	/* matrices must match */
		return -2;
	}

	dest->nRows = src1->nRows;			/* define size of result matrix */
	dest->nCols = src2->nCols;
	
	/* ok, multiply them */
	for (row = 0; row < dest->nRows; row++) {
		for (col = 0; col < dest->nCols; col++) {
			dest->coef[row][col] = 0.0;
			for (i = 0; i < src1->nCols; i++) {
				dest->coef[row][col] += src1->coef[row][i] * src2->coef[i][col];
			}
		}
	}
	
	return 1;
}


			
/* This routine "dot-multiplies" two matrices.  It multiplies each element of the
 * first matrix times the corresponding element of the second matrix.
 */

KpInt32_t
	KpMatDotMul (	KpMatrix_p src1,
					KpMatrix_p src2,
					KpMatrix_p dest)
{
KpInt32_t	row, col, error;

	error = getMatrixMinDim (src1, src2, dest);	/* get minimum dimensions */
	if (error != 1) {
		return error;
	}
		
	/* ok, divide them */
	for (row = 0; row < dest->nRows; row++) {
		for (col = 0; col < dest->nCols; col++) {
			dest->coef[row][col] = src1->coef[row][col] * src2->coef[row][col];
		}
	}
	
	return 1;
}


/* This routine "dot-divides" two matrices.  It divides each element of the
 * first matrix by the corresponding element of the second matrix.
 */

KpInt32_t
	KpMatDotDiv (	KpMatrix_p src1,
					KpMatrix_p src2,
					KpMatrix_p dest)
{
KpInt32_t	row, col, error;

	error = getMatrixMinDim (src1, src2, dest);	/* get minimum dimensions */
	if (error != 1) {
		return error;
	}
		
	/* ok, divide them */
	for (row = 0; row < dest->nRows; row++) {
		for (col = 0; col < dest->nCols; col++) {
			dest->coef[row][col] = src1->coef[row][col] / src2->coef[row][col];
		}
	}
	
	return 1;
}


/* This routine copies all of the entries of src to dest
 */

KpInt32_t
	KpMatCopy (	KpMatrix_p src,
				KpMatrix_p dest)
{
KpInt32_t	row, col;

	/* source matrix must be valid */
	if (isValidMatrix (src) != 1) {
		return -1;
	}

	dest->nRows = src->nRows;	/* copy sizes */
	dest->nCols = src->nCols;
	
	/* copy elements */
	for (row = 0; row < src->nRows; row++) {
		for (col = 0; col < src->nCols; col++) {
			dest->coef[row][col] = src->coef[row][col];
		}
	}
	
	return 1;
}


/* This routine zeros all of the entries of tMatrix 
 */

KpInt32_t
	KpMatZero (	KpMatrix_p src)
{
KpInt32_t	row, col;

	if (isValidMatrix (src) != 1) {
		return 0;
	}

	/* zero all elements */
	for (row = 0; row < KP_MATRIX_MAX_DIM; row++) {
		for (col = 0; col < KP_MATRIX_MAX_DIM; col++) {
			src->coef[row][col] = 0.0;
		}
	}
	
	return 1;
}


static KpInt32_t
	isValidMatrix (	KpMatrix_p src)
{
	if (src == NULL) {
		return 0;
	}
	
	if ((src->nRows < 0) || (src->nRows > KP_MATRIX_MAX_DIM) ||
		(src->nCols < 0) || (src->nCols > KP_MATRIX_MAX_DIM)) {
		return 0;
	}
	else {
		return 1;
	}
}


static KpInt32_t
	getMatrixMinDim (	KpMatrix_p	src1,
						KpMatrix_p	src2,
						KpMatrix_p	dest)
{

	/* matrices must be valid */
	if ((isValidMatrix (src1) != 1) ||
		(isValidMatrix (src2) != 1) ||
		(dest == NULL)) {
		return 0;
	}
	
	dest->nRows = MIN (src1->nRows, src2->nRows);	/* set size of result matrix */
	dest->nCols = MIN (src1->nCols, src2->nCols);
	
	return 1;
}
