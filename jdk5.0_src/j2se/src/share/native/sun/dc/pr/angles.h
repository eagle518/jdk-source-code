/*
 * @(#)angles.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)angles.h 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _ANGLES_H
#define _ANGLES_H


#include "dtypes.h"


/*-----------------------
 * To compute with discrete angles using modulo arithmetic in the
 * range [0, (2^anglesL2DEG360)-1]. Although i32 are used to store
 * angle values, the values used should fit in 16 bits...
 */

/* 
 * Do not change L2DEG360 lightly... 
 * Changes can have far reaching consequences,
 * and most of them very bad.
 */
#define anglesL2DEG360         12
#define anglesDEG360           (1 << anglesL2DEG360)

/*
 * Converting degrees (integer in [0-360]) to angles
 */
#define anglesDEG(d)           ((((d)*anglesDEG360)+180)/360)

/*
 * Special cases with exact values
 */
#define anglesDEG270           (3 << (anglesL2DEG360-2))
#define anglesDEG180           (1 << (anglesL2DEG360-1))
#define anglesDEG090           (1 << (anglesL2DEG360-2))
#define anglesDEG045           (1 << (anglesL2DEG360-3))
#define anglesDEG000           0

#define anglesBAD              -1

/*
 * A little modular arithmetic
 */
#define anglesMASK               ((1 << anglesL2DEG360) - 1)
#define anglesAdd(a,b)           (((a) + (b)) & anglesMASK)
#define anglesSubtract(a,b)      (((a) - (b)) & anglesMASK)

/*-----------------------
 * The usual suspects.
 */

extern i32 anglesAtan2( f32 dy, f32 dx );
extern f32 anglesSin( i32 ang );
extern f32 anglesCos( i32 ang );


/*-----------------------
 * Faster versions of the usual suspects restricted
 * to angles a such that anglesDEG000<=a<=anglesDEG045
 * used by the general forms to do their dirty deeds
 */

extern f32 anglesOct1SinTable[anglesDEG045+1];
#define anglesOct1Sin(a)        anglesOct1SinTable[a]

extern f32 anglesOct1CosTable[anglesDEG045+1];
#define anglesOct1Cos(a)        anglesOct1CosTable[a]

#define anglesATAN_L2ENTRIES    (anglesL2DEG360 - 2)
#define anglesATAN_ENTRIES      ((1 << anglesATAN_L2ENTRIES) + 1)
#define anglesTANGENT_SCALE     ((f32)(1 << anglesATAN_L2ENTRIES))

extern i16 anglesOct1AtanTable[anglesATAN_ENTRIES];
#define anglesOct1Atan2(y,x)    anglesOct1AtanTable[(i32)((anglesTANGENT_SCALE*(y/x))+0.5F)]


/*-----------------------
 * Unsigned shortest distance between two angles: always 
 * returns a value d such that anglesDEG000<=d<=anglesDEG180
 */

extern i32 anglesUnsignedSpan( i32 angle0, i32 angle1 );


/*-----------------------
 * Signed shortest distance  between two angles: always 
 * returns a value d such that -anglesDEG180<=d<=anglesDEG180.
 * Positive direction is counterclockwise.
 */

extern i32 anglesSignedSpan( i32 from, i32 to );


/*-----------------------
 * Computing the modulus of a vector.
 */

extern f32 anglesModulus( f32 dx, f32 dy );


#endif  /* _ANGLES_H */
