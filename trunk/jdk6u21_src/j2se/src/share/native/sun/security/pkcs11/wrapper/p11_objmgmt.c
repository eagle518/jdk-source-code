/*
 * @(#)p11_objmgmt.c	1.5 10/03/23
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

#ifdef P11_ENABLE_C_CREATEOBJECT
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_CreateObject
 * Signature: (J[Lsun/security/pkcs11/wrapper/CK_ATTRIBUTE;)J
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE hSession
 * @param   jobjectArray jTemplate      CK_ATTRIBUTE_PTR pTemplate
 *                                      CK_ULONG ulCount
 * @return  jlong jObjectHandle         CK_OBJECT_HANDLE_PTR phObject
 */
JNIEXPORT jlong JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1CreateObject
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jobjectArray jTemplate)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_OBJECT_HANDLE ckObjectHandle;
    CK_ATTRIBUTE_PTR ckpAttributes = NULL_PTR;
    CK_ULONG ckAttributesLength;
    jlong jObjectHandle;
    CK_ULONG i;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return 0L; }

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    jAttributeArrayToCKAttributeArray(env, jTemplate, &ckpAttributes, &ckAttributesLength);
    if ((*env)->ExceptionCheck(env)) { return 0L; }

    rv = (*ckpFunctions->C_CreateObject)(ckSessionHandle, ckpAttributes, ckAttributesLength, &ckObjectHandle);

    jObjectHandle = ckULongToJLong(ckObjectHandle);
    freeCKAttributeArray(ckpAttributes, ckAttributesLength);

    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return 0L ; }

    return jObjectHandle ;
}
#endif

#ifdef P11_ENABLE_C_COPYOBJECT
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_CopyObject
 * Signature: (JJ[Lsun/security/pkcs11/wrapper/CK_ATTRIBUTE;)J
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE hSession
 * @param   jlong jObjectHandle         CK_OBJECT_HANDLE hObject
 * @param   jobjectArray jTemplate      CK_ATTRIBUTE_PTR pTemplate
 *                                      CK_ULONG ulCount
 * @return  jlong jNewObjectHandle      CK_OBJECT_HANDLE_PTR phNewObject
 */
JNIEXPORT jlong JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1CopyObject
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jlong jObjectHandle, jobjectArray jTemplate)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_OBJECT_HANDLE ckObjectHandle;
    CK_OBJECT_HANDLE ckNewObjectHandle;
    CK_ATTRIBUTE_PTR ckpAttributes = NULL_PTR;
    CK_ULONG ckAttributesLength;
    jlong jNewObjectHandle;
    CK_ULONG i;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return 0L; }

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    ckObjectHandle = jLongToCKULong(jObjectHandle);
    jAttributeArrayToCKAttributeArray(env, jTemplate, &ckpAttributes, &ckAttributesLength);
    if ((*env)->ExceptionCheck(env)) { return 0L; }

    rv = (*ckpFunctions->C_CopyObject)(ckSessionHandle, ckObjectHandle, ckpAttributes, ckAttributesLength, &ckNewObjectHandle);

    jNewObjectHandle = ckULongToJLong(ckNewObjectHandle);
    freeCKAttributeArray(ckpAttributes, ckAttributesLength);

    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return 0L ; }

    return jNewObjectHandle ;
}
#endif

#ifdef P11_ENABLE_C_DESTROYOBJECT
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_DestroyObject
 * Signature: (JJ)V
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE hSession
 * @param   jlong jObjectHandle         CK_OBJECT_HANDLE hObject
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1DestroyObject
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jlong jObjectHandle)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_OBJECT_HANDLE ckObjectHandle;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return; }

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    ckObjectHandle = jLongToCKULong(jObjectHandle);

    rv = (*ckpFunctions->C_DestroyObject)(ckSessionHandle, ckObjectHandle);
    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return; }
}
#endif

#ifdef P11_ENABLE_C_GETOBJECTSIZE
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_GetObjectSize
 * Signature: (JJ)J
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE hSession
 * @param   jlong jObjectHandle         CK_OBJECT_HANDLE hObject
 * @return  jlong jObjectSize           CK_ULONG_PTR pulSize
 */
JNIEXPORT jlong JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1GetObjectSize
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jlong jObjectHandle)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_OBJECT_HANDLE ckObjectHandle;
    CK_ULONG ckObjectSize;
    jlong jObjectSize;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return 0L; }

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    ckObjectHandle = jLongToCKULong(jObjectHandle);

    rv = (*ckpFunctions->C_GetObjectSize)(ckSessionHandle, ckObjectHandle, &ckObjectSize);
    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return 0L ; }

    jObjectSize = ckULongToJLong(ckObjectSize);

    return jObjectSize ;
}
#endif

#ifdef P11_ENABLE_C_GETATTRIBUTEVALUE
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_GetAttributeValue
 * Signature: (JJ[Lsun/security/pkcs11/wrapper/CK_ATTRIBUTE;)[Lsun/security/pkcs11/wrapper/CK_ATTRIBUTE;
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE hSession
 * @param   jlong jObjectHandle         CK_OBJECT_HANDLE hObject
 * @param   jobjectArray jTemplate      CK_ATTRIBUTE_PTR pTemplate
 *                                      CK_ULONG ulCount
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1GetAttributeValue
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jlong jObjectHandle, jobjectArray jTemplate)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_OBJECT_HANDLE ckObjectHandle;
    CK_ATTRIBUTE_PTR ckpAttributes = NULL_PTR;
    CK_ULONG ckAttributesLength;
    CK_ULONG ckBufferLength;
    CK_ULONG i, j;
    jobject jAttribute;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return; }

    TRACE0("DEBUG: C_GetAttributeValue");
    TRACE1(", hSession=%u", jSessionHandle);
    TRACE1(", hObject=%u", jObjectHandle);
    TRACE1(", pTemplate=%p", jTemplate);
    TRACE0(" ... ");

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    ckObjectHandle = jLongToCKULong(jObjectHandle);
    TRACE1("jAttributeArrayToCKAttributeArray now with jTemplate = %d", jTemplate);
    jAttributeArrayToCKAttributeArray(env, jTemplate, &ckpAttributes, &ckAttributesLength);
    if ((*env)->ExceptionCheck(env)) { return; }

    TRACE2("DEBUG: jAttributeArrayToCKAttributeArray finished with ckpAttribute = %d, Length = %d\n", ckpAttributes, ckAttributesLength);

    /* first set all pValue to NULL, to get the needed buffer length */
    for(i = 0; i < ckAttributesLength; i++) {
    	if (ckpAttributes[i].pValue != NULL_PTR) {
            free(ckpAttributes[i].pValue);
            ckpAttributes[i].pValue = NULL_PTR;
        }
    }

    rv = (*ckpFunctions->C_GetAttributeValue)(ckSessionHandle, ckObjectHandle, ckpAttributes, ckAttributesLength);
    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) {
        free(ckpAttributes);
        return ;
    }

    /* now, the ulValueLength field of each attribute should hold the exact buffer length needed
     * allocate the needed buffers accordingly
     */
    for (i = 0; i < ckAttributesLength; i++) {
        ckBufferLength = sizeof(CK_BYTE) * ckpAttributes[i].ulValueLen;
        ckpAttributes[i].pValue = (void *) malloc(ckBufferLength);
        if (ckpAttributes[i].pValue == NULL) {
            freeCKAttributeArray(ckpAttributes, i);
            JNU_ThrowOutOfMemoryError(env, 0);
            return;
        }
        ckpAttributes[i].ulValueLen = ckBufferLength;
    }

    /* now get the attributes with all values */
    rv = (*ckpFunctions->C_GetAttributeValue)(ckSessionHandle, ckObjectHandle, ckpAttributes, ckAttributesLength);

    if (ckAssertReturnValueOK(env, rv) == CK_ASSERT_OK) {
        /* copy back the values to the Java attributes */
        for (i = 0; i < ckAttributesLength; i++) {
            jAttribute = ckAttributePtrToJAttribute(env, &(ckpAttributes[i]));
            if (jAttribute == NULL) {
                freeCKAttributeArray(ckpAttributes, ckAttributesLength);
                return;
            }
            (*env)->SetObjectArrayElement(env, jTemplate, i, jAttribute);
            if ((*env)->ExceptionCheck(env)) {
                freeCKAttributeArray(ckpAttributes, ckAttributesLength);
                return;
            }
        }
    }
    freeCKAttributeArray(ckpAttributes, ckAttributesLength);
    TRACE0("FINISHED\n");
}
#endif

#ifdef P11_ENABLE_C_SETATTRIBUTEVALUE
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_SetAttributeValue
 * Signature: (JJ[Lsun/security/pkcs11/wrapper/CK_ATTRIBUTE;)V
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE hSession
 * @param   jlong jObjectHandle         CK_OBJECT_HANDLE hObject
 * @param   jobjectArray jTemplate      CK_ATTRIBUTE_PTR pTemplate
 *                                      CK_ULONG ulCount
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1SetAttributeValue
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jlong jObjectHandle, jobjectArray jTemplate)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_OBJECT_HANDLE ckObjectHandle;
    CK_ATTRIBUTE_PTR ckpAttributes = NULL_PTR;
    CK_ULONG ckAttributesLength;
    CK_ULONG i;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return; }

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    ckObjectHandle = jLongToCKULong(jObjectHandle);
    jAttributeArrayToCKAttributeArray(env, jTemplate, &ckpAttributes, &ckAttributesLength);
    if ((*env)->ExceptionCheck(env)) { return; }

    rv = (*ckpFunctions->C_SetAttributeValue)(ckSessionHandle, ckObjectHandle, ckpAttributes, ckAttributesLength);

    freeCKAttributeArray(ckpAttributes, ckAttributesLength);

    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return; }
}
#endif

#ifdef P11_ENABLE_C_FINDOBJECTSINIT
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_FindObjectsInit
 * Signature: (J[Lsun/security/pkcs11/wrapper/CK_ATTRIBUTE;)V
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE hSession
 * @param   jobjectArray jTemplate      CK_ATTRIBUTE_PTR pTemplate
 *                                      CK_ULONG ulCount
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1FindObjectsInit
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jobjectArray jTemplate)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_ATTRIBUTE_PTR ckpAttributes = NULL_PTR;
    CK_ULONG ckAttributesLength;
    CK_ULONG i;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return; }

    TRACE0("DEBUG: C_FindObjectsInit");
    TRACE1(", hSession=%u", jSessionHandle);
    TRACE1(", pTemplate=%p", jTemplate);
    TRACE0(" ... ");

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    jAttributeArrayToCKAttributeArray(env, jTemplate, &ckpAttributes, &ckAttributesLength);
    if ((*env)->ExceptionCheck(env)) { return; }

    rv = (*ckpFunctions->C_FindObjectsInit)(ckSessionHandle, ckpAttributes, ckAttributesLength);

    freeCKAttributeArray(ckpAttributes, ckAttributesLength);
    TRACE0("FINISHED\n");

    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return; }
}
#endif

#ifdef P11_ENABLE_C_FINDOBJECTS
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_FindObjects
 * Signature: (JJ)[J
 * Parametermapping:                        *PKCS11*
 * @param   jlong jSessionHandle            CK_SESSION_HANDLE hSession
 * @param   jlong jMaxObjectCount           CK_ULONG ulMaxObjectCount
 * @return  jlongArray jObjectHandleArray   CK_OBJECT_HANDLE_PTR phObject
 *                                          CK_ULONG_PTR pulObjectCount
 */
JNIEXPORT jlongArray JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1FindObjects
    (JNIEnv *env, jobject obj, jlong jSessionHandle, jlong jMaxObjectCount)
{
    CK_RV rv;
    CK_SESSION_HANDLE ckSessionHandle;
    CK_ULONG ckMaxObjectLength;
    CK_OBJECT_HANDLE_PTR ckpObjectHandleArray;
    CK_ULONG ckActualObjectCount;
    jlongArray jObjectHandleArray = NULL;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return NULL; }

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    ckMaxObjectLength = jLongToCKULong(jMaxObjectCount);
    ckpObjectHandleArray = (CK_OBJECT_HANDLE_PTR) malloc(sizeof(CK_OBJECT_HANDLE) * ckMaxObjectLength);
    if (ckpObjectHandleArray == NULL) {
        JNU_ThrowOutOfMemoryError(env, 0);
        return NULL;
    }

    rv = (*ckpFunctions->C_FindObjects)(ckSessionHandle, ckpObjectHandleArray, ckMaxObjectLength, &ckActualObjectCount);
    if (ckAssertReturnValueOK(env, rv) == CK_ASSERT_OK) {
        jObjectHandleArray = ckULongArrayToJLongArray(env, ckpObjectHandleArray, ckActualObjectCount);
    }

    free(ckpObjectHandleArray);

    return jObjectHandleArray ;
}
#endif

#ifdef P11_ENABLE_C_FINDOBJECTSFINAL
/*
 * Class:     sun_security_pkcs11_wrapper_PKCS11
 * Method:    C_FindObjectsFinal
 * Signature: (J)V
 * Parametermapping:                    *PKCS11*
 * @param   jlong jSessionHandle        CK_SESSION_HANDLE hSession
 */
JNIEXPORT void JNICALL Java_sun_security_pkcs11_wrapper_PKCS11_C_1FindObjectsFinal
    (JNIEnv *env, jobject obj, jlong jSessionHandle)
{
    CK_SESSION_HANDLE ckSessionHandle;
    CK_RV rv;

    CK_FUNCTION_LIST_PTR ckpFunctions = getFunctionList(env, obj);
    if (ckpFunctions == NULL) { return; }

    ckSessionHandle = jLongToCKULong(jSessionHandle);
    rv = (*ckpFunctions->C_FindObjectsFinal)(ckSessionHandle);
    if (ckAssertReturnValueOK(env, rv) != CK_ASSERT_OK) { return; }
}
#endif

