/*
 * @(#)CJPathConsumer.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)CJPathConsumer.h 3.2 97/11/19
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#ifndef _CJ_PATH_CONSUMER_H
#define _CJ_PATH_CONSUMER_H

#include <jni.h>
#include "dcPathConsumer.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct CJPathConsumerFace_**		CJPathConsumer;
typedef struct CJPathConsumerFace_ {
    dcPathConsumerFace		mu;

    void	(*setJPathConsumer)	(doeE,	CJPathConsumer, jobject jpc);

} CJPathConsumerFace;

/* this path consumer forwards its call to its java counter object */
extern	CJPathConsumer		CJPathConsumer_create(doeE, jobject);

extern	void			CJPathConsumer_staticInitialize(doeE);
extern	void			CJPathConsumer_staticFinalize  (doeE);

#ifdef	__cplusplus
}
#endif

#endif  /* _CJ_PATH_CONSUMER_H */
