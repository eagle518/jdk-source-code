/*
 * @(#)CNSAdapter_SecureJNIEnv.cpp	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS6Adapter_SecureJNIEnv.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecureJNIEnv.cpp is Implementation of adapter for nsISecureJNI
// 
//
#include "StdAfx.h"
#include "jni.h"
#include "ISecurityContext.h"
#include "nsISecureEnv.h"
#include "CNSAdapter_SecurityContextPeer.h"
#include "nsAgg.h"
#include "CSecureJNIEnv.h"
#include "CNSAdapter_SecureJNIEnv.h"

// Netscape iid
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kISecureEnvIID, NS_ISECUREENV_IID);

////////////////////////////////////////////////////////////////////////////
// from nsISupports and AggregatedQueryInterface:

NS_IMPL_ADDREF(CNSAdapter_SecureJNIEnv);
NS_IMPL_RELEASE(CNSAdapter_SecureJNIEnv);

NS_METHOD
CNSAdapter_SecureJNIEnv::QueryInterface(const nsIID& aIID, void** interfacePtr)
{
    if (aIID.Equals(kISupportsIID) ||
	aIID.Equals(kISecureEnvIID))
    {
	*interfacePtr = (nsISecureEnv*)this;
	AddRef();
	return NS_OK;
    }
    else
	return NS_NOINTERFACE;
}

CNSAdapter_SecureJNIEnv::CNSAdapter_SecureJNIEnv(ISecureEnv* pSecureEnv)
{
    NS_INIT_REFCNT();
    m_pSecureEnv = pSecureEnv;
    
    if (m_pSecureEnv != NULL)
	m_pSecureEnv->AddRef();
}

CNSAdapter_SecureJNIEnv::~CNSAdapter_SecureJNIEnv()
{
    if (m_pSecureEnv)
	m_pSecureEnv->Release();
}

// This private function are here to convert the type between 
// browser and JPI
jd_jni_type
CNSAdapter_SecureJNIEnv::TypeConvert(jni_type type)
{
    jd_jni_type ret_type;

    switch (type)
    {
    case jobject_type:
    {
	ret_type = jd_jobject_type;
	break;
    }
    case jboolean_type:
    {
	ret_type = jd_jboolean_type;
	break;
    }
    case jbyte_type:
    {
	ret_type = jd_jbyte_type;
	break;
    }
    case jchar_type:
    {
	ret_type = jd_jchar_type;
	break;
    }
    case jshort_type:
    {
	ret_type = jd_jshort_type;
	break;
    }
    case jint_type:
    {
	ret_type = jd_jint_type;
	break;
    }
    case jlong_type:
    {
	ret_type = jd_jlong_type;
	break;

    }
    case jfloat_type:
    {
	ret_type = jd_jfloat_type;
	break;
    }
    case jdouble_type:
    {
	ret_type = jd_jdouble_type;
	break;
    }
    case jvoid_type:
    {
	ret_type = jd_jvoid_type;
	break;
    }
    }
    return ret_type;
}

////////////////////////////////////////////////////////////////////////////
// from nsISecureEnv:


/**
 * Create new Java object in LiveConnect.
 *
 * @param clazz      -- Java Class object.
 * @param methodID   -- Method id
 * @param args       -- arguments for invoking the constructor.
 * @param result     -- return new Java object.
 * @param ctx        -- security context
 */
NS_METHOD CNSAdapter_SecureJNIEnv::NewObject(/*[in]*/  jclass clazz,
					      /*[in]*/  jmethodID methodID,
					      /*[in]*/  jvalue *args,
					      /*[out]*/ jobject* result,
					      /*[in]*/  nsISecurityContext* ctx )
{
    // The type of ctx is CSecurityContext
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    JDSmartPtr<ISecurityContext> spSecurityContext = new CNSAdapter_SecurityContextPeer(ctx);
    if (spSecurityContext == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    return m_pSecureEnv->NewObject(clazz, methodID, args, result, spSecurityContext);
}

/**
 * Invoke method on Java object in LiveConnect.
 *
 * @param type       -- Return type
 * @param obj        -- Java object.
 * @param methodID   -- method id
 * @param args       -- arguments for invoking the constructor.
 * @param result     -- return result of invocation.
 * @param ctx        -- security context
 */
NS_METHOD CNSAdapter_SecureJNIEnv::CallMethod( /*[in]*/  jni_type ret_type,
						/*[in]*/  jobject obj,
						/*[in]*/  jmethodID methodID,
						/*[in]*/  jvalue *args,
						/*[out]*/ jvalue* result,
						/*[in]*/  nsISecurityContext* ctx )
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
    JDSmartPtr<ISecurityContext> spSecurityContext = new CNSAdapter_SecurityContextPeer(ctx);

    if (spSecurityContext == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    jd_jni_type jd_ret_type = TypeConvert(ret_type);
    return m_pSecureEnv->CallMethod(jd_ret_type, obj, methodID, args, result, spSecurityContext);
 
}

/**
 * Invoke non-virtual method on Java object in LiveConnect.
 *
 * @param type       -- Return type
 * @param obj        -- Java object.
 * @param clazz      -- Class object
 * @param methodID   -- method id
 * @param args       -- arguments for invoking the constructor.
 * @param ctx        -- security context
 * @param result     -- return result of invocation.
 */
NS_METHOD CNSAdapter_SecureJNIEnv::CallNonvirtualMethod(/*[in]*/  jni_type ret_type,
						  /*[in]*/  jobject obj,
						  /*[in]*/  jclass clazz,
						  /*[in]*/  jmethodID methodID,
						  /*[in]*/  jvalue *args,
						  /*[out]*/ jvalue* result,
						  /*[in]*/  nsISecurityContext* ctx )
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    JDSmartPtr<ISecurityContext> spSecurityContext = new CNSAdapter_SecurityContextPeer(ctx);

     if (spSecurityContext == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    jd_jni_type jd_ret_type = TypeConvert(ret_type);
    return m_pSecureEnv->CallNonvirtualMethod(jd_ret_type, obj, clazz, methodID, 
					      args, result, spSecurityContext);
}

/**
 * Get a field on Java object in LiveConnect.
 *
 * @param type       -- Return type
 * @param obj        -- Java object.
 * @param fieldID    -- field id
 * @param result     -- return field value
 * @param ctx        -- security context
 */
NS_METHOD CNSAdapter_SecureJNIEnv::GetField( /*[in]*/  jni_type field_type,
					      /*[in]*/  jobject obj,
					      /*[in]*/  jfieldID fieldID,
					      /*[out]*/ jvalue* result,
					      /*[in]*/  nsISecurityContext* ctx)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    JDSmartPtr<ISecurityContext> spSecurityContext = new CNSAdapter_SecurityContextPeer(ctx);

    if (spSecurityContext == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    jd_jni_type jd_field_type = TypeConvert(field_type);

    return m_pSecureEnv->GetField(jd_field_type, obj, fieldID, result, spSecurityContext);
}

/**
 * Set a field on Java object in LiveConnect.
 *
 * @param type       -- Return type
 * @param obj        -- Java object.
 * @param fieldID    -- field id
 * @param val        -- field value to set
 * @param ctx        -- security context
 */
NS_METHOD CNSAdapter_SecureJNIEnv::SetField( /*[in]*/ jni_type field_type,
					      /*[in]*/ jobject obj,
					      /*[in]*/ jfieldID fieldID,
					      /*[in]*/ jvalue val,
					      /*[in]*/ nsISecurityContext* ctx )
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
    
    JDSmartPtr<ISecurityContext> spSecurityContext = new CNSAdapter_SecurityContextPeer(ctx);

    if (spSecurityContext == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    jd_jni_type jd_field_type = TypeConvert(field_type);
    return m_pSecureEnv->SetField(jd_field_type, obj, fieldID, val, spSecurityContext);
}

/**
 * Invoke static method on Java object in LiveConnect.
 *
 * @param type       -- Return type
 * @param clazz      -- Class object.
 * @param methodID   -- method id
 * @param args       -- arguments for invoking the constructor.
 * @param result     -- return result of invocation.
 * @param ctx        -- security context
 */
NS_METHOD CNSAdapter_SecureJNIEnv::CallStaticMethod( /*[in]*/  jni_type ret_type,
						      /*[in]*/  jclass clazz,
						      /*[in]*/  jmethodID methodID,
						      /*[in]*/  jvalue *args,
						      /*[out]*/ jvalue* result,
						      /*[in]*/  nsISecurityContext* ctx )
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    JDSmartPtr<ISecurityContext> spSecurityContext = new CNSAdapter_SecurityContextPeer(ctx);

    if (spSecurityContext == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    jd_jni_type jd_ret_type = TypeConvert(ret_type);
    return m_pSecureEnv->CallStaticMethod(jd_ret_type, clazz, methodID, args, result, spSecurityContext);
}

/**
 * Get a static field on Java object in LiveConnect.
 *
 * @param type       -- Return type
 * @param clazz      -- Class object.
 * @param fieldID    -- field id
 * @param result     -- return field value
 * @param ctx        -- security context
 */
NS_METHOD CNSAdapter_SecureJNIEnv::GetStaticField( /*[in]*/  jni_type field_type,
						    /*[in]*/  jclass clazz,
						    /*[in]*/  jfieldID fieldID,
						    /*[out]*/ jvalue* result,
						    /*[in]*/  nsISecurityContext* ctx )
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
    
    JDSmartPtr<ISecurityContext> spSecurityContext = new CNSAdapter_SecurityContextPeer(ctx);

    if (spSecurityContext == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    jd_jni_type jd_field_type = TypeConvert(field_type);
    return m_pSecureEnv->GetStaticField(jd_field_type, clazz, fieldID, result, spSecurityContext);
}


/**
 * Set a static field on Java object in LiveConnect.
 *
 * @param type       -- Return type
 * @param clazz      -- Class object.
 * @param fieldID    -- field id
 * @param val        -- field value to set
 * @param ctx        -- security context
 */
NS_METHOD CNSAdapter_SecureJNIEnv::SetStaticField( /*[in]*/ jni_type field_type,
						    /*[in]*/ jclass clazz,
						    /*[in]*/ jfieldID fieldID,
						    /*[in]*/ jvalue val,
						    /*[in]*/ nsISecurityContext* ctx )
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
    
    JDSmartPtr<ISecurityContext> spSecurityContext = new CNSAdapter_SecurityContextPeer(ctx);

    if (spSecurityContext == NULL)
	return NS_ERROR_OUT_OF_MEMORY;

    jd_jni_type jd_field_type = TypeConvert(field_type);
    return m_pSecureEnv->SetStaticField(jd_field_type, clazz, fieldID, val, spSecurityContext);
}


NS_METHOD CNSAdapter_SecureJNIEnv::GetVersion(/*[out]*/ jint* version)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->GetVersion(version);
}

NS_METHOD CNSAdapter_SecureJNIEnv::DefineClass( /*[in]*/  const char* name,
						 /*[in]*/  jobject loader,
						 /*[in]*/  const jbyte *buf,
						 /*[in]*/  jsize len,
						 /*[out]*/ jclass* clazz)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
    
    return m_pSecureEnv->DefineClass(name, loader, buf, len, clazz);
}

NS_METHOD CNSAdapter_SecureJNIEnv::FindClass(/*[in]*/  const char* name,
				              /*[out]*/ jclass* clazz)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->FindClass(name, clazz);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetSuperclass(/*[in]*/  jclass sub,
					          /*[out]*/ jclass* super)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->GetSuperclass(sub, super);
}

NS_METHOD CNSAdapter_SecureJNIEnv::IsAssignableFrom( /*[in]*/  jclass sub,
						      /*[in]*/  jclass super,
						      /*[out]*/ jboolean* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->IsAssignableFrom(sub, super, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::Throw( /*[in]*/  jthrowable obj,
				           /*[out]*/ jint* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->Throw(obj, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::ThrowNew( /*[in]*/  jclass clazz,
					      /*[in]*/  const char *msg,
					      /*[out]*/ jint* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->ThrowNew(clazz, msg, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::ExceptionOccurred(/*[out]*/ jthrowable* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->ExceptionOccurred(result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::ExceptionDescribe(void)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->ExceptionDescribe();
}

NS_METHOD CNSAdapter_SecureJNIEnv::ExceptionClear(void)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->ExceptionClear();
}

NS_METHOD CNSAdapter_SecureJNIEnv::FatalError(/*[in]*/ const char* msg)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->FatalError(msg);
}

NS_METHOD CNSAdapter_SecureJNIEnv::NewGlobalRef(/*[in]*/  jobject lobj,
                                                 /*[out]*/ jobject* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->NewGlobalRef(lobj, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::DeleteGlobalRef(/*[in]*/ jobject gref)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->DeleteGlobalRef(gref);
}

NS_METHOD CNSAdapter_SecureJNIEnv::DeleteLocalRef(/*[in]*/ jobject obj)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->DeleteLocalRef(obj);
}

NS_METHOD CNSAdapter_SecureJNIEnv::IsSameObject( /*[in]*/  jobject obj1,
						  /*[in]*/  jobject obj2,
						  /*[out]*/ jboolean* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->IsSameObject(obj1, obj2, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::AllocObject( /*[in]*/  jclass clazz,
                                                 /*[out]*/ jobject* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->AllocObject(clazz, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetObjectClass( /*[in]*/  jobject obj,
                                                    /*[out]*/ jclass* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->GetObjectClass(obj, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::IsInstanceOf( /*[in]*/  jobject obj,
						  /*[in]*/  jclass clazz,
						  /*[out]*/ jboolean* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->IsInstanceOf(obj, clazz, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetMethodID( /*[in]*/  jclass clazz,
						 /*[in]*/  const char* name,
						 /*[in]*/  const char* sig,
						 /*[out]*/ jmethodID* id)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetMethodID(clazz, name, sig, id);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetFieldID( /*[in]*/  jclass clazz,
						/*[in]*/  const char* name,
						/*[in]*/  const char* sig,
						/*[out]*/ jfieldID* id)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetFieldID(clazz, name, sig, id);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetStaticMethodID( /*[in]*/  jclass clazz,
						       /*[in]*/  const char* name,
						       /*[in]*/  const char* sig,
						       /*[out]*/ jmethodID* id)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetStaticMethodID(clazz, name, sig, id);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetStaticFieldID( /*[in]*/  jclass clazz,
						      /*[in]*/  const char* name,
						      /*[in]*/  const char* sig,
						      /*[out]*/ jfieldID* id)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetStaticFieldID(clazz, name, sig, id);
}

NS_METHOD CNSAdapter_SecureJNIEnv::NewString( /*[in]*/  const jchar* unicode,
					       /*[in]*/  jsize len,
					       /*[out]*/ jstring* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->NewString(unicode, len, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetStringLength( /*[in]*/  jstring str,
					             /*[out]*/ jsize* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetStringLength(str, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetStringChars( /*[in]*/  jstring str,
						    /*[in]*/  jboolean *isCopy,
						    /*[out]*/ const jchar** result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetStringChars(str, isCopy, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::ReleaseStringChars( /*[in]*/  jstring str,
						        /*[in]*/  const jchar *chars)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->ReleaseStringChars(str, chars);
}

NS_METHOD CNSAdapter_SecureJNIEnv::NewStringUTF( /*[in]*/  const char *utf,
					          /*[out]*/ jstring* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->NewStringUTF(utf, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetStringUTFLength( /*[in]*/  jstring str,
					                /*[out]*/ jsize* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetStringUTFLength(str, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetStringUTFChars( /*[in]*/  jstring str,
						       /*[in]*/  jboolean *isCopy,
						       /*[out]*/ const char** result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetStringUTFChars(str, isCopy, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::ReleaseStringUTFChars( /*[in]*/  jstring str,
						           /*[in]*/  const char *chars)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->ReleaseStringUTFChars(str, chars);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetArrayLength( /*[in]*/  jarray array,
					            /*[out]*/ jsize* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetArrayLength(array, result);
}


NS_METHOD CNSAdapter_SecureJNIEnv::GetObjectArrayElement( /*[in]*/  jobjectArray array,
							   /*[in]*/  jsize index,
							   /*[out]*/ jobject* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->GetObjectArrayElement(array, index, result);
}


NS_METHOD CNSAdapter_SecureJNIEnv::SetObjectArrayElement( /*[in]*/  jobjectArray array,
							   /*[in]*/  jsize index,
							   /*[in]*/  jobject val)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->SetObjectArrayElement(array, index, val);
}

NS_METHOD CNSAdapter_SecureJNIEnv::NewObjectArray( /*[in]*/  jsize len,
    						    /*[in]*/  jclass clazz,
						    /*[in]*/  jobject init,
						    /*[out]*/ jobjectArray* result)

{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->NewObjectArray(len, clazz, init, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::NewArray( /*[in]*/ jni_type element_type,
					     /*[in]*/  jsize len,
					     /*[out]*/ jarray* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    jd_jni_type jd_element_type = TypeConvert(element_type);
	
    return m_pSecureEnv->NewArray(jd_element_type, len, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetArrayElements( /*[in]*/  jni_type element_type,
						      /*[in]*/  jarray array,
						      /*[in]*/  jboolean *isCopy,
						      /*[out]*/ void* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    jd_jni_type jd_element_type = TypeConvert(element_type);
    return m_pSecureEnv->GetArrayElements(jd_element_type, array, isCopy, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::ReleaseArrayElements( /*[in]*/ jni_type element_type,
							  /*[in]*/ jarray array,
							  /*[in]*/ void *elems,
							  /*[in]*/ jint mode)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    jd_jni_type jd_element_type = TypeConvert(element_type);
    return m_pSecureEnv->ReleaseArrayElements(jd_element_type, array, elems, mode);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetArrayRegion( /*[in]*/  jni_type element_type,
						    /*[in]*/  jarray array,
						    /*[in]*/  jsize start,
						    /*[in]*/  jsize len,
						    /*[out]*/ void* buf)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    jd_jni_type jd_element_type = TypeConvert(element_type);
    return m_pSecureEnv->GetArrayRegion(jd_element_type, array, start, len, buf);
}

NS_METHOD CNSAdapter_SecureJNIEnv::SetArrayRegion( /*[in]*/  jni_type element_type,
						    /*[in]*/  jarray array,
						    /*[in]*/  jsize start,
						    /*[in]*/  jsize len,
						    /*[in]*/  void* buf)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	 
    jd_jni_type jd_element_type = TypeConvert(element_type);
    return m_pSecureEnv->SetArrayRegion(jd_element_type, array, start, len, buf);
}

NS_METHOD CNSAdapter_SecureJNIEnv::RegisterNatives( /*[in]*/  jclass clazz,
						     /*[in]*/  const JNINativeMethod *methods,
						     /*[in]*/  jint nMethods,
						     /*[out]*/ jint* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecureEnv->RegisterNatives(clazz, methods, nMethods, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::UnregisterNatives( /*[in]*/  jclass clazz,
					               /*[out]*/ jint* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->UnregisterNatives(clazz, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::MonitorEnter( /*[in]*/  jobject obj,
			                          /*[out]*/ jint* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->MonitorEnter(obj, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::MonitorExit( /*[in]*/  jobject obj,
			                         /*[out]*/ jint* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->MonitorExit(obj, result);
}

NS_METHOD CNSAdapter_SecureJNIEnv::GetJavaVM( /*[in]*/  JavaVM **vm,
		                               /*[out]*/ jint* result)
{
    if (m_pSecureEnv == NULL)
	return NS_ERROR_NULL_POINTER;
	
    return m_pSecureEnv->GetJavaVM(vm, result);
}
