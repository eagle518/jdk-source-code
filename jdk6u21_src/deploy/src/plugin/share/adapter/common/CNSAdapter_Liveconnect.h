/*
 * @(#)CNSAdapter_Liveconnect.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_Liveconnect.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_Liveconnect.h : Declaration of the Adapter for nsILiveconnect (Java -> JS)
//
#ifndef __CNSAdapter_Liveconnect_h_
#define __CNSAdapter_Liveconnect_h_

#include "ILiveconnect.h"

class nsILiveconnect;

// This class is used for Plugin to dispatch liveconnect call
// The object is created by CPluginServiceProviderAdapter
class CNSAdapter_Liveconnect : public ILiveconnect
{
public:
    CNSAdapter_Liveconnect(nsILiveconnect* pLiveconnect);
    virtual ~CNSAdapter_Liveconnect();
    
    //ISUPPORTS
    JD_DECL_ISUPPORTS

    //ILiveconnect
    // Get member of a Native JSObject for a given name.
    JD_IMETHOD
    GetMember(JNIEnv *jEnv, jsobject jsobj, const jchar *name, jsize length, void* principalsArray[], 
	      int numPrincipals, ISupports *securitySupports, jobject *pjobj);
	
    // Get member of a Native JSObject for a given index.
    JD_IMETHOD
    GetSlot(JNIEnv *jEnv, jsobject jsobj, jint slot, void* principalsArray[], 
            int numPrincipals, ISupports *securitySupports, jobject *pjobj);

    // Set member of a Native JSObject for a given name.
    JD_IMETHOD
    SetMember(JNIEnv *jEnv, jsobject jsobj, const jchar* name, jsize length, jobject jobj, void* principalsArray[], 
              int numPrincipals, ISupports *securitySupports);

    // Set member of a Native JSObject for a given index.
    JD_IMETHOD
    SetSlot(JNIEnv *jEnv, jsobject jsobj, jint slot, jobject jobj, void* principalsArray[], 
            int numPrincipals, ISupports *securitySupports);

    // Remove member of a Native JSObject for a given name.
    JD_IMETHOD
    RemoveMember(JNIEnv *jEnv, jsobject jsobj, const jchar* name, jsize length,  void* principalsArray[], 
                 int numPrincipals, ISupports *securitySupports);

    // Call a method of Native JSObject
    JD_IMETHOD
    Call(JNIEnv *jEnv, jsobject jsobj, const jchar* name, jsize length, jobjectArray jobjArr,  void* principalsArray[], 
         int numPrincipals, ISupports *securitySupports, jobject *pjobj);

    // Get the window object for a plugin instance.
    JD_IMETHOD
    GetWindow(JNIEnv *jEnv, void *pJavaObject, void* principalsArray[], 
              int numPrincipals, ISupports *securitySupports, jsobject *pobj);

    // Evaluate a script with a Native JS Object representing scope.
    JD_IMETHOD	
    Eval(JNIEnv *jEnv, jsobject obj, const jchar *script, jsize length, void* principalsArray[], 
         int numPrincipals, ISupports *securitySupports, jobject *pjobj);

    // Garbage collect the JSObject
    JD_IMETHOD
    FinalizeJSObject(JNIEnv *jEnv, jsobject jsobj);
	
    // Convert the JSObject to a string representation
    JD_IMETHOD
    ToString(JNIEnv *jEnv, jsobject obj, jstring *pjstring);

private:
    // The nsILiveconnect object for forwarding the call
    nsILiveconnect* m_pLiveconnect;

    // Convert the ISecurityContext to nsISecurityContext
    JDresult CreateSecurityContext(ISupports* securitySupports, nsISupports* *securityContext);
};

#endif // __CNSAdapter_Liveconnect_h_
