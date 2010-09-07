/*
 * @(#)spsystem.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the system functions.

				Created by lsh, September 20, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2002 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: spsystem.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spsystem.c $
		$Revision: 12 $
		$Date: 6/20/02 9:36a $
		$Author: Msm $

	SCCS Revision:
		@(#)spsystem.c	1.41 2/16/99

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993-1998                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"
#include "attrcipg.h"
#include <string.h>

#include "spuxver.h"

#if defined (KPWIN32)
extern SpCallerId_t ICMCallerId;
#endif

static SpStatus_t SpGetInstanceGlobals (SpInstanceGlobals_p *instanceGlobals);
static SpStatus_t KSPAPI SpDoTerminate (SpCallerId_t FAR *CallerId);

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Returns the globals associated with this process.  If the globals
 *	do not exist, then create a new set.
 *------------------------------------------------------------------*/
static SpStatus_t SpGetInstanceGlobals (SpInstanceGlobals_p *instanceGlobals)
{
	SpInstanceGlobals_p	thisInstanceP;

	thisInstanceP = (SpInstanceGlobals_p)KpThreadMemFind (	&ICCRootInstanceID,
															KPPROCMEM);

	if (thisInstanceP == NULL)
	{
		thisInstanceP = KpThreadMemCreate (	&ICCRootInstanceID,
											KPPROCMEM,
											sizeof(SpInstanceGlobals_t));

		if (thisInstanceP != NULL)
		{
			thisInstanceP->currentUsers = 0;
			thisInstanceP->NextUsers = 0;
		}
		else
		{
			return (SpStatMemory);
		}
		
	}
	
	*instanceGlobals = thisInstanceP;

	return (SpStatSuccess);

}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do application specific initialization.  This must be the first
 *	Profile Processor function called.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t SpCallerIdValidate (
				SpCallerId_t	CallerId)
{
	SpCallerIdData_t	FAR *CallerIdData;
	SpStatus_t			status;

	status = SpStatSuccess;
	
	CallerIdData = lockBuffer ((KcmHandle) CallerId);
	if (NULL == CallerIdData)
		status = SpStatBadCallerId;
	else {
		if (SpCallerIdDataSig != CallerIdData->Signature)
			status = SpStatBadCallerId;
		unlockBuffer ((KcmHandle) CallerId);
	}
	
	return status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do application specific initialization.  This must be the first
 *	Profile Processor function called.
 *
 * AUTHOR
 * 	msm
 *
 * DATE CREATED
 *	April 4, 2000
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpInitCMS (
				SpCallerId_t	FAR *CallerId,
				SpProgress_t 	ProgressFunc,
				void			FAR *Data,
				KpMemoryData_t	MemoryData)
{
#if defined (KPWIN32)
	SpStatus_t	status;

	status = SpTerminate(&ICMCallerId);
	if (status != SpStatSuccess)
	{
		return (status);
	}
	ICMCallerId = 0;		/* no need to worry about ICM since we are here */
#endif

	KpUseAppMem (MemoryData);

#if defined (KPMAC)
	PTInitCMS (MemoryData);
#endif

	return (SpInitialize (CallerId, ProgressFunc, Data));
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do application specific initialization.  This must be the first
 *	Profile Processor function called.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpInitialize (
				SpCallerId_t	FAR *CallerId,
				SpProgress_t	ProgressFunc,
				void			FAR *Data)
{
#if defined (KPWIN)
	return (SpInitializeEx (CallerId, ProgressFunc,
					Data, (SpInitInfo_t *)NULL));
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do extended application specific initialization.  This must be 
 *  the first Profile Processor function called.  This function
 *  takes the place of SpInitialize.  It should only be used
 *  when the application needs to provide additional infomation
 *  to the sprofile dll at initialization time.
 *
 *  The moduleId provided is passed to the CP which uses this
 *  Id to identify the app or dll which contains the resources
 *  that are read by the CP.  This allows the application to
 *  use unique registry entries.
 *
 * AUTHOR
 * 	acr
 *
 * DATE CREATED
 *	June 13, 1996
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpInitializeEx (
				SpCallerId_t	FAR *CallerId,
				SpProgress_t	ProgressFunc,
				void			FAR *Data,
				SpInitInfo_t	FAR *InitInfo)
{
#endif
	SpCallerIdData_t	FAR *CallerIdData;
	PTErr_t				PTStat;
	SpStatus_t			status;
	SpInstanceGlobals_p	thisInstanceP;
#if defined (KPWIN)
	PTInitInfo_t		PTInitInfo;
#endif

	SpDoProgress (ProgressFunc, SpIterInit, 0, Data);

/* Get the globals */
	status = SpGetInstanceGlobals (&thisInstanceP);
	if (status != SpStatSuccess)
	{
		return (status);
	}

/* check for no current users, do global initialization */
	if (0 == thisInstanceP->currentUsers) {
#if defined (KPWIN)
		if (InitInfo == NULL)
			PTStat = PTInitialize ();
		else {
			PTInitInfo.structSize = sizeof(PTInitInfo_t);
			PTInitInfo.appModuleId = InitInfo->appModuleId;
			PTStat = PTInitializeEx (&PTInitInfo);
		}
#else
		PTStat = PTInitialize ();
#endif
		if (KCP_SUCCESS != PTStat) {
			SpDoProgress (ProgressFunc, SpIterTerm, 100, Data);
			return SpStatusFromPTErr(PTStat);
		}
		/* current and next Users are zero'd out when the
		   instance global memory is allocated.  NextUsers
		   is never set to zero nor decremented so only the
		   creation of the instance has NextUser zero.  It is
		   incremented below */
		if (thisInstanceP->NextUsers == 0)
		{
			Sp_uvL2Lab.Valid = KPFALSE;
			Sp_Lab2uvL.Valid = KPFALSE;
		}
		KpInitializeCriticalSection (&SpCacheCritFlag);
	}
	SpDoProgress (ProgressFunc, SpIterProcessing, 40, Data);

/* allocate caller id block */
	CallerIdData = SpMalloc (sizeof (*CallerIdData));
	if (NULL == CallerIdData) {
		SpDoProgress (ProgressFunc, SpIterTerm, 100, Data);
		return SpStatMemory;
	}
	SpDoProgress (ProgressFunc, SpIterProcessing, 80, Data);
	CallerIdData->Signature = SpCallerIdDataSig;

/* give the user a pointer to this block */
	*CallerId =  (SpCallerId_t) getHandleFromPtr (CallerIdData);

/* increase the number of current users */
	thisInstanceP->currentUsers++;
	thisInstanceP->NextUsers++;

/* set the caller number */
	CallerIdData->CallerId = thisInstanceP->NextUsers;
	unlockBuffer((KcmHandle)*CallerId);
	
/* Unlock the thread memory */
	KpThreadMemUnlock (&ICCRootInstanceID, KPPROCMEM);

	SpDoProgress (ProgressFunc, SpIterTerm, 100, Data);

	return SpStatSuccess;
}




/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do application specific cleanup.  This must be the last
 *	Profile Processor function called.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
static SpStatus_t KSPAPI SpDoTerminate (
				SpCallerId_t FAR *CallerId)
{
	SpInstanceGlobals_p	thisInstanceP;
	SpStatus_t			status;
	SpCallerIdData_t	FAR *CallerIdData;

	if (NULL == CallerId)
		return SpStatBadCallerId;

/* free caller id space */
	CallerIdData = lockBuffer ((KcmHandle) *CallerId);
	if (NULL == CallerIdData)
		return SpStatBadCallerId;
	
	SpFree ((void FAR *) CallerIdData);

/* null users handle, don't let them continue with a freed block */
	*CallerId = NULL;

/* decrease the number of current users */
	status = SpGetInstanceGlobals (&thisInstanceP);
	if (status != SpStatSuccess)
	{
		return (status);
	}
	else
	{
		thisInstanceP->currentUsers--;
	}


/* check for no more current users, do global clean-up */
	if (0 == thisInstanceP->currentUsers) {
		if (Sp_Lab2uvL.Valid) {
			PTCheckOut (Sp_Lab2uvL.RefNum);
			Sp_Lab2uvL.Valid = KPFALSE;
		}
		if (Sp_uvL2Lab.Valid) {
			PTCheckOut (Sp_uvL2Lab.RefNum);
			Sp_uvL2Lab.Valid = KPFALSE;
		}
		KpDeleteCriticalSection (&SpCacheCritFlag);
	}

/* Unlock the thread memory */
	KpThreadMemUnlock (&ICCRootInstanceID, KPPROCMEM);

	return SpStatSuccess;
}



SpStatus_t SpStatusFromPTErr(PTErr_t PTErr)
{
SpStatus_t theStatus;

	switch	(PTErr)
	{
		case KCP_NOT_IMPLEMENTED:
			theStatus = SpStatNotImp;
			break;
			
		case KCP_SUCCESS:
			theStatus =SpStatSuccess;
			break;

		case KCP_NO_CHECKIN_MEM:
		case KCP_NO_ACTIVATE_MEM:
		case KCP_NO_ATTR_MEM:
		case KCP_NO_MEMORY:
		case KCP_NO_SYSMEM:
		case KCP_NO_PROCESS_GLOBAL_MEM:
		case KCP_NO_THREAD_GLOBAL_MEM:
		case KCP_MEM_LOCK_ERR:
		case KCP_MEM_UNLOCK_ERR:
		case KCP_BAD_PTR:
			theStatus = SpStatMemory;
			break;

		case KCP_PT_BLOCK_TOO_SMALL:
		case KCP_ATT_SIZE_TOO_SMALL:
		case KCP_ATTR_TOO_BIG:
			theStatus = SpStatBufferTooSmall;
			break;

		case KCP_BAD_COMP_ATTR:
			theStatus = SpStatIncompatibleArguments;
			break;

		case KCP_BAD_ARG:
			theStatus = SpStatOutOfRange;
			break;

		case KCP_PT_HDR_WRITE_ERR:
		case KCP_PT_DATA_WRITE_ERR:
			theStatus = SpStatFileWriteError;
			break;

		case KCP_PT_DATA_READ_ERR:
			theStatus = SpStatFileReadError;
			break;

		default:
			theStatus = SpStatCPComp;
			break;
	}
	return theStatus;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do application specific cleanup.  This must be the last
 *	Profile Processor function called.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTerminate (
				SpCallerId_t FAR *CallerId)
{
	SpInstanceGlobals_p	thisInstanceP;
	SpStatus_t  spStatus;
	
	/* Get the globals */
	spStatus = SpGetInstanceGlobals (&thisInstanceP);
	if (spStatus != SpStatSuccess)
	{
		return (spStatus);
	}

	/* Unlock the thread memory */
	spStatus = SpDoTerminate(CallerId);
	if (spStatus != SpStatSuccess)
		return spStatus;
	
	if (0 == thisInstanceP->currentUsers)
	{
		/* Close the Color Processor */
		PTTerminate ();
		/* Remove thread memory */
		KpThreadMemDestroy (&ICCRootInstanceID, KPPROCMEM);
	}
		
	return spStatus;
}

#if defined (KPMAC)
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do application specific initialization.  This must be the first
 *	Profile Processor function called.  If the CPInstance is not 
 *  zero - then we want to share the one initialized by the KCM API
 *  so set the value before initializing.
 *
 * AUTHOR
 * 	mec
 *
 * DATE CREATED
 *	July 14, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpInitCMSComp (	KpInt32_t CPInstance,
					SpCallerId_t FAR *CallerId,
					SpProgress_t ProgressFunc,
					void FAR *Data,
					KpMemoryData_t	MemoryData)
{
	if (CPInstance != 0)
	{
		SpSetCPInstance (CPInstance);
	}
	
	return SpInitCMS (CallerId, ProgressFunc, Data, MemoryData);

}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do application specific initialization.  This must be the first
 *	Profile Processor function called.  If the CPInstance is not 
 *  zero - then we want to share the one initialized by the KCM API
 *  so set the value before initializing.
 *
 * AUTHOR
 * 	mec
 *
 * DATE CREATED
 *	July 14, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpInitializeComp (	KpInt32_t CPInstance,
					SpCallerId_t FAR *CallerId,
					SpProgress_t ProgressFunc,
					void FAR *Data)
{
	if (CPInstance != 0)
	{
		SpSetCPInstance (CPInstance);
	}
	
	return SpInitialize (CallerId,ProgressFunc,Data);

}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do application specific cleanup.  This must be the last
 *	Profile Processor function called. It is called in place of
 *  SpTerminate.  If the value of CPJointInstance is not zero, then the
 *  the application has initialize the KCM API and the Profile
 *  API is using its  (the KCM's) instance of the CP.  This is necessary 
 *  in order for the two API to use the same refnum.  The Profile API MUST 
 *  not close the CP  - KCM API will close it when it terminates.
 *  
 *
 * AUTHOR
 * 	mec
 *
 * DATE CREATED
 *	July 14, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTerminateComp (	SpCallerId_t FAR *CallerId,
					KpInt32_t CPJointInstance)
{
	SpInstanceGlobals_p	thisInstanceP;
	SpStatus_t			spStatus;
	PTErr_t				ptError;
	
	
	/* Get the globals */
	spStatus = SpGetInstanceGlobals (&thisInstanceP);
	if (spStatus != SpStatSuccess)
	{
		return (spStatus);
	}

	spStatus = SpDoTerminate (CallerId);
	if (spStatus != SpStatSuccess)
		return spStatus;
	
	/* If the CPJointInstance is not zero then the
	 			KCM API initialized and will close the CP */
	if (0 == thisInstanceP->currentUsers)
	{
		if (0 == CPJointInstance)
		{
			/* Close the Color Processor */
			PTTerminate ();
		}
		else
		{
			/* Free the CP's thread memory */
			ptError = PTTermGlue ();
		}

		/* Unlock the thread memory */
		KpThreadMemUnlock (&ICCRootInstanceID, KPPROCMEM);
		/* Remove thread memory */
		KpThreadMemDestroy (&ICCRootInstanceID, KPPROCMEM);
	}

	return spStatus;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	MACINTOSH VERSION OF SpInitThread() - DOES NOTHING TODAY!
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	November 23, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpInitThread (
				SpCallerId_t	/*CallerId*/,
				SpProgress_t	/*ProgressFunc*/,
				void			FAR */*Data*/)
{
	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	MACINTOSH VERSION OF SpTermThread() - DOES NOTHING TODAY!
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	November 23, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTermThread (
				SpCallerId_t /*CallerId*/)
{
	return SpStatSuccess;
}

#else


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do thread specific initialization.  This must be the first
 *	Profile Processor function called from a thread.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 26, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpInitThread (
				SpCallerId_t	CallerId,
				SpProgress_t	ProgressFunc,
				void			FAR *Data)
{
	PTErr_t		PTStat;

	SPArgUsed (CallerId);

	SpDoProgress (ProgressFunc, SpIterInit, 0, Data);
	PTStat = PTInitThread ();
	SpDoProgress (ProgressFunc, SpIterTerm, 100, Data);

	return SpStatusFromPTErr(PTStat);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do thread specific cleanup.  This must be the last
 *	Profile Processor function called from a thread.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 26, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTermThread (
				SpCallerId_t CallerId)
{
	PTErr_t		PTStat;

	SPArgUsed (CallerId);

	PTStat = PTTermThread ();

	return SpStatusFromPTErr(PTStat);
}
#endif

