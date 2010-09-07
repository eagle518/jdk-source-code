/*
 * @(#)arcs.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)arcs.h 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _ARCS_H
#define _ARCS_H

#include "dtypes.h"

#ifdef	__cplusplus
extern "C" {
#endif



/*-----------------------
 * Common preprocessing of quadratics and cubics: computing
 * the first order differences and modulus of the branches
 * of the control polygon, testing for branch degeneracy and 
 * fixing fixable degeneracies (for cubics only) returning
 * true iff the arc is degenerate.
 */
extern ixx arcsQuadraticDifsAndMods( f32* difs, f32* mods, f32* pnts, f32 min );
extern ixx arcsCubicDifsAndMods( f32* difs, f32* mods, f32* pnts, f32 min );

/*-----------------------
 * Dividing arcs into two halves at the
 * mid-value of the parameter
 */
extern void arcsLineDivision( f32* l, f32* l1, f32* l2 );
extern void arcsQuadraticDivision( f32* q, f32* q1, f32*q2 );
extern void arcsCubicDivision( f32* c, f32* c1, f32* c2 );


/*-----------------------
 * Dividing an arc into segments between two values of
 * the parameter
 */
extern void arcsProgressiveDifferences( f32* prdifs, ixx deg, f32* pnts );
extern void arcsSegment( f32* segpnts, ixx deg, f32* prdifs, f32 t0, f32 dt );
extern void arcsChord( f32* segpnts, ixx deg, f32* prdifs, f32 t0, f32 dt );


#ifdef	__cplusplus
}
#endif

#endif  /* _ARCS_H */
