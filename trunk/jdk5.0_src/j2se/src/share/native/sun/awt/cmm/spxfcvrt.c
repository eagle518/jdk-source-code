/*
 * @(#)spxfcvrt.c	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains conversion routines.

				Created by lsh, October 18, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2000 by Eastman Kodak Company, 
	            all rights reserved.

	Changes:
	
	SCCS Revision:
		@(#)spxfcvrt.c	1.30 1/6/99

	Revision Level:
		$Workfile: spxfcvrt.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spxfcvrt.c $
		$Revision: 6 $
		$Date: 8/16/00 1:41p $
		$Author: Arourke $

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

#include <math.h>
#include "kcmptlib.h"
#include "attrcipg.h"
#include <string.h>



KpBool_t SpWhichParadigm(SpParadigm_t ParadigmType, KpInt32_t Render)
{
	KpBool_t isAbsolute;

	if (Render == KCM_COLORIMETRIC)
		isAbsolute = KPFALSE;
	else if (ParadigmType == SpParadigmRel)
		isAbsolute = KPFALSE;
	else isAbsolute = KPTRUE;

	return(isAbsolute);
}


static void getPTFileName(KpBool_t Lab2uvL, KpInt32_t Render, SpParadigm_t ParadigmType, KpChar_p ptFileName)
{
	if (Render != KCM_COLORIMETRIC) {
		if (Lab2uvL) {
			if (ParadigmType == SpParadigmRel) {
				strcpy(ptFileName, "CP32");
			}
			else {
				strcpy(ptFileName, "l2uabs");
			}
		}
		else {
			if (ParadigmType == SpParadigmRel) {
				strcpy(ptFileName, "CP31");
			}
			else {
				strcpy(ptFileName, "u2labs");
			}
		}
	}
	else {
		if (Lab2uvL) {
			strcpy(ptFileName, "CP32");
		}
		else {
			strcpy(ptFileName, "CP31");
		}
	}
}

static void savePT(KpChar_p ptFileName, KpUInt32_t size, SpHugeBuffer_t ptData)
{
	int fileStatus;
	KpFileId fdp;
	KpFileProps_t	props;

#if defined (KPMAC)
	strcpy(props.fileType, "PT  ");
	strcpy(props.creatorType, "KEPS");
	props.vRefNum = -1;
	props.dirID = 0;
#endif

	fileStatus = KpFileOpen (ptFileName, "w", &props, &fdp);
	if (fileStatus != KCMS_IO_SUCCESS) return;

	KpFileWrite(fdp, ptData, size);

	KpFileClose(fdp);

}

/************* end debugging routines ****************/

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Construct an Xform to convert uvL-->Lab or Lab-->uvL.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	June 20, 1994
 *------------------------------------------------------------------*/
SpStatus_t SpXformBuildCnvrt (
				KpBool_t		Lab2uvL,
				KpInt32_t		Render,
				SpConnectType_t	ConnectType,
				SpParadigm_t	ParadigmType,
				PTRefNum_t		FAR *RefNum)
{
SpPTCache_t	FAR *PTCache;
SpStatus_t		Status;
KpInt32_t		SpaceIn, SpaceOut;
KpInt32_t		ChainIn, ChainOut;
KpUInt32_t		size;
SpHugeBuffer_t	ptData;
KpChar_t		ptFileName[256];
PTErr_t			PTErr;

	if (ConnectType) {}

/* determine which paradigm algoritm to use */
	if (ParadigmType == SpParadigmAbs)
		return SpStatUnsupported;

/* determine which cache entry to use */
	if (Lab2uvL) {
		PTCache = &Sp_Lab2uvL;
		SpaceIn = KCM_CIE_LAB;
		SpaceOut = KCM_RCS;
		ChainIn = KCM_CHAIN_CLASS_CIELAB1;
		ChainOut = KCM_CHAIN_CLASS_RCS;
	}
	else {
		PTCache = &Sp_uvL2Lab;
		SpaceIn = KCM_RCS;
		SpaceOut = KCM_CIE_LAB;
		ChainIn = KCM_CHAIN_CLASS_RCS;
		ChainOut = KCM_CHAIN_CLASS_CIELAB1;
	}

/* if cache entry has the right data, just return */
	if (PTCache->Valid &&
			(PTCache->Render == Render) &&
			(PTCache->ParadigmType == ParadigmType)) {
		*RefNum = PTCache->RefNum;
		return SpStatSuccess;
	}

/* clear the cache entry in preparation for building the needed entry */
	if (PTCache->Valid) {
		PTCheckOut (PTCache->RefNum);
		PTCache->Valid = KPFALSE;
	}

/* build the requested transform */
	getPTFileName (Lab2uvL, Render, ParadigmType, ptFileName);
	PTErr = PTGetAuxPT (ptFileName, RefNum);
	Status = SpStatusFromPTErr(PTErr);

	if (SpStatSuccess != Status) {
		return Status;
	}

/* set the color space attributes */
	Status = SpSetKcmAttrInt (*RefNum, KCM_SPACE_IN, SpaceIn);
	if (SpStatSuccess == Status)
		Status = SpSetKcmAttrInt (*RefNum, KCM_SPACE_OUT, SpaceOut);

/* set the chain attributes */
	if (SpStatSuccess == Status)
		Status = SpSetKcmAttrInt (*RefNum, KCM_IN_CHAIN_CLASS_2, ChainIn);
	if (SpStatSuccess == Status)
		Status = SpSetKcmAttrInt (*RefNum, KCM_OUT_CHAIN_CLASS_2, ChainOut);

	if (SpStatSuccess != Status) {
		PTCheckOut (*RefNum);
		return Status;
	}

	PTCache->Valid = KPTRUE;
	PTCache->Render = Render;
	PTCache->ParadigmType = ParadigmType;
	PTCache->RefNum = *RefNum;

/* check to see if this intermediate PT should be written to disk */
	if (writePTs())
	{
		/* get the PT data from the CP */
		Status = SpXformGetDataFromCP(*RefNum, 0, 0, &size, &ptData);
		if (Status != SpStatSuccess) return SpStatSuccess;

		/* write the PT to disk */
		savePT(ptFileName, size, ptData);

		SpFree(ptData);
	}
	return SpStatSuccess;
}


