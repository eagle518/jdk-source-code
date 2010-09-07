/*
 * @(#)angles.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)angles.c 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include "dtypes.h"
#include "angles.h"


/*-----------------------
 * Trig tables to compute sin(), cos() and atan()
 * without using math.h
 */

f32 anglesOct1SinTable[anglesDEG045+1] = {
#include "tableSin.h"
};

f32 anglesOct1CosTable[anglesDEG045+1] = {
#include "tableCos.h"
};

i16 anglesOct1AtanTable[anglesATAN_ENTRIES] = {
#include "tableAtan.h"
};

/*-----------------------
 * The usual suspects.
 */

#define q2toq1(x,y)  { f32 tmp = -x; x = y; y = tmp; }
#define q3toq1(x,y)  { x = -x; y = -y; }
#define q4toq1(x,y)  { f32 tmp = x; x = -y; y = tmp; }

i32 anglesAtan2( f32 dy, f32 dx ) {
    i32   angle;
    if( (dx > 0.0F) && (dy >= 0.0F) ) {         /* Quadrant 1 */
        angle = anglesDEG000;
    } else if( (dx <= 0.0F) && (dy > 0.0F) ) {  /* Quadrant 2 */
        q2toq1( dx, dy );
        angle = anglesDEG090;
    } else if( (dx < 0.0F) && (dy <= 0.0F) ) {  /* Quadrant 3 */
        q3toq1( dx, dy );
        angle = anglesDEG180;
    } else if( (dx >= 0.0F) && (dy < 0.0F) ) {  /* Quadrant 4 */
        q4toq1( dx, dy );
        angle = anglesDEG270;
    } else                                       /* Origin */
        return( anglesBAD );
    if( dx >= dy ) 
        angle += anglesOct1Atan2( dy, dx );
    else
        angle += anglesDEG090 - anglesOct1Atan2( dx, dy );
    return( angle );
    /* 
     * If you are linking with math.h you can use the 
     * following two lines instead of the above
     *
     *  double rads = atan2( (double)dy, (double)dx );
     *  return( (i32)((((double)anglesDEG180 / M_PI) * rads) + 0.5) );
     * 
     * You can also get rid of anglesOct1AtanTable[] and
     * the special case macro anglesOct1Atan2()
     */
}


f32 anglesSin( i32 ang ) {
    ixx   neg = 0;
    f32   res;
    if( ang < 0 ) { ang = -ang; neg = !neg; }
    if( ang > anglesDEG360 ) { ang %= anglesDEG360; }
    if( ang > anglesDEG180 ) { ang = anglesDEG360 - ang; neg = !neg; }
    if( ang > anglesDEG090 ) { ang = anglesDEG180 - ang; }
    if( ang > anglesDEG045 ) 
        res = anglesOct1Cos( anglesDEG090 - ang );
    else
        res = anglesOct1Sin( ang );
    return( neg ? -res : res );
    /* 
     * If you are linking with math.h you can use the 
     * following two lines instead of the above
     *
     *  double rads = (double)ang *( M_PI / (double)anglesDEG180 );
     *  return( (f32)sin( rads ) );
     * 
     * You can also get rid of anglesOct1SinTable[] and
     * the special case macro anglesOct1Sin()
     */
}


f32 anglesCos( i32 ang ) {
    ixx   neg = 0;
    f32   res;
    if( ang < 0 ) { ang = -ang; }
    if( ang > anglesDEG360 ) { ang %= anglesDEG360; }
    if( ang > anglesDEG180 ) { ang = anglesDEG360 - ang; }
    if( ang > anglesDEG090 ) { ang = anglesDEG180 - ang; neg = !neg; }
    if( ang > anglesDEG045 ) 
        res = anglesOct1Sin( anglesDEG090 - ang );
    else
        res = anglesOct1Cos( ang );
    return( neg ? -res : res );
    /* 
     * If you are linking with math.h you can use the 
     * following two lines instead of the above
     *
     *  double rads = (double)ang * ( M_PI / (double)anglesDEG180 );
     *  return( (f32)cos( rads ) );
     * 
     * You can also get rid of anglesOct1CosTable[] and
     * the special case macro anglesOct1Cos()
     */
}


/*-----------------------
 * Unsigned shortest distance between two angles: always 
 * returns a value d such that anglesDEG000<=d<=anglesDEG180
 */

i32 anglesUnsignedSpan( i32 angle0, i32 angle1 ) {
    i32 diff = anglesSubtract(angle1, angle0);
    if( diff > anglesDEG180 ) diff = anglesDEG360 - diff;
    return( diff );
}


/*-----------------------
 * Signed shortest distance  between two angles: always 
 * returns a value d such that anglesDEG180<=d<=anglesDEG180.
 * Positive direction is counterclockwise.
 */

i32 anglesSignedSpan( i32 from, i32 to ) {
    i32 diff = to - from;
    if( diff > anglesDEG180 ) diff = diff - anglesDEG360;
    else if( diff < -anglesDEG180 ) diff = anglesDEG360 + diff;
    return( diff );
}


/*-----------------------
 * Computing the modulus of a vector.
 */

f32 anglesModulus( f32 dx, f32 dy ) {
    i32 ang;
    if( dx < 0.0F ) dx = -dx;
    if( dy < 0.0F ) dy = -dy;
    if( dx >= dy ) {
       if( dx == 0.0F ) return( 0.0F );
       ang = anglesOct1Atan2( dy, dx );
       return( dx / anglesOct1Cos( ang ) );
    } else {
       ang = anglesOct1Atan2( dx, dy );
       return( dy / anglesOct1Cos( ang ) );
    }
}
