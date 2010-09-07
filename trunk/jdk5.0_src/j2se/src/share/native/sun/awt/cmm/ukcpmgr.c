/*
 * @(#)ukcpmgr.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ukcpmgr.c	1.19 98/07/17

	Contains:     UNIX only Color Processor management function
	
    COPYRIGHT (c) Eastman Kodak Company, 1991-2003
    As an unpublished  work pursuant to Title 17 of the United
    States Code.  All rights reserved.
*/


#include "kcms_sys.h"
#include "kcmptlib.h"
#include "kcptmgr.h"

#include <string.h>

#if defined (KPSUN) || defined (KPDU)
#include <dlfcn.h>
#include <unistd.h>
#endif

#if defined (KPSGI) || defined(KPSGIALL)
#include <unistd.h>
#endif



#if defined (KCP_ACCEL)
#include "ctelib.h"
#endif

int	KcpUsageCount = 0;
static	KpGenericPtr_t	IGPtr = NULL;

#if defined (KPSUN) && defined (KCP_ACCEL)
void	*kcp_cte_hnd;
KcpCteFunc    kcp_cte;
#endif


/*****************************************************/
/* open driver to load it into memory, then close it */
/*****************************************************/
PTErr_t
	PTInitialize (void)
{
KpInt32_t	retErr;

	if (KcpUsageCount == 0) {		/* first process */
		retErr = KCMDsetup (&IGPtr);	/* set up this shared library */
		if (retErr != KCMS_SUCCESS) {
			return KCP_NO_MEMORY;
		}
		KcpUsageCount++;
  	}

	return KCP_SUCCESS;
}

/* Initialize platform specific portions of the instance Globals */
void
KCPInitIGblP(KpGenericPtr_t FAR* theIGPtr, initializedGlobals_p iGP)
{
KpInt32_t	nProcessors;
#if defined(KCP_ACCEL)
	char	tempBuffer[100];
#endif

	if (theIGPtr) {}

	/* setup directory for CPxx files */
#if defined (KPSGI) || defined(KPSGIALL)
	strcpy (iGP->KCPDataDir, "/var/cms/cmscp/");
#elif defined (KPSUN) && defined (SOLARIS_CMM)
	strcpy (iGP->KCPDataDir, "/tmp/");
#elif defined (JAVACMM)
	strcpy(iGP->KCPDataDir, "/tmp/");
#else 
	strcpy (iGP->KCPDataDir, "/var/kodak/cmscp/");
#endif

#if defined(KCP_ACCEL)
	/* load shared library */
	kcp_cte_hnd = dlopen ("libkcme1.so.1", RTLD_NOW);
	if (kcp_cte_hnd != NULL){
		kcp_cte = (int (*)())dlsym (kcp_cte_hnd, "cte_proc_send");
		if (kcp_cte == NULL){
			strcpy (tempBuffer, "PTInitialize: ");
			strcat (tempBuffer, dlerror());
			diagWindow(tempBuffer, (int)kcp_cte_hnd);
			dlclose (kcp_cte_hnd);
			kcp_cte_hnd = NULL;
		}
	}
	else{
		strcpy (tempBuffer, "PTInitialize: "); 
		strcat (tempBuffer, dlerror()); 
		diagWindow(tempBuffer, (int)kcp_cte_hnd);
	}
#endif  /* end of KCP_ACCEL */

#if defined (KPSGI) || defined(KPSGIALL)
	nProcessors = sysconf(_SC_NPROC_ONLN);	/* get # of processors */
#elif defined (KPSUN)
	nProcessors = sysconf(_SC_NPROCESSORS_ONLN);	/* get # of processors */
#endif
#if defined (JAVACMM)
	if (thr_main() == -1) {
		nProcessors = 1;		/* no Java threading, can't use extra processors */
	}
#endif
	iGP->numProcessorsAvailable = iGP->numProcessors = nProcessors;
}


/* Platform specific terminate processing */
PTErr_t
	PTTerminatePlatform (void)
{
	if (KcpUsageCount > 0)
		KcpUsageCount--;

	return KCP_SUCCESS;
}
