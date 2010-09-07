/*
 * @(#)affine.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)affine.c 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include          "dtypes.h"

void affineT4TransformPoint( f32* t4, f32* x, f32* y ) {
    f32 newx = ((*x) * t4[0]) + ((*y) * t4[2]),
        newy = ((*x) * t4[1]) + ((*y) * t4[3]);
    *x = newx;
    *y = newy;
}

void affineT4TransformPoints( f32* t4, f32* pnts, ixx pntcnt ) {
    f32 a, b, c, d;
    a = t4[0]; b = t4[1]; 
    c = t4[2]; d = t4[3];
    while( pntcnt-- ) {
    f32 x, y;
        x = *pnts; y = *(pnts+1);
        *pnts++ = (a * x) + (c * y);
        *pnts++ = (b * x) + (d * y);
    }
}

static
f32 affineMin4( f32 a, f32 b, f32 c, f32 d ) {
    f32 min = (a < b) ? a : b;
    if( c < min ) min = c;
    return( (d < min) ? d : min );
}

static
f32 affineMax4( f32 a, f32 b, f32 c, f32 d ) {
    f32 max = (a > b) ? a : b;
    if( c > max ) max = c;
    return( (d > max) ? d : max );
}

void affineT4TransformBox( f32* t4, f32* box ) {
    float rect[8];
    rect[0] = box[0]; rect[1] = box[1];
    rect[2] = box[2]; rect[3] = rect[1];
    rect[4] = rect[2]; rect[5] = box[3];
    rect[6] = rect[0]; rect[7] = rect[5];
    affineT4TransformPoints( t4, rect, 4 );
    box[0] = affineMin4( rect[0], rect[2], rect[4], rect[6] );
    box[1] = affineMin4( rect[1], rect[3], rect[5], rect[7] );
    box[2] = affineMax4( rect[0], rect[2], rect[4], rect[6] );
    box[3] = affineMax4( rect[1], rect[3], rect[5], rect[7] );
}

ixx affineT4IsIdentity( f32* t4 ) {
    return( 
        (t4[0] == 1.0F) && (t4[1] == 0.0F) && 
        (t4[2] == 0.0F) && (t4[3] == 1.0F)
    );
}

void affineT4MakeIdentity( f32* t4 ) {
    t4[0] = 1.0F; t4[1] = 0.0F;
    t4[2] = 0.0F; t4[3] = 1.0F;
}

#define SING_LIMIT           1E-25
#define isDetSingular(d)     ((d > 0.0F) ? (d < SING_LIMIT) : (-d < SING_LIMIT))

ixx affineT4IsSingular( f32* t4 ) {
    f32 det = (t4[0] * t4[3]) - (t4[1] * t4[2] );
    return( isDetSingular( det ) );
}

void affineT4Copy( f32* t4dst, f32* t4src ) {
   *t4dst++ = *t4src++; *t4dst++ = *t4src++;
  *t4dst++ = *t4src++; *t4dst = *t4src;
}

ixx affineT4Invert( f32* t4inv, f32* t4 ) {
    f32 det = (t4[0] * t4[3]) - (t4[1] * t4[2] );
    if( isDetSingular( det ) ) return( 0 );
    t4inv[0] = t4[3] / det;
    t4inv[1] = -(t4[1] / det);
    t4inv[2] = -(t4[2] / det);
    t4inv[3] = t4[0] / det;
    return( 1 );
}

void affineT4Multiply( f32* result, f32* t41, f32* t42 ) {
    result[0] = (t41[0] * t42[0]) + (t41[1] * t42[2]);
    result[1] = (t41[0] * t42[1]) + (t41[1] * t42[3]);
    result[2] = (t41[2] * t42[0]) + (t41[3] * t42[2]);
    result[3] = (t41[2] * t42[1]) + (t41[3] * t42[3]);
}

void affineT6TransformPoint( f32* t6, f32* x, f32* y ) {
    f32 newx = ((*x) * t6[0]) + ((*y) * t6[2]) + t6[4],
        newy = ((*x) * t6[1]) + ((*y) * t6[3]) + t6[5];
    *x = newx;
    *y = newy;
}

void affineT6TransformPoints( f32* t6, f32* pnts, ixx pntcnt ) {
    f32 a, b, c, d, dx, dy;
    a = t6[0]; b = t6[1]; 
    c = t6[2]; d = t6[3];
    dx = t6[4]; dy = t6[5];
    while( pntcnt-- ) {
        f32 x, y;
        x = *pnts; y = *(pnts+1);
        *pnts++ = (a * x) + (c * y) + dx;
        *pnts++ = (b * x) + (d * y) + dy;
    }
}

void affineT6TransformBox( f32* t6, f32* box ) {
    float rect[8];
    rect[0] = box[0]; rect[1] = box[1];
    rect[2] = box[2]; rect[3] = rect[1];
    rect[4] = rect[2]; rect[5] = box[3];
    rect[6] = rect[0]; rect[7] = rect[5];
    affineT6TransformPoints( t6, rect, 4 );
    box[0] = affineMin4( rect[0], rect[2], rect[4], rect[6] );
    box[1] = affineMin4( rect[1], rect[3], rect[5], rect[7] );
    box[2] = affineMax4( rect[0], rect[2], rect[4], rect[6] );
    box[3] = affineMax4( rect[1], rect[3], rect[5], rect[7] );
}

ixx affineT6IsIdentity( f32* t6 ) {
    return( 
        (t6[0] == 1.0F) && (t6[1] == 0.0F) && 
        (t6[2] == 0.0F) && (t6[3] == 1.0F) &&
        (t6[4] == 0.0F) && (t6[5] == 0.0F)
  );
}

void affineT6MakeIdentity( f32* t6 ) {
    t6[0] = 1.0F; t6[1] = 0.0F;
    t6[2] = 0.0F; t6[3] = 1.0F;
    t6[4] = 0.0F; t6[5] = 0.0F;
}

ixx affineT6IsSingular( f32* t6 ) {
    f32 det = (t6[0] * t6[3]) - (t6[1] * t6[2] );
    return( isDetSingular( det ) );
}

void affineT6Copy( f32* t6dst, f32* t6src ) {
    *t6dst++ = *t6src++; *t6dst++ = *t6src++;
    *t6dst++ = *t6src++; *t6dst++ = *t6src++;
    *t6dst++ = *t6src++; *t6dst = *t6src;
}

void affineT6FromT4Dxy( f32* t6dst, f32* t4src, f32* dxysrc ) {
    *t6dst++ = *t4src++; *t6dst++ = *t4src++;
    *t6dst++ = *t4src++; *t6dst++ = *t4src;
    *t6dst++ = *dxysrc++; *t6dst = *dxysrc;
}

void affineT4DxyFromT6( f32* t4dst, f32* dxydst, f32* t6src ) {
    *t4dst++ = *t6src++; *t4dst++ = *t6src++;
    *t4dst++ = *t6src++; *t4dst = *t6src++;
    *dxydst++ = *t6src++; *dxydst = *t6src;
}
