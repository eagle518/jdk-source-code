/*
 * @(#)chainsu.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)chainsu.c	2.21 99/01/06

	Contains:	setup for KCP chaining

 *********************************************************************
 *    COPYRIGHT (c) 1991-1998 Eastman Kodak Company
 *    As an unpublished work pursuant to Title 17 of the United
 *    States Code.  All rights reserved.
 *********************************************************************
*/


#include <string.h>

#include "kcpfchn.h"

#define ENDOFGROUP '0'
#define EOLSIZE 1

extern KpThreadMemHdl_t	theRootID;

#define MAX_RULE_COMPONENT (1)	/* max # of rule components */

/* composition rules */
typedef struct composeRule_s {
	KpInt16_t	start;	/* start rule */
	KpInt16_t	finish;	/* finish rule */
} composeRule_t, FAR* composeRule_p;

static composeRule_t	composeRule[NUM_INPUT_CLASSES][NUM_OUTPUT_CLASSES];	/* chaining rules data base */

/* this is the rules data base for chaining */
static KpChar_t composeRulesDB[] = {
0,						/* 1 out 1 in */
'C','P','0','4',0,
'C','P','0','7',0,		/* 1 out 2 in */
'C','P','0','8',0,
'C','P','0','4',0,		/* 1 out 3 in */
'C','P','0','4',0,
0,						/* 1 out 4 in */
'C','P','0','2',0,
0,						/* 1 out 5 in */
'C','P','0','2',0,
0,						/* 1 out 6 in */
'C','P','0','2',0,
0,						/* 1 out 7 in */
'C','P','0','2',0,
0,						/* 1 out 8 in */
'C','P','0','2',0,
0,						/* 1 out 9 in */
0,
0,						/* 1 out 10 in */
0,
'S',0,					/* 1 out 11 in */
0,
0,						/* 1 out 12 in */
0,
0,						/* 2 out 1 in */
0,
'C','P','0','2',0,		/* 2 out 2 in */
0,
'C','P','0','4',0,		/* 2 out 3 in */
0,
0,						/* 2 out 4 in */
0,
0,						/* 2 out 5 in */
0,
0,						/* 2 out 6 in */
0,
0,						/* 2 out 7 in */
0,
0,						/* 2 out 8 in */
0,
0,						/* 2 out 9 in */
0,
'C','P','2','2',0,		/* 2 out 10 in */
0,
'S',0,					/* 2 out 11 in */
0,
0,						/* 2 out 12 in */
0,
0,						/* 3 out 1 in */
'C','P','0','4',0,
'C','P','0','7',0,		/* 3 out 2 in */
'C','P','0','8',0,
'C','P','0','4',0,		/* 3 out 3 in */
'C','P','0','4',0,
0,						/* 3 out 4 in */
'C','P','0','2',0,
0,						/* 3 out 5 in */
'C','P','0','2',0,
0,						/* 3 out 6 in */
'C','P','0','2',0,
0,						/* 3 out 7 in */
'C','P','0','2',0,
0,						/* 3 out 8 in */
'C','P','0','2',0,
0,						/* 3 out 9 in */
'C','P','0','4',0,
'C','P','2','2',0,		/* 3 out 10 in */
0,
'S',0,					/* 3 out 11 in */
0,
0,						/* 3 out 12 in */
'C','P','0','2',0,
0,						/* 4 out 1 in */
0,
'C','P','0','2',0,		/* 4 out 2 in */
0,
'C','P','0','4',0,		/* 4 out 3 in */
0,
0,						/* 4 out 4 in */
0,
0,						/* 4 out 5 in */
0,
0,						/* 4 out 6 in */
0,
0,						/* 4 out 7 in */
0,
0,						/* 4 out 8 in */
0,
0,						/* 4 out 9 in */
0,
'C','P','2','2',0,		/* 4 out 10 in */
0,
'S',0,					/* 4 out 11 in */
0,
0,						/* 4 out 12 in */
0,
0,						/* 5 out 1 in */
'C','P','0','2',0,
'C','P','0','2',0,		/* 5 out 2 in */
'C','P','0','2',0,
'C','P','0','4',0,		/* 5 out 3 in */
'C','P','0','2',0,
0,						/* 5 out 4 in */
'C','P','0','2',0,
0,						/* 5 out 5 in */
'C','P','0','2',0,
0,						/* 5 out 6 in */
'C','P','0','2',0,
0,						/* 5 out 7 in */
'C','P','0','2',0,
0,						/* 5 out 8 in */
'C','P','0','2',0,
0,						/* 5 out 9 in */
'C','P','0','2',0,
'C','P','2','2',0,		/* 5 out 10 in */
'C','P','0','2',0,
'S',0,					/* 5 out 11 in */
0,
0,						/* 5 out 12 in */
'C','P','0','2',0,
0,						/* 6 out 1 in */
0,
'C','P','0','2',0,		/* 6 out 2 in */
0,
'C','P','0','4',0,		/* 6 out 3 in */
0,
0,						/* 6 out 4 in */
0,
0,						/* 6 out 5 in */
0,
0,						/* 6 out 6 in */
0,
0,						/* 6 out 7 in */
0,
0,						/* 6 out 8 in */
0,
0,						/* 6 out 9 in */
0,
'C','P','2','2',0,		/* 6 out 10 in */
0,
'S',0,					/* 6 out 11 in */
0,
0,						/* 6 out 12 in */
0,
0,						/* 7 out 1 in */
0,
'C','P','0','2',0,		/* 7 out 2 in */
0,
'C','P','0','4',0,		/* 7 out 3 in */
0,
0,						/* 7 out 4 in */
0,
0,						/* 7 out 5 in */
0,
0,						/* 7 out 6 in */
0,
0,						/* 7 out 7 in */
0,
0,						/* 7 out 8 in */
0,
0,						/* 7 out 9 in */
0,
'C','P','2','2',0,		/* 7 out 10 in */
0,
'S',0,					/* 7 out 11 in */
0,
0,						/* 7 out 12 in */
0,
0,						/* 8 out 1 in */
0,
'C','P','0','2',0,		/* 8 out 2 in */
0,
'C','P','0','4',0,		/* 8 out 3 in */
0,
0,						/* 8 out 4 in */
0,
0,						/* 8 out 5 in */
0,
0,						/* 8 out 6 in */
0,
0,						/* 8 out 7 in */
0,
0,						/* 8 out 8 in */
0,
0,						/* 8 out 9 in */
0,
'C','P','2','2',0,		/* 8 out 10 in */
0,
'S',0,					/* 8 out 11 in */
0,
0,						/* 8 out 12 in */
0,
'C','P','0','2',0,		/* 9 out 1 in */
0,
'C','P','0','2',0,		/* 9 out 2 in */
0,
'C','P','0','4',0,		/* 9 out 3 in */
0,
0,						/* 9 out 4 in */
0,
0,						/* 9 out 5 in */
0,
0,						/* 9 out 6 in */
0,
0,						/* 9 out 7 in */
0,
0,						/* 9 out 8 in */
0,
0,						/* 9 out 9 in */
0,
'C','P','2','2',0,		/* 9 out 10 in */
0,
'S',0,					/* 9 out 11 in */
0,
'C','P','0','2',0,		/* 9 out 12 in */
0,
0,						/* 10 out 1 in */
0,
'C','P','0','2',0,		/* 10 out 2 in */
'C','P','2','2',0,
'C','P','0','4',0,		/* 10 out 3 in */
'C','P','2','2',0,
0,						/* 10 out 4 in */
'C','P','2','2',0,
0,						/* 10 out 5 in */
'C','P','2','2',0,
0,						/* 10 out 6 in */
'C','P','2','2',0,
0,						/* 10 out 7 in */
'C','P','2','2',0,
0,						/* 10 out 8 in */
'C','P','2','2',0,
0,						/* 10 out 9 in */
'C','P','2','2',0,
'C','P','2','2',0,		/* 10 out 10 in */
'C','P','2','2',0,
'S',0,					/* 10 out 11 in */
0,
0,						/* 10 out 12 in */
'C','P','2','2',0,
'S',0,					/* 11 out 1 in */
0,
'C','P','0','2',0,		/* 11 out 2 in */
0,
'S',0,					/* 11 out 3 in */
0,
'S',0,					/* 11 out 4 in */
0,
'S',0,					/* 11 out 5 in */
0,
0,						/* 11 out 6 in */
0,
0,						/* 11 out 7 in */
0,
0,						/* 11 out 8 in */
0,
'S',0,					/* 11 out 9 in */
0,
'S',0,					/* 11 out 10 in */
0,
'S',0,					/* 11 out 11 in */
0,
'S',0,					/* 11 out 12 in */
0,
0,						/* 12 out 1 in */
0,
'C','P','0','2',0,		/* 12 out 2 in */
0,
'C','P','0','4',0,		/* 12 out 3 in */
0,
0,						/* 12 out 4 in */
0,
0,						/* 12 out 5 in */
0,
0,						/* 12 out 6 in */
0,
0,						/* 12 out 7 in */
0,
0,						/* 12 out 8 in */
0,
0,						/* 12 out 9 in */
0,
'C','P','2','2',0,		/* 12 out 10 in */
0,
'S',0,					/* 12 out 11 in */
0,
0,						/* 12 out 12 in */
0
};



/* convert the data base into rules */

void
	KCPChainSetup (	initializedGlobals_p	iGP)
{
KpInt32_t	i, j;
KpChar_p	bufPtr;

	if (iGP) {}

	bufPtr = composeRulesDB;	/* start of rules data base */

	for (j=0; j < NUM_OUTPUT_CLASSES; j++) {
		for (i = 0; i < NUM_INPUT_CLASSES; i++) {
			composeRule[i][j].start = bufPtr - composeRulesDB;
			bufPtr += (strlen (bufPtr) + EOLSIZE);	/* next rule */
			composeRule[i][j].finish = bufPtr - composeRulesDB;
			bufPtr += (strlen (bufPtr) + EOLSIZE);	/* next rule */
		}
	}

	return;
}


KpChar_p
	getChainRule (KpInt32_t iClass, KpInt32_t oClass, KpInt32_t getStartRule)
{
KpInt32_t	offset;

	if ((iClass < 1) || (iClass > NUM_INPUT_CLASSES) ||
		(oClass < 1) || (oClass > NUM_OUTPUT_CLASSES)) {
		
		return NULL;	/* invalid class */
	}

	if (getStartRule == 1) {
		offset = composeRule[iClass -1][oClass -1].start;	/* get start rule */
	}
	else {
		offset = composeRule[iClass -1][oClass -1].finish;	/* get finish rule */
	}

	return (composeRulesDB + offset);
}


/* get the current chaining state */
PTErr_t
	getChainState (chainState_p	FAR * cS)
{

	*cS = (chainState_p) KpThreadMemFind (&theRootID, KPTHREADMEM);
	if (*cS == NULL) {
		*cS = KpThreadMemCreate (&theRootID, KPTHREADMEM, sizeof(chainState_t));
		if (*cS == NULL) {
			return KCP_NO_THREAD_GLOBAL_MEM;
		}

		KpMemSet (*cS, 0, sizeof (chainState_t));
	}

	return KCP_SUCCESS;
}


/* put the current chaining state */
PTErr_t
	putChainState (	chainState_p	cS)
{
	if (cS) {}

	KpThreadMemUnlock (&theRootID, KPTHREADMEM);

	return KCP_SUCCESS;
}


/* abort the current PT chaining function */
void
	clearChain (	chainState_p	cS)
{
	PTCheckOut (cS->currentPT);			/* free the current PT */

	KpMemSet (cS, 0, sizeof (chainState_t));	/* no chaining active */

	KpThreadMemDestroy (&theRootID, KPTHREADMEM);
}


void
	addCompType (KpInt32_p kcpFlavor)
{
	*kcpFlavor |= PT_COMPOSITION2;
}
