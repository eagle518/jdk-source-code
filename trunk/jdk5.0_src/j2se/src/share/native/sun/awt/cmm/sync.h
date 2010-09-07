/*
 * @(#)sync.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the synchronization structures 
				and constants.

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1994 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: sync.h $
		$Logfile: /DLL/KodakCMS/kpsys_lib/sync.h $
		$Revision: 2 $
		$Date: 12/01/00 2:35p $
		$Author: Doro $

	SCCS Revision:
		@(#)sync.h	1.4    12/22/97

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1994                      ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#ifndef _SYNC_H_
#define _SYNC_H_ 1

	/***************************/
	/* Windows 32 Bit Includes */
	/***************************/

	#if defined (KPWIN32)
		#include <string.h>
	#endif

	/********************/
	/* Solaris Includes */
	/********************/

	#if defined (KPSOLARIS) || defined (KPLINUX)
	#if defined (KP_POSIX_THREADS)
		#include <pthread.h>
	#else
		#include <thread.h>
	#endif
		#include <stdlib.h>
		#include <sys/types.h>
		#include <sys/ipc.h>
		#include <sys/sem.h>
	#endif

	/*****************************************/
	/* Common type definitions and constants */
	/*****************************************/

	#define KP_SEM_USAGE		0		/*	semaphore 0 of all semaphore
										 	sets is the usage count 
											semaphore.						*/

	/***********************************************************/
	/* Windows 32 Bit Semaphore type definitions and constants */
	/***********************************************************/

	#if defined (KPWIN32)

		typedef HANDLE KpSemId_t, FAR * KpSemId_p, FAR * FAR * KpSemId_h;

		typedef struct {
			KpUInt32_t		NumSemaphores;
			KpSemId_h		semId;
		} KpSemSetData_t, FAR * KpSemSetData_p, FAR * FAR * KpSemSetData_h;

		typedef  KpInt32_t	KpSemInitData_t;

		#define KP_SEM_MAX_VAL	0x7fffffff		

	#endif

	/*************************************************/
	/* Unix Semaphore type definitions and constants */
	/*************************************************/

	#if defined (KPUNIX)

		typedef int KpSemId_t;

		typedef struct {
			KpUInt32_t	NumSemaphores;
			KpSemId_t	SemId;
		} KpSemSetData_t, FAR * KpSemSetData_p, FAR * FAR * KpSemSetData_h;

		typedef KpUInt16_t	KpSemInitData_t;			

		#define KP_IPC_PERM_RWALL 0666

		#define KP_GET_SEMAPHORE		-1
		#define KP_RELEASE_SEMAPHORE	1

		/*
	 	 * for some reason the following is not in the solaris include
		 * files although the documentation says it should be.
		 */

		#if defined (KPSOLARIS) || defined (KPLINUX)

			typedef union semun_u {
				int val;
				struct semid_ds *buf;
				ushort *array;
			} semun_t;
		
		#endif

	#endif

	/***********************/
	/* Function Prototypes */
	/***********************/
	

#endif


