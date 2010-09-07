/*
 * @(#)dcPathStore-p.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathStore-p.h 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#ifndef _DC_PATH_STORE_P_H
#define _DC_PATH_STORE_P_H

#include "dcPathStore.h"
#include "dcPathConsumer-p.h"
#include "dcPool.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct dcPathStoreItemFace_**	dcPathStoreItem;

typedef struct dcPathStoreData_ {
    dcPathConsumerData	mu;
    bool		inPath;
    bool		inSubpath;
    bool		pathDefined;

    dcPathStoreItem 	itemFirst, itemLast;

    f32			loX, loY, hiX, hiY;

    dcFastPathProducer	proxy;
    dcFastPathProducer	stored;

    /* pools */
    dcPool		beginSubpathPool;
    dcPool		appendLinePool;
    dcPool		appendQuadraticPool;
    dcPool		appendCubicPool;
    dcPool		closedSubpathPool;
} dcPathStoreData;

extern void	dcPathStore_init(doeE, dcPathStore);
extern void	dcPathStore_copyinit(doeE, dcPathStore, dcPathStore src);

extern dcPathStoreFace	dcPathStoreClass;

extern void	dcPathStore_beginPath(		doeE, dcPathConsumer);
extern void	dcPathStore_beginSubpath(	doeE, dcPathConsumer,	f32 x0, f32 y0);
extern void	dcPathStore_appendLine(		doeE, dcPathConsumer,	f32 x1, f32 y1);
extern void	dcPathStore_appendQuadratic(	doeE, dcPathConsumer,	f32 x1, f32 y1,
									f32 x2, f32 y2);
extern void	dcPathStore_appendCubic(	doeE, dcPathConsumer,	f32 x1, f32 y1,
									f32 x2, f32 y2,
									f32 x3, f32 y3);
extern void	dcPathStore_closedSubpath(	doeE, dcPathConsumer);
extern void	dcPathStore_endPath(		doeE, dcPathConsumer);
extern void	dcPathStore_useProxy(		doeE, dcPathConsumer,
						dcFastPathProducer proxy);
#define dcPathStore_PARENT_METHODS	\
    dcPathStore_beginPath,		\
    dcPathStore_beginSubpath,		\
    dcPathStore_appendLine,		\
    dcPathStore_appendQuadratic,	\
    dcPathStore_appendCubic,		\
    dcPathStore_closedSubpath,		\
    dcPathStore_endPath,		\
    dcPathStore_useProxy

#define dcPathStore_SELF_METHODS	\
    dcPathStore_getFastPathProducer,	\
    dcPathStore_reset

extern dcFastPathProducer
		dcPathStore_getFastPathProducer(doeE, dcPathStore);
extern void	dcPathStore_reset(doeE, dcPathStore);

#ifdef	__cplusplus
}
#endif

#endif  /* _DC_PATH_STORE_P_H */

