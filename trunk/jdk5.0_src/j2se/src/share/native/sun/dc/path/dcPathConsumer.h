/*
 * @(#)dcPathConsumer.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathConsumer.h 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#ifndef _DC_PATH_CONSUMER_H
#define _DC_PATH_CONSUMER_H

#include "doeObject.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct dcPathConsumerFace_**		dcPathConsumer;
typedef struct dcFastPathProducerFace_**	dcFastPathProducer;

typedef struct dcPathConsumerFace_ {
    doeObjectFace	mu;
    void		(*beginPath)	  (doeE, dcPathConsumer);
    void		(*beginSubpath)	  (doeE, dcPathConsumer, f32 x0, f32 y0);
    void		(*appendLine)	  (doeE, dcPathConsumer, f32 x1, f32 y1);
    void		(*appendQuadratic)(doeE, dcPathConsumer, f32 x1, f32 y1,
								  f32 x2, f32 y2);
    void		(*appendCubic)	  (doeE, dcPathConsumer, f32 x1, f32 y1,
								  f32 x2, f32 y2,
								  f32 x3, f32 y3);
    void		(*closedSubpath)  (doeE, dcPathConsumer);
    void		(*endPath)	  (doeE, dcPathConsumer);
    void		(*useProxy)	  (doeE, dcPathConsumer, dcFastPathProducer proxy);
} dcPathConsumerFace;

typedef struct dcFastPathProducerFace_ {
    doeObjectFace	mu;
    void		(*getBox)(doeE, dcFastPathProducer, f32* box);
    void		(*sendTo)(doeE, dcFastPathProducer, dcPathConsumer dest);
} dcFastPathProducerFace;

/* This dcPathConsumer simply ignores the path */
extern	dcPathConsumer	dcPathConsumer_create(doeE);

#ifdef	__cplusplus
}
#endif

#endif  /* _DC_PATH_CONSUMER_H */
