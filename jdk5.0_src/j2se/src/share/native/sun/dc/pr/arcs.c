/*
 * @(#)arcs.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)arcs.c 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include          "dtypes.h"
#include          "angles.h"
#include          "arcs.h"

/*-----------------------
 * Common preprocessing of quadratics and cubics: computing
 * the first order differences and modulus of the branches
 * of the control polygon, testing for branch degeneracy and 
 * fixing fixable degeneracies (for cubics only) returning
 * true iff the arc is degenerate.
 */

ixx arcsQuadraticDifsAndMods( f32* difs, f32* mods, f32* pnts, f32 min ) {
    difs[0] = pnts[2] - pnts[0]; difs[1] = pnts[3] - pnts[1]; 
    difs[2] = pnts[4] - pnts[2]; difs[3] = pnts[5] - pnts[3]; 
    mods[0] = anglesModulus( difs[0], difs[1] );
    mods[1] = anglesModulus( difs[2], difs[3] );
    return( (mods[0] <= min) || (mods[1] < min) );
}

ixx arcsCubicDifsAndMods( f32* difs, f32* mods, f32* pnts, f32 min ) {
    ixx d0 = 0, d2 = 0, dcnt = 0;
    difs[0] = pnts[2] - pnts[0]; difs[1] = pnts[3] - pnts[1]; 
    difs[2] = pnts[4] - pnts[2]; difs[3] = pnts[5] - pnts[3]; 
    difs[4] = pnts[6] - pnts[4]; difs[5] = pnts[7] - pnts[5]; 
    mods[0] = anglesModulus( difs[0], difs[1] );
    mods[1] = anglesModulus( difs[2], difs[3] );
    mods[2] = anglesModulus( difs[4], difs[5] );
    if( mods[0] <= min ) { dcnt++; d0 = 1; }
    if( mods[1] <= min ) { dcnt++; }
    if( mods[2] <= min ) { dcnt++; d2 = 1; }
    if( dcnt >= 2 ) return( 1 );
    if( d0 ) {
        /* move the second point slightly towards the third */
        f32 fraction =  min / mods[1];
        pnts[2] += difs[2] * fraction;
        pnts[3] += difs[3] * fraction;
        difs[0] = pnts[2] - pnts[0]; difs[1] = pnts[3] - pnts[1];
        difs[2] = pnts[4] - pnts[2]; difs[3] = pnts[5] - pnts[3]; 
        mods[0] = anglesModulus( difs[0], difs[1] );
        mods[1] = anglesModulus( difs[2], difs[3] );
    } else if( d2 ) {
        /* move the third point slightly towards the second */
        f32 fraction = (2.0F * min) / mods[1];
        pnts[4] -= difs[2] * fraction;
        pnts[5] -= difs[3] * fraction;
        difs[2] = pnts[4] - pnts[2]; difs[3] = pnts[5] - pnts[3]; 
        difs[4] = pnts[6] - pnts[4]; difs[5] = pnts[7] - pnts[5]; 
        mods[1] = anglesModulus( difs[2], difs[3] );
        mods[2] = anglesModulus( difs[4], difs[5] );
    }
    return( 0 );
}


/*-----------------------
 * Dividing arcs into two halves at the
 * mid-value of the parameter
 */

void arcsLineDivision( f32* l, f32* l1, f32* l2 ) {
    l1[0] = l[0]; 
    l2[2] = l[2]; 
    l1[2] = l2[0] = (l[0] + l[2]) / 2.0F;
    l1[1] = l[1]; 
    l2[3] = l[3];
    l1[3] = l2[1] = (l[1] + l[3]) / 2.0F;
}

void arcsQuadraticDivision( f32* q, f32* q1, f32*q2 ) {
    f32 tmp01, tmp12;
    q1[0] = q[0]; 
    q2[4] = q[4]; 
    q1[2] = tmp01 = (q[0] + q[2]) / 2.0F;
    q2[2] = tmp12 = (q[2] + q[4]) / 2.0F;
    q1[4] = q2[0] = (tmp01 + tmp12) / 2.0F;
    q1[1] = q[1]; 
    q2[5] = q[5];
    q1[3] = tmp01 = (q[1] + q[3]) / 2.0F;
    q2[3] = tmp12 = (q[3] + q[5]) / 2.0F;
    q1[5] = q2[1] = (tmp01 + tmp12) / 2.0F;
}

void arcsCubicDivision( f32* c, f32* c1, f32* c2 ) {
    f32 tmp01, tmp12, tmp23, tmp02, tmp13;
    c1[0] = c[0]; 
    c2[6] = c[6]; 
    c1[2] = tmp01 = (c[0] + c[2]) / 2.0F;
    tmp12 = (c[2] + c[4]) / 2.0F;
    c2[4] = tmp23 = (c[4] + c[6]) / 2.0F;
    c1[4] = tmp02 = (tmp01 + tmp12) / 2.0F;
    c2[2] = tmp13 = (tmp12 + tmp23) / 2.0F;
    c1[6] = c2[0] = (tmp02 + tmp13) / 2.0F;
    c1[1] = c[1]; 
    c2[7] = c[7];
    c1[3] = tmp01 = (c[1] + c[3]) / 2.0F;
    tmp12 = (c[3] + c[5]) / 2.0F;
    c2[5] = tmp23 = (c[5] + c[7]) / 2.0F;
    c1[5] = tmp02 = (tmp01 + tmp12) / 2.0F;
    c2[3] = tmp13 = (tmp12 + tmp23) / 2.0F;
    c1[7] = c2[1] = (tmp02 + tmp13) / 2.0F;
}
