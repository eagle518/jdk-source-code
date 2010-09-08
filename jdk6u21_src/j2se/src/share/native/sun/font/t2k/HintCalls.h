/*
 * @(#)HintCalls.h	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*   Specify the memory interface (context) between T2K and tt hinting.
 
     Only performs hinting,
     All other code including in/out streaming and memory management is
     to be handled by the T2K scaler.
     Also, T2K code is reponsible for bookkeeping  of open fonts.
 */
 
#ifndef	uniMemIncludes
#define	uniMemIncludes

  /* Now the interface routines which are called by T2K */
void NewTTHintFontForT2K(T2K *	aT2KScaler);
void ReleaseTTHintFontForT2K(T2K *	aT2KScaler);

void InitTTHintTranForT2K(T2K *	aT2KScaler);
void NewTTHintTranForT2K(T2K *	aT2KScaler);
void ReleaseTTHintTranForT2K(T2K *	aT2KScaler);

/* Original T2K glyph and T2K scaler data as input */
int TTScalerHintGlyph(GlyphClass *theGlyphT2K, T2K *aT2KScaler);

#endif

