/*
 * @(#)combine.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)combine.c	2.94 99/01/24

	Contains:	execute the KCMS compose function, which is called combine

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1991-2002
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/


#include <string.h>
#include <stdio.h>

#include "kcmptlib.h"
#include "kcptmgrd.h"
#include "attrib.h"
#include "attrcipg.h"
#include "kcptmgr.h"
#include "makefuts.h"

/* prototypes */
static fut_p getGenderMendFut (	KpInt32_t out_of_fut1, KpInt32_t in_of_fut2);
static fut_p genderMendCompose (fut_p, KpInt32_t, fut_p, KpInt32_t);
static KpInt32_t hasLinPlus (fut_p fut);
static fut_p pfCompose (PTRefNum_t, fut_p, PTRefNum_t, fut_p, KpInt32_t);


/* get color space matching fut, if needed */
static fut_p
	getGenderMendFut (	KpInt32_t	out_of_fut1,
						KpInt32_t	in_of_fut2)
{
fut_p	futmatch ;

	if ((out_of_fut1 == KCM_CIE_XYZ) && (in_of_fut2 == KCM_CIE_LAB)) {
		futmatch = get_xyz2lab (KCP_GRID_DIM_SIXTEEN);
	}
	else {
		if ((out_of_fut1 == KCM_CIE_LAB) && (in_of_fut2 == KCM_CIE_XYZ)) {
			futmatch = get_lab2xyz (KCP_GRID_DIM_SIXTEEN);
		}
		else {
			futmatch = NULL;	/* already match */
		}
	}
	
	return futmatch;
}

/* do compose, taking into account possibly different color spaces */
static fut_p
	genderMendCompose (	fut_p		fut2,
						KpInt32_t	in_of_fut2,
						fut_p		fut1,
						KpInt32_t	out_of_fut1)
{
fut_p	fut1mended = NULL, futmatch = NULL, futR = NULL;

	futmatch = getGenderMendFut (out_of_fut1, in_of_fut2);

	if (futmatch != NULL) {			/* change fut1's color space to match fut2's color space */
		fut1mended = fut_comp (futmatch, fut1, 0);
		fut_free (futmatch);
	}
	else {
		fut1mended = fut1;
	}

	if (fut1mended != NULL) {		/* now combine the two futs */
		futR = fut_comp (fut2, fut1mended, 0);
		
		if (fut1mended != fut1) {
			fut_free (fut1mended);
		}
	}

	return futR;
}


/* if any input table has a majority of values
 * which are greater than the corresponding linear value,
 * return 1.
 *else, return 0
 */

static KpInt32_t hasLinPlus (fut_p fut)
{
KpInt32_t		linPlusCount, i1, i2, nEntries, initialSection, halfEntries;
fut_itbl_p		theItbl;
mf2_tbldat_p	theRefTbl;
double			linVal, linInc, delta, initialSlope;

	/* make sure the fut is in the reference state */
	if (fut_to_mft (fut) != 1) {
		return 0;
	}

	for (i1 = 0; i1 < FUT_NICHAN; i1++) {
		theItbl = fut->itbl[i1];			/* check each itbl */

		if ( IS_ITBL(theItbl)) {
			theRefTbl = theItbl->refTbl;
			linPlusCount = 0;
			linVal = 0.0;
			initialSlope = 0.0;
			nEntries = theItbl->refTblEntries;
			linInc = (double) MF2_TBL_MAXVAL / (double) (nEntries -1);
			initialSection = (nEntries -1) / 50;	/* initial section is first 2% of the curve */
			halfEntries = nEntries >> 1;

			for (i2 = 0; i2 < nEntries; i2++) {
				delta = theRefTbl [i2] - linVal;
				
				if (i2 <= initialSection) {
					initialSlope += delta;		/* add deltas to determine initial slope of the curve */
				}
				else {			
					if (initialSlope > 0.0) {	/* initial slope positive? */
						return 1;				/* yes */
					}
				}

				if (delta > 0.0) {
					linPlusCount++;				/* another one > linear */
					
					if (linPlusCount > halfEntries) {
						return 1;
					}
				}

				linVal += linInc;	/* next linear value */
			}
		}
	}
	
	return 0;
}


/**************************************************/
/* composition for transforms from profiles */
/**************************************************/
static fut_p
	pfCompose (	PTRefNum_t	PTRefNum2,
				fut_p		inFut2,
				PTRefNum_t	PTRefNum1,
				fut_p		inFut1,
				KpInt32_t	mode)
{
PTErr_t		PTErr;
KpInt32_t	dummy = 0, InSpace1, OutSpace1, InSpace2, OutSpace2, iiomask, compType;
KpInt32_t	newLUTDims, LUTDims1, NumInVars1, LUTDims2;
fut_p		fut1resized = NULL, resizeFut = NULL, tempFut = NULL, futR = NULL;
KpInt32_t	maxGridDim, maxIntervals, fixedSize[FUT_NICHAN], intervalSize[FUT_NICHAN], i1, nIntervals;
fut_p		fut1 = NULL, fut2 = NULL, matrixFut1 = NULL, matrixFut2 = NULL;
PTRefNum_t	matrixPTRefNum;
KpHandle_t	matrixPTData;
KpUInt32_t	lutConfig = 0;

	InSpace1 = getIntAttrDef (PTRefNum1, KCM_SPACE_IN);
	OutSpace1 = getIntAttrDef (PTRefNum1, KCM_SPACE_OUT);
	InSpace2 = getIntAttrDef (PTRefNum2, KCM_SPACE_IN);
	OutSpace2 = getIntAttrDef (PTRefNum2, KCM_SPACE_OUT);

	compType = mode & PT_COMBINE_TYPE;

	PTErr = getMatrixPTRefNum(PTRefNum1, &matrixPTRefNum, &lutConfig);
	if (KCP_SUCCESS == PTErr)
	{
		matrixPTData = getPTData (matrixPTRefNum);
		if (matrixPTData)
		{
			matrixFut1 = fut_lock_fut (matrixPTData);
		}
	}
	switch (inFut1->lutConfig) {
	case MBA_B_MATRIX_M_COMBO:
	case MAB_M_MATRIX_B_COMBO:
	case MBA_B_MATRIX_M_CLUT_A_COMBO:
		fut1 = matrixFut1;
		break;

	case MAB_A_CLUT_M_MATRIX_B_COMBO:
	default:
		fut1 = inFut1;
		break;
	}
	
	PTErr = getMatrixPTRefNum(PTRefNum2, &matrixPTRefNum, &lutConfig);
	if (KCP_SUCCESS == PTErr)
	{
		matrixPTData = getPTData (matrixPTRefNum);
		if (matrixPTData)
		{
			matrixFut2 = fut_lock_fut (matrixPTData);
		}
	}
	switch (inFut2->lutConfig) {
	case MBA_B_MATRIX_M_COMBO:
	case MAB_M_MATRIX_B_COMBO:
	case MBA_B_MATRIX_M_CLUT_A_COMBO:
		fut2 = matrixFut2;
		break;

	case MAB_A_CLUT_M_MATRIX_B_COMBO:
	default:
		fut2 = inFut2;
		break;
	}
	if (compType == PT_COMBINE_SERIAL) {	/* if serial composition */
		fut1resized = fut1;				/* do not resize or use default rules */
	}
	else {
		fut_mfutInfo (inFut1, &LUTDims1, &NumInVars1, &dummy, dummy, &dummy, &dummy, &dummy);
		fut_mfutInfo (inFut2, &LUTDims2, &dummy, &dummy, dummy, &dummy, &dummy, &dummy);

		PTErr = getMaxGridDim (&maxGridDim);	/* get the CP max */
		if (PTErr != KCP_SUCCESS) {
			return NULL;
		}

		if (maxGridDim != KCP_GRID_DIM_EIGHT) {	/* if the CP max has changed, fixed at that size */
			newLUTDims = maxGridDim;
			nIntervals = maxGridDim -1;
		}
		else {	/* determine resultant grid dimensions */
			newLUTDims = LUTDims1;

			if ((mode & PT_COMBINE_LARGEST) != 0) {						/* if mode "largest", */
				newLUTDims = MAX (newLUTDims, LUTDims2);				/* use larger of the two */
			}

			if ((mode & PT_COMBINE_NO_DEFAULT_RULES) != 0) {
				newLUTDims = MAX (newLUTDims, KCP_GRID_DIM_SIXTEEN);	/* minimum of 16 if using chaining rules */
			}
			else {
				newLUTDims = MAX (newLUTDims, KCP_GRID_DIM_EIGHT);		/* minimum of 8 otherwise */
			}

			switch (compType) {	/* set the maximum allowed grid size */
			case PT_COMBINE_PF_8:	
				break;							/* valid mode, max already set */

			case PT_COMBINE_PF_16:
			case PT_COMBINE_PF:
				maxGridDim = FUT_GRD_MAXDIM;	/* other modes max at memory limit */
				break;

			default:
				return NULL;
			} 

			if (NumInVars1 == KCP_FOUR_COMP) {							/* if 4 inputs, */
				maxGridDim = MIN (maxGridDim, KCP_GRID_DIM_SIXTEEN);	/* limit grid dimensions to 16 */
			}

			if (NumInVars1 >= KCP_FIVE_COMP) {							/* if 5 or more inputs, */
				maxGridDim = MIN (maxGridDim, KCP_GRID_DIM_EIGHT);		/* limit grid dimensions to 8 */
			}

			newLUTDims = MIN (newLUTDims, maxGridDim);					/* limit resultant grid dimensions to max */

			/* calculate smallest integral interval multiple which is > 90% of required size */
			maxIntervals = (newLUTDims * 900) / 1000;
			for (nIntervals = 0; nIntervals < maxIntervals; nIntervals += (LUTDims1 -1)) {}
		}

		#if defined KCP_DIAG_LOG
		{KpChar_t	string[256];
		sprintf (string, "pfCompose\n mode %x, InSpace1 %d, OutSpace1 %d, InSpace2 %d, OutSpace2 %d, LUTDims1 %d, LUTDims2 %d, maxGridDim %d, newLUTDims %d, nIntervals %d\n",
						mode, InSpace1, OutSpace1, InSpace2, OutSpace2, LUTDims1, LUTDims2, maxGridDim, newLUTDims, nIntervals);
		kcpDiagLog (string); }
		#endif

		for (i1 = 0; i1 < FUT_NICHAN; i1++) {
			fixedSize[i1] = newLUTDims;			/* define new fixed grid sizes */
			intervalSize[i1] = nIntervals +1;	/* define new interval ratio grid sizes */
		}

		if ((mode & PT_COMBINE_NO_DEFAULT_RULES) != 0) {	/* if no default rules rules */

			#if defined KCP_DIAG_LOG
			kcpDiagLog ("\n only adjust grid size");
			#endif

			fut1resized = fut_resize (fut1, fixedSize);		/* only adjust grid size */
		}
		else {						/* use default composition rules */
			switch (InSpace1) {
			case KCM_IMAGE_LAB:
			case KCM_ICC_LAB8:
			case KCM_ICC_LAB16:
			case KCM_CIE_LAB:

				#if defined KCP_DIAG_LOG
				kcpDiagLog ("\n get_linlab_fut");
				#endif

				resizeFut = get_linlab_fut (newLUTDims, fut1->itbl[0]->dataClass, fut1->itbl[0]->dataClass);
				break;

			case KCM_RGB:
				if (hasLinPlus (fut1) == 1) {		/* linearization < gamma of 1.0 info, preserve input table shape */

					#if defined KCP_DIAG_LOG
					kcpDiagLog ("\n hasLinPlus (fut1) == 1; preserve input table shape");
					#endif

					fut1resized = fut_resize (fut1, fixedSize);
				}
				else {		/* use linear input tables */

					#if defined KCP_DIAG_LOG
					kcpDiagLog ("\n hasLinPlus (fut1) != 1; use linear input tables");
					#endif

					iiomask = FUT_OUT(fut1->iomask.in) | FUT_IN(fut1->iomask.in);
					resizeFut = constructfut (iiomask, fixedSize, NULL, NULL, NULL, NULL, KCP_FIXED_RANGE, KCP_FIXED_RANGE);
				}
				break;

			case KCM_CMY:
			case KCM_CMYK:
				if (OutSpace2 == KCM_CIE_XYZ) {		/* preserve input table shape */

					#if defined KCP_DIAG_LOG
					kcpDiagLog ("\n output to XYZ, preserve input table shape");
					#endif

					fut1resized = fut_resize (fut1, fixedSize);
				}
				else {		/* use linear input tables */

					#if defined KCP_DIAG_LOG
					kcpDiagLog ("\n output to non-XYZ, use linear input tables");
					#endif

					iiomask = FUT_OUT(fut1->iomask.in) | FUT_IN(fut1->iomask.in);
					resizeFut = constructfut (iiomask, fixedSize, NULL, NULL, NULL, NULL, KCP_FIXED_RANGE, KCP_FIXED_RANGE);
				}
				break;

			case KCM_CIE_XYZ:
				if (OutSpace2 == KCM_RGB) {			/* preserve input table shape */

					#if defined KCP_DIAG_LOG
					kcpDiagLog ("\n XYZ input, preserve input table shape");
					#endif

					fut1resized = fut_resize (fut1, fixedSize);
				}
				else {		/* use linear input tables */

					#if defined KCP_DIAG_LOG
					kcpDiagLog ("\n XYZ input, use get_idenMonCurv_fut");
					#endif

					iiomask = FUT_OUT(fut1->iomask.in) | FUT_IN(fut1->iomask.in);
					resizeFut = get_idenMonCurv_fut (newLUTDims, KCP_709_INV_GAMMA, KCP_709_OFFSET);
				}
				break;

			default:
				if (NumInVars1 == KCP_THREE_COMP) {	/* preserve input table shape and maintain grid positions */

					#if defined KCP_DIAG_LOG
					kcpDiagLog ("\n preserve input table shape and maintain grid positions");
					#endif

					fut1resized = fut_resize (fut1, intervalSize);
				}
				else {	/* preserve input table shape with fixed grid size */

					#if defined KCP_DIAG_LOG
					kcpDiagLog ("\n preserve input table shape with fixed grid size");
					#endif

					fut1resized = fut_resize (fut1, fixedSize);
				}
				break;
			}
		}

		if (resizeFut != NULL) {
			fut1resized = fut_comp (fut1, resizeFut, 0);	/* make resized fut */
			fut_free (resizeFut);	/* free the resizing fut */
		}
	}

	if (fut1resized != NULL) {
		switch (inFut1->lutConfig) {
		case MAB_A_CLUT_M_MATRIX_B_COMBO:
			tempFut = fut_comp (matrixFut1, fut1resized, 0);	/* add in the matrix */
			break;

		case MBA_B_MATRIX_M_CLUT_A_COMBO:
			tempFut = fut_comp (inFut1, fut1resized, 0);		/* add in the fut */
			break;

		default:
			break;
		}
		if (NULL != tempFut) {
			if ((fut1resized != inFut1) && (fut1resized != matrixFut1)) {
				fut_free (fut1resized);		/* free the intermediate fut */
			}
			fut1resized = tempFut;
			tempFut = NULL;					/* set to NULL in case next fut has a matrix */
		}
		futR = genderMendCompose (fut2, InSpace2, fut1resized, OutSpace1);

		if ((fut1resized != inFut1) && (fut1resized != matrixFut1)) {
			fut_free (fut1resized);	/* free the resized fut */
		}
		switch (inFut2->lutConfig) {
		case MAB_A_CLUT_M_MATRIX_B_COMBO:
			tempFut = fut_comp (matrixFut2, futR, 0);	/* add in the matrix */
			break;

		case MBA_B_MATRIX_M_CLUT_A_COMBO:
			tempFut = fut_comp (inFut2, futR, 0);	/* add in the fut */
			break;

		default:
			break;
		}
		if (NULL != tempFut) {
			fut_free (futR);			/* free intermediate fut */
			futR = tempFut;
			tempFut = NULL;				/* set to NULL just because */
		}
	}
	if ((fut1 != inFut1) && (fut1 != matrixFut1))
	{
		fut_free (fut1);			/* free the composed matrix fut */
	}
	if (matrixFut1)
	{
		fut_unlock_fut (matrixFut1);
	}
	if ((fut2 != inFut2) && (fut2 != matrixFut2))
	{
		fut_free (fut2);			/* free the composed matrix fut */
	}
	if (matrixFut2)
	{
		fut_unlock_fut (matrixFut2);
	}
	
	return futR;
}


/* Compose 2 PTs
 * If the 2nd PT is NULL, duplicate the first PT 
 * deep copy for attributes, header, and data
 * shallow copy for serial data
 */
PTErr_t
	PTCombine (	KpInt32_t	mode,
				PTRefNum_t	PTRefNum1,
				PTRefNum_t	PTRefNum2,
				PTRefNum_p	PTRefNumR)
{
KpGenericPtr_t	PTHdr = NULL;
KpHandle_t		PTData1, PTData2, PTHdrR = NULL, PTDataR = NULL;
fut_p			fut1 = NULL, fut2 = NULL, futR = NULL;
PTErr_t			PTErr = KCP_FAILURE, PTErr1 = KCP_FAILURE, PTStatus1 = KCP_FAILURE, PTStatus2 = KCP_FAILURE;
KpInt32_t		compType, srcFormat;
PTRefNum_t		matrixPTRefNum;
KpUInt32_t		lutConfig;

	if (PTRefNumR == NULL) {	/* Check for valid PTRefNumP */
		goto ErrOut1;
	}

	*PTRefNumR = 0;
	compType = mode & PT_COMBINE_TYPE;	/* get composition type */

	#if defined KCP_DIAG_LOG
	{ KpChar_t	string[256];
	sprintf (string, "\nPTCombine\n mode %x, PTRefNum1 %x, PTRefNum2 %x\n", mode, PTRefNum1, PTRefNum2);
	kcpDiagLog (string); }
	#endif
	
	PTStatus1 = PTGetPTInfo (PTRefNum1, NULL, NULL, (PTAddr_p*)&PTData1);
	if (PTStatus1 == KCP_NOT_CHECKED_IN) {
		goto ErrOut3;
	}
		
	fut1 = fut_lock_fut (PTData1);	/* get the first fut */
	if (fut1 == NULL) {
		goto ErrOut6;
	}

	if (PTRefNum2 == 0) {			/* just copying? */
		futR = fut_copy (fut1);		/* copy data */
		if (futR == NULL) {
			goto ErrOut2;
		}
	
		srcFormat = PTGetSrcFormat (PTRefNum1);	/* get original format */
	}
	else {
		srcFormat = PTTYPE_COMPOSITION;

		PTStatus2 = PTGetPTInfo (PTRefNum2, NULL, NULL, (PTAddr_p*)&PTData2);
		if (PTStatus2 == KCP_NOT_CHECKED_IN) {
			goto ErrOut3;
		}

		fut2 = fut_lock_fut (PTData2);			/* get the second fut */
		if (fut2 == NULL) {
			goto ErrOut6;
		}

		switch (compType) {						/* which type of composition? */
		case PT_COMBINE_STD: {
			KpInt32_t	OutSpace1, InSpace2;

			OutSpace1 = getIntAttrDef (PTRefNum1, KCM_SPACE_OUT);
			InSpace2 = getIntAttrDef (PTRefNum2, KCM_SPACE_IN);
			futR = genderMendCompose (fut2, InSpace2, fut1, OutSpace1);
			break;
			}

		#if !defined (KP_NO_IOTBL_COMBINE)
		case PT_COMBINE_ITBL:
			futR = fut_comp_itbl (fut2, fut1, 0);
			break;
	
		case PT_COMBINE_OTBL:
			futR = fut_comp_otbl (fut2, fut1, 0);	
			break;
		#endif
				
		case PT_COMBINE_PF_8:
		case PT_COMBINE_PF_16:
		case PT_COMBINE_PF:
		case PT_COMBINE_SERIAL:
			futR = pfCompose (PTRefNum2, fut2, PTRefNum1, fut1, mode);
			break;

		default:
			goto ErrOut7;
		}
		
		if (futR == NULL) {
			goto ErrOut4;
		}

		if (fut_unlock_fut (fut2) == NULL) {	/* unlock the futs */
			goto ErrOut5;
		}
		fut2 = NULL;
	}

	if (fut_unlock_fut (fut1) == NULL) {
		goto ErrOut5;
	}
	fut1 = NULL;

	PTErr = fut2PT (&futR, -1, -1, srcFormat, PTRefNumR);	/* make it into a PT */
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut0;
	}

	if (PTRefNum2 == 0) {			/* just copying? */
		PTErr = getMatrixPTRefNum(PTRefNum1, &matrixPTRefNum, &lutConfig);
		if (KCP_SUCCESS == PTErr) {
			PTErr = setMatrixPTRefNum (*PTRefNumR, matrixPTRefNum, lutConfig);
			if (KCP_SUCCESS != PTErr) {
				goto ErrOut4;
			}
			makeActive (matrixPTRefNum, NULL);
		}
		PTErr = copyAllAttr (PTRefNum1, *PTRefNumR);	/* copy all attributes to new PT */
	}
	else {
		PTErr = ComposeAttr (PTRefNum1, PTRefNum2, compType, *PTRefNumR);	/* propagate PT attributes */			
		if (PTErr == KCP_SUCCESS) {		
			PTErr = ComposeAttrFut (PTRefNum1, PTRefNum2, *PTRefNumR);		/* propagate fut attributes */
		}
	}

	if (PTErr != KCP_SUCCESS) {
		goto ErrOut0;
	}

	if ((PTStatus1 == KCP_SERIAL_PT) || (PTStatus2 == KCP_SERIAL_PT)) {		/* serial PT? */
		KpInt32_t		OutSpace, InSpace;
		PTRefNum_t		labxyzPTRefNum;

		#if defined KCP_DIAG_LOG
		{ KpChar_t	string[256];
		sprintf (string, " addSerialData");
		kcpDiagLog (string); }
		#endif

		PTErr = addSerialData (*PTRefNumR, PTRefNum1);	/* copy transform data from 1st PT to new PT */
		if (PTErr != KCP_SUCCESS) {
			goto ErrOut0;
		}

		if (PTRefNum2 != 0) {			/* composing? */
			OutSpace = getIntAttrDef (PTRefNum1, KCM_SPACE_OUT);
			InSpace = getIntAttrDef (PTRefNum2, KCM_SPACE_IN);

			futR = getGenderMendFut (OutSpace, InSpace);	/* create color space matching fut, if needed */

			if (futR != NULL) {
				PTErr = fut2PT (&futR, InSpace, OutSpace, PTTYPE_CALCULATED, &labxyzPTRefNum);	/* make it into a PT */
				if (PTErr != KCP_SUCCESS) {
					goto ErrOut0;
				}

				#if defined KCP_DIAG_LOG
				{ KpChar_t	string[256];
				sprintf (string, " lab<->xyz %x", labxyzPTRefNum);
				kcpDiagLog (string); }
				#endif

				PTErr = addSerialData (*PTRefNumR, labxyzPTRefNum);	/* copy gender mender transform to new PT */
				PTErr1 = PTCheckOut (labxyzPTRefNum);

				if (PTErr != KCP_SUCCESS) {
					goto ErrOut0;
				}

				if (PTErr1 != KCP_SUCCESS) {
					PTErr = PTErr1;				/* return actual error */
					goto ErrOut0;
				}
			}

			PTErr = addSerialData (*PTRefNumR, PTRefNum2);	/* copy transform data from 2nd PT to new PT */
			if (PTErr != KCP_SUCCESS) {
				goto ErrOut0;
			}
		}

		#if defined KCP_DIAG_LOG
		{ KpChar_t	string[256];
		sprintf (string, "\n");
		kcpDiagLog (string); }
		#endif
	}

GetOut:
	#if defined KCP_DIAG_LOG
	{ KpChar_t	string[256];
	sprintf (string, "  PTCombine *PTRefNumR %x\n", *PTRefNumR);
	kcpDiagLog (string); }
	#endif

	return (PTErr);


ErrOut7:
	PTErr = KCP_BAD_COMP_MODE;
	goto ErrOut0;

ErrOut6:
	PTErr = KCP_MEM_LOCK_ERR;
	goto ErrOut0;

ErrOut5:
	PTErr = KCP_MEM_UNLOCK_ERR;
	goto ErrOut0;

ErrOut4:
	PTErr = KCP_COMP_FAILURE;
	goto ErrOut0;

ErrOut3:
	PTErr = KCP_NOT_CHECKED_IN;
	goto ErrOut0;

ErrOut2:
	PTErr = KCP_NO_ACTIVATE_MEM;
	goto ErrOut0;

ErrOut1:
	PTErr = KCP_BAD_PTR;

ErrOut0:
	if (PTDataR != NULL) {
		futR = fut_lock_fut (PTDataR);
	}
	if (futR != NULL) fut_free (futR);

	if (fut2 != NULL) fut_unlock_fut (fut2);
	if (fut1 != NULL) fut_unlock_fut (fut1);
	if (PTHdr != NULL) freeBuffer (PTHdr);
	if (PTHdrR != NULL) freeBuffer (PTHdrR);
	if (*PTRefNumR != 0) PTCheckOut (*PTRefNumR);

	goto GetOut;
}


/* get size of ICC profiles (pts) */
PTErr_t
	getMaxGridDim (	KpInt32_p	size)
{
initializedGlobals_p	iGP;

	iGP = getInitializedGlobals ();
	if (iGP == NULL) {
		return KCP_NO_PROCESS_GLOBAL_MEM;
	}

	*size = iGP->maxGridDim;

	return KCP_SUCCESS;
}
