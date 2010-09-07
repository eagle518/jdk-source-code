/*
 * @(#)dcPool.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPool.h 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#ifndef _DC_POOL_H
#define _DC_POOL_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct dcPoolData_*	dcPool;

extern void			dcPool_staticInitialize	(doeE);
extern void			dcPool_staticFinalize	(doeE);

extern dcPool			dcPool_create	(doeE,
						 char* poolName,
						 ixx itemBytes, ixx initialItems,
						 f32 xSigma);
extern void*			dcPool_getItem	(doeE, dcPool);
extern void			dcPool_staticReleaseItem
						(doeE, void*);
extern void			dcPool_endCycle	(doeE, dcPool);
extern void			dcPool_destroy	(doeE, dcPool);
#ifdef	__cplusplus
}
#endif

#endif /* _DC_POOL_H */

