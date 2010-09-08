/*
 * @(#)dcLLFillerS.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcLLFillerS.h 3.1 97/11/17
 *
 * -------------------------------------------------
 *	Copyright (c) 1992-1996 by Ductus, Inc.
 * -------------------------------------------------
 */

#ifndef _DC_LLFILLERS_H
#define _DC_LLFILLERS_H

#include "dcLLFiller.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define dcLLFillerS_maxWN		63
#define dcLLFillerS_tileSizeL2S		5
#define dcLLFillerS_subGridL2S		3

#define dcLLFillerS_ticsSetupArc1HV	70
#define dcLLFillerS_ticsSetupArc1	77
#define dcLLFillerS_ticsSetupArc2	172
#define dcLLFillerS_ticsSetupArc3	265
#define dcLLFillerS_ticsStepArc1HV	16
#define dcLLFillerS_ticsStepArc1	28
#define dcLLFillerS_ticsStepArc2	43
#define dcLLFillerS_ticsStepArc3	60

extern void		dcLLFillerS_staticInitialize(doeE env);
extern void		dcLLFillerS_staticFinalize  (doeE env);

/* no public creation */
extern dcLLFiller	dcLLFillerS_get(doeE env);
extern void		dcLLFillerS_release(doeE env, dcLLFiller f);


#ifdef	__cplusplus
}
#endif

#endif /* _DC_LLFILLERS_H */
