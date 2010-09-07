/*
 * @(#)doeObject.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)doeObject.h 3.2 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

#ifndef _DOE_OBJECT_H
#define _DOE_OBJECT_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "dtypes.h"
#include "doe.h"

/*
 * The partial class [doeObject] and several [extern] procedures
 * described below implement basic mechanisms that all objects obey.
 * [doeObject] should be a direct or indirect base of all classes
 * in the system.
 */

typedef struct doeObjectFace_**		doeObject;

typedef struct doeObjectEnumCbData_*	doeObjectEnumCb;
typedef struct doeObjectEnumCbData_ {
    void	(*enumerate)	(doeE env, doeObjectEnumCb, doeObject enumerated);
} doeObjectEnumCbData;

typedef struct doeObjectFace_ {
    ixx		basicStateSize;

    char*	(*className)	(doeE env, doeObject);
    doeObject	(*copy)		(doeE env, doeObject);
    void	(*_cleanup)	(doeE env, doeObject);
    void	(*_enumCoObs)	(doeE env, doeObject, doeObjectEnumCb cb);
    void	(*uproot)	(doeE env, doeObject);
} doeObjectFace;

/*
 *  NOTE: In the descriptions that follow, [<class>] stands for an
 *  object's <most derived> class which we define as follows: an
 *  object's most derived class is [<class>] if the object resulted
 *  from [<class>_create] or from copying (method [copy]) an object
 *  whose most derived class is [<class>].
 *
 *  [basicStateSize]
 *
 *	Must contain [sizeof(<class>Data)].
 *
 *  [className]
 *
 *	Should return a pointer to a string, the name of [<class>].
 *
 *  [copy]
 *
 *	Method [copy] should resemble [<class>_create], that is, it should
 *	allocate basic state space for new object. The difference is that
 *	[copy] creates a "clone" of the target. Consequently, it uses
 *	[<class>_copyinit], rather that [<class>_init]. The result of [copy]
 *	should be a "deep copy". To ensure this, we entrust
 *	[<class>_copyinit] with the responsibility of [copy]ing any coob
 *	directly in [<class>Data]. In a class whose objects cannot change
 *	after creation, [copy] may return the target rather than a copy of it.
 *	The second argument provides a mean to register that an error
 *	has occurred.
 *
 *  [_enumCoObs]
 *
 *	At the core of the object recovery mechanism is the method
 *	[enumCoObs] (the prefix [_] conventionally indicates a "protected"
 *	method, not meant to be called except in the implementation of the
 *  	class). But before we can discuss this method we must turn our
 *	attention to the declarations [_enumCb].
 *
 *	The suffix [Cb] stands for "callback", a mechanism useful in
 *	situations that can be loosely characterized as follows:
 *
 *	1. An action A must be performed on data D under conditions C.
 *	   While there is global agreement as to the type and general
 *	   nature of A, D and C, complete knowledge of them is split
 *	   between two parties, P and Q.
 *
 *	2. P knows exactly which A must be performed. It also knows some
 *	   of the data on which A must operate - we call this portion D1.
 *	   P does not know the rest of the data - D2 - nor whether
 *	   conditions C hold of not.
 *
 *	3. Q knows D2 and C. It does not know D1 nor A.
 *
 *	4. P knows Q and knows that Q knows D2 and C.
 *
 *	5. Q knows nothing of P.
 *
 *	The problem is solved if P can contract with Q the execution of
 *	A(D1,...). That is, directly or indirectly P must supply Q with access
 *	to A and D1; Q will then be in possesion of all the necessary
 *	information to finish the job.
 *
 *	A callback provides such access. It consists of a structure
 *	declaration ([CbData]) and a declaration of a pointer to it ([Cb]).
 *	The structure's only component is a pointer to a function implementing
 *	action A. Access to [CbData] implies access to the data part D1,
 *	which will normally be part of another structure headed by [CbData]
 *	and private to A's implementation. The signature of A always includes
 *	an initial argument of type [Cb]; the rest of the signature - a
 *	[doeObject] in the case of [enumCb] - and the name of the pointer
 *	member - [enumerate] - depend on the especific callback.
 *
 *	NOTE: Callbacks could be objects. Like objects, they encapsulate
 *	data and provide abstraction. Still, we have chosen not to make
 *	them objects. This saves some storage and execution overhead,
 *	Besides, instances of [CbData] are either private to an object's
 *	state or static, so the automatic collection benefits that
 *	objecthood brings would be wasted on them.
 *
 *	Now we can go back to [_enumCoObs]. The method must execute [Cb]'s
 *	callback action for each handle [CoOb] corresponding to one of
 *	[target]'s coobs, i.e.,
 *
 *	cb->enumerate(cb, coob);
 *
 *	The pointer [_enumCoObs] should be zero in classes whose objects
 *	have no coobs.
 *
 *  [_cleanup]
 *
 *	Must point to the class's [_cleanup].
 *
 *  [uproot]
 *
 *	Must point to [doeObject_uproot] (actually, it is possible to call
 *	[doeObject_uproot] directly on an object [o] (as in
 *	[doeObject_uproot(o);]); the method entry is provided for
 *	consistency of usage (so the usual [(*o)->uproot(o);] is still
 *	valid).
 */

#ifdef	__cplusplus
}
#endif

#endif	/* _DOE_OBJECT_H */
