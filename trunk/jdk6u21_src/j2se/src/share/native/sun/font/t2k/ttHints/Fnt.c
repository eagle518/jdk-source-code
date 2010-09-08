/*
 * @(#)Fnt.c	1.21 10/04/02
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/

/* Exact motivation for this define is unknown.
   Perhaps the idea was to avoid using original unscaled outlines 
   and decrease memory requirements.
   However, due to limited precision of fixed point numbers 
   sometimes this leads to small rounding errors. Unfortunately 
   these can be very visible (e.g. Tahoma 11pt, 8).
   Therefore it is disabled for now (and perhaps should be removed).
 */   
#define REMOVECorrectUnscaledOutline 0
	
#include "Hint.h"
	
#ifdef ENABLE_TT_HINTING
	
#ifdef  debugging
#include <stdio.h>
#endif
#include <setjmp.h>

 
#include "FSCdefs.h"
#include "FontMath.h"
#include "Fnt.h"
#include "PrivateFnt.h"
#include "FntDebug.h"
#include "FSError.h"
#include "sc.h"
#include "FntInstructions.h"

local F26Dot6 fnt_CheckSingleWidth(F26Dot6 value, fnt_LocalGraphicStateType *gs);
fnt_instrDef* fnt_FindIDef(fnt_LocalGraphicStateType* gs, tt_uint8 opCode);
local void fnt_DeltaEngine(register fnt_LocalGraphicStateType *gs, FntMoveFunc doIt,
								tt_int16 base, tt_int16 shift);
#ifndef NOT_ON_THE_MAC
#ifdef debugging
void fnt_DDT(tt_int8 c, tt_int32 n);
#endif
#endif

#define FasterGrowStackForPush

/* a faster version of Project and OldProject in cases where the projection vector is set to either the x or y axis */
#ifdef FasterProject
#define FastProject(s,xx,yy)	((s)->proj.x == shortFrac1 ? (xx) \
					      :  (s)->proj.y == shortFrac1 ? (yy) \
					      :  fnt_Project(s,xx,yy))

#define FastOldProject(s,x,y)	((s)->OldProject == fnt_XProject ? (x) \
					      :  (s)->OldProject == fnt_YProject ? (y) \
					      :  (s)->OldProject(s,x,y))
#else
#define FastProject(s,x,y)	(((FntProjFunc) (s)->Project)(s,x,y))
#define FastOldProject(s,x,y)	(((FntProjFunc) (s)->OldProject)(s,x,y))
#endif

/* faster version of GrowStackForPush that doesn't do a function call when it doesn't need to grow the stack */
#ifdef FasterGrowStackForPush
#define FastGrowStackForPush(gs,count)	((gs)->stackPointer + (count) > (gs)->stackEnd	\
									? GrowStackForPush(gs, count)			\
									: (gs)->stackPointer)
#else
#define FastGrowStackForPush(gs,count)	GrowStackForPush(gs, count)
#endif

/* Actual instructions for the jump table */

#define fnt_NextPt1( pt, elem, ctr )	( (pt) == elem->ep[(ctr)] ? elem->sp[(ctr)] : (pt)+1 )

/*** <13> support macros for Kanji adjust to here  ***/ 

/*************************************************************************/

/*** 2 internal gs->pfProj computation support routines ***/

static short fnt_ProjectIntegerPPEM(register fnt_LocalGraphicStateType* gs)
{
	register fnt_GlobalGraphicStateType* globalGS = gs->globalGS;

	if (globalGS->integerPPEM.x != globalGS->integerPPEM.y) {
	  if (gs->proj.y == 0)
	    return globalGS->integerPPEM.x;
	  else if (gs->proj.x == 0)
	    return globalGS->integerPPEM.y;
	  else
	    return ((Magnitude(globalGS->integerPPEM.x * gs->proj.x,
			     globalGS->integerPPEM.y * gs->proj.y)
	            + (1 << 13)) >> 14);
	}
	else
		return globalGS->integerPPEM.x;
}

/******************************************/
/******** The Actual Instructions *********/
/******************************************/

/*
 * Set Vectors To Coordinate Axis - Y
 */
void fnt_SVTCA_0( register fnt_LocalGraphicStateType* gs )
{
	gs->free.x = gs->proj.x = 0;
	gs->free.y = gs->proj.y = shortFrac1;
	gs->MovePoint = (FntMoveFunc) fnt_YMovePoint;
	gs->Project = (FntProjFunc) fnt_YProject;
	gs->OldProject = (FntProjFunc) fnt_YProject;
	gs->pfProj = shortFrac1;
#ifdef debugging
	gs->valid_pfProj = true;
#endif
	gs->projectionVectorIsNormal = false;
}

/*
 * Set Vectors To Coordinate Axis - X
 */
void fnt_SVTCA_1( register fnt_LocalGraphicStateType* gs )
{
	gs->free.x = gs->proj.x = shortFrac1;
	gs->free.y = gs->proj.y = 0;
	gs->MovePoint = (FntMoveFunc) fnt_XMovePoint;
	gs->Project = (FntProjFunc) fnt_XProject;
	gs->OldProject = (FntProjFunc) fnt_XProject;
	gs->pfProj = shortFrac1;
#ifdef debugging
	gs->valid_pfProj = true;
#endif
	gs->projectionVectorIsNormal = false;
}

/*
 * Set Projection Vector To Coordinate Axis
 */
void fnt_SPVTCA( register fnt_LocalGraphicStateType* gs )
{
	if ( BIT0( gs->opCode )  ) {
		gs->proj.x = shortFrac1;
		gs->proj.y = 0;
		gs->Project = (FntProjFunc) fnt_XProject;
		gs->pfProj = gs->free.x;
	} else {
		gs->proj.x = 0;
		gs->proj.y = shortFrac1;
		gs->Project = (FntProjFunc) fnt_YProject;
		gs->pfProj = gs->free.y;
	}
	fnt_Check_PF_Proj( gs );
	gs->MovePoint = (FntMoveFunc) fnt_MovePoint;
	gs->OldProject = (FntProjFunc) gs->Project;
	gs->projectionVectorIsNormal = false;
}

/*
 * Set Freedom Vector to Coordinate Axis
 */
void fnt_SFVTCA( register fnt_LocalGraphicStateType* gs )
{
	if ( BIT0( gs->opCode )  ) {
		gs->free.x = shortFrac1;
		gs->free.y = 0;
		gs->pfProj = gs->proj.x;
	} else {
		gs->free.x = 0;
		gs->free.y = shortFrac1;
		gs->pfProj = gs->proj.y;
	}
	fnt_Check_PF_Proj( gs );
	gs->MovePoint = (FntMoveFunc) fnt_MovePoint;
}

/*
 * Set Projection Vector To Line
 */
void fnt_SPVTL(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex arg1, arg2;

    arg2 = (ArrayIndex) CHECK_POP(gs, gs->stackPointer);
    arg1 = (ArrayIndex) CHECK_POP(gs, gs->stackPointer);

    if (IS_BAD_POINT(gs, gs->CE2, arg2) || IS_BAD_POINT(gs, gs->CE1, arg1)) {
        FatalInterpreterError(gs, interp_range_error);
    }
	fnt_Normalize(gs, gs->CE1->x[arg1] - gs->CE2->x[arg2], gs->CE1->y[arg1] - gs->CE2->y[arg2], &gs->proj );

	gs->projectionVectorIsNormal = false;
	if ( BIT0( gs->opCode ) ) {
		/* rotate 90 degrees */
		shortFrac tmp		= gs->proj.y;
		gs->proj.y		= gs->proj.x;
		gs->proj.x		= -tmp;
		gs->projectionVectorIsNormal = true;
	}
	fnt_ComputeAndCheck_PF_Proj( gs );
	gs->MovePoint = (FntMoveFunc) fnt_MovePoint;
	gs->Project = (FntProjFunc) fnt_Project;
	gs->OldProject = (FntProjFunc) gs->Project;
}


/*
 * Set Freedom Vector To Line
 */
void fnt_SFVTL(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex arg1, arg2;

    arg2 = (ArrayIndex) CHECK_POP(gs, gs->stackPointer);
    arg1 = (ArrayIndex) CHECK_POP(gs, gs->stackPointer);

    if (IS_BAD_POINT(gs, gs->CE2, arg2) || IS_BAD_POINT(gs, gs->CE1, arg1)) {
        FatalInterpreterError(gs, interp_range_error);
    }

	fnt_Normalize(gs, gs->CE1->x[arg1] - gs->CE2->x[arg2], gs->CE1->y[arg1] - gs->CE2->y[arg2], &gs->free );
	if ( BIT0( gs->opCode ) ) {
		/* rotate 90 degrees */
		shortFrac tmp		= gs->free.y;
		gs->free.y		= gs->free.x;
		gs->free.x		= -tmp;
	}
	fnt_ComputeAndCheck_PF_Proj( gs );
	gs->MovePoint = (FntMoveFunc) fnt_MovePoint;
}


/*
 * Write Projection Vector From Stack
 *
 *	This is a dangerous function, since the interpreter can't know if the projection vector
 *	should be treated as a normal or parallel to the stretch (if any).
 */
void fnt_SPVFS(register fnt_LocalGraphicStateType *gs)
{
	gs->proj.y = (shortFrac)CHECK_POP(gs, gs->stackPointer);
	gs->proj.x = (shortFrac)CHECK_POP(gs, gs->stackPointer);

	fnt_ComputeAndCheck_PF_Proj( gs );

	gs->MovePoint = (FntMoveFunc) fnt_MovePoint;
	gs->Project = (FntProjFunc) fnt_Project;
	gs->OldProject = (FntProjFunc) gs->Project;
	gs->projectionVectorIsNormal = false;		/* Can't really know */
}

/*
 * Write Freedom vector
 */
void fnt_SFVFS(register fnt_LocalGraphicStateType *gs)
{
	gs->free.y = (shortFrac)CHECK_POP(gs, gs->stackPointer);
	gs->free.x = (shortFrac)CHECK_POP(gs, gs->stackPointer);

	fnt_ComputeAndCheck_PF_Proj( gs );

	gs->MovePoint = (FntMoveFunc) fnt_MovePoint;
}

/*
 * Read Projection Vector
 */
void fnt_GPV(register fnt_LocalGraphicStateType *gs)
{
	GrowStackForPush(gs, 2);
	CHECK_PUSH( gs, gs->stackPointer, gs->proj.x );
	CHECK_PUSH( gs, gs->stackPointer, gs->proj.y );
}

/*
 * Read Freedom Vector
 */
void fnt_GFV(register fnt_LocalGraphicStateType *gs)
{
	GrowStackForPush(gs, 2);
	CHECK_PUSH( gs, gs->stackPointer, gs->free.x );
	CHECK_PUSH( gs, gs->stackPointer, gs->free.y );
}

/*
 * Set Freedom Vector To Projection Vector
 */
void fnt_SFVTPV(register fnt_LocalGraphicStateType *gs)
{
	gs->free = gs->proj;
	gs->pfProj = shortFrac1;
	gs->MovePoint = (FntMoveFunc) fnt_MovePoint;
#ifdef debugging
	gs->valid_pfProj = true;
#endif
}

/*
 * Load Minimum Distanc
 */
void fnt_SMD(register fnt_LocalGraphicStateType *gs)
{
	gs->globalGS->localParBlock.minimumDistance = CHECK_POP(gs, gs->stackPointer );
}

/*
 * Load Control Value Table Cut In
 */
void fnt_SCVTCI(register fnt_LocalGraphicStateType *gs)
{
	gs->globalGS->localParBlock.wTCI = CHECK_POP(gs, gs->stackPointer );
}

/*
 * Load Single Width Cut In
 */
void fnt_SSWCI(register fnt_LocalGraphicStateType *gs)
{
	gs->globalGS->localParBlock.sWCI = CHECK_POP(gs, gs->stackPointer );
}

/*
 * Load Single Width , assumes value comes from the original domain, not the cvt or outline
 */
void fnt_SSW(register fnt_LocalGraphicStateType *gs)
{
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
	register fnt_ParameterBlock *pb = &globalGS->localParBlock;

	pb->sW = (tt_int16)CHECK_POP(gs, gs->stackPointer );

	pb->scaledSW = FixedMultiply(globalGS->upemScale.x, pb->sW); /* measurement should not come from the outline */
}

void fnt_SRP0(register fnt_LocalGraphicStateType *gs) {
    gs->Pt0 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer);
    /* Note: we can not verify value of Pt0 right here because:
         - CE pointers can be NULL still
         - we do not know which CE will be referenced with this pt */
}

void fnt_SRP1(register fnt_LocalGraphicStateType *gs) {
    gs->Pt1 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer);
    /* Note: we can not verify value of Pt1 right here because:
         - CE pointers can be NULL still
         - we do not know which CE will be referenced with this pt */
}

void fnt_SRP2(register fnt_LocalGraphicStateType *gs) {
    gs->Pt2 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
    /* Note: we can not verify value of Pt2 right here because:
         - CE pointers can be NULL still
         - we do not know which CE will be referenced with this pt */
}

void fnt_SLOOP(register fnt_LocalGraphicStateType *gs)
{
	gs->loop = (LoopCount)CHECK_POP(gs, gs->stackPointer ) - 1;
}

void fnt_POP(register fnt_LocalGraphicStateType *gs)
{
	CHECK_POP(gs, gs->stackPointer );
}

void fnt_SetElementPtr(register fnt_LocalGraphicStateType *gs)
{
    ArrayIndex arg = (ArrayIndex) CHECK_POP(gs, gs->stackPointer);
    fnt_ElementType* element;

    if (IS_BAD_ELEM_INDEX(gs, arg)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

    element = gs->elements[arg];

	switch (gs->opCode) {
	case SCES_CODE:	gs->CE2 = element;
					gs->CE1 = element;
	case SCE0_CODE:	gs->CE0 = element; break;
	case SCE1_CODE:	gs->CE1 = element; break;
	case SCE2_CODE:	gs->CE2 = element; break;
	}
}

/*
 *	These functions just set a field of the graphics state
 *	They pop no arguments
 */
void fnt_SetRoundState(register fnt_LocalGraphicStateType *gs)
{
	FntRoundFunc *rndFunc = &gs->globalGS->localParBlock.RoundValue;

	switch (gs->opCode) {
	case RTG_CODE:  *rndFunc = (FntRoundFunc) fnt_RoundToGrid; break;
	case RTHG_CODE: *rndFunc = (FntRoundFunc) fnt_RoundToHalfGrid; break;
	case RTDG_CODE: *rndFunc = (FntRoundFunc) fnt_RoundToDoubleGrid; break;
	case ROFF_CODE: *rndFunc = (FntRoundFunc) fnt_RoundOff; break;
	case RDTG_CODE: *rndFunc = (FntRoundFunc) fnt_RoundDownToGrid; break;
	case RUTG_CODE: *rndFunc = (FntRoundFunc) fnt_RoundUpToGrid; break;
	}
}


/*
 * DUPlicate
 */
void fnt_DUP(register fnt_LocalGraphicStateType *gs) {
    F26Dot6 top;

    if (IS_BAD_STACK_POINTER(gs, gs->stackPointer-1)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }
    top = gs->stackPointer[-1];
    
    GrowStackForPush(gs, 1);
    CHECK_PUSH(gs, gs->stackPointer, top);
}

/*
 * CLEAR stack
 */
void fnt_CLEAR(register fnt_LocalGraphicStateType *gs)
{
	gs->stackPointer = gs->stackBase;
}

/*
 * SWAP
 */
void fnt_SWAP(register fnt_LocalGraphicStateType *gs)
{
	register F26Dot6* stack = gs->stackPointer;
	register F26Dot6 arg2 = CHECK_POP(gs, stack );
	register F26Dot6 arg1 = CHECK_POP(gs, stack );

	CHECK_PUSH( gs, stack, arg2 );
	CHECK_PUSH( gs, stack, arg1 );
}

/*
 * DEPTH
 */
void fnt_DEPTH(register fnt_LocalGraphicStateType *gs)
{
	F26Dot6 depth = gs->stackPointer - gs->stackBase;
	GrowStackForPush(gs, 1);
	CHECK_PUSH( gs, gs->stackPointer, depth);
}

/*
 * Copy INDEXed value
 */
void fnt_CINDEX(register fnt_LocalGraphicStateType *gs) {
    register ArrayIndex arg1;
    register F26Dot6 *p, *stack = gs->stackPointer;

    arg1 = (ArrayIndex) CHECK_POP(gs, stack);
    p = stack - arg1;
    if (IS_BAD_STACK_POINTER(gs, p)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }
    CHECK_PUSH(gs, stack, *p);
}

/*
 * Move INDEXed value
 */
void fnt_MINDEX(register fnt_LocalGraphicStateType *gs) {
    register ArrayIndex arg1;
    register F26Dot6 tmp, *p;
    register F26Dot6* stack = gs->stackPointer;

    arg1 = (ArrayIndex) CHECK_POP(gs, stack);
    p = (stack - arg1);
    if (IS_BAD_STACK_POINTER(gs, p)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }
    tmp = *p;
    if (arg1) {
        do {
            if (IS_BAD_STACK_POINTER(gs, p+1)) {
                FatalInterpreterError(gs, interp_font_program_error);
            }
            *p = *(p + 1); 
            p++;
        } while (--arg1);
        CHECK_POP(gs, stack );
    }

    CHECK_PUSH(gs, stack, tmp);
    gs->stackPointer = stack;
}

/*
 *	Rotate element 3 to the top of the stack			<4>
 *	Thanks to Oliver for the obscure code.
 */
void fnt_ROLL( register fnt_LocalGraphicStateType* gs )
{
    register F26Dot6 *stack = gs->stackPointer;
    register F26Dot6 element1, element2;

    if (IS_BAD_STACK_POINTER(gs, stack) || IS_BAD_STACK_POINTER(gs, stack-2)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }
    element1 = *--stack;
    element2 = *--stack;

	*stack = element1;
	element1 = *--stack;
	*stack = element2;
	*(stack + 2) = element1;
}

/*
 * Move Direct Absolute Point
 */
void fnt_MDAP(fnt_LocalGraphicStateType *gs)
{
	F26Dot6 proj, currProj;
	fnt_ElementType* ce0 = gs->CE0;
	ArrayIndex ptNum;
	ptNum = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );

	if (IS_BAD_POINT(gs, ce0, ptNum)) {
        FatalInterpreterError(gs, interp_range_error);
	}
	
	gs->Pt0 = gs->Pt1 = ptNum;

	currProj = proj = (F26Dot6) FastProject( gs, ce0->x[ptNum], ce0->y[ptNum] );

	if ( BIT0( gs->opCode ) )
#ifdef use_engine_characteristics_in_hints
	{	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;

		proj = globalGS->localParBlock.RoundValue( proj, globalGS->engine[0], gs );
	}
#else
		proj = ((FntRoundFunc)gs->globalGS->localParBlock.RoundValue)( proj, gs );
#endif
	gs->MovePoint( gs, ce0, ptNum, proj - currProj );
}

/*
 * Move Indirect Absolute Point
 */
void fnt_MIAP(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex ptNum, n;
	register F26Dot6 newProj, origProj, currProj;
	register fnt_ElementType* ce0 = gs->CE0;

    n = CHECK_POP(gs, gs->stackPointer);
    ptNum = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );

    if (IS_BAD_POINT(gs, ce0, ptNum) || IS_BAD_CVT_INDEX(gs, n)) {
        FatalInterpreterError(gs, interp_range_error);
    }
    newProj = gs->GetCVTEntry(gs, n);

#ifdef support_fake_variation_cvt
	if ( gs->fakeVariantCVT && ce0 != gs->elements[TWILIGHTZONE] )
		newProj = FastProject(gs, ce0->ox[ptNum], ce0->oy[ptNum]);
#endif
	gs->Pt0 = gs->Pt1 = ptNum;

	if ( ce0 == gs->elements[TWILIGHTZONE] )
	{
		ce0->x[ptNum] = ce0->ox[ptNum] = ShortFracMul( newProj, gs->proj.x );
		ce0->y[ptNum] = ce0->oy[ptNum] = ShortFracMul( newProj, gs->proj.y );
	}

	currProj = origProj = FastProject( gs, ce0->x[ptNum], ce0->y[ptNum] );

	if ( BIT0( gs->opCode ) )
	{	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
		register F26Dot6 tmp = newProj - origProj;

		if ( tmp < 0 )
			tmp = -tmp;
		if ( tmp > globalGS->localParBlock.wTCI )
			newProj = origProj;
#ifdef use_engine_characteristics_in_hints
		newProj = globalGS->localParBlock.RoundValue( newProj, globalGS->engine[0], gs );
#else
		newProj = globalGS->localParBlock.RoundValue( newProj, gs );
#endif
	}

	gs->MovePoint( gs, ce0, ptNum, newProj - currProj );
}

/*
 * Interpolate Untouched Points
 */
void fnt_IUP(fnt_LocalGraphicStateType *gs)
{
	register fnt_ElementType* CE2 = gs->CE2;
	register tt_int32 tmp32B;
	F26Dot6 *coord, *oCoord;
	short *ooCoord;
    LoopCount ctr;
	F26Dot6 dx, dx1, dx2;
	F26Dot6 dlow, dhigh;
	F26Dot6 tmp32, high, low;
	fastInt mask;
	ArrayIndex tmp16B;

	if ( gs->opCode & 0x01 ) {
		/* do x */
		coord = CE2->x;
		oCoord = CE2->ox;
		ooCoord = CE2->oox;
		mask = XMOVED;
	} else {
		/* do y */
		coord = CE2->y;
		oCoord = CE2->oy;
		ooCoord = CE2->ooy;
		mask = YMOVED;
	}
	
	for ( ctr = 0; ctr < CE2->contourCount; ctr++ )
	{
		ArrayIndex start = CE2->sp[ctr];
		tt_int16 tmp16 = CE2->ep[ctr];
		while( !(CE2->f[start] & mask) && start <= tmp16 )
			start++;
		if ( start > tmp16 )
			continue;
		tmp16B = start;
		do {
			ArrayIndex end;
			tmp16 = end = fnt_NextPt1( start, CE2, ctr );
			while( !(CE2->f[end] & mask) ) {
				end = fnt_NextPt1( end, CE2, ctr );
				if ( start == end )
					break;
			}

			if ( ooCoord[start] < ooCoord[end] ) 
			{
				dx  = coord[start];
				dx1 = oCoord[start];
				dx2 = ooCoord[start];
				high = oCoord[end];
				dhigh = coord[end] - high;
				tmp32  = coord[end] - dx;
				tmp32B = ooCoord[end] - dx2;
			} 
			else 			
			{
				dx  = coord[end];
				dx1 = oCoord[end];
				dx2 = ooCoord[end];
				high = oCoord[start];
				dhigh = coord[start] - high;
				tmp32  = coord[start] - dx;
				tmp32B = ooCoord[start] - dx2;
			}
			low = dx1;
			dlow = dx - dx1;

			if ( tmp32B ) {
				if ( tmp32B < 32768 && tmp32 < 32768 )
				{
					F26Dot6 corr = tmp32B >> 1;
					while ( tmp16 != end )
					{
						F26Dot6 tmp32C = oCoord[tmp16];
						if ( tmp32C <= low )
							tmp32C += dlow;
						else if ( tmp32C >= high )
							tmp32C += dhigh;
						else
						{
							tmp32C = ooCoord[tmp16];
							tmp32C -= dx2;
							tmp32C  = SHORTMUL(tmp32C, tmp32);
							tmp32C += corr;
							if ( tmp32C < 32768 )
							    tmp32C = SHORTDIV(tmp32C, tmp32B);
							else
							    tmp32C /= (tt_int16)tmp32B;
							tmp32C += dx;
						}
						coord[tmp16] = tmp32C;
						tmp16 = fnt_NextPt1( tmp16, CE2, ctr);
					}
				}
				else
				{	fixed ratio = 0;
					boolean firstTime = true;

					while ( tmp16 != end )
					{	F26Dot6 tmp32C = oCoord[tmp16];

						if ( tmp32C <= low )
							tmp32C += dlow;
						else if ( tmp32C >= high )
							tmp32C += dhigh;
						else
						{	if ( firstTime )
							{	ratio = FixedDivide( tmp32, tmp32B );
								firstTime = false;
							}
							tmp32C = ooCoord[tmp16];
							tmp32C -= dx2;
							tmp32C = FixedMultiply( tmp32C, ratio );
							tmp32C += dx;
						}
						coord[tmp16] = tmp32C;
						tmp16 = fnt_NextPt1( tmp16, CE2, ctr);
					}
				}
			} else {
				while ( tmp16 != end ) {
					coord[tmp16] += dx - dx1;
					tmp16 = fnt_NextPt1( tmp16, CE2, ctr);
				}
			}
			start = end;
		} while ( start != tmp16B );
	}

}

static fnt_ElementType* fnt_SH_Common(fnt_LocalGraphicStateType* gs, F26Dot6* dx, F26Dot6* dy, ArrayIndex* gxPoint)
{
	F26Dot6 proj;
	ArrayIndex pt;
	fnt_ElementType* element;

	if ( BIT0( gs->opCode ) ) {
		pt = gs->Pt1;
		element = gs->CE0;
	} else {
		pt = gs->Pt2;
		element = gs->CE1;
	}

    if (IS_BAD_POINT(gs, element, pt)) {
        FatalInterpreterError(gs, interp_range_error);
    }
	
	proj = gs->Project( gs, element->x[pt] - element->ox[pt],
                            element->y[pt] - element->oy[pt] );

	CHECK_PFPROJ( gs );
	if ( gs->pfProj != shortFrac1 )
	{	if ( gs->free.x )
			*dx = ShortMulDiv( proj, gs->free.x, gs->pfProj );
		if ( gs->free.y )
			*dy = ShortMulDiv( proj, gs->free.y, gs->pfProj );
	}
	else
	{	if ( gs->free.x )
			*dx = ShortFracMul( proj, gs->free.x );
		if ( gs->free.y )
			*dy = ShortFracMul( proj, gs->free.y );
	}
	*gxPoint = pt;
	return element;
}

static void fnt_SHP_Common(fnt_LocalGraphicStateType *gs, F26Dot6 dx, F26Dot6 dy)
{
	register fnt_ElementType* CE2 = gs->CE2;
	register LoopCount count = gs->loop;
	for (; count >= 0; --count)
	{
		ArrayIndex gxPoint = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
		if (IS_BAD_POINT(gs, CE2, gxPoint)) {
			continue;
		}

		if ( gs->free.x ) {
			CE2->x[gxPoint] += dx;
			CE2->f[gxPoint] |= XMOVED;
		}
		if ( gs->free.y ) {
			CE2->y[gxPoint] += dy;
			CE2->f[gxPoint] |= YMOVED;
		}
	}
	gs->loop = 0;
}

/*
 * SHift Point
 */
void fnt_SHP(register fnt_LocalGraphicStateType *gs)
{
	F26Dot6 dx, dy;
	ArrayIndex gxPoint;

	fnt_SH_Common(gs, &dx, &dy, &gxPoint);
	fnt_SHP_Common(gs, dx, dy);
}

/*
 * SHift Contour
 */
void fnt_SHC(register fnt_LocalGraphicStateType *gs)
{
    register fnt_ElementType *element;
	register F26Dot6 dx, dy;
	register ArrayIndex contour, gxPoint;

	{
		F26Dot6 x, y;
		ArrayIndex pt;
		element = fnt_SH_Common(gs, &x, &y, &pt);
		gxPoint = pt;
		dx = x;
		dy = y;
	}
    contour = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );

    if (IS_BAD_CONTOUR_INDEX(gs, gs->CE2, contour)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

#ifdef performRuntimeErrorChecking
	if (contour >= 0 && contour < gs->CE2->contourCount)
#endif
	{
		shortFrac fvx = gs->free.x;
		shortFrac fvy = gs->free.y;
		register fnt_ElementType* CE2 = gs->CE2;
		ArrayIndex currPt = CE2->sp[contour];
		LoopCount count = CE2->ep[contour] - currPt;

        if (IS_BAD_POINT(gs, CE2, currPt) || IS_BAD_POINT(gs, CE2, currPt + count)) {
            FatalInterpreterError(gs, interp_range_error);
        }

		for (; count >= 0; --count)
		{
			if ( currPt != gxPoint || element != CE2 )
			{
				if ( fvx ) {
					CE2->x[currPt] += dx;
					CE2->f[currPt] |= XMOVED;
				}
				if ( fvy ) {
					CE2->y[currPt] += dy;
					CE2->f[currPt] |= YMOVED;
				}
			}
			currPt++;
		}
	}
}

/*
 * SHift Element			<4>
 */
void fnt_SHZ(register fnt_LocalGraphicStateType *gs)
{
    register fnt_ElementType *element;
	register F26Dot6 dx, dy;
	ArrayIndex firstPoint, origPoint, lastPoint, arg1;

	{
		F26Dot6 x, y;
		element = fnt_SH_Common(gs, &x, &y, &origPoint);
		dx = x;
		dy = y;
	}

	arg1 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
    if (IS_BAD_ELEM_INDEX(gs, arg1)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

	lastPoint = gs->elements[arg1]->pointCount - 1;
	firstPoint  = gs->elements[arg1]->sp[0];
    if (IS_BAD_POINT(gs, gs->elements[arg1], lastPoint) || 
        IS_BAD_POINT(gs, gs->elements[arg1], firstPoint)) {
        FatalInterpreterError(gs, interp_range_error);
    }

/*** changed this			<4>
	do {
		if ( origPoint != firstPoint || element != &gs->elements[arg1] ) {
			if ( gs->free.x ) {
				gs->elements[ arg1 ].x[firstPoint] += dx;
				gs->elements[ arg1 ].f[firstPoint] |= XMOVED;
			}
			if ( gs->free.y ) {
				gs->elements[ arg1 ].y[firstPoint] += dy;
				gs->elements[ arg1 ].f[firstPoint] |= YMOVED;
			}
		}
		firstPoint++;
	} while ( firstPoint <= lastPoint );
***** To this ? *********/

	if (element != gs->elements[arg1])		/* we're in different zones */
		origPoint = -1;						/* no need to skip orig gxPoint */
	{
		register tt_int8 mask = 0;
		if ( gs->free.x )
		{
			register F26Dot6 deltaX = dx;
			register F26Dot6* x = &gs->elements[ arg1 ]->x[firstPoint];
			register LoopCount count = origPoint - firstPoint - 1;
			for (; count >= 0; --count )
				*x++ += deltaX;
			if (origPoint == -1)
				count = lastPoint - firstPoint;
			else
			{
				count = lastPoint - origPoint - 1;
				x++;							/* skip origPoint */
			}
			for (; count >= 0; --count )
				*x++ += deltaX;
			mask = XMOVED;
		}
		if ( gs->free.y )		/* fix me semore */
		{
			register F26Dot6 deltaY = dy;
			register F26Dot6* y = &gs->elements[ arg1 ]->y[firstPoint];
			register tt_uint8* f = &gs->elements[ arg1 ]->f[firstPoint];
			register LoopCount count = origPoint - firstPoint - 1;
			for (; count >= 0; --count )
			{
				*y++ += deltaY;
				*f++ |= mask;
			}
			if (origPoint == -1)
				count = lastPoint - firstPoint;
			else
			{
				count = lastPoint - origPoint - 1;
				y++, f++;						/* skip origPoint */
			}
			mask |= YMOVED;
			for (; count >= 0; --count )
			{
				*y++ += deltaY;
				*f++ |= mask;
			}
		}
	}
}

/*
 * SHift gxPoint by PIXel amount
 */
void fnt_SHPIX(register fnt_LocalGraphicStateType *gs)
{
    register F26Dot6 proj, dx, dy;
    proj = dx = dy = 0;

    proj = CHECK_POP(gs, gs->stackPointer );
    if ( gs->free.x )
      dx = ShortFracMul( proj, gs->free.x );
    if ( gs->free.y )
      dy = ShortFracMul( proj, gs->free.y );

    fnt_SHP_Common(gs, dx, dy);
}

/*
 * Interpolate Point
 */
void fnt_IP(register fnt_LocalGraphicStateType *gs)
{
	F26Dot6 oldRange, currentRange;
	register ArrayIndex RP1 = gs->Pt1;
	register ArrayIndex pt2 = gs->Pt2;
	register fnt_ElementType* CE0 = gs->CE0;
	boolean twilight =	CE0 == gs->elements[TWILIGHTZONE] || gs->CE1 == gs->elements[TWILIGHTZONE]
					|| gs->CE2 == gs->elements[TWILIGHTZONE];

    if (IS_BAD_POINT(gs, gs->CE1, pt2) || IS_BAD_POINT(gs, CE0, RP1)) {
        FatalInterpreterError(gs, interp_range_error);
    }

	currentRange = FastProject( gs, gs->CE1->x[pt2] - CE0->x[RP1], gs->CE1->y[pt2] - CE0->y[RP1] );
	if (REMOVECorrectUnscaledOutline || twilight )
		oldRange = FastOldProject( gs, gs->CE1->ox[pt2] - CE0->ox[RP1], gs->CE1->oy[pt2] - CE0->oy[RP1] );
	else
	{
		oldRange = FastOldProject( gs, gs->CE1->oox[pt2] - CE0->oox[RP1], gs->CE1->ooy[pt2] - CE0->ooy[RP1] );
	}
 	
	for (; gs->loop >= 0; --gs->loop)
	{
		register ArrayIndex arg1 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
		register F26Dot6 tmp;

        if (IS_BAD_POINT(gs, gs->CE2, arg1)) {
            FatalInterpreterError(gs, interp_range_error);
        }

		if (REMOVECorrectUnscaledOutline || twilight )
			tmp = FastOldProject( gs, gs->CE2->ox[arg1] - CE0->ox[RP1],
									  gs->CE2->oy[arg1] - CE0->oy[RP1] );
		else
			tmp = FastOldProject( gs, gs->CE2->oox[arg1] - CE0->oox[RP1],
									  gs->CE2->ooy[arg1] - CE0->ooy[RP1] );

		if ( oldRange )
			tmp = MultiplyDivide( currentRange, tmp, oldRange );

		tmp -= FastProject( gs, gs->CE2->x[arg1] - CE0->x[RP1],
							    gs->CE2->y[arg1] - CE0->y[RP1] ); /* delta = desired projection - current projection */
		gs->MovePoint( gs, gs->CE2, arg1, tmp );
	}
	gs->loop = 0;
}

/*
 * Move Stack Indirect Relative Point
 */
void fnt_MSIRP( fnt_LocalGraphicStateType* gs )
{
	register fnt_ElementType* CE0 = gs->CE0;
	register fnt_ElementType* CE1 = gs->CE1;
	register ArrayIndex Pt0 = gs->Pt0;
	register F26Dot6 dist = CHECK_POP(gs, gs->stackPointer ); /* distance   */
	register ArrayIndex pt2 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer ); /* gxPoint   */

    if (IS_BAD_POINT(gs, CE1, pt2) || IS_BAD_POINT(gs, CE0, Pt0)) {
        FatalInterpreterError(gs, interp_range_error);
    }

	if ( CE1 == gs->elements[TWILIGHTZONE] )
	{
		CE1->ox[pt2] = CE0->ox[Pt0] + ShortFracMul( dist, gs->proj.x );
		CE1->oy[pt2] = CE0->oy[Pt0] + ShortFracMul( dist, gs->proj.y );
		CE1->x[pt2] = CE0->x[Pt0];										/* <10> */
		CE1->y[pt2] = CE0->y[Pt0];										/* <10> */
	}
#ifdef support_fake_variation_cvt
	else if ( gs->fakeVariantCVT )
	{	fastInt fractionalPart = dist & 63;

		dist = gs->Project(gs, CE1->ox[pt2] - CE0->ox[Pt0], CE1->oy[pt2] - CE0->oy[Pt0]);
		if (!fractionalPart)		/* re-round dist */
#ifdef use_engine_characteristics_in_hints
			dist = fnt_RoundToGrid(dist, 0, 0);
#else
			dist = fnt_RoundToGrid(dist, 0);
#endif
	}
#endif
	dist -= gs->Project( gs, CE1->x[pt2] - CE0->x[Pt0], CE1->y[pt2] - CE0->y[Pt0] );
	gs->MovePoint( gs, CE1, pt2, dist );
	gs->Pt1 = Pt0;
	gs->Pt2 = pt2;
	if ( BIT0( gs->opCode ) )
		gs->Pt0 = pt2;		/* move the reference gxPoint */
}

/*
 * Align Relative Point
 */
void fnt_ALIGNRP(register fnt_LocalGraphicStateType *gs) {
    register fnt_ElementType* ce1 = gs->CE1;
    register F26Dot6 pt0x, pt0y, proj;
    register ArrayIndex ptNum;

    if (IS_BAD_POINT(gs, gs->CE0, gs->Pt0)) {
        FatalInterpreterError(gs, interp_range_error);
    }

    pt0x = gs->CE0->x[gs->Pt0];
    pt0y = gs->CE0->y[gs->Pt0];

    for (; gs->loop >= 0; --gs->loop) {
        ptNum = (ArrayIndex) CHECK_POP(gs, gs->stackPointer);
        if (IS_BAD_POINT(gs, ce1, ptNum)) {
            FatalInterpreterError(gs, interp_range_error);
        }
        proj = -FastProject(gs, ce1->x[ptNum] - pt0x, ce1->y[ptNum] - pt0y);
        gs->MovePoint( gs, ce1, ptNum, proj );
    }
    gs->loop = 0;
}


/*
 * Align Two Points ( by moving both of them )
 */
void fnt_ALIGNPTS(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex pt1, pt2;
	register F26Dot6 move1, dist;

	pt2  = (ArrayIndex)CHECK_POP(gs, gs->stackPointer ); /* gxPoint # 2   */
	pt1  = (ArrayIndex)CHECK_POP(gs, gs->stackPointer ); /* gxPoint # 1   */

    if (IS_BAD_POINT(gs, gs->CE0, pt2) || IS_BAD_POINT(gs, gs->CE1, pt1)) {
        FatalInterpreterError(gs, interp_range_error);
    }

	/* We do not have to check if we are in character element zero (the twilight zone)
	   since both points already have to have defined values before we execute this instruction */
	dist = gs->Project( gs, gs->CE0->x[pt2] - gs->CE1->x[pt1],
							 gs->CE0->y[pt2] - gs->CE1->y[pt1] );

	move1 = dist >> 1;
	gs->MovePoint( gs, gs->CE0, pt1, move1 );
	gs->MovePoint( gs, gs->CE1, pt2, move1 - dist ); /* make sure the total movement equals tmp32 */
}


/* 4/22/90 rwb - made more general
 * Sets lower 16 flag bits of ScanControl variable.  Sets scanContolIn if we are in one
 * of the preprograms; else sets scanControlOut.
 *
 * stack: value => -;
 *
 */
void fnt_SCANCTRL(register fnt_LocalGraphicStateType *gs)
{
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
	register fnt_ParameterBlock *pb = &globalGS->localParBlock;

	pb->scanControl = (pb->scanControl & 0xFFFF0000) | CHECK_POP(gs, gs->stackPointer );
}

/* 5/24/90 rwb
 * Sets upper 16 bits of ScanControl variable. Sets scanContolIn if we are in one
 * of the preprograms; else sets scanControlOut.
 */

void fnt_SCANTYPE(register fnt_LocalGraphicStateType *gs)
{
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
	register fnt_ParameterBlock *pb = &globalGS->localParBlock;
	register fastInt value = (fastInt)CHECK_POP(gs, gs->stackPointer );
	register tt_int32 *scanPtr = &(pb->scanControl);

	/* Apple Truetype spec defines only modes 0, 1 and 2.
	   Microsoft Opentype specification introduces modes 4 an 5
	   and also specifies that values 3, 6 and 7 should be treated as 2. */
	if (value != 0 && value != 1 && value != 4 && value != 5) {
            value = 2;
        }

	/* save scan type to upper 16 bits */
	*scanPtr = (value << 16) | (*scanPtr & 0xFFFF);
}

/* 6/28/90 rwb
 * Sets instructControl flags in global graphic state.  Only legal in pre program.
 * A selector is used to choose the flag to be set.
 * Bit0 - NOGRIDFITFLAG - if set, then truetype instructions are not executed.
 * 		A gxFont may want to use the preprogram to check if the glyph is rotated or
 * 	 	transformed in such a way that it is better to not gridfit the glyphs.
 * Bit1 - DEFAULTFLAG - if set, then changes in localParameterBlock variables in the
 *		globalGraphics state made in the CVT preprogram are not copied back into
 *		the defaultParameterBlock.  So, the original default values are the starting
 *		values for each glyph.
 *
 * stack: value, selector => -;
 *
 */
void fnt_INSTCTRL(register fnt_LocalGraphicStateType *gs)  /* old<13> */
{
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
	register tt_int32 *ic	= &globalGS->localParBlock.instructControl;
	fastInt selector 	= (fastInt)CHECK_POP(gs, gs->stackPointer );
	tt_uint32 value 		= (tt_uint32)CHECK_POP(gs, gs->stackPointer );
	if( globalGS->pgmIndex == preProgramIndex )
	{
		if( selector == 1 )
		{
			*ic &= ~NOGRIDFITFLAG;
			*ic |= (value & NOGRIDFITFLAG);
		}
		else if( selector == 2 )
		{
			*ic &= ~DEFAULTFLAG;
			*ic |= (value & DEFAULTFLAG);
		}
	}
}

/*
 *	Called by fnt_PUSHB and fnt_NPUSHB 
 */
local void fnt_PushSomeBytes(fnt_LocalGraphicStateType *gs, register LoopCount count);
local void fnt_PushSomeBytes(fnt_LocalGraphicStateType *gs, register LoopCount count)
{
	register F26Dot6* stack = FastGrowStackForPush(gs, count);
	register tt_uint8* instr = gs->insPtr;

    for (--count; count >= 0; --count) {
        if (IS_BAD_INSTR_POINTER(gs, instr)) {
            FatalInterpreterError(gs, interp_font_program_error);
        }
        CHECK_PUSH(gs, stack, GETBYTE(instr));
    }
	gs->stackPointer = stack;
	gs->insPtr = instr;
}

/*
 *	Called by fnt_PUSHW and fnt_NPUSHW
 */
local void fnt_PushSomeWords(fnt_LocalGraphicStateType *gs, register LoopCount count);
local void fnt_PushSomeWords(fnt_LocalGraphicStateType *gs, register LoopCount count)
{
	register F26Dot6* stack = FastGrowStackForPush(gs, count);
	register tt_uint8* instr = gs->insPtr;

    for (--count; count >= 0; --count) {
        tt_int16 word;
        if (IS_BAD_INSTR_POINTER(gs, instr) || IS_BAD_INSTR_POINTER(gs, instr+1)) {
            FatalInterpreterError(gs, interp_font_program_error);
        }

        word = GETBYTE(instr);
        word = (word << 8) + GETBYTE(instr);
        CHECK_PUSH(gs, stack, word);
    }
	gs->stackPointer = stack;
	gs->insPtr = instr;
}

/*
 * PUSH Bytes		<3>
 */
void fnt_PUSHB(fnt_LocalGraphicStateType *gs)
{
	fnt_PushSomeBytes(gs, (LoopCount) (gs->opCode - PUSHB_START + 1) );
}

/* a separate (and faster) handler for PUSHB 0 */
void fnt_PUSHB0(fnt_LocalGraphicStateType *gs)
{
	register F26Dot6* stack = FastGrowStackForPush(gs, 1);
	register tt_uint8* instr = gs->insPtr;
    if (IS_BAD_INSTR_POINTER(gs, instr)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }
    CHECK_PUSH(gs, stack, GETBYTE(instr));

	gs->stackPointer = stack;
	gs->insPtr = instr;
}

/*
 * N PUSH Bytes
 */
void fnt_NPUSHB(register fnt_LocalGraphicStateType *gs) {
    if (IS_BAD_INSTR_POINTER(gs, gs->insPtr)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

    fnt_PushSomeBytes(gs, GETBYTE(gs->insPtr));
}

/*
 * PUSH Words		<3>
 */
void fnt_PUSHW(register fnt_LocalGraphicStateType *gs)
{
	fnt_PushSomeWords(gs, (LoopCount) (gs->opCode - PUSHW_START + 1));
}

/* a separate (and faster) handler for PUSHW 0 */
void fnt_PUSHW0(fnt_LocalGraphicStateType *gs)
{
	register F26Dot6* stack = FastGrowStackForPush(gs, 1);
	register tt_uint8* instr = gs->insPtr;
    tt_int16 word;

    if (IS_BAD_INSTR_POINTER(gs, instr) || IS_BAD_INSTR_POINTER(gs, instr+1)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

    word = GETBYTE(instr);
    word = (word << 8) + GETBYTE(instr);
    CHECK_PUSH(gs, stack, word);

	gs->stackPointer = stack;
	gs->insPtr = instr;
}

/*
 * N PUSH Words
 */
void fnt_NPUSHW(register fnt_LocalGraphicStateType *gs)
{
	fnt_PushSomeWords(gs, GETBYTE( gs->insPtr ));
}

/*
 * Write Store
 */
void fnt_WS(register fnt_LocalGraphicStateType *gs)
{
    register F26Dot6 storage;
	register ArrayIndex storeIndex;

	storage = CHECK_POP(gs, gs->stackPointer );
	storeIndex = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );

    if (IS_BAD_STORE_INDEX(gs, storeIndex)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }
	
    gs->globalGS->store[storeIndex] = storage;
}

/*
 * Read Store
 */
void fnt_RS(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex storeIndex;

	storeIndex = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
    if (IS_BAD_STORE_INDEX(gs, storeIndex)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }
	CHECK_PUSH( gs, gs->stackPointer, gs->globalGS->store[storeIndex] );
}

/*
 * Write Control Value Table from outLine, assumes the value comes form the outline domain
 */
void fnt_WCVTP(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex cvtIndex;
    register F26Dot6 cvtValue;

    cvtValue = CHECK_POP(gs, gs->stackPointer );
    cvtIndex = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );

    if (IS_BAD_CVT_INDEX(gs, cvtIndex)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

	gs->globalGS->controlValueTable[ cvtIndex ] = cvtValue;

	/* The BASS outline is in the transformed domain but the cvt is not so apply the inverse gxTransform */
	if ( cvtValue ) 
	  {
	  	/* MTE Hack: removed cause for a warning about unwanted assignment*/
		register F26Dot6 tmpCvt= gs->GetCVTEntry( gs, cvtIndex) ;
		if ( (tmpCvt ) && tmpCvt != cvtValue ) 
		{
			gs->globalGS->controlValueTable[ cvtIndex ] = 
				MultiplyDivide(cvtValue, cvtValue, tmpCvt);
		}
	}
}

/*
 * Write Control Value Table From Original Domain, assumes the value comes from the original domain, not the cvt or outline
 */
void fnt_WCVTF(fnt_LocalGraphicStateType *gs)
{
    	register ArrayIndex cvtIndex;
	register F26Dot6 cvtValue;
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;

	cvtValue = CHECK_POP(gs, gs->stackPointer );
	cvtIndex = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
    if (IS_BAD_CVT_INDEX(gs, cvtIndex)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }
	globalGS->controlValueTable[ cvtIndex ] = FixedMultiply(globalGS->upemScale.x, cvtValue);
}



/*
 * Read Control Value Table
 */
void fnt_RCVT(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex cvtIndex;
    F26Dot6 tmp;

    cvtIndex = (ArrayIndex) CHECK_POP(gs, gs->stackPointer);
    if (!IS_BAD_CVT_INDEX(gs, cvtIndex)) {
        tmp = gs->GetCVTEntry(gs, cvtIndex); 
    } else {
        /* We may abort hinting in this case but it seems that this is popular
           problem. So, we try to proceed with some default value
           (0 was actually fetched at least in some of these fonts). 
           Some fonts failing here: ANIM____.TTF, BRAVONC.TTF, BREEZ.TTF, 
                                    CHEQ.TTF, FOOPY1.TTF, FOOPY5.TTF, .. */
        tmp = 0;
    }

    CHECK_PUSH(gs, gs->stackPointer, tmp);
}

/*
 * Read Coordinate
 */
void fnt_GC(register fnt_LocalGraphicStateType *gs)
{
    ArrayIndex pt;
	fnt_ElementType *element;
	register F26Dot6 proj;

	pt = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
	element = gs->CE2;

    if (IS_BAD_POINT(gs, element, pt)) {
        FatalInterpreterError(gs, interp_range_error);
    }

    if ( BIT0( gs->opCode ) )
	    proj = gs->OldProject( gs, element->ox[pt], element->oy[pt] );
	else
	    proj = gs->Project( gs, element->x[pt], element->y[pt] );

	CHECK_PUSH( gs, gs->stackPointer, proj );
}

/*
 * Write Coordinate
 */
void fnt_SCFS(register fnt_LocalGraphicStateType *gs)
{
    register F26Dot6 proj, coord;
	register ArrayIndex pt;
	register fnt_ElementType *element;

	coord = CHECK_POP(gs, gs->stackPointer );/* value */
	pt = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );/* gxPoint */
	element = gs->CE2;

    if (IS_BAD_POINT(gs, element, pt)) {
        FatalInterpreterError(gs, interp_range_error);
    }

	proj = gs->Project( gs, element->x[pt],  element->y[pt] );
	gs->MovePoint( gs, element, pt, coord - proj );

	if (element == gs->elements[TWILIGHTZONE])
	{
		element->ox[pt] = element->x[pt];
		element->oy[pt] = element->y[pt];
	}
}


/*
 * Measure Distance
 */
void fnt_MD(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex pt1, pt2;
	register F26Dot6 proj, *stack = gs->stackPointer;
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;

	pt2 = (ArrayIndex)CHECK_POP(gs, stack );
	pt1 = (ArrayIndex)CHECK_POP(gs, stack );

    if (IS_BAD_POINT(gs, gs->CE0, pt1) || IS_BAD_POINT(gs, gs->CE1, pt2)) {
        FatalInterpreterError(gs, interp_range_error);
    }

	if ( BIT0( gs->opCode - MD_CODE ) )
	{	
		if (REMOVECorrectUnscaledOutline)
			/* Use the previous caclulated ox and oy arrays.*/
			proj  = gs->Project( gs, gs->CE0->ox[pt1] - gs->CE1->ox[pt2], gs->CE0->oy[pt1] - gs->CE1->oy[pt2] );
		else
		{
			proj  = gs->OldProject(gs, FixedMultiply(globalGS->upemScale.x, gs->CE0->oox[pt1] - gs->CE1->oox[pt2]),
							FixedMultiply(globalGS->upemScale.y, gs->CE0->ooy[pt1] - gs->CE1->ooy[pt2]));
		}
	}
	else
		proj  = gs->Project( gs, gs->CE0->x[pt1] - gs->CE1->x[pt2], gs->CE0->y[pt1] - gs->CE1->y[pt2] );
	CHECK_PUSH( gs, stack, proj );
	gs->stackPointer = stack;
}

/*
 * Measure Pixels Per EM
 */
void fnt_MPPEM(register fnt_LocalGraphicStateType *gs)
{
	register tt_uint16 ppem;
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;

	ppem  = fnt_ProjectIntegerPPEM(gs);
	GrowStackForPush(gs, 1);
	CHECK_PUSH( gs, gs->stackPointer, ppem );
}

/*
 * Get Miscellaneous info: version number, rotated, stretched 	<6>
 * Version number is 8 bits.  This is version 0x01 : 5/1/90
 */
void fnt_GETINFO( register fnt_LocalGraphicStateType* gs )
{
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
	register fastInt selector = (fastInt)CHECK_POP(gs, gs->stackPointer );
	register fastInt info = 0;

	if( selector & versionInterpreterQuery)
		info = quickdrawGXInterpreterVersion;
	if( (selector & rotatedInterpreterQuery) && (globalGS->non90DegreeTransformation & 0x1) )
		info |= rotateInterpreterGetInfo;
	if( (selector & stretchedInterpreterQuery) &&  (globalGS->non90DegreeTransformation & 0x2))
		info |= stretchInterpreterGetInfo;
	if (selector & variationInterpreterQuery)
		info |= variationInterpreterGetInfo;
	if (selector & verticalMetricsInterpreterQuery)
		info |= verticalMetricsInterpreterGetInfo;
	CHECK_PUSH( gs, gs->stackPointer, info );
}

/*
 *	Push the normalized (-1.0 Š 0 Š 1.0) variation coordinate.
 *	Coordinates are shortFracs, so 1.0 cooresponds to 16384.
 *	All zeros means the gxFont is at its stylistic default.
*/
void fnt_GETVARIATION(register fnt_LocalGraphicStateType* gs)
{
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
	register fastInt count = globalGS->variationCoordCount;
	register shortFrac* coord = globalGS->variationCoord;

	if( count != 0 )
	{
		GrowStackForPush(gs, count);
		if (globalGS->hasVariationCoord)
			do {
				CHECK_PUSH(gs, gs->stackPointer, *coord++);
			} while (--count);
		else
			do {
				CHECK_PUSH(gs, gs->stackPointer, 0);
			} while (--count);
	}
	else
	{
	 	fnt_IDefPatch( gs );
	 }
}

/*
 *	General data retiever opcode.
 *	Takes a 16-bit action-selector.
 *	If the interpreter recognizes the selector, it is executed, and then a TRUE is pushed.
 *	else a FALSE is pushed.
*/
void fnt_GETDATA(register fnt_LocalGraphicStateType* gs)
{	
	switch (CHECK_POP(gs, gs->stackPointer)) {
	case randomFntGetData:
		{	
			tt_uint32 N = CHECK_POP(gs, gs->stackPointer);
			IfDebugMessage(N > 65536, "N too big for random", N);
#ifdef cpuPrinter
#define RandomBits(A,B)	17
#endif
			CHECK_PUSH(gs, gs->stackPointer, RandomBits(16, 0) % N);
#ifdef cputPrinter
	#undef RandomBits
#endif
		}
		break;
	default:
		CHECK_PUSH(gs, gs->stackPointer, false);
		return;
	}
	CHECK_PUSH(gs, gs->stackPointer, true);
}

/*
 * FLIP ON
 */
void fnt_FLIPON(register fnt_LocalGraphicStateType *gs)
{
	gs->globalGS->localParBlock.autoFlip = true;
}

/*
 * FLIP OFF
 */
void fnt_FLIPOFF(register fnt_LocalGraphicStateType *gs)
{
	gs->globalGS->localParBlock.autoFlip = false;
}

/*
 *	This guy is here to save space for simple insructions
 *	that pop two arguments and push one back on.
 */
void fnt_BinaryOperand(fnt_LocalGraphicStateType* gs)
{
	F26Dot6* stack = gs->stackPointer;
	F26Dot6 arg2 = CHECK_POP(gs, stack );
	F26Dot6 arg1 = CHECK_POP(gs, stack );

	switch (gs->opCode) {
    case LT_CODE:   BOOLEANPUSH(gs, stack, arg1 < arg2 );  break;
    case LTEQ_CODE: BOOLEANPUSH(gs, stack, arg1 <= arg2 ); break;
    case GT_CODE:   BOOLEANPUSH(gs, stack, arg1 > arg2 );  break;
    case GTEQ_CODE: BOOLEANPUSH(gs, stack, arg1 >= arg2 ); break;
    case EQ_CODE:   BOOLEANPUSH(gs, stack, arg1 == arg2 ); break;
    case NEQ_CODE:  BOOLEANPUSH(gs, stack, arg1 != arg2 ); break;

    case AND_CODE:  BOOLEANPUSH(gs, stack, arg1 && arg2 ); break;
    case OR_CODE:   BOOLEANPUSH(gs, stack, arg1 || arg2 ); break;

	case ADD_CODE:	CHECK_PUSH( gs, stack, arg1 + arg2 ); break;
	case SUB_CODE:	CHECK_PUSH( gs, stack, arg1 - arg2 ); break;
	case MUL_CODE:	CHECK_PUSH( gs, stack, Mul26Dot6( arg1, arg2 )); break;
	case DIV_CODE:	CHECK_PUSH( gs, stack, Div26Dot6( arg1, arg2 )); break;
	case MAX_CODE:	if (arg1 < arg2) arg1 = arg2; CHECK_PUSH( gs, stack, arg1 ); break;
	case MIN_CODE:	if (arg1 > arg2) arg1 = arg2; CHECK_PUSH( gs, stack, arg1 ); break;
	}
	gs->stackPointer = stack;
}

void fnt_UnaryOperand(fnt_LocalGraphicStateType* gs)
{
	F26Dot6* stack = gs->stackPointer;
	F26Dot6 arg = CHECK_POP(gs, stack );
	tt_uint8 opCode = gs->opCode;

	switch (opCode) {
	case ODD_CODE:
	case EVEN_CODE:
#ifdef use_engine_characteristics_in_hints
		arg = fnt_RoundToGrid( arg, 0, 0 );
#else
		arg = fnt_RoundToGrid( arg, 0 );
#endif
		arg >>= fnt_pixelShift;
		if ( opCode == ODD_CODE )
			arg++;
        BOOLEANPUSH(gs, stack, (arg & 1) == 0);
		break;
    case NOT_CODE:  BOOLEANPUSH(gs, stack, !arg);  break;

	case ABS_CODE:	CHECK_PUSH( gs, stack, arg > 0 ? arg : -arg ); break;
	case NEG_CODE:	CHECK_PUSH( gs, stack, -arg ); break;

	case CEILING_CODE:
		arg += fnt_pixelSize - 1;
	case FLOOR_CODE:
		arg &= ~(fnt_pixelSize-1);
		CHECK_PUSH( gs, stack, arg );
		break;
	}
	gs->stackPointer = stack;
}

/*
 * IF
 */
void fnt_IF(register fnt_LocalGraphicStateType *gs)
{
	register fastInt level;
	register tt_uint8 opCode;

	if ( ! CHECK_POP(gs, gs->stackPointer ) )
	{	/* Now skip instructions */
        for (level = 1; level && !IS_BAD_INSTR_POINTER(gs, gs->insPtr); ) {
			/* level = # of "ifs" minus # of "endifs" */
			if ( (gs->opCode = opCode = *gs->insPtr++) == EIF_CODE ) {
				level--;
			} else if ( opCode == IF_CODE ) {
				level++;
			} else if ( opCode == ELSE_CODE ) {
				if ( level == 1 ) break;
			} else
				fnt_SkipPushCrap( gs );
		}
	}
}

/*
 *	ELSE for the IF
 */
void fnt_ELSE( fnt_LocalGraphicStateType* gs )
{
	register fastInt level;
	register tt_uint8 opCode;

    for(level = 1; level && !IS_BAD_INSTR_POINTER(gs, gs->insPtr); ) {
		/* level = # of "ifs" minus # of "endifs" */
		if ( (gs->opCode = opCode = *gs->insPtr++) == EIF_CODE ) { /* EIF */
			level--;
		} else if ( opCode == IF_CODE ) {
			level++;
		} else
			fnt_SkipPushCrap( gs );
	}
}

/*
 * End IF
 */
void fnt_EIF( fnt_LocalGraphicStateType* gs)
{
  /* #pragma unused(gs) */
}

/*
 * Jump Relative
 */
void fnt_JMPR( register fnt_LocalGraphicStateType* gs )
{
	register ArrayIndex offset;

	offset = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
    if (offset == 0) { /* simple heuristic to prevent infinite loop.
                          it does not protect from all possible cases. */ 
        FatalInterpreterError(gs, interp_font_program_error);
    } 
	offset--; /* since the interpreter post-increments the IP */
	gs->insPtr += offset;

}

/*
 * Jump Relative On True
 */
void fnt_JROT(register fnt_LocalGraphicStateType *gs)
{
	register ArrayIndex offset;
	register F26Dot6* stack = gs->stackPointer;

	if ( CHECK_POP(gs, stack ) ) {
		offset = (ArrayIndex)CHECK_POP(gs, stack );
        if (offset == 0) { /* simple heuristic to prevent infinite loop.
                          it does not protect from all possible cases. */ 
            FatalInterpreterError(gs, interp_font_program_error);
        } 
		--offset; /* since the interpreter post-increments the IP */
		gs->insPtr += offset;
	} else {
		--stack;/* same as POP */
	}
	gs->stackPointer = stack;
}

/*
 * Jump Relative On False
 */
void fnt_JROF(register fnt_LocalGraphicStateType *gs)
{
	register ArrayIndex offset;
	register F26Dot6* stack = gs->stackPointer;

	if ( CHECK_POP(gs, stack ) ) {
		--stack;/* same as POP */
	} else {
		offset = (ArrayIndex)CHECK_POP(gs, stack );
        if (offset == 0) { /* simple heuristic to prevent infinite loop.
                          it does not protect from all possible cases. */ 
            FatalInterpreterError(gs, interp_font_program_error);
        } 
		offset--; /* since the interpreter post-increments the IP */
		gs->insPtr += offset;
	}
	gs->stackPointer = stack;
}

/*
 * ROUND
 */
void fnt_ROUND(register fnt_LocalGraphicStateType *gs)
{
    register F26Dot6 arg1;
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;

	arg1 = CHECK_POP(gs, gs->stackPointer );

	CHECK_RANGE(gs, gs->opCode, 0x68, 0x6B );

#ifdef use_engine_characteristics_in_hints
	arg1 = pb->RoundValue( arg1, gs->globalGS->engine[gs->opCode - 0x68], gs);
#else
	arg1 = pb->RoundValue( arg1, gs);
#endif
	CHECK_PUSH( gs, gs->stackPointer , arg1 );
}

/*
 * No ROUND
 */
void fnt_NROUND(register fnt_LocalGraphicStateType *gs)
{
    register F26Dot6 arg1;

	arg1 = CHECK_POP(gs, gs->stackPointer );

	CHECK_RANGE(gs, gs->opCode, 0x6C, 0x6F );

#ifdef use_engine_characteristics_in_hints
	arg1 = fnt_RoundOff( arg1, gs->globalGS->engine[gs->opCode - 0x6c], 0 );
#endif
	CHECK_PUSH( gs, gs->stackPointer , arg1 );
}

/*
 * An internal function used by MIRP an MDRP.
 */
local F26Dot6 fnt_CheckSingleWidth(register F26Dot6 value, register fnt_LocalGraphicStateType *gs)
{
	register F26Dot6 delta, scaledSW;
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;

	scaledSW = gs->GetSingleWidth( gs );

	if ( value >= 0 ) {
		delta = value - scaledSW;
		if ( delta < 0 )    delta = -delta;
		if ( delta < pb->sWCI )    value = scaledSW;
	} else {
		value = -value;
		delta = value - scaledSW;
		if ( delta < 0 )    delta = -delta;
		if ( delta < pb->sWCI )    value = scaledSW;
		value = -value;
	}
	return value;
}


/*
 * Move Direct Relative Point
 */
void fnt_MDRP(register fnt_LocalGraphicStateType *gs)
{
	register ArrayIndex pt1, pt0 = gs->Pt0;
	register F26Dot6 tmp, tmpC;
	register fnt_ElementType *CE1 = gs->CE1;
	register fnt_ElementType* CE0 = gs->CE0;
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;

	pt1 = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );

    if (IS_BAD_POINT(gs, CE0, pt0) || IS_BAD_POINT(gs, CE1, pt1)) {
        FatalInterpreterError(gs, interp_range_error);
    }

	if (REMOVECorrectUnscaledOutline || CE0 == gs->elements[TWILIGHTZONE] || CE1 == gs->elements[TWILIGHTZONE] )
		tmp  = FastOldProject( gs, CE1->ox[pt1] - CE0->ox[pt0],  CE1->oy[pt1] - CE0->oy[pt0] );
	else
	{
		if (globalGS->upemScale.x != globalGS->upemScale.y )
			tmp  = FastOldProject(gs, FixedMultiply(globalGS->upemScale.x, CE1->oox[pt1] - CE0->oox[pt0]),
							FixedMultiply(globalGS->upemScale.y, CE1->ooy[pt1] - CE0->ooy[pt0]));
		/* rwb 11/16/93 - add next path to maintain compatability with TT1 scaler for the square pixel case. 
		 It makes a difference which multiply is done first */
		else
		{	tmp  = FastOldProject(gs,  CE1->oox[pt1] - CE0->oox[pt0], CE1->ooy[pt1] - CE0->ooy[pt0]);
			tmp = FixedMultiply( globalGS->upemScale.x, tmp );
		}
	}
			
	if ( globalGS->localParBlock.sWCI )
		tmp = fnt_CheckSingleWidth( tmp, gs );

	tmpC = tmp;

	if ( BIT2( gs->opCode ) )
#ifdef use_engine_characteristics_in_hints
		tmp = globalGS->localParBlock.RoundValue( tmp, globalGS->engine[gs->opCode & 0x03], gs );
	else
		tmp = fnt_RoundOff( tmp, globalGS->engine[gs->opCode & 0x03], 0 );
#else
		tmp = globalGS->localParBlock.RoundValue( tmp, gs );
#endif

	if ( BIT3( gs->opCode ) )
	{
		F26Dot6 tmpB = globalGS->localParBlock.minimumDistance;
		if ( tmpC >= 0 ) {
			if ( tmp < tmpB ) {
				tmp = tmpB;
			}
		} else {
			tmpB = -tmpB;
			if ( tmp > tmpB ) {
				tmp = tmpB;
			}
		}
	}

	tmpC = FastProject( gs, CE1->x[pt1] - CE0->x[pt0], CE1->y[pt1] - CE0->y[pt0] );

	tmp -= tmpC;
	gs->MovePoint( gs, CE1, pt1, tmp );
	gs->Pt1 = pt0;
	gs->Pt2 = pt1;
	if ( BIT4( gs->opCode ) ) {
		gs->Pt0 = pt1; /* move the reference gxPoint */
	}
}

/*
 * Move Indirect Relative Point
 */
void fnt_MIRP(register fnt_LocalGraphicStateType *gs)
{
    register ArrayIndex ptNum, pt0 = gs->Pt0, n;
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
	register fnt_ElementType* CE1 = gs->CE1;
	register fnt_ElementType* CE0 = gs->CE0;
	F26Dot6 tmp, tmpC;

    n = (ArrayIndex) CHECK_POP(gs, gs->stackPointer);
    ptNum = (ArrayIndex) CHECK_POP(gs, gs->stackPointer);
    /* Note that according to spec we do not need CVT table value 
       if BIT2(gs->opCode) is 0. In practice we seems to request it always
       (and use it always?!?) so check it unconditionally. */
    if (IS_BAD_POINT(gs, CE1, ptNum) || IS_BAD_POINT(gs, CE0, pt0)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

    /* Theoritically in case of bad CVT index we may simply abort hinting.
       However, sometimes we get weird values here but they do not have 
       any noticeable impact on results of hinting. For instance this is the case
       for msgbl.ttf font that is primary zh font for Solaris 9.
       So, we try to proceed using some default value. */
    if (!IS_BAD_CVT_INDEX(gs, n)) {
        tmp = gs->GetCVTEntry(gs, n);
    } else {
        /* NB: 0 is definitely not ideal value in context of MIRP.
           Fortunatelly in most of known cases we get here it works good enough. */
        tmp = 0;
    }
#ifdef support_fake_variation_cvt
	if ( gs->fakeVariantCVT && CE1 != gs->elements[TWILIGHTZONE] )
		tmp = FastProject(gs, CE1->ox[ptNum] - CE1->ox[pt0], CE1->oy[ptNum] - CE1->oy[pt0]);
#endif

	if ( globalGS->localParBlock.sWCI )
		tmp = fnt_CheckSingleWidth( tmp, gs );

    if (CE1 == gs->elements[TWILIGHTZONE]) {
		CE1->ox[ptNum] = CE0->ox[pt0];
		CE1->oy[ptNum] = CE0->oy[pt0];
		CE1->ox[ptNum] += ShortFracMul( tmp, gs->proj.x );
		CE1->oy[ptNum] += ShortFracMul( tmp, gs->proj.y );
		CE1->x[ptNum] = CE0->x[pt0];			/* <10> */
		CE1->y[ptNum] = CE0->y[pt0];			/* <10> */
	}

	tmpC  = FastOldProject( gs, CE1->ox[ptNum] - CE0->ox[pt0], CE1->oy[ptNum] - CE0->oy[pt0] );

	if ( globalGS->localParBlock.autoFlip ) {
		if ( ((tt_int32)(tmpC ^ tmp)) < 0 ) {
			tmp = -tmp; /* Do the auto flip */
		}
	}

	if ( BIT2( gs->opCode )  )
	{	register F26Dot6 tmpB = tmp - tmpC;

		if ( tmpB < 0 )    tmpB = -tmpB;
		if ( tmpB > globalGS->localParBlock.wTCI )    tmp = tmpC;
#ifdef use_engine_characteristics_in_hints
		tmp = globalGS->localParBlock.RoundValue( tmp, globalGS->engine[gs->opCode & 0x03], gs );
	}
	else
		tmp = fnt_RoundOff( tmp, globalGS->engine[gs->opCode & 0x03], 0 );
#else
		tmp = globalGS->localParBlock.RoundValue( tmp, gs );
	}
#endif


	if ( BIT3( gs->opCode ) )
	{	register F26Dot6 tmpB = globalGS->localParBlock.minimumDistance;

		if ( tmpC >= 0 ) {
			if ( tmp < tmpB ) {
				tmp = tmpB;
			}
		} else {
			tmpB = -tmpB;
			if ( tmp > tmpB ) {
				tmp = tmpB;
			}
		}
	}

	tmpC = FastProject(gs, CE1->x[ptNum] - CE0->x[pt0], CE1->y[ptNum] - CE0->y[pt0] );

	gs->MovePoint( gs, CE1, ptNum, tmp  - tmpC );

	gs->Pt1 = pt0;
	gs->Pt2 = ptNum;
	if ( BIT4( gs->opCode ) )
		gs->Pt0 = ptNum; /* move the reference gxPoint */
}

/*
 * CALL a function
 */
void fnt_CALL(register fnt_LocalGraphicStateType *gs)
{
	register fnt_funcDef *funcDef;
	tt_uint8 *ins;
	fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
	ArrayIndex arg = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );

    if (IS_BAD_FDEF_INDEX(gs, arg) || globalGS->funcDef == NULL) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

	funcDef = &globalGS->funcDef[ arg ];

    if (IS_BAD_PGM_INDEX(gs, funcDef->pgmIndex)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

	ins     = globalGS->pgmList[ funcDef->pgmIndex ];

    if (ins == NULL) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

	ins += funcDef->start;
	gs->Interpreter( gs, ins, ins + funcDef->length);
}

/*
 * Function DEFinition
 */
void fnt_FDEF(register fnt_LocalGraphicStateType *gs)
{
	register fnt_funcDef *funcDef;
	tt_uint8* program, *funcStart;
	fnt_GlobalGraphicStateType *globalGS = gs->globalGS;
	ArrayIndex arg = /* Specify 0 - (n-1) FDEF index number*/
		(ArrayIndex)CHECK_POP(gs, gs->stackPointer );
	register fastInt pgmIndex = globalGS->pgmIndex;

    if (IS_BAD_FDEF_INDEX(gs, arg) || globalGS->funcDef == NULL || 
        IS_BAD_PGM_INDEX(gs, pgmIndex)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

    funcDef = &globalGS->funcDef[arg];  /* Point to funcdef storage info*/
    program = globalGS->pgmList[funcDef->pgmIndex = pgmIndex];
    if (pgmIndex == preProgramIndex) {
        globalGS->preProgramHasDefs = true;
    }
	
    funcDef->start = gs->insPtr - program;
    funcStart = gs->insPtr;
    while ((gs->opCode = *gs->insPtr++) != ENDF_CODE)
        fnt_SkipPushCrap(gs);
	
    funcDef->length = gs->insPtr - funcStart - 1; /* don't execute ENDF */
}

/*
 * LOOP while CALLing a function
 */
void fnt_LOOPCALL(register fnt_LocalGraphicStateType *gs)
{
	register tt_uint8 *start, *stop;
	register InterpreterFunc interpreter;
	register fnt_funcDef *funcDef;
	ArrayIndex arg = (ArrayIndex)CHECK_POP(gs, gs->stackPointer );
	register LoopCount loop;
	tt_uint8* program;

    if (gs->globalGS->funcDef == NULL || IS_BAD_FDEF_INDEX(gs, arg)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

	funcDef = &(gs->globalGS->funcDef[ arg ]);
    if (IS_BAD_PGM_INDEX(gs, funcDef->pgmIndex)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }

	program = gs->globalGS->pgmList[ funcDef->pgmIndex ];
	start = &program[funcDef->start];
	stop = &program[funcDef->start + funcDef->length];
	interpreter = gs->Interpreter;
	loop = (LoopCount)CHECK_POP(gs, gs->stackPointer );
	for (--loop; loop >= 0; --loop )
		interpreter( gs, start, stop );
}

/*
 * DeltaEngine, internal support routine
 */
local void fnt_DeltaEngine(register fnt_LocalGraphicStateType *gs, FntMoveFunc doIt,
								tt_int16 base, tt_int16 shift)
{
	register tt_int32 tmp;
	register tt_int32 fakePixelsPerEm, ppem;
	register tt_int32 aim, high;
	register tt_int32 tmp32;

	/* Find the beginning of data pairs for this particular size */
	high = (tt_int32)CHECK_POP(gs, gs->stackPointer ) << 1; /* -= number of pops required */
    if (IS_BAD_STACK_POINTER(gs, gs->stackPointer-high) || 
        IS_BAD_STACK_POINTER(gs, gs->stackPointer)) {
        FatalInterpreterError(gs, interp_font_program_error);
    }
	gs->stackPointer -= high;

	tmp32 = fnt_ProjectIntegerPPEM(gs);
	fakePixelsPerEm = tmp32 - base;
	if ( fakePixelsPerEm >= 16 || fakePixelsPerEm < 0 )
		return; /* Not within exception range */
	fakePixelsPerEm <<= 4;

	aim = 0;

    /* Truetype specification contains following note:

       NOTE:  Always observe that DELTA instructions expect the argument list 
              to be sorted according to ppem. The lowest ppem should be deepest
              on the stack, and the highest ppem should be topmost on the stack.

       So, in theory we can do binary search to identify subset of list related
       to the particular ppem. However, some popular fonts do not obey this rule
       (e.g. DejaVu). On other hand argument lists are often quite short and 
       performance benefits are questionable.

       Therefore we are using naive linear scan.
    */
	while ( aim < high ) {
		ppem  = gs->stackPointer[ aim ]; /* [ ppem << 4 | exception ] */
		if ( (tmp = (ppem & ~0x0f)) == fakePixelsPerEm ) {
			/* We found an exception, go ahead and apply it */
			tmp  = ppem & 0xf; /* 0 ... 15 */
			tmp -= tmp >= 8 ? 7 : 8; /* -8 ... -1, 1 ... 8 */
			tmp <<= fnt_pixelShift; /* convert to pixels */
			tmp >>= shift; /* scale to right size */
			doIt( gs, gs->CE0, gs->stackPointer[aim+1] /* gxPoint number */, tmp /* the delta */ );
        } 
		aim += 2;
	}
}

/*
 * DELTAP1
 */
void fnt_DELTAP1(register fnt_LocalGraphicStateType *gs)
{
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;
	fnt_DeltaEngine( gs, gs->MovePoint, pb->deltaBase, pb->deltaShift );
}

/*
 * DELTAP2
 */
void fnt_DELTAP2(register fnt_LocalGraphicStateType *gs)
{
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;
	fnt_DeltaEngine( gs, gs->MovePoint, (tt_int16) (pb->deltaBase+16), pb->deltaShift );
}

/*
 * DELTAP3
 */
void fnt_DELTAP3(register fnt_LocalGraphicStateType *gs)
{
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;
	fnt_DeltaEngine( gs, gs->MovePoint, (tt_int16) (pb->deltaBase+32), pb->deltaShift );
}

/*
 * DELTAC1
 */
void fnt_DELTAC1(register fnt_LocalGraphicStateType *gs)
{
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;
	fnt_DeltaEngine( gs, (FntMoveFunc) fnt_ChangeCvt, pb->deltaBase, pb->deltaShift );
}

/*
 * DELTAC2
 */
void fnt_DELTAC2(register fnt_LocalGraphicStateType *gs)
{
  register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;
  fnt_DeltaEngine( gs, (FntMoveFunc) fnt_ChangeCvt, (tt_int16) (pb->deltaBase+16), pb->deltaShift );
}

/*
 * DELTAC3
 */
void fnt_DELTAC3(register fnt_LocalGraphicStateType *gs)
{
  register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;
  fnt_DeltaEngine( gs, (FntMoveFunc) fnt_ChangeCvt, (tt_int16) (pb->deltaBase+32), pb->deltaShift );
}
#endif  
	/* #ifdef ENABLE_TT_HINTING */ 
