/*
 * @(#)PinkGlue.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
   Copyright (C) 1995 Taligent, Inc. All rights reserved.
*/

#include "Hint.h"
#include "util.h"

#ifdef ENABLE_TT_HINTING

double sqrt(double x);  
#include <stdlib.h>
#include <limits.h>

const GCoordinate dFixed1 = (GCoordinate)fixed1;
const GCoordinate dFract1 = (GCoordinate)fract1;
const GCoordinate rFixed1 = (GCoordinate)1./(GCoordinate)fixed1;
const GCoordinate rFract1 = (GCoordinate)1./(GCoordinate)fract1 ;

static GCoordinate FixedToGCoordinate(Fixed f) { return rFixed1 * (GCoordinate)f; }
static GCoordinate FractToGCoordinate(Fract f) { return rFract1 * (GCoordinate)f; }
static Fixed GCoordinateToFixed(GCoordinate f) { return (fixed)(f * dFixed1); }
static Fract GCoordinateToFract(GCoordinate f) { return (fract)(f * dFract1); }

/* #define NO_FIXEDMATH */

fract FractDivide(fract dividend, fract divisor) {
#ifndef NO_FIXEDMATH
	 return  MultiplyDivide(dividend, (tt_int32)dFract1, divisor);
	/* Old method: return FracDiv(dividend, divisor); */
#else
	return GCoordinateToFract((GCoordinate)dividend / (GCoordinate)divisor);
#endif
}

fract FractMultiply(fract multiplicand, fract multiplier) 
{
#ifndef NO_FIXEDMATH
	return	MultiplyFract(multiplicand, multiplier);
	/* old code: return FracMul(multiplicand, multiplier); */
#else
	return (fract)(rFract1 * (GCoordinate)multiplicand * (GCoordinate)multiplier);
#endif
}
 
Fixed FixedDivide(Fixed dividend, Fixed divisor) {
#ifndef NO_FIXEDMATH
	 return  MultiplyDivide(dividend, (tt_int32)dFixed1, divisor);
	/* Old method: return FixDiv(dividend, divisor); */
	
#else
	return GCoordinateToFixed((GCoordinate)dividend / (GCoordinate)divisor);
#endif
}

Fixed FixedMultiply(Fixed multiplicand, Fixed multiplier) {
#ifndef NO_FIXEDMATH
	return util_FixMul(multiplicand, multiplier);
#else
	return (fixed)(rFixed1 * (GCoordinate)multiplicand * (GCoordinate)multiplier);
#endif
}

		
		
			/* 
			 *  Fixed point multiply for positive numbers, with rounding. 
			 */
			static F16Dot16 util_FixMulRoundPositive( F16Dot16 mA, F16Dot16 mB )
			{
				tt_uint16 mA_Hi, mA_Lo;
				tt_uint16 mB_Hi, mB_Lo;
				tt_uint32 d1, d2, d3;
				F16Dot16 result;
		  
				mA_Hi =  (mA>>16);
				mA_Lo = ((tt_uint16)(mA));
				mB_Hi =  (mB>>16);
				mB_Lo = ((tt_uint16)(mB));
				
				/*
						mB_Hi 	mB_Lo
				  X		mA_Hi 	mA_Lo
				------------------
				d1		d2		d3
				*/
				d3  = mA_Lo * mB_Lo;		/* <<  0 */
				d2  = mA_Lo * mB_Hi;		/* << 16 */
				d2 += mA_Hi * mB_Lo;		/* << 16 */
				d1  = mA_Hi * mB_Hi;		/* << 32 */
				
				result 	 = (d1 << 16) + d2 + (d3 >> 16);
				if (d3&0x8000)
					result++;
			 	return result; 
			}

	
/* Round Symmetric for Fixed Point multiply. */
 Fixed FixedMultiplyRound (Fixed multiplicand, Fixed multiplier) 
{
	if (multiplicand<0) 
	{
		if (multiplier<0)
			return( util_FixMulRoundPositive( -multiplicand,-multiplier) );
		else
			return(- util_FixMulRoundPositive( -multiplicand, multiplier) );
	}
	else
	{
		if (multiplier<0)
			return( -util_FixMulRoundPositive(  multiplicand,-multiplier) );
		else
			return( util_FixMulRoundPositive(  multiplicand, multiplier) );
	}
}

#if 0

			/* The following routine correctly, and symmetric to zero, rounds a fix point multiply */
			/* However, it is encumbered by the use of "long long". Therefore,					   */
			/*  it is only used to verify that complicated and faster code (below), is performed correctly. 		*/
			static Fixed KnownCorrectFixedMultiplyRound(Fixed multiplicand, Fixed multiplier) 
			{
				long long res;
				tt_int32 rounder= 0x8000;
				tt_int32 finalResult;
			#if 1	
				/* True round symmetric with zero. */
				res= ((long long) multiplicand) * ( (long long) multiplier)  ;
				if (res<0)
					finalResult= -(((-res)+rounder)>>16);
				else 
					finalResult= (res+rounder)>>16;
			 	return (finalResult);
			#else
				/* Almost as good */
				res= ((long long) multiplicand) * ( (long long) multiplier) ;
				return (( res+rounder)>>16);
			#endif
			}
				
			/* This code is only used to compare the above fixed point routines. */
			/* When testing rename the above routine (FixedMultiplyRound) to FixedMultiplyRoundSymmetricZero */
			/* and enable this code for compilation. */
			static tt_int32 totalErrors=0;
			static tt_int32 bigErors=0;
			Fixed FixedMultiplyRound(Fixed multiplicand, Fixed multiplier) 
			{
				tt_int32 res1, res2;
				res1= KnownCorrectFixedMultiplyRound(  multiplicand,   multiplier) ;
			res2= FixedMultiplyRoundSymmetricZero(  multiplicand,   multiplier) ;
			/*	 res2= FixedMultiply (  multiplicand,   multiplier) ; */
				if (res1!=res2)		
					totalErrors++;
				if ( ( totalErrors!=0) && (totalErrors %100)==0)
					bigErors++;
				return(res2);
			}
#endif

tt_uint32 Magnitude(tt_int32 deltaX, tt_int32 deltaY) {
	GCoordinate x = (GCoordinate)deltaX;
	GCoordinate y = (GCoordinate)deltaY;
	return (tt_uint32)
		sqrt((x * x) + (y * y));
}

tt_uint32 RandomBits(tt_int32 count, tt_int32 focus) {
	return 17;
}

gxMapping *CopyToMapping(gxMapping *target, const gxMapping *source) {
	target->map[0][0] = source->map[0][0];
	target->map[0][1] = source->map[0][1];
	target->map[0][2] = source->map[0][2];
	target->map[1][0] = source->map[1][0];
	target->map[1][1] = source->map[1][1];
	target->map[1][2] = source->map[1][2];
	target->map[2][0] = source->map[2][0];
	target->map[2][1] = source->map[2][1]; 
	target->map[2][2] = source->map[2][2];
	return target;
}

	
		void convertFixedMatrixToTGraf(TGrafMatrix  *target,  const  gxMapping *source)
		{
 					target->map[0][0]= FixedToGCoordinate(source->map[0][0]);
					target->map[0][1]= FixedToGCoordinate(source->map[0][1]);
					target->map[0][2]= FractToGCoordinate(source->map[0][2]);

					target->map[1][0]= FixedToGCoordinate(source->map[1][0]);
					target->map[1][1]= FixedToGCoordinate(source->map[1][1]);
					target->map[1][2]= FractToGCoordinate(source->map[1][2]);
 							
 					target->map[2][0]= FixedToGCoordinate(source->map[2][0]);
					target->map[2][1]= FixedToGCoordinate(source->map[2][1]);
					target->map[2][2]= FractToGCoordinate(source->map[2][2]);
 		}
	
	
 		void convertTGrafMatrixToFixed(gxMapping  *target,  const  TGrafMatrix *source)
		{
			target->map[0][0] = GCoordinateToFixed(source->map[0][0]) ;
			target->map[0][1] = GCoordinateToFixed(source->map[0][1]) ;
			target->map[0][2] = GCoordinateToFract(source->map[0][2]) ;
			target->map[1][0] = GCoordinateToFixed(source->map[1][0]) ;
			target->map[1][1] = GCoordinateToFixed(source->map[1][1]) ;
			target->map[1][2] = GCoordinateToFract(source->map[1][2]) ;
			target->map[2][0] = GCoordinateToFixed(source->map[2][0]) ;
			target->map[2][1] = GCoordinateToFixed(source->map[2][1]) ;
			target->map[2][2] = GCoordinateToFract(source->map[2][2]) ;
 		}

		void concatTGrafMatrices( TGrafMatrix *c, TGrafMatrix *a, TGrafMatrix *b)
		{
			tt_int32 i,j,k;
			for (i=0;i<3;i++)
			  for (j=0;j<3;j++)
			  {
			  	c->map [i][j] = a->map[i][j] * b->map[j][i];			  
			  }
		}


 	gxMapping *MapMapping(gxMapping *target, const gxMapping *source) 
	{
 			TGrafMatrix a,b,c;
 			
 			convertFixedMatrixToTGraf(&a,   source);
 			convertFixedMatrixToTGraf(&b,  target);
 			concatTGrafMatrices(&c,&a,&b);
  			convertTGrafMatrixToFixed(target,  &c);
 			return target;
	}
	
 	void TransformPoint(TGPoint *target, TGrafMatrix *c, GCoordinate x,  GCoordinate y)
 	{
			GCoordinate w;
			w=  x * c->map[0][2] + y * c->map[1][2] + 1 * c->map[2][2];
			target->fX= ( x * c->map[0][0] + y * c->map[1][0] + 1 * c->map[2][0])/w;
			target->fY= (x * c->map[0][1] + y * c->map[1][1] + 1 * c->map[2][1])/w;
	}
	
	
	gxMapping *ResetMapping(gxMapping *target) 
		{
		/* qprintf("ResetMapping\n");*/
			memset(target->map, 0, sizeof(gxMapping));
			target->map[0][0] = target->map[1][1] = fixed1;
			target->map[2][2] = fract1;
			return target;
		}

	void MapPoints(const  gxMapping *source, tt_int32 count, gxPoint vector[]) 
		{
		/* qprintf("MapPoints\n");*/
			TGrafMatrix a;
			TGPoint res;
			tt_int32 i,j,k;
			GCoordinate fx;
		 	GCoordinate fy;
			
 			convertFixedMatrixToTGraf(&a,  source);
 			
	 		for ( i = 0; i < count; ++i)
			{
				fx= FixedToGCoordinate(vector[i].x);
				fy= FixedToGCoordinate(vector[i].y);				
				TransformPoint(&res, &a, fx, fy );
 				vector[i].x = GCoordinateToFixed(res.fX);
				vector[i].y = GCoordinateToFixed(res.fY);
			}
			
		}
		
		short GetTGrafMapType( TGrafMatrix *source)
		{
			short result=kInvalidState; 
 			
 			if(  (source->map[0][2]!=0) || (source->map[1][2]!=0))
			 	{ result=(kPerspective);goto exit;}
			 	
 			if(  (source->map[0][1]!=0) || (source->map[1][0]!=0))
 				{ return(kAffine); } /* could be just rotate!*/
			 	
 			if(  (source->map[0][0] != source->map[1][1] ))
 				{ return(kScale); } /* REally this means stretch, is it okay??*/
		
	 		if(  (source->map[2][0]!=0) || (source->map[2][1]!=0))
 				{ return(kTranslate); } /* could be just rotate!*/
	
	 		if(  (source->map[0][0]==1.) || (source->map[1][1]==1.))
 				{ return(kIdentity); } /* could be just rotate!*/
	
		
		exit:
			return(result);
		}
	
		short GetGXMapType(  gxMapping *source)
		{
			TGrafMatrix a;
			short result;
			convertFixedMatrixToTGraf(&a,  source);
			result=GetTGrafMapType(&a);
			return(result);
		}
		
		
		short MxFlags(  gxMapping *source) 
		{
		/* qprintf("MxFlags\n");*/
			short theType;
 			theType =GetGXMapType(source);
			switch (theType) 
			{
			case  kIdentity: return identityState;
			case  kTranslate: return translateState;
			case  kScale: return scaleState;
			case  kRotate:
			case  kAffine: return linearState;
			case  kPerspective: return perspectiveState;
			}
			return invalidState;
		}

		boolean MxMul(  gxMapping *source, short flags, gxPoint *vector, int count) 
		{
		/*qprintf("MxMul\n");*/
			MapPoints(source, count, vector);
			return true;
		}

void CopyBytes(  void *source, void *destination, tt_int32 length) 
{
  memcpy(destination, source, length);
 }

void FillBytes(void *destination, tt_int32 length, tt_int32 pattern) 
{
	memset(destination, pattern, length);
}

void MoveBytes( void *source, void *destination, tt_int32 length) 
{
	memmove(destination, source, length);
}
#endif  
 
 

