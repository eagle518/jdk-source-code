/*
 * @(#)p11_general.c	1.6 10/03/23
 *
 * Copyright (c) 2003, 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * ===========================================================================
 * Copyright  (c) 2002 Graz University of Technology. All rights reserved.
 *
 * Redistribution and use in  source and binary forms, with or without 
 * modification, are permitted  provided that the following conditions are met:
 *
 * 1. Redistributions of  source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in  binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  
 * 3. The end-user documentation included with the redistribution, if any, must
 *    include the following acknowledgment:
 * 
 *    "This product includes software developed by IAIK of Graz University of
 *     Technology."
 * 
 *    Alternately, this acknowledgment may appear in the software itself, if 
 *    and wherever such third-party acknowledgments normally appear.
 *  
 * 4. The names "Graz University of Technology" and "IAIK of Graz University of
 *    Technology" must not be used to endorse or promote products derived from 
 *    this software without prior written permission.
 *  
 * 5. Products derived from this software may not be called 
 *    "IAIK PKCS Wrapper", nor may "IAIK" appear in their name, without prior 
 *    written permission of Graz University of Technology.
 *  
 *  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 *  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 *  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY  OF SUCH DAMAGE.
 * ===========================================================================
 */

#include "pkcs11wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sun_security_pkcs11_wrapper_PKCS11.h"

/* declare file private functions */

void prefetchFields(JNIEnv *env, jclass thisClass);
jobject ckInfoPtrToJInfo(JNIEnv *env, const CK_INFO_PTR ckpInfo);
jobject ckSlotInfoPtrToJSlotInfo(JNIEnv *env, const CK_SLOT_INFO_PTR ckpSlotInfo);
jobject ckTokenInfoPtrToJTokenInfo(JNIEnv *env, const CK_TOKEN_INFO_PTR ckpTokenInfo);
jobject ckMechanismInfoPtrToJMechanismInfo(JNIEnv *env, const CK_MECHANISM_INFO_PTR ckpMechanismInfo);

/* define variables */

jfieldID pNativeDataID;
jfieldID mech_mechanismID;
jfieldID mech_pParameterID;

jclass jByteArrayClass;
jclass jLongClass;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    return JNI_VERSION_1_4;
}

/* ************************************************************************** */
/* The native implementation of the methods of the PKCS11Implementation class */
/* ************************************************************************** */

/*
 * This method is used to do static initialization. This method is static and
 * synchronized. Summary: use this method like a static initialization block.
 *
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    initializeLibrary
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_initializeLibrary
    (JNIEnv *env, jclass thisClass)
{
#ifndef NO_CALLBACKS
    if (notifyListLock == NULL) {
        notifyListLock = createLockObject(env);
    }
#endif

    prefetchFields(env, thisClass);
}

jclass fetchClass(JNIEnv *env, const char *name) {
    jclass tmpClass = (*env)->FindClass(env, name);
    if (tmpClass == NULL) { return NULL; }
    return (*env)->NewGlobalRef(env, tmpClass);
}

void prefetchFields(JNIEnv *env, jclass thisClass) {
    jclass tmpClass;

    /* PKCS11 */
    pNativeDataID = (*env)->GetFieldID(env, thisClass, "pNativeData", "J");
    if (pNativeDataID == NULL) { return; }

    /* CK_MECHANISM */
    tmpClass = (*env)->FindClass(env, CLASS_MECHANISM);
    if (tmpClass == NULL) { return; }
    mech_mechanismID = (*env)->GetFieldID(env, tmpClass, "mechanism", "J");
    if (mech_mechanismID == NULL) { return; }
    mech_pParameterID = (*env)->GetFieldID(env, tmpClass, "pParameter", "Ljava/lang/Object;");
    if (mech_pParameterID == NULL) { return; }
    jByteArrayClass = fetchClass(env, "[B");
    if (jByteArrayClass == NULL) { return; }
    jLongClass = fetchClass(env, "java/lang/Long");
}

/* This method is designed to do a clean-up. It releases all global resources
 * of this library. By now, this function is not called. Calling from
 * JNI_OnUnload would be an option, but some VMs do not support JNI_OnUnload.
 *
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    finalizeLibrary
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_finalizeLibrary
    (JNIEnv *env, jclass thisClass)
{
/* XXX
    * remove all left lists and release the resources and the lock 
     * objects that synchroniz access to these lists.
     *
    removeAllModuleEntries(env);
    if (moduleListHead == NULL) { * check, if we removed the last active module *
	* remove also the moduleListLock, it is no longer used *
	if (moduleListLock != NULL) {
	    destroyLockObject(env, moduleListLock);
	    moduleListLock = NULL;
	}
#ifndef NO_CALLBACKS
	* remove all left notify callback entries *
	while (removeFirstNotifyEntry(env));
	* remove also the notifyListLock, it is no longer used *
	if (notifyListLock != NULL) {
	    destroyLockObject(env, notifyListLock);
	    notifyListLock = NULL;
	}
	if (jInitArgsObject != NULL) {
	    (*env)->DeleteGlobalRef(env, jInitArgsObject);
	}
	if (ckpGlobalInitArgs != NULL_PTR) {
	    free(ckpGlobalInitArgs);
	}
#endif * NO_CALLBACKS *
    }
*/
}

#ifdef P11_ENABLE_C_INITIALIZE
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_Initialize
 * Signature: (Ljava/lang/Object;)V
 * Parametermapping:                    *PKCS11*
 * @param   jobject jInitArgs           CK_VOID_PTR pInitArgs
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1Initialize
    (JNIEnv *env, jobject obj, jobject jInitArgs)
{
    /*
     * Initalize Cryptoki
     */
    CK_C_INITIALIZE_ARGS_PTR ckpInitArgs;
    CK_RV rv;
    CK_FUNCTION_LIST_PTR ckpFunctions;

    TRACE0("DEBUG: initializing module... ");

    ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) {
        TRACE0("failed getting module entry");
        return;
    }

    ckpInitArgs = ((jInitArgs != NULL)?
        makeCKInitArgsAdapter(env, jInitArgs) : NULL_PTR);

    rv = (*ckpFunctions->C_Initialize)(ckpInitArgs);

    free(ckpInitArgs);

    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return; }

    TRACE0("FINISHED\n");
}
#endif

#ifdef P11_ENABLE_C_FINALIZE
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_Finalize
 * Signature: (Ljava/lang/Object;)V
 * Parametermapping:                    *PKCS11*
 * @param   jobject jReserved           CK_VOID_PTR pReserved
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1Finalize
    (JNIEnv *env, jobject obj, jobject jReserved)
{
    /*
     * Finalize Cryptoki
     */
    CK_VOID_PTR ckpReserved;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return; }

    ckpReserved = jObjectToCKVoidPtr(jReserved);

    rv = (*ckpFunctions->C_Finalize)(ckpReserved);

    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return; }
}
#endif

#ifdef P11_ENABLE_C_GETINFO
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_GetInfo
 * Signature: ()Lsun/security/pkcs11/wrapper/CK_INFO;
 * Parametermapping:                    *PKCS11*
 * @return  jobject jInfoObject         CK_INFO_PTR pInfo
 */
JNIEXPORT jobject JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1GetInfo
    (JNIEnv *env, jobject obj)
{
    CK_INFO ckLibInfo;
    jobject jInfoObject = NULL;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return NULL; }

    rv = (*ckpFunctions->C_GetInfo)(&ckLibInfo);

    if (ckAssertReturnValueOK(env, rv) == CK_ASSERT_OK) {
        jInfoObject = ckInfoPtrToJInfo(env, &ckLibInfo);
    }
    return jInfoObject ;
}

/*
 * converts a pointer to a CK_INFO structure into a Java CK_INFO Object.
 *
 * @param env - used to call JNI funktions to create the new Java object
 * @param ckpInfo - the pointer to the CK_INFO structure
 * @return - the new Java CK_INFO object
 */
jobject ckInfoPtrToJInfo(JNIEnv *env, const CK_INFO_PTR ckpInfo)
{
    jclass jInfoClass;
    jobject jInfoObject;
    jcharArray jTempCharArray;
    jfieldID jFieldID;
    jobject jTempVersion;

    /* load CK_INFO class */
    jInfoClass = (*env)->FindClass(env, CLASS_INFO);
    if (jInfoClass == NULL) { return NULL; };

    /* create new CK_INFO object */
    jInfoObject = (*env)->AllocObject(env, jInfoClass);
    if (jInfoObject == NULL) { return NULL; }

    /* set cryptokiVersion */
    jFieldID = (*env)->GetFieldID(env, jInfoClass, "cryptokiVersion", "Lsun/security/pkcs11/wrapper/CK_VERSION;");
    if (jFieldID == NULL) { return NULL; }
    jTempVersion = ckVersionPtrToJVersion(env, &(ckpInfo->cryptokiVersion));
    if (jTempVersion == NULL) { return NULL; }
    (*env)->SetObjectField(env, jInfoObject, jFieldID, jTempVersion);

    /* set manufacturerID */
    jFieldID = (*env)->GetFieldID(env, jInfoClass, "manufacturerID", "[C");
    if (jFieldID == NULL) { return NULL; }
    jTempCharArray = ckUTF8CharArrayToJCharArray(env, &(ckpInfo->manufacturerID[0]), 32);
    if (jTempCharArray == NULL) { return NULL; }
    (*env)->SetObjectField(env, jInfoObject, jFieldID, jTempCharArray);

    /* set flags */
    jFieldID = (*env)->GetFieldID(env, jInfoClass, "flags", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jInfoObject, jFieldID, ckULongToJLong(ckpInfo->flags));

    /* set libraryDescription */
    jFieldID = (*env)->GetFieldID(env, jInfoClass, "libraryDescription", "[C");
    if (jFieldID == NULL) { return NULL; }
    jTempCharArray = ckUTF8CharArrayToJCharArray(env, &(ckpInfo->libraryDescription[0]) ,32);
    if (jTempCharArray == NULL) { return NULL; }
    (*env)->SetObjectField(env, jInfoObject, jFieldID, jTempCharArray);

    /* set libraryVersion */
    jFieldID = (*env)->GetFieldID(env, jInfoClass, "libraryVersion", "Lsun/security/pkcs11/wrapper/CK_VERSION;");
    if (jFieldID == NULL) { return NULL; }
    jTempVersion = ckVersionPtrToJVersion(env, &(ckpInfo->libraryVersion));
    if (jTempVersion == NULL) { return NULL; }
    (*env)->SetObjectField(env, jInfoObject, jFieldID, jTempVersion);

    return jInfoObject ;
}
#endif

#ifdef P11_ENABLE_C_GETSLOTLIST
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_GetSlotList
 * Signature: (Z)[J
 * Parametermapping:                    *PKCS11*
 * @param   jboolean jTokenPresent      CK_BBOOL tokenPresent
 * @return  jlongArray jSlotList        CK_SLOT_ID_PTR pSlotList
 *                                      CK_ULONG_PTR pulCount
 */
JNIEXPORT jlongArray JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1GetSlotList
    (JNIEnv *env, jobject obj, jboolean jTokenPresent)
{
    CK_ULONG ckTokenNumber;
    CK_SLOT_ID_PTR ckpSlotList;
    CK_BBOOL ckTokenPresent;
    jlongArray jSlotList = NULL;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return NULL; }

    ckTokenPresent = jBooleanToCKBBool(jTokenPresent);

    rv = (*ckpFunctions->C_GetSlotList)(ckTokenPresent, NULL_PTR, &ckTokenNumber);
    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return NULL ; }

    ckpSlotList = (CK_SLOT_ID_PTR) malloc(ckTokenNumber * sizeof(CK_SLOT_ID));
    if (ckpSlotList == NULL) {
        JNU_ThrowOutOfMemoryError(env, 0);
        return 0;
    }

    rv = (*ckpFunctions->C_GetSlotList)(ckTokenPresent, ckpSlotList, &ckTokenNumber);
    if (ckAssertReturnValueOK(env, rv) == CK_ASSERT_OK) {
        jSlotList = ckULongArrayToJLongArray(env, ckpSlotList, ckTokenNumber);
    }
    free(ckpSlotList);

    return jSlotList ;
}
#endif

#ifdef P11_ENABLE_C_GETSLOTINFO
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_GetSlotInfo
 * Signature: (J)Lsun/security/pkcs11/wrapper/CK_SLOT_INFO;
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSlotID               CK_SLOT_ID slotID
 * @return  jobject jSlotInfoObject     CK_SLOT_INFO_PTR pInfo
 */
JNIEXPORT jobject JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1GetSlotInfo
    (JNIEnv *env, jobject obj, jlong jSlotID)
{
    CK_SLOT_ID ckSlotID;
    CK_SLOT_INFO ckSlotInfo;
    jobject jSlotInfoObject = NULL;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return NULL; }

    ckSlotID = jLongToCKULong(jSlotID);

    rv = (*ckpFunctions->C_GetSlotInfo)(ckSlotID, &ckSlotInfo);
    if (ckAssertReturnValueOK(env, rv) == CK_ASSERT_OK) {
        jSlotInfoObject = ckSlotInfoPtrToJSlotInfo(env, &ckSlotInfo);
    }
    return jSlotInfoObject ;
}

/*
 * converts a pointer to a CK_SLOT_INFO structure into a Java CK_SLOT_INFO Object.
 *
 * @param env - used to call JNI funktions to create the new Java object
 * @param ckpSlotInfo - the pointer to the CK_SLOT_INFO structure
 * @return - the new Java CK_SLOT_INFO object
 */
jobject ckSlotInfoPtrToJSlotInfo(JNIEnv *env, const CK_SLOT_INFO_PTR ckpSlotInfo)
{
    jclass jSlotInfoClass;
    jobject jSlotInfoObject;
    jcharArray jTempCharArray;
    jfieldID jFieldID;
    jobject jTempVersion;

    /* load CK_SLOT_INFO class */
    jSlotInfoClass = (*env)->FindClass(env, CLASS_SLOT_INFO);
    if (jSlotInfoClass == NULL) { return NULL; };
    /* create new CK_SLOT_INFO object */
    jSlotInfoObject = (*env)->AllocObject(env, jSlotInfoClass);
    if (jSlotInfoObject == NULL) { return NULL; }

    /* set slotDescription */
    jFieldID = (*env)->GetFieldID(env, jSlotInfoClass, "slotDescription", "[C");
    if (jFieldID == NULL) { return NULL; }
    jTempCharArray = ckUTF8CharArrayToJCharArray(env, &(ckpSlotInfo->slotDescription[0]), 64);
    if (jTempCharArray == NULL) { return NULL; }
    (*env)->SetObjectField(env, jSlotInfoObject, jFieldID, jTempCharArray);

    /* set manufacturerID */
    jFieldID = (*env)->GetFieldID(env, jSlotInfoClass, "manufacturerID", "[C");
    if (jFieldID == NULL) { return NULL; }
    jTempCharArray = ckUTF8CharArrayToJCharArray(env, &(ckpSlotInfo->manufacturerID[0]), 32);
    if (jTempCharArray == NULL) { return NULL; }
    (*env)->SetObjectField(env, jSlotInfoObject, jFieldID, jTempCharArray);

    /* set flags */
    jFieldID = (*env)->GetFieldID(env, jSlotInfoClass, "flags", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jSlotInfoObject, jFieldID, ckULongToJLong(ckpSlotInfo->flags));

    /* set hardwareVersion */
    jFieldID = (*env)->GetFieldID(env, jSlotInfoClass, "hardwareVersion", "Lsun/security/pkcs11/wrapper/CK_VERSION;");
    if (jFieldID == NULL) { return NULL; }
    jTempVersion = ckVersionPtrToJVersion(env, &(ckpSlotInfo->hardwareVersion));
    if (jTempVersion == NULL) { return NULL; }
    (*env)->SetObjectField(env, jSlotInfoObject, jFieldID, jTempVersion);

    /* set firmwareVersion */
    jFieldID = (*env)->GetFieldID(env, jSlotInfoClass, "firmwareVersion", "Lsun/security/pkcs11/wrapper/CK_VERSION;");
    if (jFieldID == NULL) { return NULL; }
    jTempVersion = ckVersionPtrToJVersion(env, &(ckpSlotInfo->firmwareVersion));
    if (jTempVersion == NULL) { return NULL; }
    (*env)->SetObjectField(env, jSlotInfoObject, jFieldID, jTempVersion);

    return jSlotInfoObject ;
}
#endif

#ifdef P11_ENABLE_C_GETTOKENINFO
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_GetTokenInfo
 * Signature: (J)Lsun/security/pkcs11/wrapper/CK_TOKEN_INFO;
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSlotID               CK_SLOT_ID slotID
 * @return  jobject jInfoTokenObject    CK_TOKEN_INFO_PTR pInfo
 */
JNIEXPORT jobject JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1GetTokenInfo
    (JNIEnv *env, jobject obj, jlong jSlotID)
{
    CK_SLOT_ID ckSlotID;
    CK_TOKEN_INFO ckTokenInfo;
    jobject jInfoTokenObject = NULL;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return NULL; }

    ckSlotID = jLongToCKULong(jSlotID);

    rv = (*ckpFunctions->C_GetTokenInfo)(ckSlotID, &ckTokenInfo);
    if (ckAssertReturnValueOK(env, rv) == CK_ASSERT_OK) {
        jInfoTokenObject = ckTokenInfoPtrToJTokenInfo(env, &ckTokenInfo);
    }
    return jInfoTokenObject ;
}

/*
 * converts a pointer to a CK_TOKEN_INFO structure into a Java CK_TOKEN_INFO Object.
 *
 * @param env - used to call JNI funktions to create the new Java object
 * @param ckpTokenInfo - the pointer to the CK_TOKEN_INFO structure
 * @return - the new Java CK_TOKEN_INFO object
 */
jobject ckTokenInfoPtrToJTokenInfo(JNIEnv *env, const CK_TOKEN_INFO_PTR ckpTokenInfo)
{
    jclass jTokenInfoClass;
    jobject jTokenInfoObject = NULL;
    jcharArray jTempCharArray;
    jfieldID jFieldID;
    jobject jTempVersion;

    /* load CK_SLOT_INFO class */
    jTokenInfoClass = (*env)->FindClass(env, CLASS_TOKEN_INFO);
    if (jTokenInfoClass == NULL)  { return NULL; };
    /* create new CK_SLOT_INFO object */
    jTokenInfoObject = (*env)->AllocObject(env, jTokenInfoClass);
    if (jTokenInfoObject  == NULL)  { return NULL; };

    /* set label */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "label", "[C");
    if (jFieldID == NULL) { return NULL; }
    jTempCharArray = ckUTF8CharArrayToJCharArray(env, &(ckpTokenInfo->label[0]), 32);
    if (jTempCharArray == NULL) { return NULL; }
    (*env)->SetObjectField(env, jTokenInfoObject, jFieldID, jTempCharArray);

    /* set manufacturerID */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "manufacturerID", "[C");
    if (jFieldID == NULL) { return NULL; }
    jTempCharArray = ckUTF8CharArrayToJCharArray(env, &(ckpTokenInfo->manufacturerID[0]), 32);
    if (jTempCharArray == NULL) { return NULL; }
    (*env)->SetObjectField(env, jTokenInfoObject, jFieldID, jTempCharArray);

    /* set model */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "model", "[C");
    if (jFieldID == NULL) { return NULL; }
    jTempCharArray = ckUTF8CharArrayToJCharArray(env, &(ckpTokenInfo->model[0]), 16);
    if (jTempCharArray == NULL) { return NULL; }
    (*env)->SetObjectField(env, jTokenInfoObject, jFieldID, jTempCharArray);

    /* set serialNumber */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "serialNumber", "[C");
    if (jFieldID == NULL) { return NULL; }
    jTempCharArray = ckUTF8CharArrayToJCharArray(env, &(ckpTokenInfo->serialNumber[0]), 16);
    if (jTempCharArray == NULL) { return NULL; }
    (*env)->SetObjectField(env, jTokenInfoObject, jFieldID, jTempCharArray);

    /* set flags */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "flags", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongToJLong(ckpTokenInfo->flags));

    /* set ulMaxSessionCount */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulMaxSessionCount", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongSpecialToJLong(ckpTokenInfo->ulMaxSessionCount));

    /* set ulSessionCount */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulSessionCount", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongSpecialToJLong(ckpTokenInfo->ulSessionCount));

    /* set ulMaxRwSessionCount */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulMaxRwSessionCount", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongSpecialToJLong(ckpTokenInfo->ulMaxRwSessionCount));

    /* set ulRwSessionCount */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulRwSessionCount", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongSpecialToJLong(ckpTokenInfo->ulRwSessionCount));

    /* set ulMaxPinLen */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulMaxPinLen", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongToJLong(ckpTokenInfo->ulMaxPinLen));

    /* set ulMinPinLen */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulMinPinLen", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongToJLong(ckpTokenInfo->ulMinPinLen));

    /* set ulTotalPublicMemory */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulTotalPublicMemory", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongSpecialToJLong(ckpTokenInfo->ulTotalPublicMemory));

    /* set ulFreePublicMemory */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulFreePublicMemory", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongSpecialToJLong(ckpTokenInfo->ulFreePublicMemory));

    /* set ulTotalPrivateMemory */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulTotalPrivateMemory", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongSpecialToJLong(ckpTokenInfo->ulTotalPrivateMemory));

    /* set ulFreePrivateMemory */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "ulFreePrivateMemory", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jTokenInfoObject, jFieldID, ckULongSpecialToJLong(ckpTokenInfo->ulFreePrivateMemory));


    /* set hardwareVersion */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "hardwareVersion", "Lsun/security/pkcs11/wrapper/CK_VERSION;");
    if (jFieldID == NULL) { return NULL; }
    jTempVersion = ckVersionPtrToJVersion(env, &(ckpTokenInfo->hardwareVersion));
    if (jTempVersion == NULL) { return NULL; }
    (*env)->SetObjectField(env, jTokenInfoObject, jFieldID, jTempVersion);

    /* set firmwareVersion */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "firmwareVersion", "Lsun/security/pkcs11/wrapper/CK_VERSION;");
    if (jFieldID == NULL) { return NULL; }
    jTempVersion = ckVersionPtrToJVersion(env, &(ckpTokenInfo->firmwareVersion));
    if (jTempVersion == NULL) { return NULL; }
    (*env)->SetObjectField(env, jTokenInfoObject, jFieldID, jTempVersion);

    /* set utcTime */
    jFieldID = (*env)->GetFieldID(env, jTokenInfoClass, "utcTime", "[C");
    if (jFieldID == NULL) { return NULL; }
    jTempCharArray = ckUTF8CharArrayToJCharArray(env, &(ckpTokenInfo->utcTime[0]), 16);
    if (jTempCharArray == NULL) { return NULL; }
    (*env)->SetObjectField(env, jTokenInfoObject, jFieldID, jTempCharArray);

    return jTokenInfoObject ;
}
#endif

#ifdef P11_ENABLE_C_WAITFORSLOTEVENT
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_WaitForSlotEvent
 * Signature: (JLjava/lang/Object;)J
 * Parametermapping:                    *PKCS11*
 * @param   jlong jFlags                CK_FLAGS flags
 * @param   jobject jReserved           CK_VOID_PTR pReserved
 * @return  jlong jSlotID               CK_SLOT_ID_PTR pSlot
 */
JNIEXPORT jlong JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1WaitForSlotEvent
    (JNIEnv *env, jobject obj, jlong jFlags, jobject jReserved)
{
    CK_FLAGS ckFlags;
    CK_SLOT_ID ckSlotID;
    jlong jSlotID;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return 0L; }

    ckFlags = jLongToCKULong(jFlags);

    rv = (*ckpFunctions->C_WaitForSlotEvent)(ckFlags, &ckSlotID, NULL_PTR);
    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return 0L; }

    jSlotID = ckULongToJLong(ckSlotID);

    return jSlotID ;
}
#endif

#ifdef P11_ENABLE_C_GETMECHANISMLIST
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_GetMechanismList
 * Signature: (J)[J
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSlotID               CK_SLOT_ID slotID
 * @return  jlongArray jMechanismList   CK_MECHANISM_TYPE_PTR pMechanismList
 *                                      CK_ULONG_PTR pulCount
 */
JNIEXPORT jlongArray JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1GetMechanismList
    (JNIEnv *env, jobject obj, jlong jSlotID)
{
    CK_SLOT_ID ckSlotID;
    CK_ULONG ckMechanismNumber;
    CK_MECHANISM_TYPE_PTR ckpMechanismList;
    jlongArray jMechanismList = NULL;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return NULL; }

    ckSlotID = jLongToCKULong(jSlotID);

    rv = (*ckpFunctions->C_GetMechanismList)(ckSlotID, NULL_PTR, &ckMechanismNumber);
    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return NULL ; }

    ckpMechanismList = (CK_MECHANISM_TYPE_PTR) malloc(ckMechanismNumber * sizeof(CK_MECHANISM_TYPE));
    if (ckpMechanismList == NULL) {
        JNU_ThrowOutOfMemoryError(env, 0);
        return NULL;
    }

    rv = (*ckpFunctions->C_GetMechanismList)(ckSlotID, ckpMechanismList, &ckMechanismNumber);
    if (ckAssertReturnValueOK(env, rv) == CK_ASSERT_OK) {
        jMechanismList = ckULongArrayToJLongArray(env, ckpMechanismList, ckMechanismNumber);
    }
    free(ckpMechanismList);

    return jMechanismList ;
}
#endif

#ifdef P11_ENABLE_C_GETMECHANISMINFO
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_GetMechanismInfo
 * Signature: (JJ)Lsun/security/pkcs11/wrapper/CK_MECHANISM_INFO;
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSlotID               CK_SLOT_ID slotID
 * @param   jlong jType                 CK_MECHANISM_TYPE type
 * @return  jobject jMechanismInfo      CK_MECHANISM_INFO_PTR pInfo
 */
JNIEXPORT jobject JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1GetMechanismInfo
    (JNIEnv *env, jobject obj, jlong jSlotID, jlong jType)
{
    CK_SLOT_ID ckSlotID;
    CK_MECHANISM_TYPE ckMechanismType;
    CK_MECHANISM_INFO ckMechanismInfo;
    jobject jMechanismInfo = NULL;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return NULL; }

    ckSlotID = jLongToCKULong(jSlotID);
    ckMechanismType = jLongToCKULong(jType);

    rv = (*ckpFunctions->C_GetMechanismInfo)(ckSlotID, ckMechanismType, &ckMechanismInfo);
    if (ckAssertReturnValueOK(env, rv) == CK_ASSERT_OK) {
        jMechanismInfo = ckMechanismInfoPtrToJMechanismInfo(env, &ckMechanismInfo);
    }
    return jMechanismInfo ;
}

/*
 * converts a pointer to a CK_MECHANISM_INFO structure into a Java CK_MECHANISM_INFO Object.
 *
 * @param env - used to call JNI funktions to create the new Java object
 * @param ckpMechanismInfo - the pointer to the CK_MECHANISM_INFO structure
 * @return - the new Java CK_MECHANISM_INFO object
 */
jobject ckMechanismInfoPtrToJMechanismInfo(JNIEnv *env, const CK_MECHANISM_INFO_PTR ckpMechanismInfo)
{
    jclass jMechanismInfoClass;
    jobject jMechanismInfoObject = NULL;
    jfieldID jFieldID;

    /* load CK_MECHANISM_INFO class */
    jMechanismInfoClass = (*env)->FindClass(env, CLASS_MECHANISM_INFO);
    if (jMechanismInfoClass == NULL) { return NULL; };
    /* create new CK_MECHANISM_INFO object */
    jMechanismInfoObject = (*env)->AllocObject(env, jMechanismInfoClass);
    if (jMechanismInfoObject == NULL) { return NULL; }

    /* set ulMinKeySize */
    jFieldID = (*env)->GetFieldID(env, jMechanismInfoClass, "ulMinKeySize", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jMechanismInfoObject, jFieldID, ckULongToJLong(ckpMechanismInfo->ulMinKeySize));

    /* set ulMaxKeySize */
    jFieldID = (*env)->GetFieldID(env, jMechanismInfoClass, "ulMaxKeySize", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jMechanismInfoObject, jFieldID, ckULongToJLong(ckpMechanismInfo->ulMaxKeySize));

    /* set flags */
    jFieldID = (*env)->GetFieldID(env, jMechanismInfoClass, "flags", "J");
    if (jFieldID == NULL) { return NULL; }
    (*env)->SetLongField(env, jMechanismInfoObject, jFieldID, ckULongToJLong(ckpMechanismInfo->flags));

    return jMechanismInfoObject ;
}
#endif

#ifdef P11_ENABLE_C_INITTOKEN
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_InitToken
 * Signature: (J[C[C)V
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSlotID               CK_SLOT_ID slotID
 * @param   jcharArray jPin             CK_CHAR_PTR pPin
 *                                      CK_ULONG ulPinLen
 * @param   jcharArray jLabel           CK_UTF8CHAR_PTR pLabel
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1InitToken
    (JNIEnv *env, jobject obj, jlong jSlotID, jcharArray jPin, jcharArray jLabel)
{
    CK_SLOT_ID ckSlotID;
    CK_CHAR_PTR ckpPin = NULL_PTR;
    CK_UTF8CHAR_PTR ckpLabel = NULL_PTR;
    CK_ULONG ckPinLength;
    CK_ULONG ckLabelLength;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return; }

    ckSlotID = jLongToCKULong(jSlotID);
    jCharArrayToCKCharArray(env, jPin, &ckpPin, &ckPinLength);
    if ((*env)->ExceptionCheck(env)) { return; }

    /* ckLabelLength <= 32 !!! */
    jCharArrayToCKUTF8CharArray(env, jLabel, &ckpLabel, &ckLabelLength);
    if ((*env)->ExceptionCheck(env)) {
        free(ckpPin);
        return;
    }

    rv = (*ckpFunctions->C_InitToken)(ckSlotID, ckpPin, ckPinLength, ckpLabel);
    TRACE1("InitToken return code: %d", rv);

    free(ckpPin);
    free(ckpLabel);

    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return; }
}
#endif

#ifdef P11_ENABLE_C_INITPIN
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_InitPIN
 * Signature: (J[C)V
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE
 * @param   jcharArray jPin             CK_CHAR_PTR pPin
 *                                      CK_ULONG ulPinLen
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1InitPIN
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jcharArray jPin)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_CHAR_PTR ckpPin = NULL_PTR;
    CK_ULONG ckPinLength;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return; }

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    jCharArrayToCKCharArray(env, jPin, &ckpPin, &ckPinLength);
    if ((*env)->ExceptionCheck(env)) { return; }

    rv = (*ckpFunctions->C_InitPIN)(ckSessionHandle, ckpPin, ckPinLength);

    free(ckpPin);

    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return; }
}
#endif

#ifdef P11_ENABLE_C_SETPIN
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_SetPIN
 * Signature: (J[C[C)V
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE hSession
 * @param   jcharArray jOldPin          CK_CHAR_PTR pOldPin
 *                                      CK_ULONG ulOldLen
 * @param   jcharArray jNewPin          CK_CHAR_PTR pNewPin
 *                                      CK_ULONG ulNewLen
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1SetPIN
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jcharArray jOldPin, jcharArray jNewPin)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_CHAR_PTR ckpOldPin = NULL_PTR;
    CK_CHAR_PTR ckpNewPin = NULL_PTR;
    CK_ULONG ckOldPinLength;
    CK_ULONG ckNewPinLength;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return; }

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    jCharArrayToCKCharArray(env, jOldPin, &ckpOldPin, &ckOldPinLength);
    if ((*env)->ExceptionCheck(env)) { return; }

    jCharArrayToCKCharArray(env, jNewPin, &ckpNewPin, &ckNewPinLength);
    if ((*env)->ExceptionCheck(env)) {
        free(ckpOldPin);
        return;
    }

    rv = (*ckpFunctions->C_SetPIN)(ckSessionHandle, ckpOldPin, ckOldPinLength, ckpNewPin, ckNewPinLength);

    free(ckpOldPin);
    free(ckpNewPin);

    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return; }
}
#endif
