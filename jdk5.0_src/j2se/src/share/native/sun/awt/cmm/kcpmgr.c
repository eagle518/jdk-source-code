/*
 * @(#)kcpmgr.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)kcpmgr.c	2.110 99/03/04

	Contains:       execute a KCMS function which was initially called through KCMS_proc_send

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1991-2003
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#define PTGLOBAL

#include "kcms_sys.h"

#include <string.h>

#include "kcmptlib.h"
#include "kcmptdef.h"
#include "kcptmgr.h"
#if defined (KCP_ACCEL)
#include "ctelib.h"
#endif

KpThreadMemHdl_t	theRootID;

static initializedGlobals_t	initializedGlobals = {KPFALSE};	/* initialized globals NOT YET */

/*--------------------------------------------------------------------
 * FUNCTION NAME
 * PTGetFlavor
 *
 * DESCRIPTION
 * This function returns the Color Processor flavor
 *
 *--------------------------------------------------------------------*/

PTErr_t PTGetFlavor (KpInt32_p kcpFlavor)
{
KpInt32_t		ptFlavor = 0;

	addCompType (&ptFlavor);

#if defined (KCP_ACCEL)
	ptFlavor |= PT_ACCELERATOR;
#endif

	ptFlavor |= PT_TLI_EVALUATION1;	/* determine the evaluation type */

	*kcpFlavor = ptFlavor;
	
	return (KCP_SUCCESS);
}


/* set up the main driver */
KpInt32_t
	KCMDsetup (KpGenericPtr_t FAR* theIGPtr)
{
#if defined (KCP_ACCEL)
PTErr_t 	PTErr;
KpInt16_t	num;
#endif

	KpInitializeCriticalSection (&PTCacheCritFlag);

	KpMemSet (&initializedGlobals, 0, sizeof (initializedGlobals_t));	/* set them all to 0 */

	KCPInitIGblP (theIGPtr, &initializedGlobals);		/* do platform specific initialization */

	KCPChainSetup (&initializedGlobals);				/* initialize chaining rules */

	initializedGlobals.maxGridDim = KCP_GRID_DIM_EIGHT;	/* setup default pt size */

#if defined (KCP_ACCEL)
	PTErr = PTProcessorReset_cte ();		/* initialize the CTE driver */
	if (PTErr == KCP_SUCCESS) {
		PTErr = PTEvaluators_cte (&num);	/* check for the CTE driver */
		if ((PTErr == KCP_SUCCESS) && (num > 0)) {
			initializedGlobals.haveCTE = 1;				/* CTE is present */
		}
	}
#endif

	initializedGlobals.isInitialized = KPTRUE;		/* show evwerything is Initialized! */
	return (KCMS_SUCCESS);
}


PTErr_t
	KCMDTerminate (void)
{
	return (KCP_SUCCESS);
}


#if (defined KCP_SINGLE_EVAL_CACHE) || (defined KCP_MACPPC_MP)

/* set up the globals for each application */
KpInt32_t
	KCPappSetup (KpGenericPtr_t theIGPtr)
{
processGlobals_p	pGP;

	if (theIGPtr) {}

	/* set up the process globals */
	pGP = KpThreadMemCreate (&theRootID, KPPROCMEM, sizeof(processGlobals_t));
	if (pGP == NULL) {
		return (KCMS_FAIL);
	}	

	KpMemSet (pGP, 0, sizeof (processGlobals_t));	/* set them all to 0 */

	unloadProcessGlobals ();	/* Unlock this app's Globals */

	return KCMS_SUCCESS;
}


PTErr_t
	KpTermProcess (void)
{
processGlobals_p	pGP;

	pGP = loadProcessGlobals();
	if (pGP == NULL) {
		return KCP_FAILURE;
	}

	KpThreadMemUnlock (&theRootID, KPPROCMEM);
	KpThreadMemDestroy (&theRootID, KPPROCMEM);

	return (KCP_SUCCESS);
}


/* load the process globals */
/* these must be unloaded with unloadProcessGlobals when done */
processGlobals_p
	loadProcessGlobals (void)
{
processGlobals_p	pGP;

	pGP = (processGlobals_p) KpThreadMemFind (&theRootID, KPPROCMEM);

	return pGP;
}


/* unload the process globals */
void
	unloadProcessGlobals (void)
{
	KpThreadMemUnlock (&theRootID, KPPROCMEM);
}

#endif


#if !defined KPMAC
/* thread dummies, only for backward compatibility */
PTErr_t
	PTInitThread (void)
{
	return KCP_SUCCESS;
}


PTErr_t
	PTTermThread (void)
{
	return KCP_SUCCESS;
}

#endif


/* reset the color processor to post-initialization state */
PTErr_t
	PTProcessorReset (void)
{
PTErr_t PTErr = KCP_SUCCESS;

#if defined (KCP_ACCEL)
	PTErr = PTProcessorReset_cte(); 	/* reset CTE */
#endif

	return (PTErr);
}


PTErr_t
	PTTerminate (void)
{
PTErr_t	PTErr;

	KpDeleteCriticalSection (&PTCacheCritFlag);

	PTErr = PTTerminatePlatform ();

	return (PTErr);
}

PTErr_t
	PTGetMPState (KpUInt32_p MP_Available, KpUInt32_p MP_Used)
{
	initializedGlobals_p	iGblP;

#if defined (KCP_MACPPC_MP)
	KCPInitializeMP ();
#endif

	iGblP = getInitializedGlobals ();
	if (iGblP == NULL) {
		return KCP_NO_PROCESS_GLOBAL_MEM;
	}

	*MP_Available = iGblP->numProcessorsAvailable;
	*MP_Used = iGblP->numProcessors;

	return KCP_SUCCESS;
}

PTErr_t
	PTSetMPState (KpUInt32_t MP_Used)
{
	initializedGlobals_p	iGblP;

#if defined (KCP_MACPPC_MP)
	KCPInitializeMP ();
#endif

	iGblP = getInitializedGlobals ();
	if (iGblP == NULL) {
		return KCP_NO_PROCESS_GLOBAL_MEM;
	}

	if (MP_Used > iGblP->numProcessorsAvailable) {
		iGblP->numProcessors = iGblP->numProcessorsAvailable; /* limit it to numProcessorsAvailable */
	} else if (MP_Used < 1) {
		iGblP->numProcessors = 1;
	} else {
		iGblP->numProcessors = MP_Used;
	}

	return KCP_SUCCESS;
}

initializedGlobals_p
	getInitializedGlobals (void)
{
	if (initializedGlobals.isInitialized == KPTRUE) {
		return &initializedGlobals;
	} else {
		return NULL;
	}
}


#if defined KCP_DIAG_LOG

#if defined KCP_MULTIPLE_LIB
/* write a string to the diagnostic log file */
void
	kcpDiagLog (	KpChar_p	string)
{
initializedGlobals_p	iGP;
KpChar_t				diagName[256];
KpFileId				fd;
KpFileProps_t			fileProps;

	iGP = getInitializedGlobals ();
	if (iGP != NULL) {
		SetKCPDataDirProps (&fileProps);

#if defined (KPMAC) || defined (KPMSMAC)
		strcpy (fileProps.fileType, "TEXT");
		strcpy (diagName, iGP->KCPDataDir);	/* make the full name */
		strcat (diagName, "kcpdiag.log");
#else
		strcpy (diagName, "kcpdiag.log");
#endif

		if (KpFileOpen (diagName, "a", &fileProps, &fd)) {
			KpFileWrite (fd, string, strlen (string));
			(void) KpFileClose (fd);
		}
	}
}
#endif

void
	saveFut (	fut_p		futp,
			 	KpChar_p	name)
{
KpFileProps_t	fileProps;
KpChar_t		fullName[256];
initializedGlobals_p	iGP;

	SetKCPDataDirProps (&fileProps);

	iGP = getInitializedGlobals ();

	if (iGP != NULL) {
		strcpy (fullName, iGP->KCPDataDir);	/* make the full name */
		strcat (fullName, name);

		makeFutTblDat (futp);	/* make the fut data tables */

		fut_store_fp (futp, fullName, fileProps);
	}
}

#endif
