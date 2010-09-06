/*
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)textboundary.h	1.3 02/06/13
 */

#ifndef	_TEXTBOUNDARY_H
#define	_TEXTBOUNDARY_H

#pragma ident	"@(#)textboundary.h	1.4	99/11/17 SMI"

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum {
	TBR_Alphabetic = 1L,
	TBR_WhiteSpace = (1L<<1),
	TBR_Control = (1L<<2),
	TBR_Digit = (1L<<3),
	TBR_Graphic = (1L<<4),
	TBR_Lowercase = (1L<<5),
	TBR_Uppercase = (1L<<6),
	TBR_Printing = (1L<<7),
	TBR_Punctuation = (1L<<8),
	TBR_HexDigit = (1L<<9),
	TBR_LineBreakCharacter = (1L<<10),
	TBR_LineBreakHyphen = (1L<<11),
	TBR_LineBreakScript = (1L<<12),
	TBR_WordBoundary = (1L<<13),
	TBR_SentenceBoundary = (1L<<14),
	TBR_ParagraphBoundary = (1L<<15),
	TBR_CharsetBoundary = (1L<<16),
	TBR_ScriptBoundary = (1L<<17),
	TBR_CompositeBoundary = (1L<<18)
} TBRScanCondition;

typedef enum {
	TBR_Forw, TBR_Back
} TBRScanDirection;


typedef void 	textboundary_t;


#define	TBR_MODULE_VERSION	1


#ifdef	__cplusplus
}
#endif

#endif	/* _TEXTBOUNDARY_H */
