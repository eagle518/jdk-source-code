/*
 * @(#)JDSmartPtr.h	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//  JDSmartPtr.h  by X.Lu
//
///=--------------------------------------------------------------------------=
// Utils of Smart COM style Pointer. Simliar to CCOMPtr
//
#ifndef _JDSmartPtr_h__
#define _JDSmartPtr_h__

#include "ISupports.h"

template <class Q>
inline Q* JDSmartPtrAssign(Q** pp, Q* lp);

#ifdef XP_UNIX
typedef long HRESULT;
#define E_POINTER 1
#define S_OK 0
#endif

//Clone of CCOMPtr
template <class T>
class JDSmartPtr
{
    public:
	typedef T _PtrClass;

	JDSmartPtr()
	{
	    p=NULL;
	}

	JDSmartPtr(T* lp)
	{
	    if ((p = lp) != NULL)
	        p->AddRef();
	}

	// Assignment
	JDSmartPtr(const JDSmartPtr<T>& lp)
	{
	    if ((p = lp.p) != NULL)
		p->AddRef();
	}

	~JDSmartPtr()
	{
	    if (p)
		p->Release();
	}

	void Release() {
	    T* pTemp = p;
	    if (pTemp)
	    {
		p = NULL;
		pTemp->Release();
	    }
	}

	operator T*() const
	{
	    return (T*)p;
	}

	T& operator*() const
	{
	    return *p;
	}
	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T** operator&()
	{
	    return &p;
	}

	T* operator=(T* lp)
	{
	    return (T*)JDSmartPtrAssign(&p, lp);
	}

	T* operator=(const JDSmartPtr<T>& lp)
	{
	    return (T*)JDSmartPtrAssign(&p, lp.p);
	}

	bool operator!() const
	{
	    return (p == NULL);
	}

	bool operator<(T* pT) const
	{
	    return p < pT;
	}

	bool operator==(T* pT) const
	{
	    return p == pT;
	}

	T* operator->() const
	{
	    return p;
	}

	void Attach(T* p2)
	{
	    if (p)
		p->Release();

	    p = p2;
	}

	T* Detach()
	{
	    T* pt = p;
	    p = NULL;
	    return pt;
	}

	HRESULT CopyTo(T** ppT)
	{
	    if (ppT == NULL)
		return E_POINTER;

	    *ppT = p;
	    if (p)
		p->AddRef();

	    return S_OK;
	}

	// Compare two objects for equivalence
	bool IsEqualObject(ISupports* pOther)
	{
	    if (p == NULL && pOther == NULL)
		return true; // They are both NULL objects

	    if (p == NULL || pOther == NULL)
		return false; // One is NULL the other is not

	    JDSmartPtr<ISupports> punk1;
	    JDSmartPtr<ISupports> punk2;
	    p->QueryInterface(JD_GET_IID(ISupports), (void**)&punk1);
	    pOther->QueryInterface(JD_GET_IID(ISupports), (void**)&punk2);

	    return punk1 == punk2;
	}

	T* p;
};

template <class Q>
inline Q* JDSmartPtrAssign(Q** pp, Q* lp)
{
    if (lp != NULL)
	lp->AddRef();

    if (*pp)
	(*pp)->Release();

    *pp = lp;
    return lp;
}
#endif //_JDSmartPtr_h__
