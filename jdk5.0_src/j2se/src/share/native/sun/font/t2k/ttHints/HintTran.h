/*
 * @(#)HintTran.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Copyright:	© 1990-1993 by Apple Computer, Inc., all rights reserved.  */
 

#ifndef	TTHintTranIncludes
#define	TTHintTranIncludes

#include "Hint.h"

/* TTHintTran.c, 
	 Replaces FontScaler.c's TTScalerNewTransform. Also adds a Close routine.
 
	 MTE: This routine was originally designed to perform many activities
			1. Still Done.	Allocate maximal storate ( twilight, storage, ...)
			2. Still Done.	Allocate maximal storate ( twilight, storage, ...)

*/

void TTScalerTTHintTran(
 	const scalerTransform *theTransform,	/*  Specify a Input Transformation?*/
 	perFont * aPerFont,						/*  Specify the context */
 	perTransformation  * aPerTransContext 	/*  Return a pointer to the perTrans*/
 	) ;
void TTScalerCloseTransform(perTransformation *aPerTransContext);

 
/*  Bring up the default variation- it does nothing.*/
void TTScalerTTHintTranDefault(
 		perFont * aPerFont,						/*  Specify the context */
		perTransformation  *aPerTransContext);

/*  Call this once at program end to cleanup the context.
    Currently, there is no cleanup required.
void TTScalerCloseTransform(perTransformation *aPerTransContext); */
 
/* Now the interface routines which are called by T2K */
	void InitTTHintTranForT2K(T2K *	aT2KScaler);
	void NewTTHintTranForT2K(T2K *	aT2KScaler);
	void ReleaseTTHintTranForT2K(T2K *	aT2KScaler);
	

#endif
