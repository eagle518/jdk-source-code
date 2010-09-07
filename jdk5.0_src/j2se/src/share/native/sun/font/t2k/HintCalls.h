/*
 * @(#)HintCalls.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
/* Original T2K glyph data as input */
int TTScalerHintGlyph(GlyphClass *theGlyphT2K,			
		      T2K * aT2KScaler, /* T2K scaler data	*/
		      void **scHintPtrVoid, tt_int32 applyStyle );/* use null if no image is to be created. */
#endif

