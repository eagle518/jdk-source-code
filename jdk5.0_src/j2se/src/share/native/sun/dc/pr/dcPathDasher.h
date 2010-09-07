/*
 * @(#)dcPathDasher.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathDasher.h 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _DC_PATH_DASHER_H
#define _DC_PATH_DASHER_H

#include "dcPathConsumer.h"

#ifdef	__cplusplus
extern "C" {
#endif



/*----------------------
 * The class definition
 */
typedef struct dcPathDasherFace_**	dcPathDasher;


/*----------------------
 * The methods of dcPathDasher
 */
typedef struct dcPathDasherFace_ {
    dcPathConsumerFace	mu;
    void	(*setDash)	(doeE, dcPathDasher, f32* pattern, ixx cnt,
						     f32  offset);
    void	(*setDashT4)	(doeE, dcPathDasher, f32* t4);
    void	(*setOutputT6)	(doeE, dcPathDasher, float* t6);
    void	(*setOutputConsumer)
				(doeE, dcPathDasher, dcPathConsumer dst);
    void	(*reset)	(doeE, dcPathDasher);
} dcPathDasherFace;

/*----------------------
 * Creating a dcPathDasher.
 */
extern	dcPathDasher	dcPathDasher_create(doeE, dcPathConsumer destination);

extern	void		dcPathDasher_staticInitialize(doeE);
extern	void		dcPathDasher_staticFinalize  (doeE);


#ifdef	__cplusplus
}
#endif

#endif  /* _DC_PATH_DASHER_H */
