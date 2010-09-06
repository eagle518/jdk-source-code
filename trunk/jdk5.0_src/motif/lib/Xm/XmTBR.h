/*
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _XMTBR_H
#define _XMTBR_H


#pragma ident	"@(#)XmTBR.h	1.4 02/06/13 SMI"

#include "textboundary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef textboundary_t	*TBRObject; 

/*
 * It was two data structures, XmTBRRec that contained TBRObject and
 * XmTBRFuncRec *, and, XmTBRFuncRec that contained textboundary.so 
 * dynamic library handle and pointers for its four functions.
 *
 * Since there is no plan at the moment to have a single XmTBRFuncRec
 * instance for an appliacation, there is no reason to have such
 * two data structures. Besides, we can save some execution time by
 * allocating a single memory block then two as it was done before.
 */
typedef struct {
	TBRObject	tbr_object;	/* text boundary object handle */
	void		*so;		/* textboundary.so dynamic library */
	TBRObject	(*m_create_tbr)();
	size_t		(*m_destroy_tbr)();
	size_t		(*m_strscanfor)();
	size_t		(*m_wcsscanfor)();
} XmTBRRec, *XmTBR;

extern size_t XmStrScanForTB(	XmTBR xm_tbr,
				char *string,
				size_t num_chars,
				Boolean is_wchar,
				XmTextPosition position,
				XmTextScanDirection dir,
				TBRScanCondition cond,
				Boolean inverse);
extern XmTBR XmCreateXmTBR();
extern void XmDestroyXmTBR(XmTBR xm_tbr);

#ifdef __cplusplus
}
#endif

#endif /* _XMTBR_H */
