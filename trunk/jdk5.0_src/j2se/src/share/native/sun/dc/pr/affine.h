/*
 * @(#)affine.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)affine.h 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _AFFINE_H
#define _AFFINE_H

#include "dtypes.h"

#ifdef	__cplusplus
extern "C" {
#endif



extern void affineT4TransformPoint  ( f32* t4, f32* x, f32* y );
extern void affineT4TransformPoints ( f32* t4, f32* pnts, ixx pntcnt );
extern void affineT4TransformBox    ( f32* t4, f32* box );
extern ixx  affineT4IsIdentity	    ( f32* t4 );
extern void affineT4MakeIdentity    ( f32* t4 );
extern ixx  affineT4IsSingular	    ( f32* t4 );
extern void affineT4Copy	    ( f32* t4dst, f32* t4src );
extern ixx  affineT4Invert	    ( f32* t4inv, f32* t4 );
extern ixx  affineT4Multiply	    ( f32* result, f32* t41, f32* t42 );

extern void affineT6TransformPoint  ( f32* t6, f32* x, f32* y );
extern void affineT6TransformPoints ( f32* t6, f32* pnts, ixx pntcnt );
extern void affineT6TransformBox    ( f32* t6, f32* box );
extern ixx  affineT6IsIdentity      ( f32* t6 );
extern void affineT6MakeIdentity    ( f32* t6 );
extern ixx  affineT6IsSingular	    ( f32* t6 );
extern void affineT6Copy	    ( f32* t6dst, f32* t6src );

extern void affineT6FromT4Dxy	    ( f32* t6dst, f32* t4src, f32* dxysrc );
extern void affineT4DxyFromT6       ( f32* t4dst, f32* dxydst, f32* t6src );



#ifdef	__cplusplus
}
#endif

#endif  /* _AFFINE_H */
