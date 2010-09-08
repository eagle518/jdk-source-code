/*
 * @(#)dcLLFillerH.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcLLFillerH.c 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#include "dcLLFillerH.h"
#include "doe.h"

static doeMutex		fillerMutex;
static ixx clients = 0;

void
dcLLFillerH_staticInitialize(doeE env)
{
/*
    if (clients++ > 0) return;
    fillerMutex = doeMutex_create(env);
*/
}

void
dcLLFillerH_staticFinalize(doeE env)
{
/*
    if (--clients > 0) return;
    doeMutex_destroy(env, fillerMutex);
*/
}

int
dcLLFillerH_exists(doeE env) {
    return 0;
}

dcLLFiller
dcLLFillerH_get(doeE env) {
/*  doeMutex_lock(env, fillerMutex); */
    return NULL;
}

void
dcLLFillerH_release(doeE env, dcLLFiller f) {
/*  doeMutex_unlock(env, fillerMutex); */
}
