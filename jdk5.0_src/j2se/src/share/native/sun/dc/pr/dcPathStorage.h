/*
 * @(#)dcPathStorage.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPathStorage.h 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _DC_PATH_STORAGE_H
#define _DC_PATH_STORAGE_H

#include "dcPathConsumer.h"

#ifdef	__cplusplus
extern "C" {
#endif


/*----------------------
 * The class definition
 */
typedef struct dcPathStorageFace_**	dcPathStorage;


/*----------------------
 * The methods of dcPathStorage
 */
typedef struct dcPathStorageFace_ {
    dcPathConsumerFace	mu;
    void	(*sendToConsumer)
				(doeE,	dcPathStorage,	dcPathConsumer where,
							ixx mask);
    void	(*appendTangent)(doeE, dcPathStorage,	i32 tan);
    void	(*appendTangents)(doeE, dcPathStorage,	i32 tan1, i32 tan2);
    u8*		(*getElements)	(doeE, dcPathStorage);
    f32*	(*getPoints)	(doeE, dcPathStorage);
    i32*	(*getTangents)	(doeE, dcPathStorage);
    void	(*reset)	(doeE, dcPathStorage);
} dcPathStorageFace;


/*----------------------
 * Interpretation of the content of
 * the array returned by getElements
 */
#define dcPathStorageBEGIN_PATH_NO_BOX     0  /* no points (or tangents) */
#define dcPathStorageBEGIN_PATH_WITH_BOX   1  /* 2 points (n0 tangents) */
#define dcPathStorageBEGIN_SUBPATH         2  /* 1 point (no tangents) */
#define dcPathStorageLINE                  3  /* 1 point (1 tangent) */
#define dcPathStorageQUADRATIC             4  /* 2 points (2 tangent) */
#define dcPathStorageCUBIC                 5  /* 3 points (2 tangent) */
#define dcPathStorageEND_OPEN_SUBPATH      6  /* no points (or tangents) */
#define dcPathStorageEND_CLOSED_SUBPATH    7  /* no points (or tangents) */
#define dcPathStorageEND_OF_PATH           8  /* no points (or tangents) */


/*----------------------
 * Fields for "mask" argument of sendToConsumer
 */

#define dcPathStoragePATH_MASK        1  /* issue begin and end path */
#define dcPathStorageSUBPATH_MASK     2  /* issue begin subpath */
#define dcPathStorageARC_MASK         4  /* issue arc-level */
#define dcPathStorageCLOSED_MASK      8  /* issue closed subpath */


/*----------------------
 * Creating a dcPathStorage.
 */
extern	dcPathStorage  dcPathStorage_create(doeE, ixx withtangentinfo);

extern	void		dcPathStorage_staticInitialize(doeE);
extern	void		dcPathStorage_staticFinalize  (doeE);

#ifdef	__cplusplus
}
#endif

#endif  /* _DC_PATH_STORAGE_H */
