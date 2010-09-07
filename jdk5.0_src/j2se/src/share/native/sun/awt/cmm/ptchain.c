/*
 * @(#)ptchain.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)ptchain.c	2.34	99/01/24

	Contains:	KCMS PT chaining function

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1991-1999
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/


#include "kcms_sys.h"

#if defined (KPMAC)
	#include <Memory.h>
#endif

#include <string.h>
#include <stdio.h>

#include "kcmptlib.h"
#include "kcptmgrd.h"
#include "attrib.h"
#include "attrcipg.h"
#include "kcpfchn.h"
#include "fut_util.h"


PTErr_t
	PTChain (	PTRefNum_t	PTRefNum)
{
chainState_p	cS;
PTErr_t			PTErr, PTErr1;
KpHandle_t		PTData;
PTRefNum_t		PTRefNum1 = NULL, PTRefNum2 = NULL;
KpInt32_t		mode;
fut_p			fut = NULL;
KpChar_t		auxPTName[30];

	PTErr = getChainState (&cS);
	if (PTErr != KCP_SUCCESS)	return PTErr;

	if (cS->chainLength == 0) {
		PTErr = KCP_NO_CHAININIT;
		goto GetOut;
	}
	
	if (cS->chainIndex >= cS->chainLength) {
		PTErr = KCP_EXCESS_PTCHAIN;
		goto GetOut;
	}

	if (cS->chainDef[cS->chainIndex] != PTRefNum) {
		PTErr = KCP_INVAL_PT_SEQ;
		goto GetOut;
	}

	PTErr = PTGetPTInfo (PTRefNum, NULL, NULL, (PTAddr_p*)&PTData);	
	if (PTErr != KCP_PT_ACTIVE) {
		goto GetOut;	
	}

	mode = cS->compMode & PT_COMBINE_TYPE;
	
	if (cS->currentPT == 0) {	/* first PT in chain? */
		KpInt32_t	srcFormat;

		srcFormat = PTGetSrcFormat (PTRefNum);		/* get original format */

		/* we hard coded this rule just for pts and not for profiles */
		if ((srcFormat == PTTYPE_FUTF) && (cS->iComp == KCM_CHAIN_CLASS_CMYK)) {
			if (cS->oComp == KCM_CHAIN_CLASS_MON_RGB) {
				strcpy (auxPTName, "CP10i");
			}
			else {
				strcpy (auxPTName, "CP05");
			}

			PTErr = loadAuxPT (auxPTName, cS->inSense, &PTRefNum1);	/* get the aux PT */
			if (PTErr != KCP_SUCCESS) {
				goto GetOut;
			}

			PTRefNum2 = PTRefNum;
		}
		else {
			PTRefNum1 = PTRefNum;	/* first PT in chain */
			PTRefNum2 = NULL;
		}
	}
	else {
		KpInt32_t	OutSpace, InSpace;

		OutSpace = getIntAttrDef (cS->currentPT, KCM_SPACE_OUT);
		InSpace = getIntAttrDef (PTRefNum, KCM_SPACE_IN);
	
				/* if the color spaces are not the same, and */
		if ((OutSpace != InSpace)  &&
				/* neither color space is unknown, and */
			(OutSpace != KCM_UNKNOWN) && (InSpace != KCM_UNKNOWN) &&
				/* this is a profile composition, then */
			((mode == PT_COMBINE_PF_8) || (mode == PT_COMBINE_PF_16) || (mode == PT_COMBINE_PF)) &&
				/* both color spaces must be ICC PCS */
			(((OutSpace != KCM_CIE_LAB) && (OutSpace != KCM_CIE_XYZ)) ||
			 ((InSpace != KCM_CIE_LAB) && (InSpace != KCM_CIE_XYZ)))) {

			PTErr = KCP_OUTSPACE_INSPACE_ERR;	/* that's not allowed */
			goto GetOut;
		}
					
		PTRefNum1 = cS->currentPT;	/* set up composition */
		PTRefNum2 = PTRefNum;
	}

	/* finally, compose the PTs */
	PTErr = PTCombine (cS->compMode, PTRefNum1, PTRefNum2, &cS->currentPT);

	if (PTRefNum1 != PTRefNum) {
		PTErr1 = PTCheckOut (PTRefNum1);	/* free the internal PT */
		if (PTErr1 != KCP_SUCCESS) {
			PTErr = PTErr1;				/* return actual error */
			goto GetOut;
		}
	}

	if (mode == PT_COMBINE_SERIAL) {
		makeSerial (cS->currentPT);		/* it's a serial PT */
	}

	cS->chainIndex++;		/* next PT */
	
GetOut:
	if (PTErr == KCP_SUCCESS) {
		putChainState (cS);			/* save chain state */
	}
	else {
		clearChain (cS);			/* abort PT chaining */
	}

	return PTErr;
}
