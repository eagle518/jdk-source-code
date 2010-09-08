/*
 * @(#)dcPool.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

