/*
 * @(#)PinkGlue.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 /*
   Copyright (C) 1995 Taligent, Inc. All rights reserved.
*/


#ifndef	PinkGlueIncludes
#define	PinkGlueIncludes

typedef double GCoordinate;
typedef struct   { GCoordinate	map[3][3];}  TGrafMatrix;
typedef struct {GCoordinate	fX,fY;}TGPoint ;

fract FractDivide(fract dividend, fract divisor);

fract FractMultiply(fract multiplicand, fract multiplier);
 
Fixed FixedDivide(Fixed dividend, Fixed divisor) ;

Fixed FixedMultiply(Fixed multiplicand, Fixed multiplier);
Fixed FixedMultiplyRound(Fixed multiplicand, Fixed multiplier);
tt_uint32 Magnitude(tt_int32 deltaX, tt_int32 deltaY);
tt_uint32 RandomBits(tt_int32 count, tt_int32 focus);
gxMapping *MapMapping(gxMapping *target, const gxMapping *source);
void TransformPoint(TGPoint *target, TGrafMatrix *c, GCoordinate x,  GCoordinate y);

gxMapping *CopyToMapping(gxMapping *target, const gxMapping *source);
void CopyBytes( void *source, void *destination, tt_int32 length) ;

void FillBytes(void *destination, tt_int32 length, tt_int32 pattern)  ;

void MoveBytes( void *source, void *destination, tt_int32 length)  ;

void convertFixedMatrixToTGraf(TGrafMatrix  *target, const  gxMapping *source);
void convertTGrafMatrixToFixed(gxMapping  *target,  const TGrafMatrix *source);
/* gxMapping *MapMapping(gxMapping *target, const gxMapping *source); */
void concatTGrafMatrices( TGrafMatrix *c, TGrafMatrix *a, TGrafMatrix *b);
gxMapping *ResetMapping(gxMapping *target);
void MapPoints(const  gxMapping *source, tt_int32 count, gxPoint vector[]) ;

/* void MapPoints( gxMapping *source, tt_int32 count, gxPoint vector[]);*/
void TransformPoint(TGPoint *target, TGrafMatrix *c,GCoordinate x, GCoordinate y) ;
short MxFlags( gxMapping *source);

enum {	
  kInvalidState=0,
  kIdentity=1, 
  kTranslate=2, 
  kScale=3, 
  kRotate=4, 
  kAffine =5, 
  kPerspective=6  
};
	
enum {
  invalidState=0,
  identityState=1, 
  translateState=2, 
  scaleState=3, 
  linearState=4, 
  perspectiveState =5 
};

short GetGXMapType( gxMapping *source);
short GetTGrafMapType( TGrafMatrix *source);
boolean MxMul(  gxMapping *source, short flags, gxPoint *vector, int count);

#endif
