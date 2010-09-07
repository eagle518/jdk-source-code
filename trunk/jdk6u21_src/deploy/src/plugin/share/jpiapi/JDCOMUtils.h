/*
 * @(#)JDCOMUtils.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *  JDCOMUtils.h  by X.Lu
 * 
 *
 * Definetion of various data type and macros such as ISupports implementation
 */
/* COM helper functions */
#ifndef _JDCOMUTILS_H_
#define _JDCOMUTILS_H_

#include "JDData.h"
#include "JDPluginData.h"

struct JDID {
	JDUint32 m0;
	JDUint16 m1;
	JDUint16 m2;
	JDUint8	 m3[8];

	inline JDBool Equals(const JDID& other) const {
		return (JDBool)
		((((JDUint32*) &m0)[0] == ((JDUint32*) &other.m0)[0]) &&
		 (((JDUint32*) &m0)[1] == ((JDUint32*) &other.m0)[1]) &&
		 (((JDUint32*) &m0)[2] == ((JDUint32*) &other.m0)[2]) &&
		 (((JDUint32*) &m0)[3] == ((JDUint32*) &other.m0)[3]));
	}
};

typedef JDID		JDIID;
typedef JDID		JDCID;
typedef JDUint32    JDresult;
typedef const JDIID&    JDREFNSIID;
#define JD_DEFINE_STATIC_IID_ACCESSOR(the_iid) \
static const JDIID& GetIID() {static const JDIID iid = the_iid; return iid;}

#define JD_DEFINE_STATIC_CID_ACCESSOR(the_clsid) \
static const JDCID& GetCID() {static const JDCID clsid = the_clsid; return clsid;}

#define JD_DEFINE_IID(name, the_iid) \
const JDIID name = the_iid

#define JD_DEFINE_CID(name, the_clsid) \
const JDCID name = the_clsid

#define JD_BEGIN_MACRO	    do {
#define JD_END_MACRO	    } while(0)

#define JD_INIT_REFCNT()   (mRefCnt = 0)
#define JD_INIT_ISUPPORTS() JD_INIT_REFCNT()

template <class T>
struct JDTypeInfo
{
    static const JDIID& GetIID() { return T::GetIID(); }
};

#define JD_GET_IID(T) JDTypeInfo<T>::GetIID()

/* Put this in your class's constructor:*/
#define JD_DECL_ISUPPORTS						    \
public:									    \
    JD_IMETHOD QueryInterface(const JDIID &uuid, void* *result);	    \
    JD_IMETHOD_(JDREFCNT) AddRef(void);					    \
    JD_IMETHOD_(JDREFCNT) Release(void);				    \
protected:								    \
    JDREFCNT	mRefCnt;						    \
public:

/* Put this in your class's declaration:*/
#define JD_DECL_AGGREGATED                                                   \
    JD_DECL_ISUPPORTS							    \
                                                                            \
public:                                                                     \
                                                                            \
    /* You must implement this operation instead of the nsISupports */      \
    /* methods if you inherit from nsAggregated. */                         \
    JD_IMETHOD								    \
    AggregatedQueryInterface(const JDIID& aIID, void** aInstancePtr);       \
                                                                            \
protected:                                                                  \
                                                                            \
    class Internal : public ISupports {					    \
    public:                                                                 \
                                                                            \
        Internal() {}                                                       \
                                                                            \
        JD_IMETHOD QueryInterface(const JDIID& aIID,			    \
                                        void** aInstancePtr);               \
        JD_IMETHOD_(JDREFCNT) AddRef(void);				    \
        JD_IMETHOD_(JDREFCNT) Release(void);				    \
                                                                            \
    };                                                                      \
                                                                            \
    friend class Internal;                                                  \
                                                                            \
    ISupports*        	fOuter;                                             \
    Internal            fAggregated;                                        \
                                                                            \
    ISupports* GetInner(void) { return &fAggregated; }			    \
                                                                            \
public:                                                                     \

/* Put this in your class's constructor:*/
#define JD_INIT_AGGREGATED(outer)                                           \
    JD_BEGIN_MACRO                                                          \
    JD_INIT_REFCNT();                                                       \
    fOuter = outer ? outer : &fAggregated;                                  \
  JD_END_MACRO

/* Put this in your class's implementation file: */
#define JD_IMPL_AGGREGATED(_class)                                          \
JD_IMETHODIMP                                                               \
_class::QueryInterface(const JDIID& aIID, void** aInstancePtr)              \
{                                                                           \
    return fOuter->QueryInterface(aIID, aInstancePtr);                      \
}                                                                           \
                                                                            \
JD_IMETHODIMP_(JDREFCNT)                                                    \
_class::AddRef(void)                                                        \
{                                                                           \
    return fOuter->AddRef();                                                \
}                                                                           \
                                                                            \
JD_IMETHODIMP_(JDREFCNT)                                                    \
_class::Release(void)                                                       \
{                                                                           \
    return fOuter->Release();                                               \
}                                                                           \
                                                                            \
JD_IMETHODIMP                                                               \
_class::Internal::QueryInterface(const JDIID& aIID, void** aInstancePtr)    \
{                                                                           \
    _class* agg = (_class*)((char*)(this) - offsetof(_class, fAggregated)); \
    return agg->AggregatedQueryInterface(aIID, aInstancePtr);               \
}                                                                           \
                                                                            \
JD_IMETHODIMP_(JDREFCNT)                                                    \
_class::Internal::AddRef(void)                                              \
{                                                                           \
    _class* agg = (_class*)((char*)(this) - offsetof(_class, fAggregated)); \
    ++agg->mRefCnt;                                                         \
    return agg->mRefCnt;                                                    \
}                                                                           \
                                                                            \
JD_IMETHODIMP_(JDREFCNT)                                                    \
_class::Internal::Release(void)                                             \
{                                                                           \
    _class* agg = (_class*)((char*)(this) - offsetof(_class, fAggregated)); \
    --agg->mRefCnt;                                                         \
    if (agg->mRefCnt == 0) {                                                \
        agg->mRefCnt = 1; /* stabilize */                                   \
        delete agg;                                                         \
        return 0;                                                           \
    }                                                                       \
    return agg->mRefCnt;                                                    \
}

#define JD_IMPL_ADDREF(_class)						    \
JD_IMETHODIMP_(JDREFCNT) _class::AddRef(void)				    \
{									    \
  ++mRefCnt;								    \
  return mRefCnt;							    \
}

#define JD_IMPL_RELEASE(_class)						    \
JD_IMETHODIMP_(JDREFCNT) _class::Release(void)				    \
{									    \
  --mRefCnt;								    \
  if (mRefCnt == 0) {							    \
    mRefCnt = 1; /* stabilize */					    \
    delete this;							    \
    return 0;								    \
  }									    \
  return mRefCnt;							    \
}

#define JD_IMPL_QUERY_HEAD(_class)					    \
JD_IMETHODIMP _class::QueryInterface(JDREFNSIID aIID, void** aIJDtancePtr)  \
{									    \
  if ( !aIJDtancePtr )							    \
    return JD_ERROR_NULL_POINTER;					    \
  	ISupports* foundInterface;

#define JD_IMPL_QUERY_BODY(_interface)					    \
  if ( aIID.Equals(JD_GET_IID(_interface)) )				    \
    foundInterface = static_cast<_interface*>(this);			    \
  else

#define JD_IMPL_QUERY_BODY_AMBIGUOUS(_interface, _implClass)		    \
  if ( aIID.Equals(JD_GET_IID(_interface)) )				    \
    foundInterface = static_cast<_interface*>(static_cast<_implClass*>(this)); \
  else

#define JD_IMPL_QUERY_TAIL_GUTS						    \
    foundInterface = 0;							    \
  JDresult status;							    \
  if ( !foundInterface )						    \
    status = JD_NOINTERFACE;						    \
  else									    \
    {									    \
	foundInterface->AddRef();                                           \
	status = JD_OK;                                                     \
    }									    \
  *aIJDtancePtr = foundInterface;					    \
  return status;							    \
}

#define JD_IMPL_QUERY_TAIL_INHERITING(_baseclass)			    \
    foundInterface = 0;							    \
  JDresult status;							    \
  if ( !foundInterface )						    \
    status = _baseclass::QueryInterface(aIID, (void**)&foundInterface);	    \
  else									    \
    {									    \
      foundInterface->AddRef();						    \
      status = JD_OK;							    \
    }									    \
  *aIJDtancePtr = foundInterface;					    \
  return status;							    \
}

#define JD_IMPL_QUERY_TAIL(_supports_interface)				    \
  JD_IMPL_QUERY_BODY_AMBIGUOUS(ISupports, _supports_interface)		    \
  JD_IMPL_QUERY_TAIL_GUTS

#define JD_INTERFACE_MAP_BEGIN(_implClass)                         JD_IMPL_QUERY_HEAD(_implClass)
#define JD_INTERFACE_MAP_ENTRY(_interface)                         JD_IMPL_QUERY_BODY(_interface)
#define JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(_interface, _implClass)   JD_IMPL_QUERY_BODY_AMBIGUOUS(_interface, _implClass)
#define JD_INTERFACE_MAP_END                                       JD_IMPL_QUERY_TAIL_GUTS
#define JD_INTERFACE_MAP_END_INHERITING(_baseClass)                JD_IMPL_QUERY_TAIL_INHERITING(_baseClass)

#define JD_IMPL_QUERY_INTERFACE0(_class)                                      \
  JD_INTERFACE_MAP_BEGIN(_class)                                              \
    JD_INTERFACE_MAP_ENTRY(ISupports)                                         \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE1(_class, _i1)                                  \
  JD_INTERFACE_MAP_BEGIN(_class)                                               \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)			       \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE2(_class, _i1, _i2)                                            \
  JD_INTERFACE_MAP_BEGIN(_class)                                                              \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i2)                                                               \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)                                          \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE3(_class, _i1, _i2, _i3)                                       \
  JD_INTERFACE_MAP_BEGIN(_class)                                                              \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i2)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i3)                                                               \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)                                          \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE4(_class, _i1, _i2, _i3, _i4)                                  \
  JD_INTERFACE_MAP_BEGIN(_class)                                                              \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i2)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i3)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i4)                                                               \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)                                          \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE5(_class, _i1, _i2, _i3, _i4, _i5)                             \
  JD_INTERFACE_MAP_BEGIN(_class)                                                              \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i2)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i3)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i4)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i5)                                                               \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)                                          \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE6(_class, _i1, _i2, _i3, _i4, _i5, _i6)                        \
  JD_INTERFACE_MAP_BEGIN(_class)                                                              \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i2)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i3)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i4)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i5)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i6)                                                               \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)                                          \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)                   \
  JD_INTERFACE_MAP_BEGIN(_class)                                                              \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i2)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i3)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i4)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i5)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i6)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i7)                                                               \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)                                          \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE8(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8)              \
  JD_INTERFACE_MAP_BEGIN(_class)                                                              \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i2)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i3)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i4)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i5)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i6)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i7)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i8)                                                               \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)                                          \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE9(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8, _i9)         \
  JD_INTERFACE_MAP_BEGIN(_class)                                                              \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i2)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i3)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i4)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i5)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i6)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i7)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i8)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i9)                                                               \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)                                          \
  JD_INTERFACE_MAP_END

#define JD_IMPL_QUERY_INTERFACE10(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8, _i9, _i10)  \
  JD_INTERFACE_MAP_BEGIN(_class)                                                              \
    JD_INTERFACE_MAP_ENTRY(_i1)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i2)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i3)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i4)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i5)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i6)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i7)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i8)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i9)                                                               \
    JD_INTERFACE_MAP_ENTRY(_i10)                                                              \
    JD_INTERFACE_MAP_ENTRY_AMBIGUOUS(ISupports, _i1)                                          \
  JD_INTERFACE_MAP_END

#define JD_IMPL_ISUPPORTS(_class,_classiiddef) \
  JD_IMPL_ADDREF(_class)                       \
  JD_IMPL_RELEASE(_class)                      \
  JD_IMPL_QUERY_INTERFACE(_class,_classiiddef)

#define JD_IMPL_ISUPPORTS0(_class)             \
  JD_IMPL_ADDREF(_class)                       \
  JD_IMPL_RELEASE(_class)                      \
  JD_IMPL_QUERY_INTERFACE0(_class)

#define JD_IMPL_ISUPPORTS1(_class, _interface) \
  JD_IMPL_ADDREF(_class)                       \
  JD_IMPL_RELEASE(_class)                      \
  JD_IMPL_QUERY_INTERFACE1(_class, _interface)

#define JD_IMPL_ISUPPORTS2(_class, _i1, _i2)   \
  JD_IMPL_ADDREF(_class)                       \
  JD_IMPL_RELEASE(_class)                      \
  JD_IMPL_QUERY_INTERFACE2(_class, _i1, _i2)

#define JD_IMPL_ISUPPORTS3(_class, _i1, _i2, _i3)   \
  JD_IMPL_ADDREF(_class)                            \
  JD_IMPL_RELEASE(_class)                           \
  JD_IMPL_QUERY_INTERFACE3(_class, _i1, _i2, _i3)

#define JD_IMPL_ISUPPORTS4(_class, _i1, _i2, _i3, _i4)   \
  JD_IMPL_ADDREF(_class)                                 \
  JD_IMPL_RELEASE(_class)                                \
  JD_IMPL_QUERY_INTERFACE4(_class, _i1, _i2, _i3, _i4)

#define JD_IMPL_ISUPPORTS5(_class, _i1, _i2, _i3, _i4, _i5)   \
  JD_IMPL_ADDREF(_class)                                      \
  JD_IMPL_RELEASE(_class)                                     \
  JD_IMPL_QUERY_INTERFACE5(_class, _i1, _i2, _i3, _i4, _i5)

#define JD_IMPL_ISUPPORTS6(_class, _i1, _i2, _i3, _i4, _i5, _i6)   \
  JD_IMPL_ADDREF(_class)                                      \
  JD_IMPL_RELEASE(_class)                                     \
  JD_IMPL_QUERY_INTERFACE6(_class, _i1, _i2, _i3, _i4, _i5, _i6)

#define JD_IMPL_ISUPPORTS7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)   \
  JD_IMPL_ADDREF(_class)                                      \
  JD_IMPL_RELEASE(_class)                                     \
  JD_IMPL_QUERY_INTERFACE7(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7)

#define JD_IMPL_ISUPPORTS8(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8)   \
  JD_IMPL_ADDREF(_class)                                      \
  JD_IMPL_RELEASE(_class)                                     \
  JD_IMPL_QUERY_INTERFACE8(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8)

#define JD_IMPL_ISUPPORTS9(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8, _i9)   \
  JD_IMPL_ADDREF(_class)                                      \
  JD_IMPL_RELEASE(_class)                                     \
  JD_IMPL_QUERY_INTERFACE9(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8, _i9)

#define JD_IMPL_ISUPPORTS10(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8, _i9, _i10)   \
  JD_IMPL_ADDREF(_class)                                      \
  JD_IMPL_RELEASE(_class)                                     \
  JD_IMPL_QUERY_INTERFACE10(_class, _i1, _i2, _i3, _i4, _i5, _i6, _i7, _i8, _i9, _i10)
#endif // _JDCOMUTILS_H_
