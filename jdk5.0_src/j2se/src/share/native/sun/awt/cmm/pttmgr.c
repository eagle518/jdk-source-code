/*
 * @(#)pttmgr.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)pttmgr.c	2.36 99/03/04

	Contains:	PT table management

	Windows Revision Level:
		$Workfile: pttmgr.c $
		$Logfile: /DLL/KodakCMS/kpcp_lib/pttmgr.c $
		$Revision: 6 $
		$Date: 3/15/02 3:14p $
		$Author: Msm $

	COPYRIGHT (c) 1991-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */


#if defined (KPMAC)
	#include <Packages.h>
	#include <Memory.h>
#endif

#include "kcms_sys.h"
#include "kcmptlib.h"
#include "kcptmgrd.h"
#include "kcptmgr.h"

#if defined KPMAC
#define PTTP(handle) (*(PTTable_p FAR*)handle)
#define PTHP(handle) (*(fut_hdr_p FAR*)handle)
#else
#define PTTP(handle) ((PTTable_p)handle)
#define PTHP(handle) ((fut_hdr_p)handle)
#endif


/* prototypes */
static PTErr_t freeSerialData (PTRefNum_t);


/***********************************************************************/
/* get the pointer to a PT table */
PTTable_p
	lockPTTable (	PTRefNum_t	PTRefNum)
{
PTTable_p	PTTableP = NULL;

	if (checkPT (PTRefNum) == KCP_SUCCESS) {

		PTTableP = (PTTable_p)lockBuffer ((KpHandle_t)PTRefNum);	/* get the table pointer */

		if (PTTableP != NULL) {
			if ((PTTableP->uid != PTT_UID) || (PTTableP->refNum != PTRefNum)) {
				unlockBuffer ((KpHandle_t)PTRefNum);
				PTTableP = NULL;
			}
		}
	}

	return PTTableP;
}


/***********************************************************************/
/* unlock a PT table */
void
	unlockPTTable (	PTRefNum_t	PTRefNum)
{
	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return;
	}

	unlockBuffer ((KpHandle_t)PTRefNum);
}


/***********************************************************************/
/* delete the given PT table */
void
	deletePTTable (	PTRefNum_t	PTRefNum)
{
PTTable_p	PTTableP;
KpInt32_p		attrTagCountP;

	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return;
	}
	
	PTTableP = (PTTable_p)lockBuffer ((KpHandle_t)PTRefNum);	/* get the table pointer */
	
	if (PTTableP->attrBase != NULL) {	/* free space allocated to this PTs' attributes */
		attrTagCountP = lockBuffer (PTTableP->attrBase);
		if (*attrTagCountP != 0) {
			freeAttributes(PTTableP->attrBase);
		}
		freeBuffer (PTTableP->attrBase);
	}

	KpMemSet (PTTableP, 0, sizeof (PTTable_t));	/* just being careful */

	unlockPTTable (PTRefNum);

	freeBuffer ((KpHandle_t)PTRefNum);	/* free space allocated to this table */
}


/**************************************************************************/
/* register a PT by setting its implicit attributes and entering its info */
/* into the PT table */
PTErr_t
	registerPT (	KpHandle_t	hdr, 
					KpHandle_t	attrBase,
					PTRefNum_p	PTRefNumP)
{
PTRefNum_t	PTRefNum;
PTTable_p	PTTableP;
PTErr_t		PTErr;
KpInt32_t	i;

	PTRefNum = (PTRefNum_t)allocBufferHandle (sizeof (PTTable_t));	/* allocate new PT and get a handle to it */

	if ((PTRefNum == NULL) || (PTMemTest () == 0)) {
		freeBuffer (PTRefNum);		/* in case PTMemTest() failed */
		PTErr = KCP_NO_CHECKIN_MEM;
	}
	else {
		PTTableP = (PTTable_p)lockBuffer ((KpHandle_t)PTRefNum);	/* get the table pointer */

		KpMemSet (PTTableP, 0, sizeof (PTTable_t));	/* just being careful */

		PTTableP->uid = PTT_UID;				/* insert unique id */
		PTTableP->refNum = PTRefNum;			/* save reference number */
		PTTableP->attrBase = attrBase;			/* save attribute address */
		PTTableP->data = NULL;					/* show there's no PT data */
		PTTableP->checkInFlag = IS_CHECKED_IN;	/* show this pt is checked in */
		PTTableP->serialPTflag = NOT_SERIAL_PT;	/* show this pt not a serial PT */
		PTTableP->inUseCount = 0;				/* set in use count */
		PTTableP->serialCount = 0;				/* no serial PTs yet */

		for (i = 0; i < MAX_PT_CHAIN_SIZE; i++) {
			PTTableP->serialDef[i] = NULL;
		}

		nullEvalTables (PTTableP);				/* set evaluation state to null */

		unlockPTTable (PTRefNum);

		PTErr = initAttrib (PTRefNum);			/* set standard attributes */
		if (PTErr != KCP_SUCCESS) {
			goto ErrOut;
		}
		
		PTErr = setPTHdr (PTRefNum, hdr);		/* set header */
		if (PTErr != KCP_SUCCESS) {
			goto ErrOut;
		}
	}
	
GetOut:
	*PTRefNumP = PTRefNum;

	return (PTErr);
	
ErrOut:
	deletePTTable (PTRefNum);
	PTRefNum = NULL;
	goto GetOut;
}


PTErr_t
	hasPTData (PTRefNum_t	PTRefNum)
{
PTErr_t		PTErr;

	PTErr = getPTStatus (PTRefNum);

	if ((PTErr == KCP_SERIAL_PT) || (PTErr == KCP_PT_ACTIVE)) {
		PTErr = KCP_SUCCESS;
	}

	return PTErr;
}


/***********************************************************************/
/* add transform data from one PT to a second PT */
PTErr_t
	addSerialData (	PTRefNum_t	CurrentPTRefNum,
					PTRefNum_t	InputPTRefNum)
{
PTErr_t		PTErr, tempPTErr;
KpInt32_t	newSerialCount, inputSerialCount, i1;
PTRefNum_t	matrixPTRefNum = 0, PTRefNum1 = 0, PTRefNum2 = 0;
KpUInt32_t	lutConfig;

	PTErr = hasPTData (CurrentPTRefNum);
	if (PTErr != KCP_SUCCESS) {
		return PTErr;
	}
	
	PTErr = hasPTData (InputPTRefNum);
	if (PTErr != KCP_SUCCESS) {
		return PTErr;
	}

	newSerialCount = PTTP(CurrentPTRefNum)->serialCount;	/* initialize to the current count */
	inputSerialCount = PTTP(InputPTRefNum)->serialCount;
	
	if (inputSerialCount == 0) {	/* add in the "input" PT */
		if ((newSerialCount + 1) > MAX_PT_CHAIN_SIZE) {
			goto ErrOut1;
		}
		tempPTErr = getMatrixPTRefNum (InputPTRefNum, &matrixPTRefNum, &lutConfig);
		if (tempPTErr == KCP_NOT_CHECKED_IN)	/* there is no matrix PTs to worry about */
		{
			PTRefNum1 = InputPTRefNum;
		} else {
			switch (lutConfig) {
			case MBA_B_MATRIX_M_COMBO:
			case MAB_M_MATRIX_B_COMBO:
				PTRefNum1 = matrixPTRefNum;
				break;

			case MBA_B_MATRIX_M_CLUT_A_COMBO:
				PTRefNum1 = matrixPTRefNum;
				PTRefNum2 = InputPTRefNum;
				break;

			case MAB_A_CLUT_M_MATRIX_B_COMBO:
				PTRefNum1 = InputPTRefNum;
				PTRefNum2 = matrixPTRefNum;
				break;

			default:
				PTRefNum1 = InputPTRefNum;
				break;
			}
		}
		if (((newSerialCount + 2) > MAX_PT_CHAIN_SIZE) && (0 != PTRefNum2)) {
			goto ErrOut1;
		}
		PTTP(CurrentPTRefNum)->serialDef[newSerialCount] = PTRefNum1;
		makeActive (PTRefNum1, NULL);
		newSerialCount++;
		if (0 != PTRefNum2)
		{
			PTTP(CurrentPTRefNum)->serialDef[newSerialCount] = PTRefNum2;
			makeActive (PTRefNum2, NULL);
			newSerialCount++;
		}
	}
	else {
		if ((newSerialCount + inputSerialCount) > MAX_PT_CHAIN_SIZE) {
			goto ErrOut1;
		}

		for (i1 = 0; i1 < inputSerialCount; i1++) {
			PTTP(CurrentPTRefNum)->serialDef[newSerialCount] = PTTP(InputPTRefNum)->serialDef[i1];
			makeActive (PTTP(InputPTRefNum)->serialDef[i1], NULL);
			newSerialCount++;
		}
	}

	PTTP(CurrentPTRefNum)->serialCount = newSerialCount;
	makeSerial (CurrentPTRefNum);			/* it's a serial PT now */

GetOut:
	return (PTErr);
	
ErrOut1:
	PTErr = KCP_EXCESS_PTCHAIN;
	goto GetOut;
}


/***********************************************************************/
/* make the given PT active */
void
	makeActive (PTRefNum_t	PTRefNum,
				KpHandle_t	PTData)
{
	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return;
	}
	
	if (PTTP(PTRefNum)->data != NULL) {			/* already have data? */
		PTTP(PTRefNum)->inUseCount++;			/* count the PT */
	}
	else {
		PTTP(PTRefNum)->data = PTData;			/* define PT data */
		PTTP(PTRefNum)->inUseCount = 1;			/* used once */
	}
}


/***********************************************************************/
/* make the given PT active */
void
	makeSerial (	PTRefNum_t	PTRefNum)
{
	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return;
	}

	PTTP(PTRefNum)->serialPTflag = IS_SERIAL_PT;	/* show this is a serial PT */
}


/***********************************************************************/
/* make the given PT inactive */
PTErr_t
	makeInActive (	PTRefNum_t	PTRefNum, KpBool_t bChkMatrix)
{
KpHandle_t	PTData;
PTErr_t		PTErr;

	if ((PTErr = checkPT (PTRefNum)) != KCP_SUCCESS) {
		return PTErr;
	}

	if (bChkMatrix && PTTP(PTRefNum)->matrixRefNum) {
		PTErr = makeInActive (PTTP(PTRefNum)->matrixRefNum, KPFALSE);
		if (KCP_PT_TABLE_DELETED == PTErr)
		{
			PTTP(PTRefNum)->matrixRefNum = 0;
			PTErr = KCP_SUCCESS;
		}
	}

	PTErr = freeSerialData ( PTRefNum);	/* free any serial data */

	if (PTTP(PTRefNum)->data != NULL) {
		PTTP(PTRefNum)->inUseCount--;
		if (PTTP(PTRefNum)->inUseCount == 0) {

			PTData = PTTP(PTRefNum)->data;
			PTTP(PTRefNum)->data = NULL;
			PTErr = TpFreeData (PTData);

			freeEvalTables (PTRefNum);			/* free the evaluation state memory */

			if (PTTP(PTRefNum)->checkInFlag == NOT_CHECKED_IN) {
	 			deletePTTable (PTRefNum);
				PTErr = KCP_PT_TABLE_DELETED;
			}
		}
	}

	return (PTErr);
}


/***********************************************************************/
/* release the serial data of the given PT */
static PTErr_t
	freeSerialData (	PTRefNum_t  PTRefNum)
{
PTErr_t		PTErr = KCP_SUCCESS;
KpInt32_t	i, theCount;
PTRefNum_t	theRefNum;	

	if ((PTErr = checkPT (PTRefNum)) != KCP_SUCCESS) {
		return PTErr;
	}

	theCount = PTTP(PTRefNum)->serialCount;
	PTTP(PTRefNum)->serialCount = 0;
	PTTP(PTRefNum)->serialPTflag = NOT_SERIAL_PT;		/* show this pt not a serial PT */
	
	for (i = 0; i < theCount; i++) {
		theRefNum = PTTP(PTRefNum)->serialDef[i];
		PTTP(PTRefNum)->serialDef[i] = NULL;
		PTErr = makeInActive (theRefNum, KPFALSE);		/* deactivate the PT */
		if (KCP_PT_TABLE_DELETED == PTErr)
		{
			PTErr = KCP_SUCCESS;
		}
	}

	return (PTErr);
}


/***********************************************************************/
/* make the given PT inactive */
PTErr_t
	makeCheckedOut (	PTRefNum_t	PTRefNum)
{
KpHandle_t	PTHdr, PTAttrBase;
KpUInt32_t	PTinUseCount;
PTErr_t		PTErr = KCP_SUCCESS, errnum1 = KCP_SUCCESS;

	if ((PTErr = checkPT (PTRefNum)) != KCP_SUCCESS) {
		return PTErr;
	}

	if (PTTP(PTRefNum)->checkInFlag == NOT_CHECKED_IN) {
		PTErr = KCP_NOT_CHECKED_IN;
	}
	else {
		PTHdr = PTTP(PTRefNum)->hdr;
		PTTP(PTRefNum)->hdr = NULL;
		PTAttrBase = PTTP(PTRefNum)->attrBase;
		PTinUseCount = PTTP(PTRefNum)->inUseCount;
		PTTP(PTRefNum)->checkInFlag = NOT_CHECKED_IN;
		
		PTErr = TpFreeHdr (PTHdr);				/* free the header */
		errnum1 = freeAttributes(PTAttrBase);	/* free the attributes */

		if (PTinUseCount == 0) {
 			deletePTTable (PTRefNum);
		}
	}

	if (PTErr == KCP_SUCCESS) {
		PTErr = errnum1;
	}

	return (PTErr);
}


/***********************************************************************/
/* PTGetPTInfo returns the header, attribute, and data of a PT. */
PTErr_t
	PTGetPTInfo (	PTRefNum_t	PTRefNum,
					PTAddr_p*	PTHdr,
					PTAddr_p*	PTAttr,
					PTAddr_p*	PTData)
{
PTErr_t PTErr;

	PTErr = getPTStatus (PTRefNum);

	if ((PTErr == KCP_PT_ACTIVE) || (PTErr == KCP_PT_INACTIVE) || (PTErr == KCP_SERIAL_PT)) {
		/* return the requested information */
		if (PTHdr != NULL) {
			*PTHdr = (PTAddr_p)getPTHdr (PTRefNum);
		}
		if (PTAttr != NULL) {
			*PTAttr = (PTAddr_p)getPTAttr (PTRefNum);
		}
		if ( ((PTErr == KCP_PT_ACTIVE) || (PTErr == KCP_SERIAL_PT)) && (PTData != NULL)) {
			*PTData = (PTAddr_p)getPTData (PTRefNum);
		}
	}

	return (PTErr);
}


/***********************************************************************/
/* get the status for a given PT */
PTErr_t
	getPTStatus (	PTRefNum_t	PTRefNum)
{
PTErr_t 	PTErr;
	
	if ((PTErr = checkPT (PTRefNum)) != KCP_SUCCESS) {
		return PTErr;
	}

	if (PTTP(PTRefNum)->serialPTflag == IS_SERIAL_PT) {
		PTErr = KCP_SERIAL_PT;					/* must be a serial pt */
	}
	else {
		if (PTTP(PTRefNum)->checkInFlag == NOT_CHECKED_IN) {
			PTErr = KCP_PT_INVISIBLE;			/* must be a serial component */
		}
		else {
			if (PTTP(PTRefNum)->inUseCount == 0) {
				PTErr = KCP_PT_INACTIVE;		/* not active */
			}
			else {
				PTErr = KCP_PT_ACTIVE;			/* active */
			}
		}
	}

	return (PTErr);
}


/***********************************************************************/
/* get the PT header address for a given PT */
KpHandle_t
	getPTHdr (	PTRefNum_t	PTRefNum)
{
KpHandle_t	hdrHandle;

	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return NULL;
	}

	hdrHandle = PTTP(PTRefNum)->hdr;

	return (hdrHandle);
}

/***********************************************************************/
/* get the PT header address for a given PT */
PTErr_t
	setPTHdr (	PTRefNum_t	PTRefNum,
				KpHandle_t	hdrHandle)
{
PTErr_t	PTErr;

	if ((PTErr = checkPT (PTRefNum)) != KCP_SUCCESS) {
		return PTErr;
	}

	PTTP(PTRefNum)->hdr = hdrHandle;		/* set header */
	PTErr = TpSetImplicitAttr (PTRefNum);	/* set attributes using header info */
	
	return (PTErr);
}


/***********************************************************************/
/* get the attribute base for a given PT */
KpHandle_t
	getPTAttr (	PTRefNum_t	PTRefNum)
{
KpHandle_t	attrHandle;

	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return NULL;
	}

	attrHandle = PTTP(PTRefNum)->attrBase;

	return (attrHandle);
}


/***********************************************************************/
/* set the attribute base for a given PT reference number */
void
	setPTAttr (	PTRefNum_t	PTRefNum,
				KpHandle_t	attrBase)
{
	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return;
	}

	PTTP(PTRefNum)->attrBase = attrBase;
}


/***********************************************************************/
/* get the PT data location for a given PT */
KpHandle_t
	getPTData (	PTRefNum_t	PTRefNum)
{
KpHandle_t	dataHandle;

	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return NULL;
	}

	dataHandle = PTTP(PTRefNum)->data;

	return (dataHandle);
}


/***********************************************************************/
/* get the PT data location for a given PT */
PTErr_t
	setMatrixPTRefNum (PTRefNum_t PTRefNum, PTRefNum_t matrixPTRefNum, KpUInt32_t lutConfig)
{
	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return KCP_INVAL_REFNUM;
	}

	PTTP(PTRefNum)->matrixRefNum = matrixPTRefNum;
	PTTP(PTRefNum)->lutConfig = lutConfig;

	return (KCP_SUCCESS);
}


/***********************************************************************/
/* get the PT data location for a given PT */
PTErr_t
	getMatrixPTRefNum (PTRefNum_t PTRefNum, PTRefNum_p matrixPTRefNum, KpUInt32_p lutConfig)
{
PTErr_t	PTErr = KCP_NOT_CHECKED_IN;		/* assume there is no matrix PT */

	if (checkPT (PTRefNum) != KCP_SUCCESS) {
		return KCP_INVAL_REFNUM;
	}

	*matrixPTRefNum = PTTP(PTRefNum)->matrixRefNum;
	*lutConfig = PTTP(PTRefNum)->lutConfig;
	if (*matrixPTRefNum)
	{
		PTErr = KCP_SUCCESS;
	}
	return (PTErr);
}


/***********************************************************************/
/* get the source data format for a given PT */
KpInt32_t
	PTGetSrcFormat (PTRefNum_t	PTRefNum)
{
KpHandle_t	hdrHandle;
KpInt32_t	srcFormat;
	
	if ((hdrHandle = getPTHdr (PTRefNum)) == NULL) {
		srcFormat = 0;
	}
	else {
		srcFormat = PTHP(hdrHandle)->srcFormat;
	}

	return (srcFormat);
}


/***********************************************************************/
/* get the PT data location for a given PT */
PTErr_t
	resolvePTData (	PTRefNum_t	PTRefNum,
					KpInt32_p	SerialCount,
					PTRefNum_p	SerialRefNum)
{
KpInt32_t	i;
PTErr_t		PTErr, tempPTErr;
PTRefNum_t	matrixPTRefNum = 0, PTRefNum1 = 0, PTRefNum2 = 0;
KpUInt32_t	lutConfig;

	if ((PTErr = checkPT (PTRefNum)) != KCP_SUCCESS) {
		return PTErr;
	}

	if ((PTTP(PTRefNum)->serialPTflag == IS_SERIAL_PT) && (PTTP(PTRefNum)->serialCount > 0)) {
		*SerialCount = PTTP(PTRefNum)->serialCount;
		for (i = 0; i < PTTP(PTRefNum)->serialCount; i++) {
			SerialRefNum[i] = PTTP(PTRefNum)->serialDef[i];
		}
	}
	else {
		tempPTErr = getMatrixPTRefNum (PTRefNum, &matrixPTRefNum, &lutConfig);
		if (tempPTErr == KCP_NOT_CHECKED_IN)	/* there is no matrix PTs to worry about */
		{
			*SerialCount = 1;
			SerialRefNum[0] = PTRefNum;
		} else {
			switch (lutConfig) {
			case MBA_B_MATRIX_M_COMBO:
			case MAB_M_MATRIX_B_COMBO:
				*SerialCount = 1;
				SerialRefNum[0] = matrixPTRefNum;
				break;

			case MBA_B_MATRIX_M_CLUT_A_COMBO:
				*SerialCount = 2;
				SerialRefNum[0] = matrixPTRefNum;
				SerialRefNum[1] = PTRefNum;
				break;

			case MAB_A_CLUT_M_MATRIX_B_COMBO:
				*SerialCount = 2;
				SerialRefNum[0] = PTRefNum;
				SerialRefNum[1] = matrixPTRefNum;
				break;

			default:
				*SerialCount = 1;
				SerialRefNum[0] = PTRefNum;
				break;
			}
		}
	}
	return (PTErr);
}


/***********************************************************************/
PTErr_t
	checkPT (	PTRefNum_t	PTRefNum)
{
	if (PTRefNum == NULL) {
		return KCP_NOT_CHECKED_IN;
	}

	if (PTTP(PTRefNum) == NULL) {
		return KCP_NOT_CHECKED_IN;
	}

	if ((PTTP(PTRefNum)->uid == PTT_UID) && (PTTP(PTRefNum)->refNum == PTRefNum)){
		return KCP_SUCCESS;
	}
	
	return KCP_INVAL_REFNUM;		
}
