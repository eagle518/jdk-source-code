/*
 * @(#)doeObject-p.h	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)doeObject-p.h 3.2 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _DOE_OBJECT_P_H
#define _DOE_OBJECT_P_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "doeObject.h"

typedef struct doeObjectData_ {
    doeObjectFace*	face;
/*  void*		xxxx; */
} doeObjectData;

extern	void	doeObject_init		(doeE, doeObject);
extern	void	doeObject_copyinit	(doeE, doeObject, doeObject source);

extern	void	doeObject_uproot	(doeE, doeObject);

extern	doeObjectFace	doeObjectClass;

#define CAT(l,r)	l ## r
#define XCAT(l,r)	CAT(l,r)
#define BASE_init(env,o)		XCAT(BASE,_init)	(env, (BASE)(o))
#define BASE_copyinit(env, o, src)	XCAT(BASE,_copyinit)	(env, (BASE)(o), (BASE)(src))
#define BASE__cleanup(env, o)		((doeObjectFace*)&XCAT(BASE,Class))->_cleanup	\
								(env, (doeObject)(o))
#define BASE__enumCoObs(env, o, cb)	((doeObjectFace*)&XCAT(BASE,Class))->_enumCoObs	\
								(env, (doeObject)(o), cb)

/*
 *  NOTE: In the description of both a method's intended semantics and
 *  an [extern] function actual semantics we use argument identifiers
 *  as they appear in the respective interface or function
 *  declaration. The exception is the identifier [target], which
 *  implicitly refers to the first argument (always an object).
 *
 *  [doeObject_init]
 *
 *	Initializes the [doeObjectData] part which directly or indirectly
 *	heads the [<class>Data] state pointed to by [target]. The pointer
 *	[face] is set to ZERO.  This underlines an additional responsibility
 *	of all [_init]: they must set [face] to point to the appropriate
 *	class variable. The policy of initializing the base state before the
 *	derived state guarantees that the pointer will be set correctly to the
 *	most derived class variable.
 *
 *  [doeObject_copyinit]
 *
 *	Same as [doeObjectInit], it initializes the [doeObjectData] part
 *	of [target]. Unlike it, it sets the [face] to point to [source]'s
 *	class variable.
 *
 *  [doeObject_uproot]
 *
 *	DOE implements an automatic garbage collection mechanism which
 *	collects inaccessible objects. Collection may happen at any time.
 *	Object accessibility is determined as follows:
 *
 *	1. When created, all objects are "roots".
 *
 *	2. An object ceases to be a root the first time the method
 *	   [uproot] is called on the object.  It is an error to call
 *	   [uproot] more than once on an object.  There is no way for an
 *	   uprooted object to become a root again.
 *
 *	3. A root is considered accessible and thus uncollectable.
 *
 *	4. All coobs enumerated by an accessible object ([enumCoOb])
 *	   are accessible and thus uncollectable.
 *
 *	5. All other objects are considered inaccessible and thus
 *	   collectable.
 *
 *	Notice that uprooting an object does not necessarily make it
 *	collectable: it simply denies it the protection afforded to roots. The
 *	object may still be uncollectable under rule 4.  Clearly, all objects
 *	should become uprooted at some time of their lives (otherwise there
 *	would be leaks).
 *
 *	Not surprisingly, [doeObject_uproot] uproots [target].
 */

#ifdef	__cplusplus
}
#endif

#endif	/* _DOE_OBJECT_P_H */
