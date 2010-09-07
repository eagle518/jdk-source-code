/*
 * @(#)kcpfchn.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)kcpfchn.h	2.31 99/01/06

	Contains:	header file for fut chaining in KCM driver

	COPYRIGHT (c) 1991-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#ifndef _KCMFCHAIN_H_
#define _KCMFCHAIN_H_ 1

#include "kcptmgr.h"

#define RULE_SERIAL_EVAL 'S'	/* rule control to indicate Serial Evaluation */
#define NUM_INPUT_CLASSES (12)	/* # of input chaining classes */
#define NUM_OUTPUT_CLASSES (12)	/* # of output chaining classes */

typedef struct chainState_s {
	KpInt32_t	compMode;					/* composition mode for entire chain */
	KpInt32_t	chainLength;				/* # of PTs in the chain */
	KpInt32_t	chainIndex;					/* current position in chain */
	PTRefNum_t	currentPT;					/* current result of PT chain */
	KpInt32_t 	iComp;						/* input chain rule of first PT in chain */
	KpInt32_t 	oComp;						/* output chain rule of last PT in chain */
	KpInt32_t 	inSense;					/* input sense of first PT in chain */
	KpInt32_t 	outSense;					/* output sense of last PT in chain */
	PTRefNum_t	chainDef[MAX_PT_CHAIN_SIZE];	/* list of PTs to chain */
} chainState_t, FAR* chainState_p;

PTErr_t loadAuxPT (const KpChar_p, KpInt32_t, PTRefNum_p);
KpChar_p getChainRule (KpInt32_t, KpInt32_t, KpInt32_t);
void clearChain (chainState_p);
PTErr_t getChainState (chainState_p FAR*);
PTErr_t putChainState (chainState_p);

#endif
