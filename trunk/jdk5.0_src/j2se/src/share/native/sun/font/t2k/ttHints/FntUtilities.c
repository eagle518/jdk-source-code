/*
 * @(#)FntUtilities.c	1.9 03/12/19
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

#include "FSCdefs.h"
#include "FSError.h"
#include "FSglue.h"		/* Access to key structure */
#include "Fnt.h"
 

#include "FntDebug.h"
#include "PrivateFnt.h"
#include "FontMath.h"

#ifdef UIDebug
#include "HintState.h"
#endif

#define	STACKSIZE_INCREMENT	1024		/* Must not be less than 1K! */
#undef	really_slow_debugging
/* #define really_slow_debugging */
 /*	Need a debugging call-out in the local graphics state, set by the user.
*/

#ifndef cpuPrinter
void PostInterpreterError(fnt_LocalGraphicStateType *gs, tt_int32 error)
{
  /* #pragma unused(gs) */
  /*#pragma unused(error) */

#ifdef debugging
#ifdef be_mean_to_fonts_debugging
Debugger();
#endif
#endif
}
#else
#define PostInterpreterError(g,e)
#endif

void FatalInterpreterError(fnt_LocalGraphicStateType *gs, tt_int32 error)
{
#ifdef debugging
#ifdef be_mean_to_fonts_debugging
		Debugger();
#endif
#endif
	longjmp(gs->env, scaler_hinting_error); /* Do a graceful recovery  */
}

void fnt_Normalize(fnt_LocalGraphicStateType* gs, F26Dot6 x, F26Dot6 y, shortVector* v)
{
	fract x1, y1;
	/*
	 *	Since x and y are 26.6, and currently that means they are really 16.6,
	 *	when treated as Fract, they are 0.[8]22, so shift up to 0.30 for accuracy
	 */
#ifdef HeiseiKakuGothic_fails_this_test
	CHECK_RANGE(gs, x, -32768L << 6, 32767L << 6);
	CHECK_RANGE(gs, y, -32768L << 6, 32767L << 6);
#endif
#ifdef cpuPrinter
#pragma unused (gs)
#endif
	{	int shift, count;
		F26Dot6 xx = x;
		F26Dot6 yy = y;
		
		if (xx < 0)	xx = -xx;
		if (yy < 0) yy = -yy;
		if (xx < yy) xx = yy;
		/*
		 *	0.5 <= max(x,y) < 1
		 */
		bitcount(xx, count); 
		shift = 8 * sizeof(fract) - 2 - count;
		x1 = (fract) x << shift;
		y1 = (fract) y << shift;
	}
	{	fract length;
			/* MTE Hack: fixed possible unwanted assignment */
			length = Magnitude(x1, y1);
		if (length)
		{	v->x = FixedRound( FractDivide( x1, length ) );
			v->y = FixedRound( FractDivide( y1, length ) );
		}	
		else
		{	PostInterpreterError(gs, interp_normalize_error);
			v->x = shortFrac1;
			v->y = 0;
		}
	}
}

/*
 * Internal function for fnt_IF(), and fnt_FDEF()
 */
void fnt_SkipPushCrap(register fnt_LocalGraphicStateType *gs)
{
	register tt_uint8 opCode = gs->opCode;
	register tt_uint8* instr = gs->insPtr;
	register ArrayIndex count;

	if ( opCode == NPUSHB_CODE ) {
		count = (ArrayIndex)*instr++;
		instr += count;
	} else if ( opCode == NPUSHW_CODE ) {
		count = (ArrayIndex)*instr++;
		instr += count + count;
	} else if ( opCode >= PUSHB_START && opCode <= PUSHB_END ) {
		count = (ArrayIndex)(opCode - PUSHB_START + 1);
		instr += count;
	} else if ( opCode >= PUSHW_START && opCode <= PUSHW_END ) {
		count = (ArrayIndex)(opCode - PUSHW_START + 1);
		instr += count + count;
	}
	gs->insPtr = instr;
}

/******************** BEGIN Rounding Routines ***************************/

#define SIGN_HAS_FLIPPED(origValue, newValue)	((F26Dot6)((newValue) ^ (origValue)) < 0 && (newValue))
/*
 * Internal rounding routine
 */
F26Dot6 fnt_RoundToDoubleGrid(register F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType* gs)
{
  /* #pragma unused(gs) */
	register F26Dot6 x = xin;

	if ( x >= 0 )
	{	EngineCharacteristicCode(x += engine);
		x += fnt_pixelSize/4;
		x &= ~(fnt_pixelSize/2-1);
	}
	else
	{	x = -x;
		EngineCharacteristicCode(x += engine);
		x += fnt_pixelSize/4;
		x &= ~(fnt_pixelSize/2-1);
		x = -x;
	}
	if (SIGN_HAS_FLIPPED(xin, x))
		x = 0;
	return x;
}

/*
 * Internal rounding routine
 */
F26Dot6 fnt_RoundDownToGrid(register F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType* gs)
{
  /*#pragma unused(gs) */
	register F26Dot6 x = xin;

	if ( x >= 0 )
	{	EngineCharacteristicCode(x += engine);
		x &= ~(fnt_pixelSize-1);
	}
	else
	{	x = -x;
		EngineCharacteristicCode(x += engine);
		x &= ~(fnt_pixelSize-1);
		x = -x;
	}
	if (SIGN_HAS_FLIPPED(xin, x))
		x = 0;
	return x;
}

/*
 * Internal rounding routine
 */
F26Dot6 fnt_RoundUpToGrid(register F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType* gs)
{
  /* #pragma unused(gs) */
	register F26Dot6 x = xin;

	if ( x >= 0 )
	{	EngineCharacteristicCode(x += engine);
		x += fnt_pixelSize - 1;
		x &= ~(fnt_pixelSize-1);
	}
	else
	{	x = -x;
		EngineCharacteristicCode(x += engine);
		x += fnt_pixelSize - 1;
		x &= ~(fnt_pixelSize-1);
		x = -x;
	}
	if (SIGN_HAS_FLIPPED(xin, x))
		x = 0;
	return x;
}

/*
 * Internal rounding routine
 */
F26Dot6 fnt_RoundToGrid(register F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType* gs)
{
  /* #pragma unused(gs) */
	register F26Dot6 x = xin;

	if ( x >= 0 )
	{	EngineCharacteristicCode(x += engine);
		x += fnt_pixelSize/2;
		x &= ~(fnt_pixelSize-1);
	}
	else
	{	x = -x;
		EngineCharacteristicCode(x += engine);
		x += fnt_pixelSize/2;
		x &= ~(fnt_pixelSize-1);
		x = -x;
	}
	if (SIGN_HAS_FLIPPED(xin, x))
		x = 0;
	return x;
}

/*
 * Internal rounding routine
 */
F26Dot6 fnt_RoundToHalfGrid(register F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType* gs)
{
  /* #pragma unused(gs) */
	register F26Dot6 x = xin;

    	if ( x >= 0 )
	{	EngineCharacteristicCode(x += engine);
		x &= ~(fnt_pixelSize-1);
		x += fnt_pixelSize/2;
	}
	else
	{	x = -x;
		EngineCharacteristicCode(x += engine);
		x &= ~(fnt_pixelSize-1);
		x += fnt_pixelSize/2;
		x = -x;
	}
	if (SIGN_HAS_FLIPPED(xin, x))		/* The sign flipped, make equal to smallest valid value */
		x = xin > 0 ? fnt_pixelSize/2 : -fnt_pixelSize/2;
	return x;
}

/*
 * Internal rounding routine
 */
F26Dot6 fnt_RoundOff(register F26Dot6 xin ENGINE_PARAMETER(engine), fnt_LocalGraphicStateType* gs)
{
  /* #pragma unused(gs) */
#ifdef use_engine_characteristics_in_hints
	register F26Dot6 x = xin;

	if ( x >= 0 )
		x += engine;
	else
		x -= engine;
	if (SIGN_HAS_FLIPPED(xin, x))
		x = 0;
	return x;
#else
	return xin;
#endif
}

/*
 * Internal rounding routine
 */
F26Dot6 fnt_SuperRound(register F26Dot6 xin ENGINE_PARAMETER(engine), register fnt_LocalGraphicStateType *gs)
{
	register F26Dot6 x = xin;
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;

	if ( x >= 0 )
	{	EngineCharacteristicCode(x += engine);
		x += pb->threshold - pb->phase;
		x &= pb->periodMask;
		x += pb->phase;
	}
	else
	{	x = -x;
		EngineCharacteristicCode(x += engine);
		x += pb->threshold - pb->phase;
		x &= pb->periodMask;
		x += pb->phase;
		x = -x;
	}
	if (SIGN_HAS_FLIPPED(xin, x))
		x = xin > 0 ? pb->phase : -pb->phase; /* The sign flipped, make equal to smallest phase */
	return x;
}

/*
 * Internal rounding routine
 */
F26Dot6 fnt_Super45Round(register F26Dot6 xin ENGINE_PARAMETER(engine), register fnt_LocalGraphicStateType *gs)
{
	register F26Dot6 x = xin;
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;

	if ( x >= 0 )
	{	EngineCharacteristicCode(x += engine);
		x += pb->threshold - pb->phase;
		x = FractDivide( x, pb->period45 );
		x  &= ~(fnt_pixelSize-1);
		x = FractMultiply( x, pb->period45 );
		x += pb->phase;
	}
	else
	{	x = -x;
		EngineCharacteristicCode(x += engine);
		x += pb->threshold - pb->phase;
		x = FractDivide( x, pb->period45 );
		x  &= ~(fnt_pixelSize-1);
		x = FractMultiply( x, pb->period45 );
		x += pb->phase;
		x = -x;
	}
	if (SIGN_HAS_FLIPPED(xin, x))
		x = xin > 0 ? pb->phase : -pb->phase; /* The sign flipped, make equal to smallest phase */
	return x;
}


/******************** END Rounding Routines ***************************/


/* 3-versions ************************************************************************/

/*
 * Moves the gxPoint in element by delta (measured against the projection vector)
 * along the freedom vector.
 */
void fnt_MovePoint(register fnt_LocalGraphicStateType *gs,
							register fnt_ElementType *element,
							register ArrayIndex gxPoint,
							register F26Dot6 delta)
{
	register shortFrac pfProj = gs->pfProj;
	register shortFrac fx = gs->free.x;
	register shortFrac fy = gs->free.y;

	CHECK_PFPROJ( gs );
	CHECK_POINT( gs, element, gxPoint );

	if ( pfProj != shortFrac1 )
	{	if ( fx )
		{	element->x[gxPoint] += ShortMulDiv( delta, fx, pfProj );
			element->f[gxPoint] |= XMOVED;
		}
		if ( fy )
		{	element->y[gxPoint] += ShortMulDiv( delta, fy, pfProj );
			element->f[gxPoint] |= YMOVED;
		}
	}
	else
	{	if ( fx )
		{	element->x[gxPoint] += ShortFracMul( delta, fx );
			element->f[gxPoint] |= XMOVED;
		}
		if ( fy )
		{	element->y[gxPoint] += ShortFracMul( delta, fy );
			element->f[gxPoint] |= YMOVED;
		}
	}
}

void fnt_MoveAPoint( fnt_LocalGraphicStateType* gs, F26Dot6* x, F26Dot6* y, F26Dot6 delta)
{
	register shortFrac pfProj = gs->pfProj;
	register shortFrac fx = gs->free.x;
	register shortFrac fy = gs->free.y;

	CHECK_PFPROJ( gs );
	CHECK_ASSERTION( gs, x != y );

	if ( pfProj != shortFrac1 )
	{	if ( fx )
			*x += ShortMulDiv( delta, fx, pfProj );
		if ( fy )
			*y += ShortMulDiv( delta, fy, pfProj );
	}
	else
	{	if ( fx )
			*x += ShortFracMul( delta, fx );
		if ( fy )
			*y += ShortFracMul( delta, fy );
	}
}

/*
 * For use when the projection and freedom vectors coincide along the x-axis.
 */
void fnt_XMovePoint( fnt_LocalGraphicStateType* gs, fnt_ElementType* element, ArrayIndex gxPoint, register F26Dot6 delta )
{
#ifndef debugging
  /*#pragma unused(gs) */
#endif
	CHECK_POINT( gs, element, gxPoint );
	element->x[gxPoint] += delta;
	element->f[gxPoint] |= XMOVED;
}

/*
 * For use when the projection and freedom vectors coincide along the y-axis.
 */
void fnt_YMovePoint( fnt_LocalGraphicStateType* gs, register fnt_ElementType *element, ArrayIndex gxPoint, F26Dot6 delta )
{
#ifndef debugging
  /*#pragma unused(gs) */
#endif
	CHECK_POINT( gs, element, gxPoint );
	element->y[gxPoint] += delta;
	element->f[gxPoint] |= YMOVED;
}

/*
 * projects x and y into the projection vector.
 */
F26Dot6 fnt_Project(fnt_LocalGraphicStateType* gs, F26Dot6 x, F26Dot6 y)
{
    return ShortFracMul( x, gs->proj.x ) + ShortFracMul( y, gs->proj.y );
}

/*
 * projects x and y into the old projection vector.
 */
F26Dot6 fnt_OldProject(fnt_LocalGraphicStateType* gs, F26Dot6 x, F26Dot6 y)
{
    return ShortFracMul( x, gs->oldProj.x ) + ShortFracMul( y, gs->oldProj.y );
}

/*
 * Projects when the projection vector is along the x-axis
 */
F26Dot6 fnt_XProject(fnt_LocalGraphicStateType* gs, F26Dot6 x, F26Dot6 y)
{
  /*#pragma unused(gs,y) */
    return x;
}

/*
 * Projects when the projection vector is along the y-axis
 */
F26Dot6 fnt_YProject(fnt_LocalGraphicStateType* gs, F26Dot6 x, F26Dot6 y)
{
  /*#pragma unused(gs,x) */
    return y;
}
/*************************************************************************/

/*** Compensation for Transformations ***/

/*
 * Internal support routine, keep this guy FAST!!!!!!!		<3>
 */
fixed fnt_GetCVTScale(register fnt_LocalGraphicStateType* gs)
{
	register fnt_GlobalGraphicStateType *globalGS = gs->globalGS;

	if ( gs->proj.y )
	{	if ( gs->proj.x )
			if (gs->projectionVectorIsNormal)	/* thanks to the great Rob Johnson */
				return Magnitude(ShortFracMul(globalGS->cvtStretch.x, gs->proj.x),
								ShortFracMul(globalGS->cvtStretch.y, gs->proj.y));
			else
			{	fixed foo = ShortFracMul(globalGS->cvtStretch.y, gs->proj.x);
				fixed bar = ShortFracMul(globalGS->cvtStretch.x, gs->proj.y);
	
				return MultiplyDivide(globalGS->cvtStretch.x, globalGS->cvtStretch.y, Magnitude(foo, bar));
			}
		else	/* pvy == +1 or -1 */
			return globalGS->cvtStretch.y;
	}
	else	/* pvx == +1 or -1 */
		return globalGS->cvtStretch.x;
}


/*	Functions for function pointer in local graphic state
*/
F26Dot6 fnt_GetCVTEntryFast(fnt_LocalGraphicStateType* gs, ArrayIndex n)
{
	CHECK_CVT_READ( gs, n );
 	return gs->globalGS->controlValueTable[ n ];
}

F26Dot6 fnt_GetCVTEntrySlow(register fnt_LocalGraphicStateType *gs, ArrayIndex n)
{
	register fixed scale;

	CHECK_CVT_READ( gs, n );
	scale = fnt_GetCVTScale( gs );
	return ( FixedMultiply( gs->globalGS->controlValueTable[ n ], scale ) );
}

F26Dot6 fnt_GetSingleWidthFast(register fnt_LocalGraphicStateType *gs)
{
 	return gs->globalGS->localParBlock.scaledSW;
}

/*
 *
 */
F26Dot6 fnt_GetSingleWidthSlow(register fnt_LocalGraphicStateType *gs)
{
	return ( FixedMultiply( gs->globalGS->localParBlock.scaledSW, fnt_GetCVTScale( gs ) ) );
}



/*************************************************************************/

void fnt_ChangeCvt(fnt_LocalGraphicStateType* gs, fnt_ElementType* elem,
							ArrayIndex number, F26Dot6 delta)
{
  /* #pragma unused(elem) */
	CHECK_CVT_WRITE( gs, number, gs->globalGS->controlValueTable[ number ] + delta );
	gs->globalGS->controlValueTable[ number ] += delta;
}

#define shortFrac16th			(shortFrac1 >> 4)

/*
 * Only does the check of gs->pfProj
 */
void fnt_Check_PF_Proj(fnt_LocalGraphicStateType *gs)
{
	register shortFrac pfProj = gs->pfProj;

	if ( pfProj > -shortFrac16th && pfProj < shortFrac16th)
	{	gs->pfProj = pfProj < 0 ? -shortFrac1 : shortFrac1; /* Prevent divide by small number */
#ifdef debugging
		gs->valid_pfProj = false;
#endif
	}	
}


/*
 * Computes gs->pfProj and then does the check
 */
void fnt_ComputeAndCheck_PF_Proj(register fnt_LocalGraphicStateType *gs)
{
	register shortFrac pfProj;

#ifdef debugging
	gs->valid_pfProj = true;
#endif
	pfProj = ShortFracDot( gs->proj.x, gs->free.x ) + ShortFracDot( gs->proj.y, gs->free.y );
	if ( pfProj > -shortFrac16th && pfProj < shortFrac16th)
	{	pfProj = pfProj < 0 ? -shortFrac1 : shortFrac1; /* Prevent divide by small number */
#ifdef debugging
		gs->valid_pfProj = false;
#endif
	}
	gs->pfProj = pfProj;
}

/*
 * Illegal instruction panic
 */
void fnt_IllegalInstruction(register fnt_LocalGraphicStateType *gs)
{
	FatalInterpreterError(gs, interp_unimplemented_instruction_error);
}

/*****************************************************************************/

#ifdef debugging
tt_int32 fnt_NilFunction(fnt_LocalGraphicStateType *gs, F26Dot6 value);
tt_int32 fnt_NilFunction(fnt_LocalGraphicStateType *gs, F26Dot6 value)
{
  /* #pragma unused(value) */

	PostInterpreterError(gs, interp_font_program_error);
	return 0;
}

tt_int32 fnt_NilFunction2(fnt_LocalGraphicStateType *gs);
tt_int32 fnt_NilFunction2(fnt_LocalGraphicStateType *gs)
{
	PostInterpreterError(gs, interp_font_program_error);
	return 0;
}
#endif

/*****************************************************************************/

/*
 * This is the tracing interpreter.
 */
void fnt_InnerTraceExecute(register fnt_LocalGraphicStateType *gs, tt_uint8 *ptr, register tt_uint8 *eptr)
{
    register FntFunc* function;
	register tt_uint8 *oldInsPtr;
	register fnt_ParameterBlock *pb = &gs->globalGS->localParBlock;

	oldInsPtr = gs->insPtr;
	gs->insPtr = ptr;
	function = gs->globalGS->function;

	if ( !gs->TraceFunc ) return; /* so we exit properly out of CALL() */

	while ( gs->insPtr < eptr ) {
		/* The interpreter does not use gs->roundToGrid, so set it here */
		if ( pb->RoundValue == (FntRoundFunc) fnt_RoundToGrid )
			gs->roundToGrid = 1;
		else if ( pb->RoundValue == (FntRoundFunc) fnt_RoundToHalfGrid )
			gs->roundToGrid = 0;
		else if ( pb->RoundValue == (FntRoundFunc) fnt_RoundToDoubleGrid )
			gs->roundToGrid = 2;
		else if ( pb->RoundValue == (FntRoundFunc) fnt_RoundDownToGrid )
			gs->roundToGrid = 3;
		else if ( pb->RoundValue == (FntRoundFunc) fnt_RoundUpToGrid )
			gs->roundToGrid = 4;
		else if ( pb->RoundValue == (FntRoundFunc) fnt_RoundOff )
			gs->roundToGrid = 5;
		else if ( pb->RoundValue == (FntRoundFunc) fnt_SuperRound )
			gs->roundToGrid = 6;
		else if ( pb->RoundValue == (FntRoundFunc) fnt_Super45Round )
			gs->roundToGrid = 7;
		else
			gs->roundToGrid = -1;

	/* MTE   don't use trace	gs->TraceFunc(gs->context->callbacks, gs );*/

		if ( !gs->TraceFunc ) break; /* in case the editor wants to exit */

		function[ gs->opCode = *gs->insPtr++ ]( gs );
	}
	gs->insPtr = oldInsPtr;
}

#ifdef linkedIn
/*	#define record_opCodes		*/
#endif

#ifdef record_opCodes
	extern tt_int32 opCodeCounter[];
#endif

/*
 * This is the fast non-tracing interpreter.
 */

/* This is the fast non-tracing interpreter.*/
void fnt_InnerExecute(register fnt_LocalGraphicStateType *gs, tt_uint8 *ptr, tt_uint8 *eptr)
{
    register FntFunc* function;
    tt_uint8 *oldInsPtr = gs->insPtr;		
    gs->insPtr = ptr;
    function = gs->globalGS->function;
#ifdef really_slow_debugging
    CHECK_STATE( gs );
#endif
    while ( gs->insPtr < eptr ){
			
#ifdef record_opCodes
      opCodeCounter[*gs->insPtr]++;
#endif
      function[ gs->opCode = *gs->insPtr++ ]( gs );
#ifdef really_slow_debugging
      CHECK_STATE( gs );
#endif
    }
			
    gs->insPtr = oldInsPtr;
}

/*
 *   This guy gets called if a composit glyph has instructions, and it then calls IP, IUP
 *   MDRP or MD. These instructions use the oox, ooy arrays, which do not necessarily 
 *   reflect the hinted outlines. This function inverts the hinted outline back into 
 *   the unscaled outline. 
 *   This is only done once, and is triggered by the flag unscaledOutlineIsWrong.
*/

void CorrectUnscaledOutline(fnt_LocalGraphicStateType* gs)
{
	fnt_ElementType* elem = gs->elements[GLYPHELEMENT];

	IfDebugMessage(gs->unscaledOutlineIsWrong == false, 
		       "CorrectUnscaledOutline improperly called", 0);

	if (elem->pointCount) {	
	    F26Dot6 *x, *y;
		FWord *oox, *ooy, *ooStopX;
		Fixed scaleX, scaleY;

		scaleX = FixedDivide(fixed1, gs->globalGS->upemScale.x);
		scaleY = FixedDivide(fixed1, gs->globalGS->upemScale.y);
		x = elem->ox; // fix for bug #4177514 - IUP - unhinted outlines - not hinted outline
		y = elem->oy;
		oox = elem->oox;
		ooy = elem->ooy;
		ooStopX = oox + elem->pointCount;
		do {
			*oox++ = FixedMultiply(*x++, scaleX);
			*ooy++ = FixedMultiply(*y++, scaleY);
		} while (oox < ooStopX);
	}
	gs->unscaledOutlineIsWrong = false;
}

/*
 * Executes the gxFont instructions.
 * This is the external interface to the interpreter.
 *
 * Parameter Description
 *
 * elements points to the character elements. Element 0 is always
 * reserved and not used by the actual character.
 *
 * ptr points at the first instruction.
 * eptr points to right after the last instruction
 *
 * globalGS points at the global graphics state
 *
 * TraceFunc is pointer to a callback functioned called with a pointer to the
 *		local graphics state if TraceFunc is not null.
 *
 * Note: The stuff globalGS is pointing at must remain intact
 *       between calls to this function.
 */
void fnt_Execute(fnt_ElementType* elements[], 
			fnt_GlobalGraphicStateType *globalGS, 
			tt_uint8 *ptr, register tt_uint8 *eptr,
			voidFunc TraceFunc, memoryContext* context, boolean hasStyleCoord, boolean hasVariantCVT,
			boolean hintingACompositGlyph)
{
	fnt_LocalGraphicStateType GS;
	register fnt_LocalGraphicStateType *gs; /* the local graphics state pointer */

	/* #pragma unused (TraceFunc,hasStyleCoord,hasVariantCVT) */


	gs = &GS;
	gs->globalGS = globalGS;

	gs->elements = elements;
	gs->Pt0 = gs->Pt1 = gs->Pt2 = 0;
	gs->CE0 = gs->CE1 = gs->CE2 = elements[GLYPHELEMENT];
	gs->free.x = gs->proj.x = gs->oldProj.x = shortFrac1;
	gs->free.y = gs->proj.y = gs->oldProj.y = 0;
	gs->pfProj = shortFrac1;
	gs->MovePoint = (FntMoveFunc) fnt_XMovePoint;
	gs->Project   = (FntProjFunc) fnt_XProject;
	gs->OldProject = (FntProjFunc) fnt_XProject;
	gs->loop = 0;		/* 1 less than count for faster loops. mrr */
#ifdef support_fake_variation_cvt
	gs->fakeVariantCVT = hasStyleCoord && !hasVariantCVT;
#endif
#ifdef debugging
	gs->valid_pfProj = true;
	IfDebugMessage(globalGS->pgmIndex != noProgramIndex && 
		       globalGS->pgmIndex != preProgramIndex &&
		       globalGS->pgmIndex != fontProgramIndex, "bad pgmIndex", 
		       globalGS->pgmIndex);
#endif
	gs->projectionVectorIsNormal = false;
	gs->unscaledOutlineIsWrong = hintingACompositGlyph;

	if (globalGS->pgmIndex == fontProgramIndex)
	{
#ifdef debugging
	  gs->GetCVTEntry = (F26Dot6 (*) (struct fnt_LocalGraphicStateType *, ArrayIndex)) fnt_NilFunction;
	  gs->GetSingleWidth = (F26Dot6 (*) (struct fnt_LocalGraphicStateType *)) fnt_NilFunction2;
#endif
	  goto ASSIGN_POINTERS;
	}

#ifdef check_cvt_access_direction
	if (globalGS->pgmIndex == noProgramIndex)
	{	char* flags = globalGS->cvtFlags;
		char* stop = flags + globalGS->cvtCount;

		while (flags < stop)
			*flags++ &= ~kCvtWrittenToByThisGlyph;
	}
#endif

	if (globalGS->identityTransformation)
	{	gs->GetCVTEntry = fnt_GetCVTEntryFast;
		gs->GetSingleWidth = fnt_GetSingleWidthFast;
	}
	else
	{	gs->GetCVTEntry = fnt_GetCVTEntrySlow;
		gs->GetSingleWidth = fnt_GetSingleWidthSlow;
	}
	 
	if (globalGS->localParBlock.sW)
		globalGS->localParBlock.scaledSW = FixedMultiply(globalGS->upemScale.x, globalGS->localParBlock.sW);

ASSIGN_POINTERS:

	/* MTE: use the pre-allocated stack */
	gs->stackPointer =
		gs->stackBase = globalGS->stackZone;
			gs->stackEnd = (F26Dot6*)
					(   ( (Ptr) globalGS->stackZone)+ globalGS->stackSize);
	gs->stackSize = globalGS->stackSize;
	gs->context = context;

	if (setjmp(gs->env) != 0)
	{	
		ResetHintedOutline(elements[GLYPHELEMENT]);
		goto CLEAN_UP;
	}
 			gs->TraceFunc = 0L;
			gs->Interpreter = (InterpreterFunc) fnt_InnerExecute ;
			gs->Interpreter( gs, ptr, eptr );
 CLEAN_UP:
	{
		/*  MTE  */
		if (gs->stackBase != nil
			/* MTE  :not needed && context->callbacks->ScalerFunction == nil*/
			)
		{	
			IfDebugMessage(gs->stackBase != gs->stackPointer, "leftover stack elements", gs->stackPointer - gs->stackBase);
			/* MTE  not needed	ScalerDisposeBlock(context, gs->stackBase, scalerScratchBlock);*/
		}
		
	}
}

F26Dot6* GrowStackForPush(fnt_LocalGraphicStateType* gs, tt_int32 count)
{
/* MTE    #ifdef debugging*/
/* For TT, stack should never overflow because there is a maximum.*/
#ifdef debugging 
 		IfDebug(gs->stackPointer > gs->stackEnd,"*** GS->STACKPOINTER OVERFLOWED!!!");
 #endif

#if 0
	if (gs->stackPointer + count > gs->stackEnd)	/* Do we need to grow the stack? */
	{	F26Dot6 *newStack;
		tt_int32 extraSize = count * sizeof(F26Dot6);
		
		/* We'll be changing stackBase, stackEnd, and stackPointer */
		
		if (extraSize < STACKSIZE_INCREMENT)
			extraSize = STACKSIZE_INCREMENT;
		
		newStack = (F26Dot6*)ScalerNewBlock(gs->context, gs->stackSize + extraSize,
					scalerScratchBlock, gs->stackBase, kRequiredNewBlock);

		gs->stackSize += extraSize;
		gs->stackEnd = (F26Dot6 *)((char *)newStack + gs->stackSize);

		/* Make the relevent stack pointers gxPoint to their new relative positions */
		gs->stackPointer = newStack + (gs->stackPointer - gs->stackBase);
		gs->stackBase = newStack;		/* Now using the new stack */
	}
#endif
	return gs->stackPointer;
}	
#endif  
	/* #ifdef ENABLE_TT_HINTING */ 
