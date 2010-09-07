/*
 * @(#)kcpmgru.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)kcpmgru.c	2.40 98/12/01

	Contains:	KCM Driver utility routines

	Windows Revision Level:
		$Workfile:  $
		$Logfile:  $
		$Revision:  $
		$Date:  $
		$Author:  $

	COPYRIGHT (c) 1992-2000 Eastman Kodak Company.
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "kcms_sys.h"

#include "kcmptlib.h"
#include "kcptmgrd.h"
#include "kcptmgr.h"

#include <string.h>

#if defined (KPMAC)

#include <Types.h>
#include <OSUtils.h>

#endif

#if defined (KPMACPPC) && !(TARGET_API_MAC_CARBON)

#include <Traps.h>

/* declare the universal procedure pointer info */
enum {
	uppCallProgressProcInfo =
		kCStackBased |
		RESULT_SIZE (SIZE_CODE (sizeof (PTErr_t) ) ) |
		STACK_ROUTINE_PARAMETER (1, SIZE_CODE (sizeof (KpInt32_t) ) ),

	upprelayProcInfo =
		kCStackBased |
		RESULT_SIZE (SIZE_CODE (sizeof (PTErr_t) ) ) |
		STACK_ROUTINE_PARAMETER (1, SIZE_CODE (sizeof (long) ) ) |
		STACK_ROUTINE_PARAMETER (2, SIZE_CODE (sizeof (long) ) ) |
		STACK_ROUTINE_PARAMETER (3, SIZE_CODE (sizeof (PTProgress_t) ) ) |
		STACK_ROUTINE_PARAMETER (4, SIZE_CODE (sizeof (KpInt32_t) ) )
};

#endif


#if defined (KPMAC) && !(TARGET_API_MAC_CARBON)

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * callProgress - Macintosh Version
 *
 * DESCRIPTION
 * This function calls the user's call back function
 * must be done as separate subroutine so that progress function
 * address is addressed via the A6 stack
 *
 *--------------------------------------------------------------------*/
static PTErr_t FAR PASCAL
	callProgress (	callBack_p	callBack,
					KpInt32_t	percent)
{
initializedGlobals_p	iGP;
PTRelay_t		relay;
PTErr_t 		theReturn;

#if defined (KPMACPPC)
long			thisA5;
#endif

	if (callBack->progressFunc == NULL) {
		return KCP_SUCCESS;
	}

	iGP = getInitializedGlobals ();
	if (iGP == NULL) {
		return KCP_NO_PROCESS_GLOBAL_MEM;
	}

	relay = iGP->callBackRelay;

	/* do the progress call-back */
#if defined (KPMACPPC)
	thisA5 = SetA5 (callBack->gHostA5);
	
		/* PPC or 68K callback code? */
	if ( ((UniversalProcPtr)callBack->progressFunc)->goMixedModeTrap == _MixedModeMagic) {
		theReturn = (PTErr_t) CallUniversalProc ((UniversalProcPtr)callBack->progressFunc,
												uppCallProgressProcInfo, percent);
	}
	else {						/* callback code is 68K */
		theReturn = (PTErr_t) CallUniversalProc ((UniversalProcPtr)relay, upprelayProcInfo,
						callBack->gHostA5, callBack->gHostA4, callBack->progressFunc, percent);
	}
	SetA5 (thisA5);
#endif
#if defined (KPMAC68K)
		theReturn = (PTErr_t)relay (callBack->gHostA5, callBack->gHostA4, callBack->progressFunc, percent);
#endif

	return (theReturn);
}

#else

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * callProgress - Windows and all others version
 *
 * DESCRIPTION
 * This function calls the user's call back function
 *
 *--------------------------------------------------------------------*/
static PTErr_t FAR PASCAL
	callProgress (	callBack_p	callBack,
					KpInt32_t	percent)
{
PTErr_t	theReturn = KCP_SUCCESS;

	if (callBack->progressFunc != NULL) {
		theReturn = callBack->progressFunc (percent);	/* do the progress call-back */
	}

	return theReturn;
}

#endif


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * initProgressPasses
 *
 * DESCRIPTION
 * This function sets up the total number of passes that will
 * be performed.
 *
 *--------------------------------------------------------------------*/
void
	initProgressPasses (	KpInt32_t	numPasses,
							callBack_p	callBack)
{
	if (callBack != NULL) {
		callBack->currPasses = 0;			/* number passes completed 	*/
		callBack->totalPasses = numPasses;	/* total passes to be done 	*/
		callBack->lastProg100 = KPFALSE;
	}
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * initProgress
 *
 * DESCRIPTION
 * This function sets up the progress call back function
 *
 *--------------------------------------------------------------------*/
void
	initProgress (	KpInt32_t	loopMax,
					callBack_p	callBack)
{
	if (callBack != NULL) {
		callBack->loopStart = loopMax;		/* define the repetition rate */
		callBack->loopCount = 0;			/* force call-back next time */
	}
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * doProgress
 *
 * DESCRIPTION
 * This function executes the call back function.
 *
 *--------------------------------------------------------------------*/
PTErr_t
	doProgress (	callBack_p	callBack,
					KpInt32_t	percent)
{
PTErr_t		theReturn = KCP_SUCCESS;
KpInt32_t	realPercent;

	if (callBack != NULL) {
		if ((callBack->loopCount <= 0) || (percent == 100)) {

			/* if previous percent was 100 and this one is not, go to next pass */
			if ((percent != 100) && callBack->lastProg100) {
				callBack->currPasses++;
				callBack->lastProg100 = KPFALSE;
			}

			if (callBack->progressFunc != NULL) {
				realPercent = ((callBack->currPasses * 100) + percent) / callBack->totalPasses;
			
				/* do the progress call-back */
				theReturn = callProgress (callBack, realPercent);

				/* see if this pass is done  - don't incr if multiple 100% in a row */
				if (percent == 100) {
					callBack->lastProg100 = KPTRUE;
				}
			}

			callBack->loopCount = callBack->loopStart;
		}
		else {
			callBack->loopCount--;
		}
	}
	
	return (theReturn);
}


/*--------------------------------------------------------------------
 * FUNCTION NAME
 * SetKCPDataDirProps
 *
 * DESCRIPTION
 * This function returns the Color Processor working directory
 *
 *--------------------------------------------------------------------*/

PTErr_t
	SetKCPDataDirProps (	KpFileProps_p	KCPDataDirProps)
{

#if defined (KPMAC)
	strcpy (KCPDataDirProps->creatorType, "KEPS");	/* set up file properties */
	strcpy (KCPDataDirProps->fileType, "PT  ");
	KpGetBlessed (&KCPDataDirProps->vRefNum, &KCPDataDirProps->dirID);	/* get the system folder volume reference number */
#else
	memset (KCPDataDirProps, 0, sizeof (KpFileProps_t));
#endif

	return (KCP_SUCCESS);
}

