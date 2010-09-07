/*
 * @(#)chnrules.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)chnrules.c	2.47 99/01/24

	Contains:	KCMS PT chaining initialization function

 *********************************************************************
 *    COPYRIGHT (c) 1991-1998 Eastman Kodak Company
 *    As an unpublished work pursuant to Title 17 of the United
 *    States Code.  All rights reserved.
 *********************************************************************
*/


#include <string.h>
#include <stdio.h>

#include "attrib.h"
#include "attrcipg.h"
#include "kcpfchn.h"

/* prototypes */
static PTErr_t doChainInit (chainState_p, KpInt32_t, PTRefNum_p, KpInt32_t, KpInt32_t);
static PTErr_t doChainEnd (chainState_p, PTRefNum_p);
static PTErr_t applyRule (chainState_p, KpChar_p, KpInt32_t);
static PTErr_t getChainClass (PTRefNum_t, KcmAttribute, KcmAttribute, KpInt32_t, KpInt32_p);


PTErr_t
	PTChainInit (	KpInt32_t	nPT,
					PTRefNum_p	PTList,
					KpInt32_t	validate,
					KpInt32_p	index)
{
PTErr_t	PTErr;

	if (validate) {}
	if (index) {}

	PTErr = PTChainInitM (nPT, PTList, PT_COMBINE_STD, 1);

	return PTErr;
}


PTErr_t
	PTChainInitM (	KpInt32_t	nPT,
					PTRefNum_p	PTList,
					KpInt32_t	compMode,
					KpInt32_t	rulesKey)
{
PTErr_t			PTErr;
chainState_p	cS;

	PTErr = getChainState (&cS);

	if (PTErr == KCP_SUCCESS) {
		PTErr = doChainInit (cS, nPT, PTList, compMode, rulesKey);
	
		putChainState (cS);
	}

	return PTErr;
}


/* initialize a PT chaining function */
static PTErr_t
	doChainInit (	chainState_p	cS,
					KpInt32_t		nPT,
					PTRefNum_p		PTList,
					KpInt32_t		compMode,
					KpInt32_t		rulesKey)
{
PTErr_t		PTErr;
KpInt32_t	i1;
KpChar_p	startRule, finishRule;
PTRefNum_t	firstPT, lastPT;

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256], str2[256];
	KpInt32_t	i1;
	sprintf (string, "\ndoChainInit\n PTList[]");
	for (i1 = 0; i1 < nPT; i1++) {
		sprintf (str2, " %x", PTList[i1]);
		strcat (string, str2);
	}
	sprintf (str2, ", compMode %x, rulesKey %d ", compMode, rulesKey);
	strcat (string, str2);
	kcpDiagLog (string); }
	#endif

	if (nPT > MAX_PT_CHAIN_SIZE) {
		PTErr = KCP_EXCESS_PTCHAIN;	/* too many PTs in chain */
		goto GetOut;
	}

	KpMemSet (cS, 0, sizeof (chainState_t));	/* set state to null */

	cS->chainLength = nPT;			/* save # of PTs in chain */
	cS->compMode = compMode;		/* composition mode for the entire chain */

	for (i1 = 0; i1 < nPT; i1++) {
		cS->chainDef[i1] = PTList[i1];	/* save PT sequence for error checking */
	}

	if (rulesKey != 0) {				/* apply any necessary chaining rules */
		firstPT = cS->chainDef[0];		/* get input sense and input index */
		cS->inSense = getIntAttrDef (firstPT, KCM_MEDIUM_SENSE_IN);
		PTErr = getChainClass (firstPT, KCM_IN_CHAIN_CLASS_2, KCM_IN_CHAIN_CLASS, NUM_INPUT_CLASSES, &cS->iComp);
		if (PTErr != KCP_SUCCESS) {
			goto GetOut;
		}
		else {
			lastPT = cS->chainDef[cS->chainLength-1];	/* get output sense and output index */
			cS->outSense = getIntAttrDef (lastPT, KCM_MEDIUM_SENSE_OUT);
			PTErr = getChainClass (lastPT, KCM_OUT_CHAIN_CLASS_2, KCM_OUT_CHAIN_CLASS, NUM_OUTPUT_CLASSES, &cS->oComp);
			if (PTErr != KCP_SUCCESS) {
				goto GetOut;
			}
			else {		/* index into the rules matrix to get start and finish rules */
				#if defined KCP_DIAG_LOG
				{ KpChar_t	string[256];
				sprintf (string, " cS->iComp %d, cS->oComp %d\n", cS->iComp, cS->oComp);
				kcpDiagLog (string); }
				#endif

				startRule = getChainRule (cS->iComp, cS->oComp, 1);		/* get start rule */
				finishRule = getChainRule (cS->iComp, cS->oComp, 0);	/* get finish rule */

				if ((startRule == NULL) || (finishRule == NULL)) {
					#if defined KCP_DIAG_LOG
					{ kcpDiagLog (" default rules\n"); }
					#endif
				}
				else {
					if ((startRule[0] == RULE_SERIAL_EVAL) && (startRule[1] == 0)) {
						cS->compMode &= ~PT_COMBINE_TYPE;		/* use serial evaluation */
						cS->compMode |= PT_COMBINE_SERIAL;
					}
					else {
						#if defined KCP_DIAG_LOG
						{ kcpDiagLog (" chaining rules\n"); }
						#endif

						cS->compMode |= PT_COMBINE_NO_DEFAULT_RULES;	/* apply start rule */
						PTErr = applyRule (cS, startRule, cS->inSense);
					}
				}
			}
		}
	}

	#if defined KCP_DIAG_LOG
	{if ((cS->compMode & PT_COMBINE_TYPE) == PT_COMBINE_SERIAL) {	/* serial evaluation? */
		 kcpDiagLog (" serial evaluation\n"); }}
	#endif

GetOut:
	if (PTErr != KCP_SUCCESS) {
		clearChain (cS);					/* abort PT chaining */
	}

	return PTErr;
}


PTErr_t
	PTChainEnd (	PTRefNum_p	PTRefNum)
{
PTErr_t			PTErr;
chainState_p	cS;

	PTErr = getChainState (&cS);

	if (PTErr == KCP_SUCCESS) {
		PTErr = doChainEnd (cS, PTRefNum);
	}

	return PTErr;
}


/* finish up a PT chaining function */
static PTErr_t
	doChainEnd	(	chainState_p	cS,
					PTRefNum_p		PTRefNum)
{
PTErr_t		PTErr = KCP_SUCCESS;
KpChar_p	finishRule;

	*PTRefNum = NULL;

	if (cS->chainLength == 0) {
		PTErr = KCP_NO_CHAININIT;
		goto GetOut;
	}

	if (cS->chainIndex != cS->chainLength) {
		PTErr = KCP_NOT_CHAINEND;
		goto GetOut;
	}
	
	finishRule = getChainRule (cS->iComp, cS->oComp, 0);	/* get finish rule */

	PTErr = applyRule (cS, finishRule, cS->outSense);		/* apply finish (as opposed to norwegian) rule */

	if (PTErr == KCP_SUCCESS) {		/* return reference number to caller */
		*PTRefNum = cS->currentPT;
		cS->currentPT = NULL;		/* no longer owned by the CP */

		#if defined KCP_DIAG_LOG
		{PTRefNum_t	PTList[MAX_PT_CHAIN_SIZE];
		KpInt32_t	theSerialCount, i1;
		KpChar_t	string[256], str2[256];
		sprintf (string, "\ndoChainEnd\n");
		resolvePTData (*PTRefNum, &theSerialCount, PTList);
		if (theSerialCount == 1) {
			sprintf (str2, " *PTRefNum %x", *PTRefNum);
			strcat (string, str2);
		} else {
			sprintf (string, " serial PT");
		for (i1 = 0; i1 < theSerialCount; i1++) {
			sprintf (str2, " %x", PTList[i1]);
			strcat (string, str2);
		}	}
		strcat (string, "\n");
		kcpDiagLog (string); }
		#endif
	}

GetOut:
	clearChain (cS);				/* done with this chain */
	
	return PTErr;
}


/* apply a composition rule */
static PTErr_t
	applyRule (	chainState_p	cS,
				KpChar_p		rule,
				KpInt32_t		invert)
{
PTErr_t		PTErr = KCP_SUCCESS, errnum1 = KCP_SUCCESS;
PTRefNum_t	PTRefNum, PTRefNum1, PTRefNum2, PTRefNum3;

	if ((rule != NULL) && (rule[0] != 0) &&							/* if valid rule */
		((cS->compMode & PT_COMBINE_TYPE) != PT_COMBINE_SERIAL)) {	/* and not serial */

		PTErr = loadAuxPT (rule, invert, &PTRefNum);	/* load the rule's PT */
		if (PTErr == KCP_SUCCESS) {
			if (cS->currentPT == 0) {		/* pre-compose */
				cS->currentPT = PTRefNum;	/* define current PT to first */
			}
			else {							/* post-compose */
				PTRefNum1 = cS->currentPT;
				PTRefNum2 = PTRefNum;

				PTErr = PTCombine (cS->compMode, PTRefNum1, PTRefNum2, &PTRefNum3);	/* compose the PTs */
				errnum1 = PTCheckOut (PTRefNum);	/* free the rule's PT */
				if (errnum1 != KCP_SUCCESS) {
					if (PTErr == KCP_SUCCESS) {
						PTErr = errnum1;
					}
				}

				/* free the old current PT */
				errnum1 = PTCheckOut (cS->currentPT);
				if (errnum1 != KCP_SUCCESS) {
					if (PTErr == KCP_SUCCESS) {
						PTErr = errnum1;
					}
				}
				cS->currentPT = PTRefNum3;		/* resultant PT is current PT */
			}
		}
	}

	return PTErr;	/* return the first error */
}

		
static PTErr_t
	getChainClass (	PTRefNum_t		PTRefNum,
					KcmAttribute	chainTag1,
					KcmAttribute	chainTag2,
					KpInt32_t		maxValue,
					KpInt32_p		valueP)
{
PTErr_t		PTErr;

	PTErr = getIntAttr (PTRefNum, chainTag1, maxValue, valueP);
	if (PTErr == KCP_INVAL_PTA_TAG) {
		PTErr = getIntAttr (PTRefNum, chainTag2, maxValue, valueP);
	}

	if ((PTErr != KCP_SUCCESS) && (PTErr != KCP_BAD_COMP_ATTR)) {
		*valueP = 0;			/* not present is "no rules" class */
		PTErr = KCP_SUCCESS;
	}
	
	return PTErr;
}

