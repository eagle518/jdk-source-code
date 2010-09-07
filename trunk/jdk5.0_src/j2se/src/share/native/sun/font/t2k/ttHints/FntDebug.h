/*
 * @(#)FntDebug.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 /*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/
#ifndef fntDebugIncludes
#define fntDebugIncludes

#include "Fnt.h"

#ifdef check_cvt_access_direction
	enum cvtDirection {
		kCvtNoDirection		= 0x00,
		kCvtHoriDirection		= 0x01,
		kCvtVertDirection		= 0x02,
		kCvtDirectionMask		= kCvtHoriDirection | kCvtVertDirection,

		kCvtWrittenToByAGlyph		= 0x04,
		kCvtWrittenToByThisGlyph	= 0x08
	};
	void CHECK_CVT_READ(fnt_LocalGraphicStateType* gs, int cvt);
	void CHECK_CVT_WRITE(fnt_LocalGraphicStateType* gs, int cvt, F26Dot6 value);
#else
#define CHECK_CVT_READ(gs, cvt)			CHECK_RANGE(gs, cvt, 0, gs->globalGS->cvtCount-1)
#define CHECK_CVT_WRITE(gs, cvt, value)	CHECK_RANGE(gs, cvt, 0, gs->globalGS->cvtCount-1)
#endif

#ifdef debugging

	void CHECK_RANGE(fnt_LocalGraphicStateType* gs, tt_int32 n, tt_int32 min, tt_int32 max);
	void CHECK_ASSERTION(fnt_LocalGraphicStateType* gs, int expression );
	void CHECK_FDEF(fnt_LocalGraphicStateType* gs, int fdef);
	void CHECK_PROGRAM(fnt_LocalGraphicStateType* gs, int pgmIndex);
	void CHECK_ELEMENT(fnt_LocalGraphicStateType* gs, int elem);
	void CHECK_ELEMENTPTR(fnt_LocalGraphicStateType* gs, fnt_ElementType* elem);
	void CHECK_STORAGE(fnt_LocalGraphicStateType* gs, int index);
	void CHECK_POINT(fnt_LocalGraphicStateType* gs, fnt_ElementType* elem, int pt);
	void CHECK_CONTOUR(fnt_LocalGraphicStateType* gs, fnt_ElementType* elem, int ctr);
	void CHECK_PFPROJ(fnt_LocalGraphicStateType* gs);
	void CHECK_STATE( fnt_LocalGraphicStateType* );
	
#else

#define CHECK_RANGE(g,a,b,c)
#define CHECK_ASSERTION(g,a)
#define CHECK_POINT(a,b,c)
#define CHECK_CONTOUR(a,b,c)
#define CHECK_PFPROJ(a)
#define CHECK_FDEF(a,b)
#define CHECK_PROGRAM(g,a)
#define CHECK_ELEMENT(a,b)
#define CHECK_ELEMENTPTR(a,b)
#define CHECK_STORAGE(a,b)
#define CHECK_STATE(a)

#endif

#define CHECK_POP(gs, s)		POP(s)
#define CHECK_PUSH(gs, s, v)	PUSH(s, v)

#endif
