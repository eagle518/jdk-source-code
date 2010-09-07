/*
 * @(#)calcgtbl.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	calcgtbl.c

	Contains:	calcGtbl3

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include <math.h>
#include "kcmsos.h"
#include "fut.h"
#include "makefuts.h"

/*-------------------------------------------------------------------------------
 *  calcGtbl3 -- calculate 3 grid tables from a (3 x 3) matrix
 *-------------------------------------------------------------------------------
 */
void calcGtbl3 (mf2_tbldat_p tables[], KpInt32_t gridSizes[], 
				double_p rows[], double offset[])
{
double			input[3], temp, scale;
KpInt32_t		i, j, k, row, col, it;

	scale = (double) MF2_TBL_MAXVAL;

     /* Loop over 3D grid, converting indices to floating-point input variables:  */	
	for (k = 0, it = 0; k < gridSizes[0]; k++) {
	
		input[0] = (double)k / (double)(gridSizes[0] - 1);
		for (j = 0; j < gridSizes[1]; j++) {
		
			input[1] = (double)j / (double)(gridSizes[1] - 1);
			for (i = 0; i < gridSizes[2]; i++, it++) {
			
				input[2] = (double)i / (double)(gridSizes[2] - 1);

			     /* Loop over output variables (matrix rows):  */
				for (row = 0; row < 3; row++) {

				     /* Multiply input vector by row of matrix:  */
				    temp = offset[row];	 /* include offset for extended range, if required:  */
					for (col = 0; col < 3; col++) {
						temp += rows[row][col] * input[col];
					}

				     /* Quantize for 12-bit grid table and insert:  */
					tables[row][it] = QUANT_MF2 (temp, scale);
				}
			}
		}
	}
}
