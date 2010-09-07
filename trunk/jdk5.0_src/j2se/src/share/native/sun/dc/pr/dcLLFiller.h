/*
 * @(#)dcLLFiller.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcLLFiller.h 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#ifndef _DC_LLFILLER_H
#define _DC_LLFILLER_H

#include "doeObject.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define dcLLFiller_EOFILL	1
#define dcLLFiller_NZFILL	2

typedef struct dcLLFillerFace_**	dcLLFiller;
typedef struct dcLLFillerFace_ {
    doeObjectFace		mu;
    void (*setParams)		(doeE env, dcLLFiller,	ixx fillmode, ixx sizex, ixx sizey);
    void (*processLeftRun)	(doeE env, dcLLFiller,	i32 fromy, i32 toy);
    void (*beginSubpath)	(doeE env, dcLLFiller,	i32 x0, i32 y0);
    void (*appendArc1)		(doeE env, dcLLFiller,	i32 x1, i32 y1);
    void (*appendArc2)		(doeE env, dcLLFiller,	i32 x1, i32 y1,
							i32 x2, i32 y2);
    void (*appendArc3)		(doeE env, dcLLFiller,	i32 x1, i32 y1,
							i32 x2, i32 y2,
							i32 x3, i32 y3);
    void (*writeAlpha8)		(doeE env, dcLLFiller,	 u8* alpha, i32 xstride, i32 ystride,
							i32 pix0offset);
    void (*writeAlpha16)	(doeE env, dcLLFiller,	u16* alpha, i32 xstride, i32 ystride,
							i32 pix0offset);
} dcLLFillerFace;

extern ixx		dcLLFiller_maxWN;
extern ixx		dcLLFiller_tileSizeL2S;
extern ixx		dcLLFiller_subGridL2S;
extern f32		dcLLFiller_tileSizeSub;
extern f32		dcLLFiller_pixSizeSub;

#define	dcLLFiller_tss2U(tss)		\
	(i32)(tss * dcLLFiller_tileSizeSub + ((tss > 0.0F)? 0.5F : -0.5F))

#define	dcLLFiller_pix2U(pix)		\
	(i32)(pix * dcLLFiller_pixSizeSub + ((pix > 0.0F)? 0.5F : -0.5F))

extern ixx		dcLLFiller_ticsSetupArc1HV;
extern ixx		dcLLFiller_ticsSetupArc1;
extern ixx		dcLLFiller_ticsSetupArc2;
extern ixx		dcLLFiller_ticsSetupArc3;
extern ixx		dcLLFiller_ticsStepArc1HV;
extern ixx		dcLLFiller_ticsStepArc1;
extern ixx		dcLLFiller_ticsStepArc2;
extern ixx		dcLLFiller_ticsStepArc3;


extern void		dcLLFiller_staticInitialize(doeE env);
extern void		dcLLFiller_staticFinalize  (doeE env);

/* no public creation */
extern dcLLFiller	dcLLFiller_get(doeE env);
extern void		dcLLFiller_release(doeE env, dcLLFiller f);

#ifdef	__cplusplus
}
#endif

#endif /* _DC_LLFILLER_H */

