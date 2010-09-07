/*
 * @(#)dcLLFillerH.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcLLFillerH.h 3.1 97/11/17
 *
 * -------------------------------------------------
 *	Copyright (c) 1992-1996 by Ductus, Inc.
 * -------------------------------------------------
 */

#ifndef _DC_LLFILLERH_H
#define _DC_LLFILLERH_H

#include "dcLLFiller.h"

#ifdef	__cplusplus
extern "C" {
#endif


#define dcLLFillerH_maxWN		63
#define dcLLFillerH_tileSizeL2S		5
#define dcLLFillerH_subGridL2S		3

#define dcLLFillerH_ticsSetupArc1HV	7
#define dcLLFillerH_ticsSetupArc1	7
#define dcLLFillerH_ticsSetupArc2	8
#define dcLLFillerH_ticsSetupArc3	9
#define dcLLFillerH_ticsStepArc1HV	1
#define dcLLFillerH_ticsStepArc1	1
#define dcLLFillerH_ticsStepArc2	1
#define dcLLFillerH_ticsStepArc3	1

extern void		dcLLFillerH_staticInitialize(doeE env);
extern void		dcLLFillerH_staticFinalize  (doeE env);

/* no public creation */
extern dcLLFiller	dcLLFillerH_get(doeE env);
extern void		dcLLFillerH_release(doeE env, dcLLFiller f);

extern int		dcLLFillerH_exists(doeE env);


#ifdef	__cplusplus
}
#endif

#endif /* _DC_LLFILLERH_H */
