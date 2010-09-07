/*
 * @(#)dcLLFiller.c	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcLLFiller.c 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#include "doe.h"
#include "dcLLFiller.h"
#include "dcLLFillerH.h"
#include "dcLLFillerS.h"

ixx	dcLLFiller_maxWN;
ixx	dcLLFiller_tileSizeL2S;
ixx	dcLLFiller_subGridL2S;
f32	dcLLFiller_tileSizeSub;
f32	dcLLFiller_pixSizeSub;
ixx	dcLLFiller_ticsSetupArc1HV;
ixx	dcLLFiller_ticsSetupArc1;
ixx	dcLLFiller_ticsSetupArc2;
ixx	dcLLFiller_ticsSetupArc3;
ixx	dcLLFiller_ticsStepArc1HV;
ixx	dcLLFiller_ticsStepArc1;
ixx	dcLLFiller_ticsStepArc2;
ixx	dcLLFiller_ticsStepArc3;

static int	fillerHExists;

static int clients = 0;
void
dcLLFiller_staticInitialize(doeE env) {
    if (clients++ > 0) return;

    dcLLFillerH_staticInitialize(env);
    if (doeError_occurred(env))	return;
    dcLLFillerS_staticInitialize(env);
    if (doeError_occurred(env))	return;

    fillerHExists = dcLLFillerH_exists(env);
    if (fillerHExists) {
	dcLLFiller_maxWN =		dcLLFillerH_maxWN;
	dcLLFiller_tileSizeL2S =	dcLLFillerH_tileSizeL2S;
	dcLLFiller_subGridL2S =		dcLLFillerH_subGridL2S;

	dcLLFiller_ticsSetupArc1HV =	dcLLFillerH_ticsSetupArc1HV;
	dcLLFiller_ticsSetupArc1 =	dcLLFillerH_ticsSetupArc1;
	dcLLFiller_ticsSetupArc2 =	dcLLFillerH_ticsSetupArc2;
	dcLLFiller_ticsSetupArc3 =	dcLLFillerH_ticsSetupArc3;
	dcLLFiller_ticsStepArc1HV =	dcLLFillerH_ticsStepArc1HV;
	dcLLFiller_ticsStepArc1 =	dcLLFillerH_ticsStepArc1;
	dcLLFiller_ticsStepArc2 =	dcLLFillerH_ticsStepArc2;
	dcLLFiller_ticsStepArc3 =	dcLLFillerH_ticsStepArc3;
    } else {
	dcLLFiller_maxWN =		dcLLFillerS_maxWN;
	dcLLFiller_tileSizeL2S =	dcLLFillerS_tileSizeL2S;
	dcLLFiller_subGridL2S =		dcLLFillerS_subGridL2S;

	dcLLFiller_ticsSetupArc1HV =	dcLLFillerS_ticsSetupArc1HV;
	dcLLFiller_ticsSetupArc1 =	dcLLFillerS_ticsSetupArc1;
	dcLLFiller_ticsSetupArc2 =	dcLLFillerS_ticsSetupArc2;
	dcLLFiller_ticsSetupArc3 =	dcLLFillerS_ticsSetupArc3;
	dcLLFiller_ticsStepArc1HV =	dcLLFillerS_ticsStepArc1HV;
	dcLLFiller_ticsStepArc1 =	dcLLFillerS_ticsStepArc1;
	dcLLFiller_ticsStepArc2 =	dcLLFillerS_ticsStepArc2;
	dcLLFiller_ticsStepArc3 =	dcLLFillerS_ticsStepArc3;
    }
    dcLLFiller_tileSizeSub	= (float)
				  (1 << (dcLLFiller_tileSizeL2S + dcLLFiller_subGridL2S));
    dcLLFiller_pixSizeSub	= (float)
				  (1 << dcLLFiller_subGridL2S);
}

void
dcLLFiller_staticFinalize(doeE env) {
  if (--clients > 0) return;

    dcLLFillerH_staticFinalize(env);
    dcLLFillerS_staticFinalize(env);
}

dcLLFiller
dcLLFiller_get(doeE env) {
    if (fillerHExists)
	return dcLLFillerH_get(env);
    else
	return dcLLFillerS_get(env);
}

void
dcLLFiller_release(doeE env, dcLLFiller f) {
    if (fillerHExists)
	dcLLFillerH_release(env, f);
    else
	dcLLFillerS_release(env, f);
}
