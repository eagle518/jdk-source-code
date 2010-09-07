/*
 * @(#)protocol.h	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Protocol for JNI/JS calls */

/* The protocol for exchanging the data between JNI stubs (client) and remote 
   VM. The general message format is 
    4 bytes     message-specific
   -----------------------------
   | code    |      ......     |
   -----------------------------
   Some messages block until they receive a response from the JVM, some do not
   require a response. In general, a code for a particular JNI call
   corresponds to the position of that function in JNIEnv function table.
   */

/* JVM can also call into the client. This can happen if VM tries to call a
   native method which has been registered via client's RegisterNatives. This
   call can occur in two scenarios:
   a. A random VM thread tries to execute a remote native method. In that
   case, we need to create a thread in the client, and process the request.
   The request from VM to the client is always prefixed with
   JAVA_PLUGIN_REQUEST. 

   b. A VM thread which is executing client's command tries to call a remote
   native method. That can happen iff the client issued a method call command,
   getting and setting fields, strings or array elements cannot produce this
   condition. Any request to JVM which executes Java methods, must call
   handle_request(JNIEnv *env) before recovering the result. Upon successful
   completion, JVM will return JAVA_PLUGIN_RETURN before returning any
   values. 
   */
   
#include "sun_plugin_javascript_navig5_JSObject.h"

/* The regular protocol */
#define JAVA_PLUGIN_RETURN      0x10000000
#define JAVA_PLUGIN_REQUEST     0x00000001

#define invokecode(x) sun_plugin_javascript_navig5_JSObject_##x

/* The JSObject Requests are on a different pipe, but we also keep them
   in a separate part of the protocol space */
#define JNIJS_BASE      0x00010000
#define JAVA_PLUGIN_JNIJS_GET_NATIVE   (JNIJS_BASE + invokecode(JSOBJECT_GETWINDOW))
#define JAVA_PLUGIN_JNIJS_GETMEMBER    (JNIJS_BASE + invokecode(JSOBJECT_GETMEMBER))
#define JAVA_PLUGIN_JNIJS_GETSLOT      (JNIJS_BASE + invokecode(JSOBJECT_GETSLOT))
#define JAVA_PLUGIN_JNIJS_SETMEMBER    (JNIJS_BASE + invokecode(JSOBJECT_SETMEMBER))
#define JAVA_PLUGIN_JNIJS_SETSLOT      (JNIJS_BASE + invokecode(JSOBJECT_SETSLOT))
#define JAVA_PLUGIN_JNIJS_REMOVEMEMBER (JNIJS_BASE + invokecode(JSOBJECT_REMOVEMEMBER))
#define JAVA_PLUGIN_JNIJS_CALL         (JNIJS_BASE + invokecode(JSOBJECT_CALL))
#define JAVA_PLUGIN_JNIJS_EVAL         (JNIJS_BASE + invokecode(JSOBJECT_EVAL))
#define JAVA_PLUGIN_JNIJS_TOSTRING     (JNIJS_BASE + invokecode(JSOBJECT_TOSTRING))
#define JAVA_PLUGIN_JNIJS_FINALIZE     (JNIJS_BASE + invokecode(JSOBJECT_FINALIZE))

#define JAVA_PLUGIN_GET_BROWSER_AUTHINFO             (JAVA_PLUGIN_SECURE_BASE + 10)

/* For calls from the CSecurityContext to verify security in java */
#define JAVA_PLUGIN_CSECURITYCONTEXT_IMPLIES (JNIJS_BASE + 13)


/* The secure protocol occupies a different part of the protocol space */
#define JAVA_PLUGIN_SECURE_BASE            0x00001000
#define JAVA_PLUGIN_SECURE_NEW_OBJECT      (JAVA_PLUGIN_SECURE_BASE + 1)
#define JAVA_PLUGIN_SECURE_CALL            (JAVA_PLUGIN_SECURE_BASE + 2)
#define JAVA_PLUGIN_SECURE_CALL_NONVIRTUAL     (JAVA_PLUGIN_SECURE_BASE + 3)
#define JAVA_PLUGIN_SECURE_GET_FIELD       (JAVA_PLUGIN_SECURE_BASE + 4)
#define JAVA_PLUGIN_SECURE_SET_FIELD       (JAVA_PLUGIN_SECURE_BASE + 5)
#define JAVA_PLUGIN_SECURE_CALL_STATIC           (JAVA_PLUGIN_SECURE_BASE + 6)
#define JAVA_PLUGIN_SECURE_GET_STATIC_FIELD      (JAVA_PLUGIN_SECURE_BASE + 7)
#define JAVA_PLUGIN_SECURE_SET_STATIC_FIELD      (JAVA_PLUGIN_SECURE_BASE + 8)
/* Call to get the applet object */
#define JAVA_PLUGIN_GET_JAVA_OBJECT              (JAVA_PLUGIN_SECURE_BASE + 9)


#define JAVA_PLUGIN_JNI_VERSION            0x00000004
#define JAVA_PLUGIN_DEFINE_CLASS           0x00000005
#define JAVA_PLUGIN_FIND_CLASS             0x00000006



#define JAVA_PLUGIN_GET_SUPER_CLASS        0x0000000a
#define JAVA_PLUGIN_IS_SUBCLASS_OF         0x0000000b

#define JAVA_PLUGIN_THROW                  0x0000000d
#define JAVA_PLUGIN_THROW_NEW              0x0000000e
#define JAVA_PLUGIN_EXCEPTION_OCCURED      0x0000000f
#define JAVA_PLUGIN_EXCEPTION_DESCRIBE     0x00000010
#define JAVA_PLUGIN_EXCEPTION_CLEAR        0x00000011
#define JAVA_PLUGIN_FATAL_ERROR            0x00000012


/* Global references: because of the out-of-process limitations, all the
   objects returned to the client are wrapped as global
   references. RELEASE_LOCAL_REF from the VM's point of view is the same as
   RELEASE_GLOBAL_REF. */
#define JAVA_PLUGIN_NEW_GLOBAL_REF         0x00000015
#define JAVA_PLUGIN_RELEASE_GLOBAL_REF     0x00000016
#define JAVA_PLUGIN_RELEASE_LOCAL_REF      0x00000017
#define JAVA_PLUGIN_IS_SAME_OBJECT         0x00000018

#define JAVA_PLUGIN_ALLOC_OBJECT           0x0000001b
#define JAVA_PLUGIN_NEW_OBJECT_METHOD      0x0000001e
#define JAVA_PLUGIN_GET_OBJECT_CLASS       0x0000001f
#define JAVA_PLUGIN_IS_INSTANCE_OF         0x00000020

/* on the client side we need some extra information about the method. We
   exploit the fact that the standard jni.h uses jmethodID as a pointer to an
   opaque reference.
   */
#define JAVA_PLUGIN_GET_METHOD_ID          0x00000021


/* Call<modifiers>Method[A|V]?():
   in all cases we call the Call<modifiers>MethodA in the VM. Since jmethodID
   in this implementation contains reference to the method signature, it is
   easy to deduce the argument types, and marshal them into an array of
   jvalues. Note, that all of the client stubs have to call
   handle_response(env) before they recover the results. */
#define JAVA_PLUGIN_CALL_METHOD            0x00000024

#define JAVA_PLUGIN_CALL_OBJECT_METHOD JAVA_PLUGIN_CALL_METHOD
#define JAVA_PLUGIN_CALL_BOOLEAN_METHOD (JAVA_PLUGIN_CALL_METHOD+3)
#define JAVA_PLUGIN_CALL_BYTE_METHOD (JAVA_PLUGIN_CALL_METHOD+6)
#define JAVA_PLUGIN_CALL_CHAR_METHOD (JAVA_PLUGIN_CALL_METHOD+9)
#define JAVA_PLUGIN_CALL_SHORT_METHOD (JAVA_PLUGIN_CALL_METHOD+12)
#define JAVA_PLUGIN_CALL_INT_METHOD (JAVA_PLUGIN_CALL_METHOD+15)
#define JAVA_PLUGIN_CALL_LONG_METHOD (JAVA_PLUGIN_CALL_METHOD+18)
#define JAVA_PLUGIN_CALL_FLOAT_METHOD (JAVA_PLUGIN_CALL_METHOD+21)
#define JAVA_PLUGIN_CALL_DOUBLE_METHOD (JAVA_PLUGIN_CALL_METHOD+24)
#define JAVA_PLUGIN_CALL_VOID_METHOD (JAVA_PLUGIN_CALL_METHOD+27)

#define JAVA_PLUGIN_CALL_NV_METHOD (JAVA_PLUGIN_CALL_METHOD+30)

#define JAVA_PLUGIN_CALL_NV_OBJECT_METHOD JAVA_PLUGIN_CALL_NV_METHOD
#define JAVA_PLUGIN_CALL_NV_BOOLEAN_METHOD (JAVA_PLUGIN_CALL_NV_METHOD+3)
#define JAVA_PLUGIN_CALL_NV_BYTE_METHOD (JAVA_PLUGIN_CALL_NV_METHOD+6)
#define JAVA_PLUGIN_CALL_NV_CHAR_METHOD (JAVA_PLUGIN_CALL_NV_METHOD+9)
#define JAVA_PLUGIN_CALL_NV_SHORT_METHOD (JAVA_PLUGIN_CALL_NV_METHOD+12)
#define JAVA_PLUGIN_CALL_NV_INT_METHOD (JAVA_PLUGIN_CALL_NV_METHOD+15)
#define JAVA_PLUGIN_CALL_NV_LONG_METHOD (JAVA_PLUGIN_CALL_NV_METHOD+18)
#define JAVA_PLUGIN_CALL_NV_FLOAT_METHOD (JAVA_PLUGIN_CALL_NV_METHOD+21)
#define JAVA_PLUGIN_CALL_NV_DOUBLE_METHOD (JAVA_PLUGIN_CALL_NV_METHOD+24)
#define JAVA_PLUGIN_CALL_NV_VOID_METHOD (JAVA_PLUGIN_CALL_NV_METHOD+27)

#define JAVA_PLUGIN_GET_FIELD_ID           0x0000005e

#define JAVA_PLUGIN_GET_FIELD_BASE         0x0000005f

#define JAVA_PLUGIN_GET_OBJECT_FIELD       JAVA_PLUGIN_GET_FIELD_BASE
#define JAVA_PLUGIN_GET_BOOLEAN_FIELD      (JAVA_PLUGIN_GET_FIELD_BASE+1)
#define JAVA_PLUGIN_GET_BYTE_FIELD         (JAVA_PLUGIN_GET_FIELD_BASE+2)
#define JAVA_PLUGIN_GET_CHAR_FIELD         (JAVA_PLUGIN_GET_FIELD_BASE+3)
#define JAVA_PLUGIN_GET_SHORT_FIELD        (JAVA_PLUGIN_GET_FIELD_BASE+4)
#define JAVA_PLUGIN_GET_INT_FIELD          (JAVA_PLUGIN_GET_FIELD_BASE+5)
#define JAVA_PLUGIN_GET_LONG_FIELD         (JAVA_PLUGIN_GET_FIELD_BASE+6)
#define JAVA_PLUGIN_GET_FLOAT_FIELD        (JAVA_PLUGIN_GET_FIELD_BASE+7)
#define JAVA_PLUGIN_GET_DOUBLE_FIELD       (JAVA_PLUGIN_GET_FIELD_BASE+8)

/* Set<mod>Field does not require a response from the VM */

#define JAVA_PLUGIN_SET_FIELD_BASE         (JAVA_PLUGIN_GET_FIELD_BASE+9)

#define JAVA_PLUGIN_SET_OBJECT_FIELD       JAVA_PLUGIN_SET_FIELD_BASE
#define JAVA_PLUGIN_SET_BOOLEAN_FIELD      (JAVA_PLUGIN_SET_FIELD_BASE+1)
#define JAVA_PLUGIN_SET_BYTE_FIELD         (JAVA_PLUGIN_SET_FIELD_BASE+2)
#define JAVA_PLUGIN_SET_CHAR_FIELD         (JAVA_PLUGIN_SET_FIELD_BASE+3)
#define JAVA_PLUGIN_SET_SHORT_FIELD        (JAVA_PLUGIN_SET_FIELD_BASE+4)
#define JAVA_PLUGIN_SET_INT_FIELD          (JAVA_PLUGIN_SET_FIELD_BASE+5)
#define JAVA_PLUGIN_SET_LONG_FIELD         (JAVA_PLUGIN_SET_FIELD_BASE+6)
#define JAVA_PLUGIN_SET_FLOAT_FIELD        (JAVA_PLUGIN_SET_FIELD_BASE+7)
#define JAVA_PLUGIN_SET_DOUBLE_FIELD       (JAVA_PLUGIN_SET_FIELD_BASE+8)

/* Same as with methods */

#define JAVA_PLUGIN_GET_STATIC_METHOD_ID   0x00000071

#define JAVA_PLUGIN_CALL_STATIC_METHOD     0x00000074

#define JAVA_PLUGIN_CALL_STATIC_OBJECT_METHOD JAVA_PLUGIN_CALL_STATIC_METHOD
#define JAVA_PLUGIN_CALL_STATIC_BOOLEAN_METHOD (JAVA_PLUGIN_CALL_STATIC_METHOD+3)
#define JAVA_PLUGIN_CALL_STATIC_BYTE_METHOD (JAVA_PLUGIN_CALL_STATIC_METHOD+6)
#define JAVA_PLUGIN_CALL_STATIC_CHAR_METHOD (JAVA_PLUGIN_CALL_STATIC_METHOD+9)
#define JAVA_PLUGIN_CALL_STATIC_SHORT_METHOD (JAVA_PLUGIN_CALL_STATIC_METHOD+12)
#define JAVA_PLUGIN_CALL_STATIC_INT_METHOD (JAVA_PLUGIN_CALL_STATIC_METHOD+15)
#define JAVA_PLUGIN_CALL_STATIC_LONG_METHOD (JAVA_PLUGIN_CALL_STATIC_METHOD+18)
#define JAVA_PLUGIN_CALL_STATIC_FLOAT_METHOD (JAVA_PLUGIN_CALL_STATIC_METHOD+21)
#define JAVA_PLUGIN_CALL_STATIC_DOUBLE_METHOD (JAVA_PLUGIN_CALL_STATIC_METHOD+24)
#define JAVA_PLUGIN_CALL_STATIC_VOID_METHOD (JAVA_PLUGIN_CALL_STATIC_METHOD+27)

/* same as with other fields */

#define JAVA_PLUGIN_GET_STATIC_FIELD_ID    0x00000090

#define JP_GET_STATIC_FIELD_BASE         0x00000091

#define JAVA_PLUGIN_GET_STATIC_OBJECT_FIELD       JP_GET_STATIC_FIELD_BASE
#define JAVA_PLUGIN_GET_STATIC_BOOLEAN_FIELD      (JP_GET_STATIC_FIELD_BASE+1)
#define JAVA_PLUGIN_GET_STATIC_BYTE_FIELD         (JP_GET_STATIC_FIELD_BASE+2)
#define JAVA_PLUGIN_GET_STATIC_CHAR_FIELD         (JP_GET_STATIC_FIELD_BASE+3)
#define JAVA_PLUGIN_GET_STATIC_SHORT_FIELD        (JP_GET_STATIC_FIELD_BASE+4)
#define JAVA_PLUGIN_GET_STATIC_INT_FIELD          (JP_GET_STATIC_FIELD_BASE+5)
#define JAVA_PLUGIN_GET_STATIC_LONG_FIELD         (JP_GET_STATIC_FIELD_BASE+6)
#define JAVA_PLUGIN_GET_STATIC_FLOAT_FIELD        (JP_GET_STATIC_FIELD_BASE+7)
#define JAVA_PLUGIN_GET_STATIC_DOUBLE_FIELD       (JP_GET_STATIC_FIELD_BASE+8)

#define JP_SET_STATIC_FIELD_BASE         (JP_GET_STATIC_FIELD_BASE+9)

#define JAVA_PLUGIN_SET_STATIC_OBJECT_FIELD       JP_SET_STATIC_FIELD_BASE
#define JAVA_PLUGIN_SET_STATIC_BOOLEAN_FIELD      (JP_SET_STATIC_FIELD_BASE+1)
#define JAVA_PLUGIN_SET_STATIC_BYTE_FIELD         (JP_SET_STATIC_FIELD_BASE+2)
#define JAVA_PLUGIN_SET_STATIC_CHAR_FIELD         (JP_SET_STATIC_FIELD_BASE+3)
#define JAVA_PLUGIN_SET_STATIC_SHORT_FIELD        (JP_SET_STATIC_FIELD_BASE+4)
#define JAVA_PLUGIN_SET_STATIC_INT_FIELD          (JP_SET_STATIC_FIELD_BASE+5)
#define JAVA_PLUGIN_SET_STATIC_LONG_FIELD         (JP_SET_STATIC_FIELD_BASE+6)
#define JAVA_PLUGIN_SET_STATIC_FLOAT_FIELD        (JP_SET_STATIC_FIELD_BASE+7)
#define JAVA_PLUGIN_SET_STATIC_DOUBLE_FIELD       (JP_SET_STATIC_FIELD_BASE+8)

/* strings come in two flavors: UNICODE, and UTF. To transmit a UTF or a C
   string, find its length (short int), send it, then send the string
   characters. For UNICODE strings and arrays do the same, except that length
   is encoded as 4 bytes. Note that since strings are constants,
   ReleaseStringChars is a noop. 
   */
#define JAVA_PLUGIN_NEW_STRING              0x00000104
#define JAVA_PLUGIN_GET_STRING_SIZE         0x00000105
#define JAVA_PLUGIN_GET_STRING_CHARS        0x00000106

#define JAVA_PLUGIN_NEW_STRING_UTF          0x00000108
#define JAVA_PLUGIN_GET_STRING_UTF_SIZE     0x00000109
#define JAVA_PLUGIN_GET_STRING_UTF_CHARS    0x0000010a

#define JAVA_PLUGIN_GET_ARRAY_LENGTH        0x0000010c
#define JAVA_PLUGIN_NEW_OBJECT_ARRAY        0x0000010d
#define JAVA_PLUGIN_GET_OBJECT_ARRAY_ELEMENT 0x0000010e
#define JAVA_PLUGIN_SET_OBJECT_ARRAY_ELEMENT 0x0000010f

#define JAVA_PLUGIN_NEW_ARRAY_BASE          0x00000110
#define JAVA_PLUGIN_NEW_BOOL_ARRAY   (JAVA_PLUGIN_NEW_ARRAY_BASE)
#define JAVA_PLUGIN_NEW_BYTE_ARRAY   (JAVA_PLUGIN_NEW_ARRAY_BASE + 1)
#define JAVA_PLUGIN_NEW_CHAR_ARRAY   (JAVA_PLUGIN_NEW_ARRAY_BASE + 2)
#define JAVA_PLUGIN_NEW_SHORT_ARRAY  (JAVA_PLUGIN_NEW_ARRAY_BASE + 3)
#define JAVA_PLUGIN_NEW_INT_ARRAY    (JAVA_PLUGIN_NEW_ARRAY_BASE + 4)
#define JAVA_PLUGIN_NEW_LONG_ARRAY   (JAVA_PLUGIN_NEW_ARRAY_BASE + 5)
#define JAVA_PLUGIN_NEW_FLOAT_ARRAY  (JAVA_PLUGIN_NEW_ARRAY_BASE + 6)
#define JAVA_PLUGIN_NEW_DOUBLE_ARRAY (JAVA_PLUGIN_NEW_ARRAY_BASE + 7)

/* touching array elements: if possible we let the client know that the data
   he's been given is merely a copy. The JVM acts accordingly. Upon calling
   Release<modifiers>ArrayElements(), we copy all the array data into the
   VM. Between Get and Release the array is not pinned (copy semantics). We do
   not worry about the array dissapering on us, since we still hold a global
   reference to it (just like to any other object).
   */

#define JAVA_PLUGIN_CAP_BASE                0x00000118

#define JAVA_PLUGIN_CAP_BOOL_AREL   (JAVA_PLUGIN_CAP_BASE)
#define JAVA_PLUGIN_CAP_BYTE_AREL   (JAVA_PLUGIN_CAP_BASE + 1)
#define JAVA_PLUGIN_CAP_CHAR_AREL   (JAVA_PLUGIN_CAP_BASE + 2)
#define JAVA_PLUGIN_CAP_SHORT_AREL  (JAVA_PLUGIN_CAP_BASE + 3)
#define JAVA_PLUGIN_CAP_INT_AREL    (JAVA_PLUGIN_CAP_BASE + 4)
#define JAVA_PLUGIN_CAP_LONG_AREL   (JAVA_PLUGIN_CAP_BASE + 5)
#define JAVA_PLUGIN_CAP_FLOAT_AREL  (JAVA_PLUGIN_CAP_BASE + 6)
#define JAVA_PLUGIN_CAP_DOUBLE_AREL (JAVA_PLUGIN_CAP_BASE + 7)

#define JAVA_PLUGIN_REL_BASE                0x00000120
#define JAVA_PLUGIN_REL_BOOL_AREL   (JAVA_PLUGIN_REL_BASE)
#define JAVA_PLUGIN_REL_BYTE_AREL   (JAVA_PLUGIN_REL_BASE + 1)
#define JAVA_PLUGIN_REL_CHAR_AREL   (JAVA_PLUGIN_REL_BASE + 2)
#define JAVA_PLUGIN_REL_SHORT_AREL  (JAVA_PLUGIN_REL_BASE + 3)
#define JAVA_PLUGIN_REL_INT_AREL    (JAVA_PLUGIN_REL_BASE + 4)
#define JAVA_PLUGIN_REL_LONG_AREL   (JAVA_PLUGIN_REL_BASE + 5)
#define JAVA_PLUGIN_REL_FLOAT_AREL  (JAVA_PLUGIN_REL_BASE + 6)
#define JAVA_PLUGIN_REL_DOUBLE_AREL (JAVA_PLUGIN_REL_BASE + 7)

#define JAVA_PLUGIN_GET_BASE                0x00000128
#define JAVA_PLUGIN_GET_BOOL_AREL   (JAVA_PLUGIN_GET_BASE)
#define JAVA_PLUGIN_GET_BYTE_AREL   (JAVA_PLUGIN_GET_BASE + 1)
#define JAVA_PLUGIN_GET_CHAR_AREL   (JAVA_PLUGIN_GET_BASE + 2)
#define JAVA_PLUGIN_GET_SHORT_AREL  (JAVA_PLUGIN_GET_BASE + 3)
#define JAVA_PLUGIN_GET_INT_AREL    (JAVA_PLUGIN_GET_BASE + 4)
#define JAVA_PLUGIN_GET_LONG_AREL   (JAVA_PLUGIN_GET_BASE + 5)
#define JAVA_PLUGIN_GET_FLOAT_AREL  (JAVA_PLUGIN_GET_BASE + 6)
#define JAVA_PLUGIN_GET_DOUBLE_AREL (JAVA_PLUGIN_GET_BASE + 7)

#define JAVA_PLUGIN_SET_BASE                0x00000130
#define JAVA_PLUGIN_SET_BOOL_AREL   (JAVA_PLUGIN_SET_BASE)
#define JAVA_PLUGIN_SET_BYTE_AREL   (JAVA_PLUGIN_SET_BASE + 1)
#define JAVA_PLUGIN_SET_CHAR_AREL   (JAVA_PLUGIN_SET_BASE + 2)
#define JAVA_PLUGIN_SET_SHORT_AREL  (JAVA_PLUGIN_SET_BASE + 3)
#define JAVA_PLUGIN_SET_INT_AREL    (JAVA_PLUGIN_SET_BASE + 4)
#define JAVA_PLUGIN_SET_LONG_AREL   (JAVA_PLUGIN_SET_BASE + 5)
#define JAVA_PLUGIN_SET_FLOAT_AREL  (JAVA_PLUGIN_SET_BASE + 6)
#define JAVA_PLUGIN_SET_DOUBLE_AREL (JAVA_PLUGIN_SET_BASE + 7)

/* RegisterNatives does perhaps the most interesting thing: it allows the VM
   to call native methods in client's space. Normal native methods execute in
   the VM's address space, however, the methods which were registered by the
   client with RegisterNatives, execute in the client's space. Here is how
   this is done:
   1. when register natives is called the client registers the function
   pointer and its Java signature in some array, and transmits the means of
   locating that function to the VM. 
   2. VM registers an appropriate RPC stub in place of the real function. 
   3. When that native function gets called in the VM, the RPC stub ensures
   that there is someone waiting to receive the call on the client side. The
   data is transmitted, and the call is executed.
   */
#define JAVA_PLUGIN_REGISTER_NATIVES        0x00000138
#define JAVA_PLUGIN_UNREGISTER_NATIVES      0x00000139

/* Perhaps this is obvious, but these functions are used to synchronize with
   other Java threads. It is a lot cheaper to synchronize client's threads
   with client's locks. Caution: using Java locks and client locks
   simultaneously, may lead to deadlock. */
#define JAVA_PLUGIN_MONITOR_ENTER           0x0000013a
#define JAVA_PLUGIN_MONITOR_EXIT            0x0000013b




/* Convert a protocol integer to a string for debugging.  Defined in jni.h */
char *protocol_descriptor_to_str(int );

