/*
 * @(#)doeObject.c	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)doeObject.c 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#include "doeObject-p.h"

#define	ROOTED_OBJECT		0
#define	UPROOTED_OBJECT		1

static	char*	className	(doeE, doeObject);
static	doeObject
		copy		(doeE, doeObject);
static	void	_cleanup	(doeE, doeObject);
static	void	_enumCoObs	(doeE, doeObject, doeObjectEnumCb);

doeObjectFace	doeObjectClass = {	sizeof(doeObjectData),	/* Object i/f */
					className,
					copy,
					_cleanup,
					_enumCoObs,
					doeObject_uproot
				  };
void
doeObject_init(doeE env, doeObject o)
{
    doeObjectData*	po = (doeObjectData*)o;

    po->face = 0;	/* since object is a partial class, the derived
			 * class must take up this responsiblity */
/*  po->xxxx = (void*)ROOTED_OBJECT;	*/
}

void
doeObject_copyinit(doeE env, doeObject target, doeObject source)
{
    doeObjectData*	ptarget = (doeObjectData*)target;
    doeObjectData*	psource = (doeObjectData*)source;

    ptarget->face = psource->face;
/*  ptarget->xxxx = (void*)ROOTED_OBJECT; 	*/
}

void
doeObject_uproot(doeE env, doeObject o)
{
    doeObjectData*	po = (doeObjectData*)o;

/*  po->xxxx = (void*)UPROOTED_OBJECT;	*/
}

char*
className(doeE env, doeObject target)
{
    return ("doeObject");
}
doeObject
copy(doeE env, doeObject target)
{
    return target;
}
void
_cleanup(doeE env, doeObject target)
{
}
void
_enumCoObs(doeE env, doeObject target, doeObjectEnumCb cb)
{
}
