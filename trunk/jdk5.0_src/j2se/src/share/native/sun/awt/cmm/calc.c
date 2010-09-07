/*
 * @(#)calc.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	calc.c

	functions to calculate input, output, and grid tables from user defined functions.

	COPYRIGHT (c) 1991-2000 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include <string.h>
#include <stdio.h>
#include "makefuts.h"


typedef struct fData_s {
	fut_calcData_t	std;
	double			scale;
} fData_t, FAR* fData_p;

#define MFT_QUANT(inData, quantData) \
	inData = RESTRICT(inData, 0.0, 1.0); \
	quantData = (KpInt32_t) ((inData * mftMaxData) + 0.499999);


/* construct a fut from a set of [iog] functions
 * each array has the functions in order corresponding to the channel position in iomask
 * the first element of fData must be a KpInt32_t and will contain the zero-based channel #
 * being built when the function is called.  The remaining elements are caller defined.
 * If any of the input function arrays are NULL, identity functions are used.
 */

fut_p
	constructfut (	KpInt32_t		iomask,
					KpInt32_p		sizeArray,
					fut_calcData_p	fData,
					fut_ifunc_p		ifunArray,
					fut_gfunc_p		gfunArray,
					fut_ofunc_p		ofunArray,
					PTDataClass_t	iClass,
					PTDataClass_t	oClass)
{
fut_p		futp;
KpInt32_t	i1, imask, omask;
fut_itbl_p	itbls[FUT_NICHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_gtbl_p	gtbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_otbl_p	otbls[FUT_NOCHAN] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
fut_ifunc_t	ifun;
fut_gfunc_t	gfun;
fut_ofunc_t	ofun;
fData_t			fDataL;
fut_calcData_p	fDataP;

	if (sizeArray == NULL) return NULL;

	if (fData == NULL) {
		fDataP = &fDataL.std;
	}
	else {
		fDataP = fData;
	}
	
	imask = FUT_IMASK(iomask);
	omask = FUT_OMASK(iomask);

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256], str2[256];
	KpInt32_t	i1;
	sprintf (string, "constructfut\n iomask %x, sizeArray[]", iomask);
	for (i1 = 0; i1 < FUT_NICHAN; i1++) {
		if ((FUT_BIT(i1) & imask) != 0) {
			sprintf (str2, " %d", sizeArray[i1]);
			strcat (string, str2);
		}
	}
	sprintf (str2, ", fData %x, ifunArray %x, gfunArray %x, ofunArray %x, iClass %d, oClass %d\n",
					fData, ifunArray, gfunArray, ofunArray, iClass, oClass);
	strcat (string, str2);
	kcpDiagLog (string);}
	#endif
	
	/* Compute shared input tables:  */
	for (i1 = 0; i1 < FUT_NICHAN; i1++) {
		if ((imask & FUT_BIT(i1)) != 0) {
			if ((ifunArray == NULL) || (ifunArray[i1] == NULL)) {
				ifun = fut_irampEx;
				fDataP = &fDataL.std;
				if (iClass == KCP_VARIABLE_RANGE) {
					fDataL.scale = KCP_16_TO_8_ENCODING;
				}
				else {
					fDataL.scale = 1.0;
				}
			}
			else {
				ifun = ifunArray[i1];
			}

			fDataP->chan = i1;	/* define the channel # */

			itbls[i1] = fut_new_itblEx (KCP_REF_TABLES, iClass, sizeArray[i1], ifun, fDataP);
			itbls[i1]->id = fut_unique_id ();
			itbls[i1]->dataClass = iClass;
		}
	}

	/* Compute grid tables and output tables:  */
	for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
		if ((omask & FUT_BIT(i1)) != 0) {
			if ((gfunArray == NULL) || (gfunArray[i1] == NULL)) {
				gfun = fut_grampEx;
			}
			else {
				gfun = gfunArray[i1];
			}

			fDataP->chan = i1;	/* define the channel # */

			gtbls[i1] = fut_new_gtblEx (KCP_REF_TABLES, iomask, gfun, fDataP, sizeArray);
			gtbls[i1]->id = fut_unique_id();

			if ((ofunArray == NULL) || (ofunArray[i1] == NULL)) {
				ofun = fut_orampEx;
				fDataP = &fDataL.std;
				if (oClass == KCP_VARIABLE_RANGE) {
					fDataL.scale = KCP_8_TO_16_ENCODING;
				}
				else {
					fDataL.scale = 1.0;
				}
			}
			else {
				ofun = ofunArray[i1];
			}

			otbls[i1] = fut_new_otblEx (KCP_REF_TABLES, oClass, ofun, fDataP);
			otbls[i1]->id = fut_unique_id();
			otbls[i1]->dataClass = oClass;
		}
	}

	/* Assemble FuT:  */
	futp = fut_new (iomask, itbls, gtbls, otbls);

	fut_free_tbls (FUT_NICHAN, (KpGenericPtr_t *)itbls);
	fut_free_tbls (FUT_NOCHAN, (KpGenericPtr_t *)gtbls);
	fut_free_tbls (FUT_NOCHAN, (KpGenericPtr_t *)otbls);

	if (fut_to_mft (futp) != 1) {		/* convert to reference tables */
		fut_free (futp);
		futp = NULL;
	}

	return (futp);
}


/* fut_calc_itbl computes the values of an input table from a user
 * defined function.  Ifun must be a pointer to a function accepting a
 * double and returning a double, both in the range (0.0,1.0).  (NULL is
 * a legal value - it returns leaving the table uninitialized)
 * fut_calc_itbl returns 0 (FALSE) if an error occurs (ifun returned
 * value out of range) and  1 (TRUE) otherwise.
 *
 */
KpInt32_t
	fut_calc_itblEx (	fut_itbl_p		itbl,
						fut_ifunc_t		ifun,
						fut_calcData_p	data)
{
mf2_tbldat_p	theItbl;
KpInt32_t		i, mftData;
double			val, indexNorm, indexInc, mftMaxData = MF2_TBL_MAXVAL;
fData_t			fDataL;
fut_calcData_p	fDataP;
	
	if ( ! IS_ITBL(itbl) ) {
		return (0);
	}

	if (ifun != NULL) {
		itbl->id = fut_unique_id ();		/* new table values, get new unique id */

		if (data == NULL) {
			fDataP = &fDataL.std;
			fDataL.scale = 1.0;
		}
		else {
			fDataP = data;
		}

		theItbl = itbl->refTbl;
		indexInc = 1.0 / (double) (itbl->refTblEntries -1);

		for (i = 0, indexNorm = 0.0; i < itbl->refTblEntries; i++, indexNorm += indexInc) {
			val = (*ifun) (indexNorm, fDataP);
			
			MFT_QUANT(val, mftData)

			theItbl[i] = (mf2_tbldat_t)mftData;
		}
	}

	return (1);
}


/* fut_calc_gtblEx computes the values of a grid table from a user
 * defined function.  Gfun must be a pointer to a function accepting
 * doubles in the range (0.0,1.0) and returning a double in the
 * interval (0.0,1.0). (NULL is a legal value - it just returns
 * leaving the table uninitialized).
 * fut_calc_gtblEx returns 0 (FALSE) if an error occurs (gfun returned
 * value out of range), 1 (TRUE) otherwise.
 */

#define GCLOOP(x) for (i[x] = 0, cList[x] = -norm[x]; i[x] < n[x]; i[x]++ ) { \
		    cList[x] += norm[x];
#define GCLEND }

KpInt32_t
	fut_calc_gtblEx (	fut_gtbl_p		gtbl,
						fut_gfunc_t		gfun,
						fut_calcData_p	data)
{
KpInt32_t		index, n[FUT_NICHAN], i[FUT_NICHAN], mftData;
double			norm[FUT_NICHAN], cList[FUT_NICHAN], val, mftMaxData = MF2_TBL_MAXVAL;
mf2_tbldat_p	grid;

	if ( ! IS_GTBL(gtbl) ) {
		return (0);
	}

	if (gfun != NULL) {
		/* set up grid size in each dimension */
		for (index = 0; index < FUT_NICHAN; index++) {
			n[index] = gtbl->size[index];

			if (n[index] == 1) {
				norm[index] = 0.0;
			}
			else {
				norm[index] = 1.0 / (double) (n[index] -1);
			}
		}

		gtbl->id = fut_unique_id();	/* new table data, new id */

		/* construct function of 1 to 8 input variables */
		grid = gtbl->refTbl;

		GCLOOP(0)
			GCLOOP(1)
				GCLOOP(2)
					GCLOOP(3)
						GCLOOP(4)
							GCLOOP(5)
								GCLOOP(6)
									GCLOOP(7)
										val = (*gfun)(cList, data);
										
										MFT_QUANT(val, mftData)
										
										*grid++ = (mf2_tbldat_t)mftData;
									GCLEND
								GCLEND
							GCLEND
						GCLEND
				    GCLEND
				GCLEND
		    GCLEND
		GCLEND
	}

	return (1);
}


/* fut_calc_otbl computes the values of an output table from a user defined
 * function.  Ofun must be a pointer to a function accepting accepting a
 * double and returning a double, both in the range (0.0,1.0). (NULL is a
 * legal value - it just returns, leaving the table uninitialized).
 * fut_calc_otbl returns 0 (FALSE) if an error occurs (ofun returned
 * value out of range) and 1 (TRUE) otherwise.
 */
KpInt32_t
	fut_calc_otblEx (	fut_otbl_p		otbl,
						fut_ofunc_t		ofun,
						fut_calcData_p	data)
{
KpInt32_t		i, mftData;
mf2_tbldat_p	theOtbl;
double			val, indexNorm, indexInc, mftMaxData = MF2_TBL_MAXVAL;
fData_t			fDataL;
fut_calcData_p	fDataP;

	if ( ! IS_OTBL(otbl) ) {
		return (0);
	}

	if (ofun != NULL) {
		otbl->id = fut_unique_id();

		if (data == NULL) {
			fDataP = &fDataL.std;
			fDataL.scale = 1.0;
		}
		else {
			fDataP = data;
		}

		theOtbl = otbl->refTbl;
		indexInc = 1.0 / (double) (otbl->refTblEntries -1);

		for (i = 0, indexNorm = 0.0; i < otbl->refTblEntries; i++, indexNorm += indexInc) {
			val = (*ofun) (indexNorm, fDataP);

			MFT_QUANT(val, mftData)

			theOtbl[i] = (mf2_tbldat_t)mftData;
		}
	}

	return (1);
}


/* identity functions for initializing and calculating tables. */

double
	fut_irampEx	(double x, fut_calcData_p data)
{
	x *=  ((fData_p) data)->scale;
	
	return (x);
}

double
	fut_grampEx	(double_p dP, fut_calcData_p data)
{
KpInt32_t		chan;

	chan = data->chan;

	return dP[chan];
}

double
	fut_orampEx	(double x, fut_calcData_p data)
{
	x *=  ((fData_p) data)->scale;
	
	return (x);
}
