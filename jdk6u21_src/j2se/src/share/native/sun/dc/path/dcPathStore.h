/*
 * @(#)dcPathStore.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathStore.h 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#ifndef _DC_PATH_STORE_H
#define _DC_PATH_STORE_H

#include "dcPathConsumer.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct dcPathStoreFace_**	dcPathStore;
typedef struct dcPathStoreFace_ {
    dcPathConsumerFace	mu;
    dcFastPathProducer	(*getFastPathProducer)(doeE, dcPathStore);
    void		(*reset)(doeE, dcPathStore);
} dcPathStoreFace;

extern void		dcPathStore_staticInitialize(doeE);
extern void		dcPathStore_staticFinalize  (doeE);

extern dcPathStore	dcPathStore_create(doeE);

#ifdef	__cplusplus
}
#endif

#endif  /* _DC_PATH_STORE_H */

