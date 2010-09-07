/*
 * @(#)dcPathStroker.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathStroker.h 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _DC_PATH_STROKER_H
#define _DC_PATH_STROKER_H

#include "dcPathConsumer.h"

#ifdef	__cplusplus
extern "C" {
#endif



/*----------------------
 * The class definition
 */
typedef struct dcPathStrokerFace_**	dcPathStroker;


/*----------------------
 * The methods of dcPathStroker
 */
typedef struct dcPathStrokerFace_ {
    dcPathConsumerFace	mu;
    void	(*setPenDiameter)(doeE, dcPathStroker,	f32  diameter);
    void	(*setPenT4)	 (doeE, dcPathStroker,	f32* t4);
    void	(*setPenFitting) (doeE, dcPathStroker,	f32  unit,
							ixx  mindiameter);
    void	(*setCaps)	 (doeE, dcPathStroker,	ixx  caps);
    void	(*setCorners)	 (doeE, dcPathStroker,	ixx  corners,
							f32  miterlimit);
    void	(*setOutputT6)	 (doeE, dcPathStroker,	float* t6out);
    void	(*setOutputConsumer)
				 (doeE, dcPathStroker,	dcPathConsumer dst);
    void	(*reset)	 (doeE, dcPathStroker);
} dcPathStrokerFace;

/*
 * Values for caps and corners
 */
#define dcPathStroker_ROUND           1
#define dcPathStroker_SQUARE          2
#define dcPathStroker_BUTT            3
#define dcPathStroker_BEVEL           4
#define dcPathStroker_MITER           5


/*-------------------
 * creation functions
 */
extern	void		dcPathStroker_staticInitialize(doeE);
extern	void		dcPathStroker_staticFinalize  (doeE);

extern	dcPathStroker	dcPathStroker_create(doeE, dcPathConsumer destination);

#ifdef	__cplusplus
}
#endif

#endif  /* _DC_PATH_STROKER_H */
