/*
 * @(#)ILiveconnect.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// ILiveconnect.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
// Contains interface for Java calls into Javascript function based on netscape.
// javascript.JSObject interface
//
#ifndef _ILIVECONNECT_H
#define _ILIVECONNECT_H

#include "ISupports.h"
#include "jni.h"

//{86624C71-9C34-11d6-94E4-0010A412869A}
#define ILIVECONNECT_IID	\
    {0x86624C71, 0x9C34, 0x11d6, {0x94, 0xE4, 0x00, 0x10,0xA4, 0x12, 0x86, 0x9A} }

//{86624C72-9C34-11d6-94E4-0010A412869A}
#define CLIVECONNECT_CID \
    {0x86624C72, 0x9C34, 0x11d6, {0x94, 0xE4, 0x00, 0x10, 0xA4, 0x12, 0x86, 0x9A} }

typedef jint jsobject;

class ILiveconnect : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(ILIVECONNECT_IID);
    JD_DEFINE_STATIC_CID_ACCESSOR(CLIVECONNECT_CID);

    /**
     * get member of a Native JSObject for a given name.
     *
     * @param obj        - A Native JS Object.
     * @param name       - Name of a member.
     * @param pjobj      - return parameter as a java object representing 
     *                     the member. If it is a basic data type it is converted to
     *                     a corresponding java type. If it is a NJSObject, then it is
     *                     wrapped up as java wrapper netscape.javascript.JSObject.
     */
    JD_IMETHOD
    GetMember(JNIEnv *jEnv, jsobject jsobj, const jchar *name, jsize length, void* principalsArray[], 
	      int numPrincipals, ISupports *securitySupports, jobject *pjobj) = 0;

    /**
     * get member of a Native JSObject for a given index.
     *
     * @param obj        - A Native JS Object.
     * @param slot      - Index of a member.
     * @param pjobj      - return parameter as a java object representing 
     *                     the member. 
     */
    JD_IMETHOD
    GetSlot(JNIEnv *jEnv, jsobject jsobj, jint slot, void* principalsArray[], 
	    int numPrincipals, ISupports *securitySupports, jobject *pjobj) = 0;

    /**
     * set member of a Native JSObject for a given name.
     *
     * @param obj        - A Native JS Object.
     * @param name       - Name of a member.
     * @param jobj       - Value to set. If this is a basic data type, it is converted
     *                     using standard JNI calls but if it is a wrapper to a JSObject
     *                     then a internal mapping is consulted to convert to a NJSObject.
     */
    JD_IMETHOD
    SetMember(JNIEnv *jEnv, jsobject jsobj, const jchar* name, jsize length, jobject jobj, void* principalsArray[], 
	      int numPrincipals, ISupports *securitySupports) = 0;

    /**
     * set member of a Native JSObject for a given index.
     *
     * @param obj        - A Native JS Object.
     * @param index      - Index of a member.
     * @param jobj       - Value to set. If this is a basic data type, it is converted
     *                     using standard JNI calls but if it is a wrapper to a JSObject
     *                     then a internal mapping is consulted to convert to a NJSObject.
     */
    JD_IMETHOD
    SetSlot(JNIEnv *jEnv, jsobject jsobj, jint slot, jobject jobj, void* principalsArray[], 
	    int numPrincipals, ISupports *securitySupports) = 0;

    /**
     * remove member of a Native JSObject for a given name.
     *
     * @param obj        - A Native JS Object.
     * @param name       - Name of a member.
     */
    JD_IMETHOD
    RemoveMember(JNIEnv *jEnv, jsobject jsobj, const jchar* name, jsize length,  void* principalsArray[], 
		 int numPrincipals, ISupports *securitySupports) = 0;

    /**
     * call a method of Native JSObject. 
     *
     * @param obj        - A Native JS Object.
     * @param name       - Name of a method.
     * @param jobjArr    - Array of jobjects representing parameters of method being caled.
     * @param pjobj      - return value.
     */
    JD_IMETHOD
    Call(JNIEnv *jEnv, jsobject jsobj, const jchar* name, jsize length, jobjectArray jobjArr,  void* principalsArray[], 
	 int numPrincipals, ISupports *securitySupports, jobject *pjobj) = 0;

    /**
     * Evaluate a script with a Native JS Object representing scope.
     *
     * @param obj                - A Native JS Object.
     * @param principalsArray    - Array of principals to be used to compare privileges.
     * @param numPrincipals      - Number of principals being passed.
     * @param script             - Script to be executed.
     * @param pjobj              - return value.
     */
    JD_IMETHOD	
    Eval(JNIEnv *jEnv, jsobject obj, const jchar *script, jsize length, void* principalsArray[], 
	 int numPrincipals, ISupports *securitySupports, jobject *pjobj) = 0;

    /**
     * Get the window object for a plugin instance.
     *
     * @param pJavaObject        - Either a jobject or a pointer to a plugin instance 
     *                             representing the java object.
     * @param pjobj              - return value. This is a native js object 
     *                             representing the window object of a frame 
     *                             in which a applet/bean resides.
     */
    JD_IMETHOD
    GetWindow(JNIEnv *jEnv, void *pJavaObject, void* principalsArray[], 
	      int numPrincipals, ISupports *securitySupports, jsobject *pobj) = 0;

    /**
     * Get the window object for a plugin instance.
     *
     * @param jEnv       - JNIEnv on which the call is being made.
     * @param obj        - A Native JS Object.
     */
    JD_IMETHOD
    FinalizeJSObject(JNIEnv *jEnv, jsobject jsobj) = 0;

    /**
     * Get the window object for a plugin instance.
     *
     * @param jEnv       - JNIEnv on which the call is being made.
     * @param obj        - A Native JS Object.
     * @param jstring    - Return value as a jstring representing a JS object.
     */
    JD_IMETHOD
    ToString(JNIEnv *jEnv, jsobject obj, jstring *pjstring) = 0;
};

#endif // ILiveconnect_h___
