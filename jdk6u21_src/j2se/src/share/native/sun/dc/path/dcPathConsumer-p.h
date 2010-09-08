/*
 * @(#)dcPathConsumer-p.h	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathConsumer-p.h 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#ifndef _DC_PATH_CONSUMER_P_H
#define _DC_PATH_CONSUMER_P_H

#include "doeObject-p.h"
#include "dcPathConsumer.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern dcPathConsumerFace   dcPathConsumerClass;


/*
 * No data purely specific to dcPathConsumer exists,
 * the class is really a "pure interface" in disguise.
 */
typedef struct dcPathConsumerData_ {
    doeObjectData	obj;
} dcPathConsumerData;


extern void    dcPathConsumer_init(doeE, dcPathConsumer);
extern void    dcPathConsumer_copyinit(doeE, dcPathConsumer, dcPathConsumer src);

extern dcPathConsumerFace dcPathConsumerClass;

#ifdef	__cplusplus
}
#endif

#endif  /* _DC_PATH_CONSUMER_P_H */
