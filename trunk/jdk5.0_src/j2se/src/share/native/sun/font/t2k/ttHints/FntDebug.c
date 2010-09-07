/*
 * @(#)FntDebug.c	1.8 03/12/19
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

#define noObsoleteMacros

#include "FntDebug.h"
#include "PrivateFnt.h"
	/* #include "private_macros.h" */

#define F26Dot6IsRounded(value)	(((value) & 63) == 0)

#ifdef check_cvt_access_direction
		static char ClassifyVectorDirection(const shortVector* vec)
		{
			char direction = 0;

			if (vec->x)
				direction |= kCvtHoriDirection;
			if (vec->y)
				direction |= kCvtVertDirection;
			return direction;
		}
	#endif

	void CHECK_RANGE(fnt_LocalGraphicStateType* gs, tt_int32 n, tt_int32 min, tt_int32 max)
	{
#ifdef applec
#pragma unused(gs)
#endif
		IfDebugMessage(n > max || n < min, "interpreter value out of range", n);
	}

	void CHECK_ASSERTION(fnt_LocalGraphicStateType* gs, int expression )
	{
#ifdef applec
#pragma unused(gs)
#endif
		IfDebugMessage(!expression, "interpreter assertion failed", 0);
	}

	#ifdef check_cvt_access_direction
		void CHECK_CVT_READ(fnt_LocalGraphicStateType* gs, int cvt)
		{
			char flags = gs->globalGS->cvtFlags[cvt];

			CHECK_RANGE(gs, cvt, 0, gs->globalGS->cvtCount-1);
			
			{	char direction = flags & kCvtDirectionMask;
				char currDirection = ClassifyVectorDirection(&gs->proj);
				if (direction != kCvtNoDirection && direction != currDirection)
					DebugMessage("CVT reading in inconsistent direction", cvt);
			}
			if (gs->globalGS->pgmIndex == noProgramIndex && flags & kCvtWrittenToByAGlyph && !(flags & kCvtWrittenToByThisGlyph))
				DebugMessage("reading CVT written to by another glyph", cvt);
		}

		void CHECK_CVT_WRITE(fnt_LocalGraphicStateType* gs, int cvt, F26Dot6 value)
		{
			CHECK_RANGE(gs, cvt, 0, gs->globalGS->cvtCount-1);
			if (value && F26Dot6IsRounded(value))		/* only flag rounded writes */
			{	gs->globalGS->cvtFlags[cvt] &= ~kCvtDirectionMask;
				gs->globalGS->cvtFlags[cvt] |= ClassifyVectorDirection(&gs->proj);
			}
			if (gs->globalGS->pgmIndex == noProgramIndex)
				gs->globalGS->cvtFlags[cvt] |= kCvtWrittenToByAGlyph | kCvtWrittenToByThisGlyph;
		}
	#endif

	void CHECK_FDEF(fnt_LocalGraphicStateType* gs, int fdef)
	{
		CHECK_RANGE(gs, fdef, 0, gs->globalGS->maxp->maxFunctionDefs-1);
	}

	void CHECK_PROGRAM(fnt_LocalGraphicStateType* gs, int pgmIndex)
	{
		CHECK_RANGE(gs, pgmIndex, fontProgramIndex, preProgramIndex);
	}

	void CHECK_ELEMENT(fnt_LocalGraphicStateType* gs, int elem)
	{
		CHECK_RANGE(gs, elem, 0, gs->globalGS->maxp->maxElements-1);
	}

	void CHECK_ELEMENTPTR(fnt_LocalGraphicStateType* gs, fnt_ElementType* elem)
	{	
		if (elem == gs->elements[GLYPHELEMENT])
		{
			int maxctrs, maxpts;

			maxctrs = MAX(gs->globalGS->maxp->maxContours, gs->globalGS->maxp->maxCompositeContours);
			maxpts  = MAX(gs->globalGS->maxp->maxPoints, gs->globalGS->maxp->maxCompositePoints);

			CHECK_RANGE(gs, elem->contourCount, 1, maxctrs);
			CHECK_RANGE(gs, elem->pointCount, 1, maxpts);
		}
		else
			IfDebugMessage(elem != gs->elements[TWILIGHTZONE], "bad elem ptr", (tt_int32)elem);
	}

	void CHECK_STORAGE(fnt_LocalGraphicStateType* gs, int index)
	{
		CHECK_RANGE(gs, index, 0, gs->globalGS->maxp->maxStorage-1);
	}

	void CHECK_POINT(fnt_LocalGraphicStateType* gs, fnt_ElementType* elem, int pt)
	{
		CHECK_ELEMENTPTR(gs, elem);
		if (gs->elements[TWILIGHTZONE] == elem)
			CHECK_RANGE(gs, pt, 0, gs->globalGS->maxp->maxTwilightPoints - 1);
		else
			CHECK_RANGE(gs, pt, 0, elem->pointCount + metricPointCount - 1);
	}

	void CHECK_CONTOUR(fnt_LocalGraphicStateType* gs, fnt_ElementType* elem, int ctr)
	{
		CHECK_ELEMENTPTR(gs, elem);
		CHECK_RANGE(gs, ctr, 0, elem->contourCount - 1);
	}

	void CHECK_PFPROJ(fnt_LocalGraphicStateType* gs)
	{
#pragma unused(gs)

#if be_mean_to_fonts_debugging
			CHECK_ASSERTION(gs, gs->valid_pfProj );
#endif
	}

	void CHECK_STATE( fnt_LocalGraphicStateType *gs )
	{
		fnt_ElementType* elem;
		F26Dot6* x, *y, *ox, *oy;
		tt_int16 count;
		F26Dot6 cutin = 4*64;

		if (gs->globalGS->pgmIndex != noProgramIndex) 
			return;

		elem = gs->elements[GLYPHELEMENT];
		x = elem->x;
		y = elem->y;
		ox = elem->ox;
		oy = elem->oy;
		count = elem->pointCount
				/*   + 4; MTE: only 2 phantom points! */
				+2;
		do {
			F26Dot6 diffx,diffy,anox,anoy;
			anox= *ox++;
			anoy= *oy++;
			diffx = *x - anox;
			diffy = *y - anoy;
			if (diffx < 0)
				 diffx = -diffx;
			if (diffx > cutin) 
				DebugMessage("error", 0);
			if (diffy < 0) 
				diffy = -diffy;
			if (diffy > cutin) 
				DebugMessage("error", 0);

			x++, y++;
		} while (--count);
	}

#endif
#endif  
/* #ifdef ENABLE_TT_HINTING */
