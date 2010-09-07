/*
 * @(#)spcback.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1997 - 2000               ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

/*  spcback.h

This file provides macros which allow the same code to work for both 
PPC and non-PPC. It helps setting up Universal Proc Pointers for Mac 
PPC Mixed Mode.

It is used by the app to set up a Universal Proc for a callBack and 
by the profile to call the callBack.

	Windows Revision Level:
		$Workfile: $
		$Logfile: $
		$Revision: $
		$Date: $
		$Author: $

	SCCS Info:
		@(#)spcback.h	1.7  12/22/97
*/

#ifndef _SPCBACK_H_
#define _SPCBACK_H_


#if defined (KPMACPPC) && !(TARGET_API_MAC_CARBON)

typedef  UniversalProcPtr spCallBackUPP;
enum {
	uppspProgressProcInfo = kCStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(SpProgress_t)))  
			| STACK_ROUTINE_PARAMETER(1, (SIZE_CODE(sizeof(SpIterState_t))))  /*theState */
			| STACK_ROUTINE_PARAMETER(2, (SIZE_CODE(sizeof(KpInt32_t))))  /*percent */
			| STACK_ROUTINE_PARAMETER(3, (SIZE_CODE(sizeof(void FAR *))))  /*theData */
};
#define CallSPCallBackFunc(userRoutine,theState, thePercent, theData) \
		CallUniversalProc((spCallBackUPP)userRoutine,uppspProgressProcInfo,theState, thePercent, theData)

#define NewSPCallbackFunc(userRoutine)		\
		 (spCallBackUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppspProgressProcInfo,  GetCurrentISA())




typedef  UniversalProcPtr spIterCallBackUPP;
enum {
	uppSPIterProcInfo = kCStackBased
			| RESULT_SIZE(SIZE_CODE(sizeof(SpStatus_t)))  
			| STACK_ROUTINE_PARAMETER(1, (SIZE_CODE(sizeof(SpIterState_t))))  /*theState */
			| STACK_ROUTINE_PARAMETER(2, (SIZE_CODE(sizeof(SpProfile_t))))  /*profile */
			| STACK_ROUTINE_PARAMETER(3, (SIZE_CODE(sizeof(SpTagId_t))))  /*tagID */
			| STACK_ROUTINE_PARAMETER(4, (SIZE_CODE(sizeof(void FAR *))))  /*theData */
};
#define CallSPIterCallBackFunc(userRoutine,theState, Profile, TadID, theData) \
		CallUniversalProc((spIterCallBackUPP)userRoutine,uppSPIterProcInfo,theState, Profile, TadID, theData)

#define NewSPIterCallbackFunc(userRoutine)		\
		 (spIterCallBackUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), uppSPIterProcInfo,  GetCurrentISA())


#define SpDisposeRoutineDescriptor(SpCallBackUPP)	\
			DisposeRoutineDescriptor((UniversalProcPtr)SpCallBackUPP);	


#else

typedef SpProgress_t spCallBackUPP;

#define CallSPCallBackFunc(userRoutine,theState, thePercent, theData) \
		(*userRoutine)(theState, thePercent, theData)
 
#define NewSPCallbackFunc(userRoutine)		\
		(spCallBackUPP)(userRoutine)


typedef SpTagIter_t spIterCallBackUPP;

#define CallSPIterCallBackFunc(userRoutine,theState, Profile, TadID, theData) \
		(*userRoutine)(theState, Profile, TadID,theData)
 
#define NewSPIterCallbackFunc(userRoutine)		\
		(spIterCallBackUPP)(userRoutine)
		

#define SpDisposeRoutineDescriptor(SpCallbackUPP)

#endif

#if defined (KPMAC)
void SPretrieveCallbackA4A5 ( KpInt32_t  *theA4, KpInt32_t  *theA5);
void SPsaveCallbackA4A5 (KpInt32_t theA4, KpInt32_t theA5);
#endif

#endif /*_spCALLBACK_H_*/
