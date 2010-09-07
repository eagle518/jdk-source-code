/*
 * @(#)FntRare.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/

#include "Hint.h"
#ifdef ENABLE_TT_HINTING

#ifdef  debugging
#include <stdio.h>
#endif
#include "PrivateFnt.h"
#include "FntDebug.h"
#include "FntInstructions.h"

/*** <13> support macros for Kanji adjust from here  ***/ 

/* Decrease the number of covered scanlines between yLo and yHi by 1 pixel.
If jMove is already set to jLo or jHi, then calculate the amount that edge
must be moved and put it in *amounP.  If jMove == -1, find the edge that
must be moved least and set the minimum movement to accomplish this in *amountP. 
*/
#define COMPRESS( yLo, yHi, amountP, jLo, jHi, jMove )			\
{																\
	subPixel moveHi = yHi - (fntfloor( yHi - HALFPIXEL ) + HALFPIXELM);		\
	subPixel moveLo = (fntfloor( yLo + HALFPIXELM ) + HALFPIXELP) - yLo;		\
	if( jMove == jLo ) *amountP = moveLo;						\
	else if( jMove == jHi ) *amountP = -moveHi;					\
	else if( moveLo <= moveHi )									\
	{															\
		*amountP = moveLo;										\
		jMove = jLo;											\
	}															\
	else 														\
	{															\
		*amountP = -moveHi;										\
		jMove = jHi;											\
	}															\
}																\

/* Increase the number of covered scanlines between yLo and yHi by 1 pixel.
If jMove is already set to jLo or jHi, then calculate the amount that edge
must be moved and put it in *amounP.  If jMove == -1, find the edge that
must be moved least and set the minimum movement to accomplish this in *amountP. 
*/
#define EXPAND( yLo, yHi, amountP, jLo, jHi, jMove )			\
{																\
	subPixel moveHi = fntfloor( yHi - HALFPIXEL ) + HALFPIXEL + PIXEL - yHi;	\
	subPixel moveLo = yLo - (fntfloor( yLo + HALFPIXELM ) - HALFPIXEL);		\
	if( jMove == jLo ) *amountP = -moveLo;						\
	else if( jMove == jHi ) *amountP = moveHi;					\
	else if( moveLo <= moveHi )									\
	{															\
		*amountP = -moveLo;										\
		jMove = jLo;											\
	}															\
	else 														\
	{															\
		*amountP = moveHi;										\
		jMove = jHi;											\
	}															\
}																\

/* Figure out whether width needs to be increased or decreased and which
 * edge to move to accomplish the task.  If jMove is already set, only figure
 * out how much to move it.
 */
#define CALCEDGE( target, j1, j2, coord, moveP, jMove )						\
{																			\
	subPixel scanHi, scanLo;												\
	pixel nPixels;															\
	subPixel yLo = coord[j1];												\
	subPixel yHi = coord[j2];												\
	ArrayIndex	 jLo = j1;														\
	ArrayIndex	 jHi = j2;														\
	if( yHi < yLo )															\
	{																		\
		subPixel temp = yHi;												\
		yHi = yLo;															\
		yLo = temp;															\
		jLo = j2;															\
		jHi = j1;															\
	}																		\
	scanHi = fntfloor( yHi - HALFPIXEL ) + HALFPIXEL;									\
	scanLo = fntfloor( yLo + HALFPIXELM ) + HALFPIXEL;									\
	nPixels = (scanHi - scanLo + PIXEL) >> LG2PIXEL;						\
	if( nPixels == target ) jMove = -1;										\
	else if( nPixels > target )												\
		COMPRESS( yLo, yHi, moveP, jLo, jHi, jMove )						\
	else  EXPAND( yLo, yHi, moveP, jLo, jHi, jMove )						\
}																			\

/* Move all the points on the same stroke edge as jMove by movement. Also do
serif adjustment; move adjoining points that have coordinates between original
edge and moved edge to moved edge.
*/
#define MOVEEDGE( jMove, movement, coord, sp, ep, nContours, flags, direction )		\
{																	\
int nPoints;														\
int nc = nContours;													\
ArrayIndex start, finish, j;												\
register subPixel coordMin, coordMax;								\
register subPixel coordEdge = coord[jMove];							\
subPixel coordNew = coordEdge + movement;							\
if( coordEdge < coordNew )											\
{																	\
	coordMin = coordEdge;											\
	coordMax = coordNew;											\
}																	\
else 																\
{																	\
	coordMin = coordNew;											\
	coordMax = coordEdge;											\
}																	\
																	\
--nc;																\
while( jMove < sp[nc] ) --nc;										\
finish = ep[nc];													\
start  = sp[nc];													\
																	\
j = jMove;															\
nPoints = finish - start;											\
while( coord[j] >= coordMin && coord[j] <= coordMax && nPoints-- )	\
{																	\
	coord[j] = coordNew;											\
	flags[j] |= direction;											\
	if( ++j > finish ) j = start;									\
}																	\
																	\
j = jMove;															\
if( --j < start )	j = finish;										\
nPoints = finish - start;											\
while( coord[j] >= coordMin && coord[j] <= coordMax && nPoints-- )	\
{																	\
	coord[j] = coordNew;											\
	flags[j] |= direction;											\
	if( --j < start )	j = finish;									\
}																	\
}																	\

/***************** end of Adjust macros **************/

/*
 * Set Dual Projection Vector To Line
 */
void fnt_SDPVTL(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex arg1, arg2;

	arg2 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
	arg1 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );

	CHECK_POINT( gs, gs->CE2, arg2 );
	CHECK_POINT( gs, gs->CE1, arg1 );

	/* Do the current domain */
	fnt_Normalize(gs, gs->CE1->x[arg1] - gs->CE2->x[arg2], gs->CE1->y[arg1] - gs->CE2->y[arg2], &gs->proj );

	/* Do the old domain */
	fnt_Normalize(gs, gs->CE1->ox[arg1] - gs->CE2->ox[arg2], gs->CE1->oy[arg1] - gs->CE2->oy[arg2], &gs->oldProj );

	gs->projectionVectorIsNormal = false;
	if ( BIT0( gs->opCode ) ) {
		/* rotate 90 degrees */
		shortFrac tmp		= gs->proj.y;
		gs->proj.y		= gs->proj.x;
		gs->proj.x		= -tmp;

		tmp				= gs->oldProj.y;
		gs->oldProj.y	= gs->oldProj.x;
		gs->oldProj.x	= -tmp;

		gs->projectionVectorIsNormal = true;
	}
	fnt_ComputeAndCheck_PF_Proj( gs );

	gs->MovePoint = (FntMoveFunc) fnt_MovePoint;
	gs->Project = (FntProjFunc) fnt_Project;
	gs->OldProject = (FntProjFunc) fnt_OldProject;
}

/*
 * fnt_ISECT()
 *
 * Computes the intersection of two lines without using floating gxPoint!!
 *
 * (1) Bx + dBx * t0 = Ax + dAx * t1
 * (2) By + dBy * t0 = Ay + dAy * t1
 *
 *  1  =>  (t1 = Bx - Ax + dBx * t0 ) / dAx
 *  +2 =>   By + dBy * t0 = Ay + dAy/dAx * [ Bx - Ax + dBx * t0 ]
 *     => t0 * [dAy/dAx * dBx - dBy = By - Ay - dAy/dAx*(Bx-Ax)
 *     => t0(dAy*DBx - dBy*dAx) = dAx(By - Ay) + dAy(Ax-Bx)
 *     => t0 = [dAx(By-Ay) + dAy(Ax-Bx)] / [dAy*dBx - dBy*dAx]
 *     => t0 = [dAx(By-Ay) - dAy(Bx-Ax)] / [dBx*dAy - dBy*dAx]
 *     t0 = N/D
 *     =>
 *	    N = (By - Ay) * dAx - (Bx - Ax) * dAy;
 *		D = dBx * dAy - dBy * dAx;
 *      A simple floating gxPoint implementation would only need this, and
 *      the check to see if D is zero.
 *		But to gain speed we do some tricks and avoid floating gxPoint.
 *
 */
void fnt_ISECT(fnt_LocalGraphicStateType *gs)
{
	register F26Dot6 N, D;
	register fract t;
	register ArrayIndex arg1, arg2;
	F26Dot6 Bx, By, Ax, Ay;
	F26Dot6 dBx, dBy, dAx, dAy;

	{
		register fnt_ElementType* element = gs->CE0;
		register F26Dot6* stack = gs->stackPointer;

		arg2 = (ArrayIndex)CHECK_POP(gs, stack ); /* get one gxLine */
		arg1 = (ArrayIndex)CHECK_POP(gs, stack );
		dAx = element->x[arg2] - (Ax = element->x[arg1]);
		dAy = element->y[arg2] - (Ay = element->y[arg1]);

		element = gs->CE1;
		arg2 = (ArrayIndex)CHECK_POP(gs, stack ); /* get the other gxLine */
		arg1 = (ArrayIndex)CHECK_POP(gs, stack );
		dBx = element->x[arg2] - (Bx = element->x[arg1]);
		dBy = element->y[arg2] - (By = element->y[arg1]);

		arg1 = (ArrayIndex)CHECK_POP(gs, stack ); /* get the gxPoint number */
		gs->stackPointer = stack;
	}
	gs->CE2->f[arg1] |= XMOVED | YMOVED;
	{
		register F26Dot6* elementx = gs->CE2->x;
		register F26Dot6* elementy = gs->CE2->y;
		if ( dAy == 0 ) {
			if ( dBx == 0 ) {
				elementx[arg1] = Bx;
				elementy[arg1] = Ay;
				return;
			}
			N = By - Ay;
			D = -dBy;
		} else if ( dAx == 0 ) {
			if ( dBy == 0 ) {
				elementx[arg1] = Ax;
				elementy[arg1] = By;
				return;
			}
			N = Bx - Ax;
			D = -dBx;
		} else if ( MABS( dAx ) > MABS( dAy ) ) {
			/* To prevent out of range problems divide both N and D with the max */
			t = FractDivide( dAy, dAx );
			N = (By - Ay) - FractMultiply( (Bx - Ax), t );
			D = FractMultiply( dBx, t ) - dBy;
		} else {
			t = FractDivide( dAx, dAy );
			N = FractMultiply( (By - Ay), t ) - (Bx - Ax);
			D = dBx - FractMultiply( dBy, t );
		}

		if ( D ) {
			if ( MABS( N ) < MABS( D ) ) {
				/* this is the normal case */
				t = FractDivide( N, D );
				elementx[arg1] = Bx + FractMultiply( dBx, t );
				elementy[arg1] = By + FractMultiply( dBy, t );
			} else {
				if ( N ) {
					/* Oh well, invert t and use it instead */
					t = FractDivide( D, N );
					elementx[arg1] = Bx + FractDivide( dBx, t );
					elementy[arg1] = By + FractDivide( dBy, t );
				} else {
					elementx[arg1] = Bx;
					elementy[arg1] = By;
				}
			}
		} else {
			/* degenerate case: parallell lines, put gxPoint in the middle */
			elementx[arg1] = (Bx + (dBx>>1) + Ax + (dAx>>1)) >> 1;
			elementy[arg1] = (By + (dBy>>1) + Ay + (dAy>>1)) >> 1;
		}
	}
}

#define FRACSQRT2DIV2 759250125
/*
 * Internal support routine for the super rounding routines
 */
static void fnt_SetRoundValues(register fnt_LocalGraphicStateType *gs,
								register int arg1, register int normalRound)
{
	register int tmp;
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;

	tmp = arg1 & 0xC0;

	if ( normalRound ) {
		switch ( tmp ) {
		case 0x00:
			pb->period = fnt_pixelSize/2;
			break;
		case 0x40:
			pb->period = fnt_pixelSize;
			break;
		case 0x80:
			pb->period = fnt_pixelSize*2;
			break;
		default:
			pb->period = 999; /* Illegal */
		}
		pb->periodMask = ~(pb->period-1);
	} else {
		pb->period45 = FRACSQRT2DIV2;
		switch ( tmp ) {
		case 0x00:
			pb->period45 >>= 1;
			break;
		case 0x40:
			break;
		case 0x80:
			pb->period45 <<= 1;
			break;
		default:
			pb->period45 = 999; /* Illegal */
		}
		tmp = (sizeof(fract) * 8 - 2 - fnt_pixelShift);
		pb->period = (tt_int16)((pb->period45 + (1L<<(tmp-1))) >> tmp); /*convert from 2.30 to 26.6 */
	}

	tmp = arg1 & 0x30;
	switch ( tmp ) {
	case 0x00:
		pb->phase = 0;
		break;
	case 0x10:
		pb->phase = (pb->period + 2) >> 2;
		break;
	case 0x20:
		pb->phase = (pb->period + 1) >> 1;
		break;
	case 0x30:
		pb->phase = (pb->period + pb->period + pb->period + 2) >> 2;
		break;
	}
	tmp = arg1 & 0x0f;
	if ( tmp == 0 ) {
	    pb->threshold = pb->period-1;
	} else {
	    pb->threshold = (SHORTMUL(tmp - 4, pb->period) + 4) >> 3;
	}
}

/*
 * Super Round
 */
void fnt_SROUND(register fnt_LocalGraphicStateType *gs)
{
	register int arg1 = (int)CHECK_POP(gs, gs->stackPointer );
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;

	fnt_SetRoundValues( gs, arg1, true );
	pb->RoundValue = (FntRoundFunc) fnt_SuperRound;
}

/*
 * Super Round
 */
void fnt_S45ROUND(register fnt_LocalGraphicStateType *gs)
{
	register int arg1 = (int)CHECK_POP(gs, gs->stackPointer );
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;

	fnt_SetRoundValues( gs, arg1, false );
	pb->RoundValue = (FntRoundFunc) fnt_Super45Round;
}

/*
 * Read Advance Width
 */
void fnt_RAW(register fnt_LocalGraphicStateType *gs)
{
	fnt_ElementType* elem = gs->elements[GLYPHELEMENT];
	F26Dot6* ox = elem->ox;
	ArrayIndex index = elem->pointCount;		/* lsb gxPoint */

	GrowStackForPush(gs, 1);
	CHECK_PUSH( gs, gs->stackPointer, ox[index+1] - ox[index] );
}

/*
 * Set Angle Weight
 */
void fnt_SANGW(register fnt_LocalGraphicStateType *gs)
{
	CHECK_POP(gs, gs->stackPointer );
}

/*
 * AdjustAngle         <4>    		OBSOLETE as of TT version 3     <19> 
 */
void fnt_AA(register fnt_LocalGraphicStateType *gs)
{
CHECK_POP(gs, gs->stackPointer );
}

/*
 * DEBUG
 */
void fnt_DEBUG(register fnt_LocalGraphicStateType *gs)
{
	F26Dot6 arg = CHECK_POP(gs, gs->stackPointer );

	DebugMessage("DEBUG opcode", arg);
}

/*
 * UnTouch Point
 */
void fnt_UTP(register fnt_LocalGraphicStateType *gs)
{
	register ArrayIndex gxPoint = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
	register tt_uint8* f = gs->CE0->f;

	if ( gs->free.x ) {
		f[gxPoint] &= ~XMOVED;
	}
	if ( gs->free.y ) {
		f[gxPoint] &= ~YMOVED;
	}
}

/*
 * Set Delta Base
 */
void fnt_SDB(register fnt_LocalGraphicStateType *gs)
{
	gs->globalGS->localParBlock.deltaBase = (tt_int16)CHECK_POP(gs, gs->stackPointer );
}

/*
 * Set Delta Shift
 */
void fnt_SDS(register fnt_LocalGraphicStateType *gs)
{
	gs->globalGS->localParBlock.deltaShift = (tt_int16)CHECK_POP(gs, gs->stackPointer );
}

/* This code adjusts strokes so that their width is closer to the target width.
 * Only 1 bit is added or subtracted.
 * If the boolean part of the opCode is set, the edge that is moved is specified
 * the first argument; otherwise the edge that needs to move the least is moved.  
 */

void fnt_ADJUST( fnt_LocalGraphicStateType *gs )  /* <13> */
{
	ArrayIndex j1, j2, jMove;
	pixel target;
	subPixel movement = 0, *coord;
	tt_uint8 direction;
	boolean moveFirstOne =  gs->opCode == (ADJUSTBASE + 1);
	fnt_ElementType* ce0 = gs->CE0;

	if( gs->free.x == 0 ) {
	    coord = ce0->y;
	    direction = YMOVED;
	}		
	else {
	    coord = ce0->x;
	    direction = XMOVED;
	}
	target = gs->GetCVTEntry( gs, (ArrayIndex)CHECK_POP(gs, gs->stackPointer ) );
	CHECK_RANGE(gs, target, 0, 32767);
	target += HALFPIXEL;
	target >>= LG2PIXEL;
	if( target <= 0 ) target = 1;
	for (; gs->loop >= 0; --gs->loop) {
		j1 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
		j2 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
		CHECK_POINT( gs, ce0, j1 );
		CHECK_POINT( gs, ce0, j2 );
		jMove = moveFirstOne ? j1 : -1; 
		CALCEDGE(target, j1, j2, coord, &movement, jMove );
		if( jMove >= 0 )
			MOVEEDGE( jMove, movement, coord, ce0->sp,
					ce0->ep, ce0->contourCount, ce0->f, direction );
	}
	gs->loop = 0;
}

/*
 * Flip Point
 */
void fnt_FLIPPT(fnt_LocalGraphicStateType *gs)
{
	register tt_uint8 *onCurve = gs->CE0->onCurve;
	register F26Dot6* stack = gs->stackPointer;
	register LoopCount count = gs->loop;

	for (; count >= 0; --count) {
		register ArrayIndex gxPoint = (ArrayIndex)CHECK_POP(gs, stack );
		onCurve[ gxPoint ] ^= ONCURVE;
	}
	gs->loop = 0;

	gs->stackPointer = stack;
}

/*
 * Flip On a Range
 */
void fnt_FLIPRGON(register fnt_LocalGraphicStateType *gs)
{
	register ArrayIndex lo, hi;
	register LoopCount count;
	register tt_uint8 *onCurve = gs->CE0->onCurve;
	register F26Dot6* stack = gs->stackPointer;

	hi = (ArrayIndex)CHECK_POP(gs, stack );
	CHECK_POINT( gs, gs->CE0, hi );
	lo = (ArrayIndex)CHECK_POP(gs, stack );
	CHECK_POINT( gs, gs->CE0, lo );

	onCurve += lo;
	for (count = (LoopCount)(hi - lo); count >= 0; --count)
		*onCurve++ |= ONCURVE;
	gs->stackPointer = stack;
}

/*
 * Flip On a Range
 */
void fnt_FLIPRGOFF(register fnt_LocalGraphicStateType *gs)
{
	register ArrayIndex lo, hi;
	register LoopCount count;
	register tt_uint8 *onCurve = gs->CE0->onCurve;

	hi = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
	CHECK_POINT( gs, gs->CE0, hi );
	lo = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
	CHECK_POINT( gs, gs->CE0, lo );

	onCurve += lo;
	for (count = (LoopCount)(hi - lo); count >= 0; --count)
		*onCurve++ &= ~ONCURVE;
}

/*
 * Measure Point Size
 */
void fnt_MPS(register fnt_LocalGraphicStateType *gs)
{
	GrowStackForPush(gs, 1);
	CHECK_PUSH( gs, gs->stackPointer, gs->globalGS->pointSize );
}

/*
 *	This guy returns the index of the given opCode, or 0 if not found <4>
 */
static fnt_instrDef* fnt_FindIDef(fnt_LocalGraphicStateType* gs, register tt_uint8 opCode)
{
	register fnt_GlobalGraphicStateType* globalGS = gs->globalGS;
	register LoopCount count = globalGS->instrDefCount;
	register fnt_instrDef* instrDef = globalGS->instrDef;
	for (--count; count >= 0; instrDef++, --count)
		if (instrDef->opCode == opCode)
			return instrDef;
	return 0;
}

/*
 *	This guy gets called for opCodes that has been patch by the gxFont's IDEF	<4>
 *	or if they have never been defined.  If there is no corresponding IDEF,
 *	flag it as an illegal instruction.
 */
void fnt_IDefPatch( register fnt_LocalGraphicStateType* gs )
{
	register fnt_instrDef* instrDef = fnt_FindIDef(gs, gs->opCode);

	if (instrDef == 0)
		fnt_IllegalInstruction( gs );
	else {	
	    register tt_uint8* program;

	    CHECK_PROGRAM(gs, instrDef->pgmIndex);
	    program = gs->globalGS->pgmList[ instrDef->pgmIndex ];

	    program += instrDef->start;
	    gs->Interpreter( gs, program, program + instrDef->length);
	}
}

/*
 * Instruction DEFinition	<4>
 */
void fnt_IDEF( register fnt_LocalGraphicStateType* gs )
{
	register tt_uint8 opCode = (tt_uint8)CHECK_POP(gs, gs->stackPointer );
	register fnt_instrDef* instrDef = fnt_FindIDef(gs, opCode);
	register ArrayIndex pgmIndex = (ArrayIndex)gs->globalGS->pgmIndex;
	tt_uint8* program = gs->globalGS->pgmList[ pgmIndex ];
	tt_uint8* instrStart = gs->insPtr;

	CHECK_PROGRAM(gs, pgmIndex);

	if (!instrDef)
		instrDef = gs->globalGS->instrDef + gs->globalGS->instrDefCount++;
	if (pgmIndex == preProgramIndex)
		gs->globalGS->preProgramHasDefs = true;

	instrDef->pgmIndex = pgmIndex;
	instrDef->opCode = opCode;		/* this may or may not have been set */
	instrDef->start = gs->insPtr - program;

	while ( (gs->opCode = *gs->insPtr++) != ENDF_CODE )
		fnt_SkipPushCrap( gs );

	instrDef->length = gs->insPtr - instrStart - 1; /* don't execute ENDF */
}
#endif  
	/* #ifdef ENABLE_TT_HINTING */ 
