/*
 * @(#)dcPool.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)dcPool.c 3.1 97/11/17
 *
 * -----------------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * -----------------------------------------------------------------------------
 *
 */

#include "doe.h"
#include "dcPool.h"

#ifdef DEBUG
#define DEBUG_MEMSTATS
#define DEBUG_MEMSTATS_PER_CYCLE
#endif

typedef struct dcPoolItemData_*	dcPoolItem;
typedef struct dcPoolItemData_ {
    dcPool		origin;
    dcPoolItem		next;
} dcPoolItemData;

#define dcPool_samples	10

typedef struct dcPoolData_ {
    char*		poolName;
    ixx			itemSize;
    ixx			itemMinInPool;
    f32			xSigma;

    dcPoolItem		freeItems;
    i32			itemsInUse;
    i32			itemsMaxInUse;	/* in cycle */
    i32			itemsInPool;

    i32			sumU;
    i32			sumUU;
    ixx			ui;
    i32			U[dcPool_samples];

#ifdef DEBUG_MEMSTATS
#ifdef DEBUG_MEMSTATS_PER_CYCLE
    char*		aux;
#endif
    i32			totalPool;
    i32			totalUsed;
    i32			totalMem_malloc;
    i32			totalMem_free;
    i32			totalCycles;
#endif
} dcPoolData;

static int clients = 0;

#ifdef DEBUG_MEMSTATS
static int memstat_verbose;
#endif

void
dcPool_staticInitialize(doeE env)
{
    if (clients++ > 0)	return;

#ifdef DEBUG_MEMSTATS
    memstat_verbose = (getenv("DCPR_MEMSTATS") != NULL);
#endif
}

void
dcPool_staticFinalize(doeE env)
{
    if (--clients > 0) return;
}

static void
dcPool_init(doeE env, dcPool pool, char* poolname, ixx itembytes, ixx initialitems, f32 xsigma)
{
    ixx		i;
    dcPoolItem	item;

    for (i = 0; poolname[i] != 0; i++);
    pool->poolName = doeMem_malloc(env, i + 1);
    if (pool->poolName == NULL) {
	doeError_setNoMemory(env);
	return;
    }
    while (i >= 0) {
	pool->poolName[i] = poolname[i];
	i--;
    }

    pool->itemSize = itembytes =
	sizeof(void*)*(((itembytes + sizeof(void*) - 1) / sizeof(void*)) + 1);
    pool->itemMinInPool = initialitems;
    pool->xSigma = xsigma;

    for (i = 0; i < dcPool_samples; i++)
	pool->U[i] = initialitems;
    pool->sumU  = dcPool_samples * initialitems;
    pool->sumUU = dcPool_samples * initialitems * initialitems;
    pool->ui    = 0;

#ifdef DEBUG_MEMSTATS
#ifdef DEBUG_MEMSTATS_PER_CYCLE
    pool->aux = doeMem_malloc(env, 102);
    for (i = 0; i <= 100; i++) {
	pool->aux[i] = ' ';
	if (0 == i % 10)	pool->aux[i] = '|';
	if (i == initialitems)	pool->aux[i] = 'A';
    }
    pool->aux[101] = 0;
#endif
    pool->totalPool =  0;
    pool->totalUsed =	    0;
    pool->totalMem_malloc = 0;
    pool->totalMem_free =   0;
    pool->totalCycles =	    0;
#endif

    item = NULL;
    for (i = 0; i < initialitems; i++) {
        dcPoolItem newitem = doeMem_malloc(env, itembytes);
	if (newitem == NULL) {
	    doeError_setNoMemory(env);
	    return;
	}
#ifdef DEBUG_MEMSTATS
	pool->totalMem_malloc++;
#endif
	newitem->origin = pool;
	newitem->next = item;
	item = newitem;
    }
    pool->freeItems = item;
    pool->itemsInUse = pool->itemsMaxInUse = 0;
    pool->itemsInPool = initialitems;

}

static void
dcPool_cleanup(doeE env, dcPool pool)
{
    doeMem_free(env, pool->poolName);
    while (pool->freeItems != NULL) {
	dcPoolItem item = pool->freeItems;
	pool->freeItems = item->next;
#ifdef DEBUG_MEMSTATS
	pool->totalMem_free++;
#endif
	doeMem_free(env, item);
    }
}

dcPool
dcPool_create(doeE env, char* poolname, ixx itembytes, ixx initialitems, f32 xsigma)
{
    dcPool	pool = doeMem_malloc(env, sizeof(dcPoolData));
    if (pool == NULL) {
	doeError_setNoMemory(env);
	return NULL;
    }
    dcPool_init(env, pool, poolname, itembytes, initialitems, xsigma);
    if (doeError_occurred(env)) {
	dcPool_cleanup(env, pool);
	doeMem_free(env, pool);
	return NULL;
    }

    return pool;
}

void
dcPool_destroy(doeE env, dcPool pool)
{
    if (pool == NULL) return;
#ifdef DEBUG_MEMSTATS
  if (memstat_verbose) {
    printf("\n%20s 999999 averages/cycle: pool=%5.1f, used=%5.1f, malloc=%5.1f, free=%5.1f; %5dcycles\n",
	pool->poolName,
	(f32)(pool->totalPool)		/pool->totalCycles,
	(f32)(pool->totalUsed)		/pool->totalCycles,
	(f32)(pool->totalMem_malloc)	/pool->totalCycles,
	(f32)(pool->totalMem_free)	/pool->totalCycles,
	pool->totalCycles);
  }
#endif
    dcPool_cleanup(env, pool);
    doeMem_free(env, pool);
}

void*
dcPool_getItem(doeE env, dcPool pool)
{
    dcPoolItem	item;

    item = pool->freeItems;
    if (item == NULL) {
        item = doeMem_malloc(env, pool->itemSize);
	if (item == NULL) {
	    doeError_setNoMemory(env);
	    return NULL;
	}
	item->origin = pool;
	item->next   = NULL;
	pool->freeItems = item;
	pool->itemsInPool++;
#ifdef DEBUG_MEMSTATS
	pool->totalMem_malloc++;
#endif
    }
    pool->freeItems = item->next;
    pool->itemsInUse++;
    if (pool->itemsInUse > pool->itemsMaxInUse)
	pool->itemsMaxInUse = pool->itemsInUse;
    return (void*)((void**)item + 1);
}

void
dcPool_staticReleaseItem(doeE env, void* data)
{
    dcPoolItem	item = (dcPoolItem)((void**)data - 1);
    dcPool	pool = item->origin;

    item->next = pool->freeItems;
    pool->freeItems = item;

    pool->itemsInUse--;
}

void
dcPool_endCycle(doeE env, dcPool pool)
{
	dcPoolItem	item;
	ixx		ui = pool->ui;
	i32		u  = pool->U[ui];
	f32		sigma;
	f32		umean;
	ixx		bestpoolsize;
	ixx		poolsize;

	pool->sumU  -= u;
	pool->sumUU -= u*u;
	pool->U[ui] = u = pool->itemsMaxInUse;
	pool->itemsMaxInUse = 0;
	pool->sumU  += u;
	pool->sumUU += u*u;
	ui++;
	if (ui >= dcPool_samples) ui = 0;
	pool->ui = ui;

	umean = (f32)(pool->sumU) / dcPool_samples;
	sigma  = pool->sumUU + dcPool_samples*umean*umean - 2.0F*umean*pool->sumU;
	sigma /= (dcPool_samples - 1);
	sigma  = sqrt(sigma);
	bestpoolsize = ceil(umean + pool->xSigma * sigma);
	if (bestpoolsize < pool->itemMinInPool)
	    bestpoolsize = pool->itemMinInPool;

	poolsize = pool->itemsInPool;
#ifdef DEBUG_MEMSTATS
	pool->totalCycles++;
	pool->totalPool += poolsize;
	pool->totalUsed += u;
/*
      if (memstat_verbose) {
	printf("%20s: Uin=%3d   Umean=%6.2f sigma=%6.2f  allocation: curr=%3d, best=%3d",
		pool->poolName, u, umean, sigma, poolsize, bestpoolsize);
	if (poolsize > bestpoolsize)	printf(" excess freed");
	printf("\n");
      }
*/
#ifdef DEBUG_MEMSTATS_PER_CYCLE
	{
	    ixx i;
	    for (i = 0; i <= 100; i++) {
		if (i == u)	pool->aux[i] = 'U';
	    }
	}
      if (memstat_verbose) {
	printf("%20s %6d %s\n", pool->poolName, pool->totalCycles, pool->aux);
      }
#endif
#endif
	while (poolsize > bestpoolsize) {
	    poolsize--;
	    item = pool->freeItems;
	    pool->freeItems = item->next;
	    doeMem_free(env, item);
#ifdef DEBUG_MEMSTATS
	    pool->totalMem_free++;
#endif
	}
	pool->itemsInPool = poolsize;
#ifdef DEBUG_MEMSTATS_PER_CYCLE
	{
	    ixx i;
	    ixx min = floor(umean - pool->xSigma * sigma);
	    ixx max = ceil (umean + pool->xSigma * sigma);
	    ixx ave = floor(umean + 0.5);
	    for (i = 0; i <= 100; i++) {
		pool->aux[i] = ' ';
		if (i >= min && i <= max)	pool->aux[i] = '-';
		if (0 == i % 10)		pool->aux[i] = '|';
		if (i == ave)			pool->aux[i] = 'o';
		if (i == poolsize)		pool->aux[i] = 'A';
	    }
	}
#endif
}


