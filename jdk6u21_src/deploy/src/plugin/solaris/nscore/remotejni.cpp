/*
 * @(#)remotejni.cpp	1.41 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Implementation of the remote jni interface over the wire.
 * This implemenation works together with the JavaVM5 to communicate
 * with the VM. Communication takes place by send requests over the command
 * pipe and replies are returned over the worker pipe
 */

#include "commonhdr.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <poll.h>
#include "protocol.h"
#include "remotejni.h"
#include "CSecurityContext.h"
#include "IPluginServiceProvider.h"
#include "ILiveconnect.h"
#include "IUnixService.h"
#include "IJVMManager.h"
#include "JavaPluginFactory5.h"
#include "JavaPluginInstance5.h"
#include "JavaVM5.h"
#include "JSObject.h"

/* A table of envs for mapping from indices to env pointers.  (linear
   search through table only needed when registering and unregistering
   an env. Otherwise use indices directly.)
*/
#ifndef NO_MINIMAL_TRACE
#define TRACE_ENV(e,m) trace("[RemoteEnv%d]: %s\n", e->functions->environmentInfo->env_index, m);
#define TRACE_ENV_STR(e,m,p) trace("[RemoteEnv%d]: %s %s\n", e->functions->environmentInfo->env_index, m,p);
#else
#define TRACE_ENV(e, m)
#define TRACE_ENV_STR(e,m,p)
#endif

#define STR_OR_NULL(s) ((s==NULL)?"NULL":s)    

void handle_response(RemoteJNIEnv *env);
void * getAndPackSecurityInfo(ISecurityContext * ctx, int * len);

extern IUnixService* g_unixService;

#define GET_SIGNATURE(methodID) methodID->signature

void print_jvalue(jd_jni_type type, jvalue v, const char* str)
{
    char temp[256];
    
    switch (type)
    {
    case jd_jboolean_type: sprintf(temp, "Boolean %s = %s\n", str, 
				(v.z ? "true" : "false")); break;
    case jd_jbyte_type: sprintf(temp, "Byte %s = %d\n", str, v.b); break;
    case jd_jchar_type: sprintf(temp, "Char %s = %c\n", str, v.c); break;
    case jd_jshort_type: sprintf(temp, "Short %s = %d\n", str, v.s); break;
    case jd_jint_type: sprintf(temp, "Int %s = %d\n", str, (int) v.i); break;
    case jd_jlong_type: { 
	sprintf(temp, "Long %s = %d\n", str, (int) v.j); 
	break;
    }
    case jd_jfloat_type: sprintf(temp, "Float %s = %f\n", str, v.f); break;
    case jd_jdouble_type: sprintf(temp, "Double %s = %g\n", str, v.d); break;
    case jd_jvoid_type: sprintf(temp, "Void %s = void\n", str); break;
    case jd_jobject_type: sprintf(temp, "Object %s = Object\n", str); break;
    default:    sprintf(temp, "Error type\n"); 
    }
    trace(temp);
}

const char*
get_jni_name(jd_jni_type t) {
    switch(t) {
    case jd_jboolean_type: return "boolean";
    case jd_jbyte_type: return "byte";
    case jd_jchar_type: return "char";
    case jd_jshort_type: return "short";
    case jd_jint_type: return "int";
    case jd_jlong_type: return "jlong";
    case jd_jfloat_type: return "jfloat";
    case jd_jdouble_type: return "jdouble";
    case jd_jobject_type: return "jobject";
    case jd_jvoid_type: return "jvoid";
    default:
	plugin_error("get_jni_name: Unknown jni_type %d\n", (int) t);
	return "Unknown jni_type!!!";
    }
}

static int message_counter = 0;

void
debug(char *msg) {
  fprintf(stdout, "[remotejni%d] %s\n", message_counter, msg);
}

void
print_send(char *msg, int len) {
  int code;
  memcpy(&code, msg + 4, 4);

  UNUSED (len);
  //fprintf(stdout, "remotejni: Sending code: %X len=%d\n", code, len);
}

/* 
 * Send an env message on the per-thread pipe. First send a generic header,
 * indicating the current thread and then send 'buffer'.
 */

void 
send_msg(RemoteJNIEnv* env, void *buffer, int length) {
  message_counter++;
  void* p = env->GetPipe();
  
  if (length < 32) {
    int len = 32;
    char *to_send = (char *) malloc(36);
    memcpy(to_send, &len, 4);
    memcpy(to_send + 4, buffer, length);
    /* print_send(to_send, 36);*/
    write_JD_fully("send_msg", p, to_send, 36);
    free(to_send);
  } else {
    char *to_send = (char *) malloc(length + 4);
    memcpy(to_send, &length, 4);
    memcpy(to_send + 4, buffer, length);
    write_JD_fully("send_msg", p, to_send, length + 4);
    /* print_send(to_send, length + 4); */
    free(to_send);
  }
  
    /*
    write_JD_fully("send_msg", p, (char *) &length, 4);
    write_JD_fully("send_msg", p, (char *) buffer, length);
    */

}


/* Trace the buffer */
void trace_buffer(char* msg, char* buf, int nbytes) {
    trace("%s :[%d]\n", msg, nbytes);
    for(int i = 0; i < nbytes; i++, buf++) {
	if (*buf == 0) trace("\0");
	else trace("<%d>", (int) (*buf));
    } 
    trace("\n");
}


/* Get a message on the per-thread pipe associated with env */
int get_msg(RemoteJNIEnv *env, void *buffer, int length) {
  
    int rv;
    void* p = env->GetPipe();
    struct pollfd fds[2];
    
    int fd1 = g_unixService->JDFileDesc_To_FD(p);

    if (length == 0) return 0;

    JavaPluginFactory5* plugin_factory = get_global_factory();
    JavaVM5* jvm = plugin_factory->GetJavaVM();

    if (jvm == NULL) return 0;
    
    void* op  = jvm->GetWorkPipe();
    int wp = g_unixService->JDFileDesc_To_FD(op);

    if(wp >= 0) {
        fds[0].fd = fd1;
        fds[0].events = POLLRDNORM;
        fds[1].fd = wp;
        fds[1].events = POLLRDNORM;
    
        for(;;) {
            fds[0].revents = 0;
            fds[1].revents = 0;
            rv = poll(fds,2,-1);
            if(rv == -1) {
                // Deal with error case here
                if(errno != EINTR) {
                    break;
                }
            } else {
                // Make sure we are out of poll for the right reason 
                // and handle the worker pipe if need be
                if(fds[0].revents & POLLRDNORM) {
                    break;
                }
                if(fds[1].revents & POLLRDNORM) {
                    jvm->ProcessWorkQueue();
                }
            }
        }
    } else {

    }

    read_JD_fully("get_msg", p, (char *) buffer, length);

    return 0;
}


/* Read a result from the env's pipe that is of a specified type
   into the right field of the jvalue union */
void
get_result_of_type(RemoteJNIEnv* env, jd_jni_type result_type, jvalue* result) {
    

    switch (result_type) {
    case jd_jboolean_type: {
	get_msg(env, &(result->z), sizeof(jboolean)); 
	break;
    }
    case jd_jbyte_type:  get_msg(env, &(result->b), sizeof(jbyte)); break;
    case jd_jchar_type:  get_msg(env, &(result->c), sizeof(jchar)); break;
    case jd_jshort_type:  get_msg(env, &(result->s), sizeof(jshort)); break;
    case jd_jint_type:  get_msg(env, &(result->i), sizeof(jint)); break;
    case jd_jlong_type:  get_msg(env, &(result->j), sizeof(jlong)); break;
    case jd_jfloat_type:  get_msg(env, &(result->f), sizeof(jfloat)); break;
    case jd_jdouble_type:  { 
      get_msg(env, &(result->d), sizeof(jdouble)); 
      //fprintf(stderr, " remotejni: double %f\n", (float) result->d);
      break;
    }
    case jd_jobject_type:  get_msg(env, &(result->l), sizeof(jobject)); break;
	/* Handle a void like nothing */
    case jd_jvoid_type:  break; 
    default: 
	plugin_error("Unknown return type in SecureCallMethod");
    }
}


/* Get the JNI version from the JVM */
extern "C" jint JNICALL
jni_GetVersion(RemoteJNIEnv *env)
{
    jint result;
    int code = JAVA_PLUGIN_JNI_VERSION;
    send_msg(env, &code, 4);
    get_msg(env, &result, 4);
    return result;
}

/* Defining a class requires us to send the class data to the VM iself. The
   args need to be marshalled properly. */
extern "C" jclass JNICALL
jni_DefineClass(RemoteJNIEnv *env, const char *name, jobject loaderRef,
		const jbyte *buf, jsize bufLen)
{
    int code = JAVA_PLUGIN_DEFINE_CLASS;
    char *msg;
    jclass retval;
    short length1 = slen(name);
    msg = (char *) checked_malloc(length1 + bufLen + 14);
    memcpy(msg, &code, 4);
    memcpy(msg+4, &length1, 2);
    memcpy(msg+6, name, length1);
    memcpy(msg+6+length1, &loaderRef, 4);
    memcpy(msg+10+length1, &bufLen, 4);
    memcpy(msg+14+length1, buf, bufLen);
    send_msg(env, msg, length1 + bufLen + 14);
    /* we can potentially get called here although I don't know how - Rob*/
    free(msg);
    handle_response(env);
    get_msg(env, &retval, 4);
    return retval;
}


extern "C" jclass JNICALL
jni_FindClass(RemoteJNIEnv *env, const char *name)
{
    int code = JAVA_PLUGIN_FIND_CLASS;
    char *msg;
    jclass retval;
    short length1 = slen(name);
    TRACE_ENV_STR(env, "FindClass", name);
    msg = (char* )checked_malloc(length1 + 6);
    memcpy(msg, &code, 4);
    memcpy(msg+4, &length1, 2);
    memcpy(msg+6, name, length1);
    send_msg(env, msg, length1 + 6);
    free(msg);

    /* we can potentially get called here although I don't know how - Rob*/
    handle_response(env);
    get_msg(env, &retval, 4);
    TRACE_ENV_STR(env, "FindClass done", name);
    return retval;
}



extern "C" jclass JNICALL
jni_GetSuperclass(RemoteJNIEnv *env, jclass sub)
{
    int code = JAVA_PLUGIN_GET_SUPER_CLASS;
    char msg[8];
    jclass retval;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &sub, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" jboolean JNICALL
jni_IsSubclassOf(RemoteJNIEnv *env, jclass sub, jclass super)
{
    int code = JAVA_PLUGIN_IS_SUBCLASS_OF;
    char msg[12];
    jboolean retval;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &sub, 4);
    memcpy(msg+8, &super, 4);
    send_msg(env, msg, 12);
    get_msg(env, &retval, sizeof(jboolean));
    return retval;
}

extern "C" jclass JNICALL
jni_GetObjectClass(RemoteJNIEnv *env, jobject obj)
{
    int code = JAVA_PLUGIN_GET_OBJECT_CLASS;
    char msg[8];
    jclass retval;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &obj, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" jboolean JNICALL
jni_IsInstanceOf(RemoteJNIEnv *env, jobject obj, jclass super)
{
    int code = JAVA_PLUGIN_IS_INSTANCE_OF;
    char msg[12];
    jboolean retval;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &obj, 4);
    memcpy(msg+8, &super, 4);
    send_msg(env, msg, 12);
    get_msg(env, &retval, sizeof(jboolean));
    return retval;
}

extern "C" jboolean JNICALL
jni_IsSameObject(RemoteJNIEnv *env, jobject obj, jobject obj1)
{
    int code = JAVA_PLUGIN_IS_SAME_OBJECT;
    char msg[12];
    jboolean retval;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &obj, 4);
    memcpy(msg+8, &obj1, 4);
    send_msg(env, msg, 12);
    get_msg(env, &retval, sizeof(jboolean));
    return retval;
}

extern "C" jobject JNICALL
jni_NewGlobalRef(RemoteJNIEnv *env, jobject ref)
{
    int code = JAVA_PLUGIN_NEW_GLOBAL_REF;
    char msg[8];
    jobject retval;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &ref, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;
}
    
extern "C" void JNICALL
jni_DeleteGlobalRef(RemoteJNIEnv *env, jobject ref)
{
    int code = JAVA_PLUGIN_RELEASE_GLOBAL_REF;
    char msg[8];
    if (ref) {
	memcpy(msg, &code, 4);
	memcpy(msg+4, &ref, 4);
	send_msg(env, msg, 8);
	handle_response(env);
    }
}

extern "C" void JNICALL
jni_DeleteLocalRef(RemoteJNIEnv *env, jobject ref)
{
    int code = JAVA_PLUGIN_RELEASE_LOCAL_REF;
    char msg[8];
    if (ref) {
	memcpy(msg, &code, 4);
	memcpy(msg+4, &ref, 4);
	send_msg(env, msg, 8);
	handle_response(env);
    }
}



extern "C" jint JNICALL
jni_Throw(RemoteJNIEnv *env, jthrowable obj)
{
    trace("remotejni:jni_Throw()\n");
    int code = JAVA_PLUGIN_THROW;
    char msg[8];
    jint retval;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &obj, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" jint JNICALL
jni_ThrowNew(RemoteJNIEnv *env, jclass clazz, const char *message)
{
    trace("remotejni:jni_ThrowNew()\n");
    char *msg;
    short length;
    int code = JAVA_PLUGIN_THROW_NEW;
    jint retval;
    length = slen(message);
    msg = (char *) checked_malloc(10+length);
    memcpy(msg, &code, 4);
    memcpy(msg+4, &clazz, 4);
    memcpy(msg+8, &length, 2);
    memcpy(msg+10, message, length);
    send_msg(env, msg, 10+length);
    free(msg);
    handle_response(env);
    get_msg(env, &retval, 4);
    return retval;
}


extern "C" jthrowable JNICALL
jni_ExceptionOccurred(RemoteJNIEnv *env)
{
    int code = JAVA_PLUGIN_EXCEPTION_OCCURED;
    char resp[8];
    int testval = 0;

    jthrowable retval = NULL;

    send_msg(env, &code, 4);

    get_msg(env, resp, 8);
    memcpy(&retval, resp, 4);
    memcpy(&testval, resp + 4, 4);
    
    return retval;
}

extern "C" void JNICALL
jni_ExceptionDescribe(RemoteJNIEnv *env)
{
    int code =JAVA_PLUGIN_EXCEPTION_DESCRIBE;
    send_msg(env, &code, 4);
}

extern "C" void JNICALL
jni_ExceptionClear(RemoteJNIEnv *env)
{
    int code =JAVA_PLUGIN_EXCEPTION_CLEAR;
    send_msg(env, &code, 4);
    /* No need for a return value, but performance is immensely improved by
       following a strict call-response protocol. 
    */
    handle_response(env); 
}

extern "C" void JNICALL
jni_FatalError(RemoteJNIEnv *env, const char *message)
{
    char *msg;
    short length;
    int code = JAVA_PLUGIN_FATAL_ERROR;
    length = slen(message);
    msg = (char *) checked_malloc(6+length);
    memcpy(msg, &code, 4);
    memcpy(msg+4, &length, 2);
    memcpy(msg+6, message, length);
    send_msg(env, msg, 10+length);
    free(msg);
    exit(1);
}



/* GetFieldID marshals the strings, sends them to the VM and gets back a
   fieldID. */

extern "C" jfieldID JNICALL
jni_GetFieldID(RemoteJNIEnv *env, jclass clazz, 
	       const char *name, const char *sig) {
    char * msg;
    short length1, length2;
    int code = JAVA_PLUGIN_GET_FIELD_ID;
    jfieldID retval;
    length1 = slen(name);
    length2 = slen(sig);
    
    msg = (char *) checked_malloc(12+length1 +length2);
    memcpy(msg, &code, 4);
    memcpy(msg+4, &clazz, 4);
    memcpy(msg+8, &length1, 2);
    memcpy(msg+10, name, length1);
    memcpy(msg+10+length1, &length2, 2);
    memcpy(msg+12+length1, sig, length2);
    send_msg(env, msg, 12+length1 +length2);
    free(msg);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" jfieldID JNICALL
jni_GetStaticFieldID(RemoteJNIEnv *env, jclass clazz, 
	       const char *name, const char *sig) {
    char * msg;
    short length1, length2;
    int code = JAVA_PLUGIN_GET_STATIC_FIELD_ID;
    jfieldID retval;
    length1 = slen(name);
    length2 = slen(sig);
    
    msg = (char *) checked_malloc(12+length1 +length2);
    memcpy(msg, &code, 4);
    memcpy(msg+4, &clazz, 4);
    memcpy(msg+8, &length1, 2);
    memcpy(msg+10, name, length1);
    memcpy(msg+10+length1, &length2, 2);
    memcpy(msg+12+length1, sig, length2);
    send_msg(env, msg, 12+length1 +length2);
    free(msg);
    get_msg(env, &retval, 4);
    return retval;
}

/*Get<Modifiers>Field() is very simple. We send the parameters, and get back
  wither a primitive, or an opaque reference. That's cool. */



#define DEFINE_GETFIELD(ResultType, Result, base)			\
extern "C" ResultType JNICALL						\
jni_Get##Result##Field(RemoteJNIEnv *env, jobject obj, jfieldID fieldID)	\
{									\
    char msg[12];							\
    int code = base;							\
    ResultType retval;							\
    memcpy(msg, &code, 4);						\
    memcpy(msg+4, &obj, 4);						\
    memcpy(msg+8, &fieldID, 4);						\
    send_msg(env, msg, 12);				\
    get_msg(env, &(retval), sizeof(ResultType));	\
    return retval;							\
}

DEFINE_GETFIELD(jobject, Object, JAVA_PLUGIN_GET_OBJECT_FIELD)
DEFINE_GETFIELD(jboolean, Boolean, JAVA_PLUGIN_GET_BOOLEAN_FIELD)
DEFINE_GETFIELD(jbyte, Byte, JAVA_PLUGIN_GET_BYTE_FIELD)
DEFINE_GETFIELD(jchar, Char, JAVA_PLUGIN_GET_CHAR_FIELD)
DEFINE_GETFIELD(jshort, Short, JAVA_PLUGIN_GET_SHORT_FIELD)
DEFINE_GETFIELD(jint, Int, JAVA_PLUGIN_GET_INT_FIELD)
DEFINE_GETFIELD(jlong, Long, JAVA_PLUGIN_GET_LONG_FIELD)
DEFINE_GETFIELD(jfloat, Float, JAVA_PLUGIN_GET_FLOAT_FIELD)
DEFINE_GETFIELD(jdouble, Double, JAVA_PLUGIN_GET_DOUBLE_FIELD)

#define DEFINE_GETSTATICFIELD(ResultType, Result, base)			\
extern "C" ResultType JNICALL						\
jni_GetStatic##Result##Field(RemoteJNIEnv *env, jclass cls, jfieldID fieldID)	\
{									\
    char msg[12];							\
    int code = base;							\
    ResultType retval;							\
    memcpy(msg, &code, 4);						\
    memcpy(msg+4, &cls, 4);						\
    memcpy(msg+8, &fieldID, 4);						\
    send_msg(env, msg, 12);				\
    get_msg(env, &(retval), sizeof(ResultType));	\
    return retval;							\
}

DEFINE_GETSTATICFIELD(jobject, Object, JAVA_PLUGIN_GET_STATIC_OBJECT_FIELD)
DEFINE_GETSTATICFIELD(jboolean, Boolean, JAVA_PLUGIN_GET_STATIC_BOOLEAN_FIELD)
DEFINE_GETSTATICFIELD(jbyte, Byte, JAVA_PLUGIN_GET_STATIC_BYTE_FIELD)
DEFINE_GETSTATICFIELD(jchar, Char, JAVA_PLUGIN_GET_STATIC_CHAR_FIELD)
DEFINE_GETSTATICFIELD(jshort, Short, JAVA_PLUGIN_GET_STATIC_SHORT_FIELD)
DEFINE_GETSTATICFIELD(jint, Int, JAVA_PLUGIN_GET_STATIC_INT_FIELD)
DEFINE_GETSTATICFIELD(jlong, Long, JAVA_PLUGIN_GET_STATIC_LONG_FIELD)
DEFINE_GETSTATICFIELD(jfloat, Float, JAVA_PLUGIN_GET_STATIC_FLOAT_FIELD)
DEFINE_GETSTATICFIELD(jdouble, Double, JAVA_PLUGIN_GET_STATIC_DOUBLE_FIELD)


    /* AllocObject is rather similar to the Get<Modifier>Field family of
       functions. You send a fixed length message, and you expect a jobject
       response. Furthermore, there is no Java method call involved, so THE
       ONLY thing we can get is the object (i.e. Java will not call to us to
       do something.  */

extern "C" jobject JNICALL
jni_AllocObject(RemoteJNIEnv *env, jclass clazz) {
    char msg[12];							
    int code = JAVA_PLUGIN_ALLOC_OBJECT;
    jobject retval;							
    memcpy(msg, &code, 4);						
    memcpy(msg+4, &clazz, 4);						
    send_msg(env, msg, 8);
    /* read the response code */					
    get_msg(env, &code, 4);				
    if (code != JNI_OK) 						
	return NULL;							
    get_msg(env, &(retval), sizeof(jobject));	
    return retval;							
}

    /*Set<Modifiers>Field() is relatively simple. We package the parameters
      into a message, and send them to the VM. Since this deals with only the
      primitive types ot opaque object references, we do not need to worry
      about pass by value vs. pass by ref. The neat thing is that we don't
      care about the result, so we just write our message, and go play
      somewhere else.*/

#define DEFINE_SETFIELD(ParamType, Param, base) 			\
extern "C" void JNICALL							\
jni_Set##Param##Field(RemoteJNIEnv *env, jobject obj, jfieldID fieldID, 	\
			  ParamType value) {				\
    char msg[16];							\
    int code = base;							\
    memcpy(msg, &code, 4);						\
    memcpy(msg+4, &obj, 4);						\
    memcpy(msg+8, &fieldID,4);						\
    memcpy(msg+12, &value, sizeof(ParamType));				\
    send_msg(env, msg, 12+sizeof(ParamType));	\
    /* response is not necessary */					\
}

DEFINE_SETFIELD(jobject, Object, JAVA_PLUGIN_SET_OBJECT_FIELD)
DEFINE_SETFIELD(jboolean, Boolean, JAVA_PLUGIN_SET_BOOLEAN_FIELD)
DEFINE_SETFIELD(jbyte, Byte, JAVA_PLUGIN_SET_BYTE_FIELD)
DEFINE_SETFIELD(jchar, Char, JAVA_PLUGIN_SET_CHAR_FIELD)
DEFINE_SETFIELD(jshort, Short, JAVA_PLUGIN_SET_SHORT_FIELD)
DEFINE_SETFIELD(jint, Int, JAVA_PLUGIN_SET_INT_FIELD)
DEFINE_SETFIELD(jlong, Long, JAVA_PLUGIN_SET_LONG_FIELD)
DEFINE_SETFIELD(jfloat, Float, JAVA_PLUGIN_SET_FLOAT_FIELD)
DEFINE_SETFIELD(jdouble, Double, JAVA_PLUGIN_SET_DOUBLE_FIELD)

#define DEFINE_SETSTATICFIELD(ParamType, Param, base) 			\
extern "C" void JNICALL							\
jni_SetStatic##Param##Field(RemoteJNIEnv *env, jclass cls, jfieldID fieldID, 	\
			  ParamType value) {				\
    char msg[16];							\
    int code = base;							\
    memcpy(msg, &code, 4);						\
    memcpy(msg+4, &cls, 4);						\
    memcpy(msg+8, &fieldID,4);						\
    memcpy(msg+12, &value, sizeof(ParamType));				\
    send_msg(env, msg, 12+sizeof(ParamType));	\
    /* response is not necessary */					\
}

DEFINE_SETSTATICFIELD(jobject, Object, JAVA_PLUGIN_SET_STATIC_OBJECT_FIELD)
DEFINE_SETSTATICFIELD(jboolean, Boolean, JAVA_PLUGIN_SET_STATIC_BOOLEAN_FIELD)
DEFINE_SETSTATICFIELD(jbyte, Byte, JAVA_PLUGIN_SET_STATIC_BYTE_FIELD)
DEFINE_SETSTATICFIELD(jchar, Char, JAVA_PLUGIN_SET_STATIC_CHAR_FIELD)
DEFINE_SETSTATICFIELD(jshort, Short, JAVA_PLUGIN_SET_STATIC_SHORT_FIELD)
DEFINE_SETSTATICFIELD(jint, Int, JAVA_PLUGIN_SET_STATIC_INT_FIELD)
DEFINE_SETSTATICFIELD(jlong, Long, JAVA_PLUGIN_SET_STATIC_LONG_FIELD)
DEFINE_SETSTATICFIELD(jfloat, Float, JAVA_PLUGIN_SET_STATIC_FLOAT_FIELD)
DEFINE_SETSTATICFIELD(jdouble, Double, JAVA_PLUGIN_SET_STATIC_DOUBLE_FIELD)


/* we will redefine jmethodID: it will be:
   struct jmethodID_ {
   char *signature;
   int java_method;
   }
   typedef struct jmethodID * jmethodID;

   the RPC message (JNI->Java) looks like this:

   --------------------------------------------------------------------
   | 4bytes | .... variable ........ |    4bytes     | ..variable.... |
   --------------------------------------------------------------------
   | action | known fixed parameters | # var params  | array of jvals |
   --------------------------------------------------------------------

   We need the signature to figure out the size of the argument list for all
   the possible variants of methods. The getMethodID procedure needs to be
   modified accordingly. By convention we are going to translate
   all the method variants into Call<Modifiers>MethodA();
   Java process does not need do any parameter conversion.
   */


/* Compress the resulting signature down to a character per arg */
char *create_signature(const char *sig) {
    int len;
    char *retval;
    char *src, *dest;
    len = slen(sig);
    dest = retval = (char *) checked_malloc(len);
    src = (char *)sig;
    while(*src) {
	switch (*src) {
	case '(':
	    /* Ignore parens */
	    src++;
	    break;
	case '[':
	    /* If an array, then just use the character L for object */
	    if ((src[1] != '[') && (src[1] != 'L')) {
		*dest++ = 'L';
		src++;
	    }
	    src++;
	    break;

	case 'L':
	    /* Omit the actual class name */
	    *dest++ = *src++;
	    while (*src++ != ';');
	    break;
	case ')':
	    /* Ignore parens */
	    *dest = 0;
	    return retval;
	default:
	    *dest++ = *src++;
	    break;
	}
    }
    free(retval);
    if (tracing) trace(" Sig %s-> NULL\n", sig);
    return NULL;
}

extern "C" jmethodID JNICALL
jni_GetMethodID(RemoteJNIEnv *env, jclass clazz, 
		const char *name, const char *sig) {
    char *msg;
    short length1, length2;
    int code;
    jmethodID retval;
    code = JAVA_PLUGIN_GET_METHOD_ID;
    length1 = slen(name);
    length2 = slen(sig);
    msg = (char *) checked_malloc(length1 + length2 + 12);
    memcpy(msg, &code, 4);
    memcpy(msg+4, &clazz, 4);
    memcpy(msg+8, &length1, 2);
    memcpy(msg+10, name, length1);
    memcpy(msg+10+length1, &length2, 2);
    memcpy(msg+12+length1, sig, length2);
    send_msg(env, msg, length1 + length2 + 12);
    free(msg);

    /* read the response code */
    retval = (jmethodID) checked_malloc(sizeof(struct _jmethodID));
    get_msg(env, &(retval->java_method), 4);
    if (retval->java_method == NULL) {
	    free(retval);
	    return NULL;
    }
    /* Store the signature for future reference */
    retval->signature = create_signature(sig);
    return retval;
}

extern "C" jmethodID JNICALL
jni_GetStaticMethodID(RemoteJNIEnv *env, jclass clazz, 
		const char *name, const char *sig) {
    char *msg;
    short length1, length2;
    int code;
    jmethodID retval;
    code = JAVA_PLUGIN_GET_STATIC_METHOD_ID;
    length1 = slen(name);
    length2 = slen(sig);
    msg = (char *) checked_malloc(length1 + length2 + 12);
    memcpy(msg, &code, 4);
    memcpy(msg+4, &clazz, 4);
    memcpy(msg+8, &length1, 2);
    memcpy(msg+10, name, length1);
    memcpy(msg+10+length1, &length2, 2);
    memcpy(msg+12+length1, sig, length2);
    send_msg(env, msg, length1 + length2 + 12);
    /* read the response code */
    free(msg);
    retval = (jmethodID) checked_malloc(sizeof(struct _jmethodID));
    get_msg(env, &(retval->java_method), 4);
    if (retval->java_method == NULL) {
	    free(retval);
	    return NULL;
    }
    retval->signature = create_signature(sig);
    return retval;
}

#define VALIST_TO_JVALS(sig, args, ptr) 			\
for (; *sig != 0; sig++, ptr += sizeof(jvalue)) {		\
	jvalue *ptr1 = (jvalue*) ptr;                           \
	switch(*sig) {						\
	case 'Z':						\
	    ptr1->z = (jboolean)va_arg(args, jint);		\
	    break;						\
	case 'B':						\
	    ptr1->b = (jbyte)va_arg(args, jint);		\
	    break;						\
	case 'C':						\
	    ptr1->c = (jchar)va_arg(args, jint);		\
	    break;						\
	case 'S':						\
	    ptr1->s = (jshort)va_arg(args, jint);		\
	    break;						\
	case 'I':						\
	    ptr1->i = va_arg(args, jint);			\
	    break;						\
	case 'J':						\
	    ptr1->j = va_arg(args, jlong);			\
	    break;						\
	case 'F':						\
	    ptr1->f = (jfloat)va_arg(args, jdouble);		\
	    break;						\
	case 'D':						\
	    ptr1->d = va_arg(args, jdouble);			\
	    break;						\
	case 'L':						\
	    ptr1->l = va_arg(args, jobject);			\
	    break;						\
	default:						\
	    fprintf(stderr, "Invalid signature: %s\n", sig);	\
	    exit(JNI_EINVAL);					\
	}                                                       \
    }


void argarr_to_jvals(jvalue* args, int nargs, char* ptr) {
    int i = 0;  
    for (; i < nargs; i++, ptr += sizeof(jvalue)) {	
         //jvalue *ptr1 = (jvalue*) ptr;               
	 memcpy(ptr, &(args[i]), sizeof(jvalue));
    }
}




extern "C" void
jni_CallVoidMethod(RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...) { 
    int code = JAVA_PLUGIN_CALL_VOID_METHOD;
    int nargs;								      
    char * ss, *msg;						      
    char * ptr;								      
    va_list args;							      
    va_start(args, methodID);						      
    
    ss = GET_SIGNATURE(methodID);					      
    nargs = slen(ss);							      
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      
    memcpy(msg, &code, 4);						      
    memcpy(msg+4, &obj, 4);						      
    memcpy(msg+8, &(methodID->java_method), 4);				      
    memcpy(msg+12, &nargs, 4);						      
    ptr = &(msg[16]);							      
    VALIST_TO_JVALS(ss, args, ptr);					      
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      
    handle_response(env);                              
    free(msg);								      
    va_end(args);							      
}									      
extern "C" void 							      
jni_CallVoidMethodV(RemoteJNIEnv *env, jobject obj, jmethodID methodID,       
			 va_list args) {				      
    int code = JAVA_PLUGIN_CALL_VOID_METHOD;
    int nargs;								      
    char * ss, *msg;						      
    char * ptr;								      
    ss = GET_SIGNATURE(methodID);					      
    nargs = slen(ss);							      
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      
    memcpy(msg, &code, 4);						      
    memcpy(msg+4, &obj, 4);						      
    memcpy(msg+8, &(methodID->java_method), 4);				      
    memcpy(msg+12, &nargs, 4);						      
    ptr = &(msg[16]);							      
    VALIST_TO_JVALS(ss, args, ptr);					      
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      
    handle_response(env);                              
    free(msg);								      
}									      
									      
extern "C" void
jni_CallVoidMethodA(RemoteJNIEnv *env, jobject obj, jmethodID methodID,       
			  jvalue *args) {				      
    int code = JAVA_PLUGIN_CALL_VOID_METHOD;
    int nargs;								      
    char * ss, *msg;						      
    char * ptr;								      
    ss = GET_SIGNATURE(methodID);					      
    nargs = slen(ss);							      
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      
    memcpy(msg, &code, 4);						      
    memcpy(msg+4, &obj, 4);						      
    memcpy(msg+8, &(methodID->java_method), 4);				      
    memcpy(msg+12, &nargs, 4);						      
    ptr = &(msg[16]);							      
    memcpy(ptr, args, nargs*sizeof(jvalue));				      
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      
    handle_response(env);                              
    free(msg);								      
}

#define DEFINE_CALLMETHOD(ResultType, Result, baseNum) 			      \
extern "C" ResultType 							      \
jni_Call##Result##Method(RemoteJNIEnv *env, jobject obj, jmethodID methodID, ...) { \
    ResultType result;							      \
    int code = baseNum;							      \
    int nargs;								      \
    char * ss, *msg;							      \
    char * ptr;								      \
    va_list args;							      \
    va_start(args, methodID);						      \
    									      \
     ss = GET_SIGNATURE(methodID);					      \
    nargs = slen(ss);							      \
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      \
    memcpy(msg, &code, 4);						      \
    memcpy(msg+4, &obj, 4);						      \
    memcpy(msg+8, &(methodID->java_method), 4);				      \
    memcpy(msg+12, &nargs, 4);						      \
    ptr = &(msg[16]);							      \
    VALIST_TO_JVALS(ss, args, ptr);					      \
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      \
    handle_response(env);                              \
    get_msg(env, &result, sizeof(ResultType));	      \
    free(msg);								      \
    va_end(args);							      \
    return result;							      \
}									      \
extern "C" ResultType 							      \
jni_Call##Result##MethodV(RemoteJNIEnv *env, jobject obj, jmethodID methodID,       \
			 va_list args) {				      \
    ResultType result;							      \
    int code = baseNum;							      \
    int nargs;								      \
    char * ss, *msg;							      \
    char * ptr;								      \
    ss = GET_SIGNATURE(methodID);					      \
    nargs = slen(ss);							      \
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      \
    memcpy(msg, &code, 4);						      \
    memcpy(msg+4, &obj, 4);						      \
    memcpy(msg+8, &(methodID->java_method), 4);				      \
    memcpy(msg+12, &nargs, 4);						      \
    ptr = &(msg[16]);							      \
    VALIST_TO_JVALS(ss, args, ptr);					      \
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      \
    handle_response(env);                              \
    get_msg(env, &result, sizeof(ResultType));	      \
    free(msg);								      \
    return result;							      \
}									      \
									      \
extern "C" ResultType 							      \
jni_Call##Result##MethodA(RemoteJNIEnv *env, jobject obj, jmethodID methodID,       \
			  jvalue *args) {				      \
    ResultType result;							      \
    int code = baseNum;							      \
    int nargs;								      \
    char * ss, *msg;							      \
    char * ptr;								      \
    ss = GET_SIGNATURE(methodID);					      \
    nargs = slen(ss);							      \
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      \
    memcpy(msg, &code, 4);						      \
    memcpy(msg+4, &obj, 4);						      \
    memcpy(msg+8, &(methodID->java_method), 4);				      \
    memcpy(msg+12, &nargs, 4);						      \
    ptr = &(msg[16]);							      \
    memcpy(ptr, args, nargs*sizeof(jvalue));				      \
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      \
    handle_response(env);                              \
    get_msg(env, &result, sizeof(ResultType));	      \
    free(msg);								      \
    return result;							      \
}

DEFINE_CALLMETHOD(jobject, Object, JAVA_PLUGIN_CALL_OBJECT_METHOD)
DEFINE_CALLMETHOD(jboolean, Boolean, JAVA_PLUGIN_CALL_BOOLEAN_METHOD)
DEFINE_CALLMETHOD(jbyte, Byte, JAVA_PLUGIN_CALL_BYTE_METHOD)
DEFINE_CALLMETHOD(jchar, Char, JAVA_PLUGIN_CALL_CHAR_METHOD)
DEFINE_CALLMETHOD(jshort, Short, JAVA_PLUGIN_CALL_SHORT_METHOD)
DEFINE_CALLMETHOD(jint, Int, JAVA_PLUGIN_CALL_INT_METHOD)
DEFINE_CALLMETHOD(jlong, Long, JAVA_PLUGIN_CALL_LONG_METHOD)
DEFINE_CALLMETHOD(jfloat, Float, JAVA_PLUGIN_CALL_FLOAT_METHOD)
DEFINE_CALLMETHOD(jdouble, Double, JAVA_PLUGIN_CALL_DOUBLE_METHOD)

#define DEFINE_CALLNONVIRTUALMETHOD(ResultType, Result, baseNum)	      \
extern "C" ResultType 							      \
jni_CallNonvirtual##Result##Method(RemoteJNIEnv *env, jobject obj,		      \
				   jclass clazz, jmethodID methodID, ...) {   \
    ResultType result;							      \
    int code = baseNum;							      \
    int nargs;								      \
    char * ss, *msg;							      \
    char * ptr;								      \
    va_list args;							      \
    va_start(args, methodID);						      \
    									      \
    ss = GET_SIGNATURE(methodID);					      \
    nargs = slen(ss);							      \
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 20);				      \
    memcpy(msg, &code, 4);						      \
    memcpy(msg+4, &obj, 4);						      \
    memcpy(msg+8, &clazz, 4);						      \
    memcpy(msg+12,&(methodID->java_method), 4);				      \
    memcpy(msg+16, &nargs, 4);						      \
    ptr = &(msg[20]);							      \
    VALIST_TO_JVALS(ss, args, ptr);					      \
    send_msg(env, msg, nargs*sizeof(jvalue) + 20);	      \
    handle_response(env);                              \
    get_msg(env, &result, sizeof(ResultType));	      \
    free(msg);								      \
    va_end(args);							      \
    return result;							      \
}									      \
extern "C" ResultType 							      \
jni_CallNonvirtual##Result##MethodV(RemoteJNIEnv *env, jobject obj,              \
				    jclass clazz, jmethodID methodID,       \
			 va_list args) {				      \
    ResultType result;							      \
    int code = baseNum;							      \
    int nargs;								      \
    char * ss, *msg;							      \
    char * ptr;								      \
    ss = GET_SIGNATURE(methodID);					      \
    nargs = slen(ss);							      \
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 20);				      \
    memcpy(msg, &code, 4);						      \
    memcpy(msg+4, &obj, 4);						      \
    memcpy(msg+8, &clazz, 4);						      \
    memcpy(msg+12,&(methodID->java_method), 4);				      \
    memcpy(msg+16, &nargs, 4);						      \
    ptr = &(msg[20]);							      \
    VALIST_TO_JVALS(ss, args, ptr);					      \
    send_msg(env, msg, nargs*sizeof(jvalue) + 20);	      \
    handle_response(env);                              \
    get_msg(env, &result, sizeof(ResultType));	      \
    free(msg);								      \
    return result;							      \
}									      \
									      \
extern "C" ResultType 							      \
jni_CallNonvirtual##Result##MethodA(RemoteJNIEnv *env, jobject obj, jclass clazz,   \
				    jmethodID methodID, jvalue *args) {	      \
    ResultType result;							      \
    int code = baseNum;							      \
    int nargs;								      \
    char * ss, *msg;							      \
    char * ptr;								      \
    ss = GET_SIGNATURE(methodID);					      \
    nargs = slen(ss);							      \
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 20);				      \
    memcpy(msg, &code, 4);						      \
    memcpy(msg+4, &obj, 4);						      \
    memcpy(msg+8, &clazz, 4);						      \
    memcpy(msg+12,&(methodID->java_method), 4);				      \
    memcpy(msg+16, &nargs, 4);						      \
    ptr = &(msg[20]);							      \
    memcpy(ptr, args, nargs*sizeof(jvalue));				      \
    send_msg(env, msg, nargs*sizeof(jvalue) + 20);	      \
    handle_response(env);                              \
    get_msg(env, &result, sizeof(ResultType));	      \
    free(msg);								      \
    return result;							      \
}


DEFINE_CALLNONVIRTUALMETHOD(jobject, Object, JAVA_PLUGIN_CALL_NV_OBJECT_METHOD)
DEFINE_CALLNONVIRTUALMETHOD(jboolean, Boolean, JAVA_PLUGIN_CALL_NV_BOOLEAN_METHOD)
DEFINE_CALLNONVIRTUALMETHOD(jbyte, Byte, JAVA_PLUGIN_CALL_NV_BYTE_METHOD)
DEFINE_CALLNONVIRTUALMETHOD(jchar, Char, JAVA_PLUGIN_CALL_NV_CHAR_METHOD)
DEFINE_CALLNONVIRTUALMETHOD(jshort, Short, JAVA_PLUGIN_CALL_NV_SHORT_METHOD)
DEFINE_CALLNONVIRTUALMETHOD(jint, Int, JAVA_PLUGIN_CALL_NV_INT_METHOD)
DEFINE_CALLNONVIRTUALMETHOD(jlong, Long, JAVA_PLUGIN_CALL_NV_LONG_METHOD)
DEFINE_CALLNONVIRTUALMETHOD(jfloat, Float, JAVA_PLUGIN_CALL_NV_FLOAT_METHOD)
DEFINE_CALLNONVIRTUALMETHOD(jdouble, Double, JAVA_PLUGIN_CALL_NV_DOUBLE_METHOD)

extern "C" void
jni_CallNonvirtualVoidMethod(RemoteJNIEnv *env, jobject obj,		      
				   jclass clazz, jmethodID methodID, ...) {   
    int code = JAVA_PLUGIN_CALL_NV_VOID_METHOD;
    int nargs;								      
    char * ss, *msg;							      
    char * ptr;								      
    va_list args;							      
    va_start(args, methodID);						      
    									      
    ss = GET_SIGNATURE(methodID);					      
    nargs = slen(ss);							      
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 20);				      
    memcpy(msg, &code, 4);						      
    memcpy(msg+4, &obj, 4);						      
    memcpy(msg+8, &clazz, 4);						      
    memcpy(msg+12,&(methodID->java_method), 4);				      
    memcpy(msg+16, &nargs, 4);						      
    ptr = &(msg[20]);							      
    VALIST_TO_JVALS(ss, args, ptr);					      
    send_msg(env, msg, nargs*sizeof(jvalue) + 20);	      
    handle_response(env);                              
    free(msg);								      
    va_end(args);							      
}									      
extern "C" void
jni_CallNonvirtualVoidMethodV(RemoteJNIEnv *env, jobject obj,              
				    jclass clazz, jmethodID methodID,       
			 va_list args) {				      
    int code = JAVA_PLUGIN_CALL_NV_VOID_METHOD;
    int nargs;								      
    char * ss, *msg;						      
    char * ptr;								      
    ss = GET_SIGNATURE(methodID);					      
    nargs = slen(ss);							      
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 20);				      
    memcpy(msg, &code, 4);						      
    memcpy(msg+4, &obj, 4);						      
    memcpy(msg+8, &clazz, 4);						      
    memcpy(msg+12,&(methodID->java_method), 4);				      
    memcpy(msg+16, &nargs, 4);						      
    ptr = &(msg[20]);							      
    VALIST_TO_JVALS(ss, args, ptr);					      
    send_msg(env, msg, nargs*sizeof(jvalue) + 20);	      
    handle_response(env);                              
    free(msg);								      
}									      
									      
extern "C" void
jni_CallNonvirtualVoidMethodA(RemoteJNIEnv *env, jobject obj, jclass clazz,
			      jmethodID methodID,jvalue *args) {
    int code = JAVA_PLUGIN_CALL_NV_VOID_METHOD;
    int nargs;								      
    char * ss, *msg;						      
    char * ptr;								      
    ss = GET_SIGNATURE(methodID);					      
    nargs = slen(ss);							      
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 20);				      
    memcpy(msg, &code, 4);						      
    memcpy(msg+4, &obj, 4);						      
    memcpy(msg+8, &clazz, 4);						      
    memcpy(msg+12,&(methodID->java_method), 4);				      
    memcpy(msg+16, &nargs, 4);						      
    ptr = &(msg[20]);							      
    memcpy(ptr, args, nargs*sizeof(jvalue));				      
    send_msg(env, msg, nargs*sizeof(jvalue) + 20);	      
    handle_response(env);                              
    free(msg);								      
}


#define DEFINE_CALLSTATICMETHOD(ResultType, Result, baseNum) 		      \
extern "C" ResultType 							      \
jni_CallStatic##Result##Method(RemoteJNIEnv *env, jclass obj, jmethodID methodID, ...) { \
    ResultType result;							      \
    int code = baseNum;							      \
    int nargs;								      \
    char * ss, *msg;							      \
    char * ptr;								      \
    va_list args;							      \
    va_start(args, methodID);						      \
    									      \
    ss = GET_SIGNATURE(methodID);					      \
    nargs = slen(ss);							      \
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      \
    memcpy(msg, &code, 4);						      \
    memcpy(msg+4, &obj, 4);						      \
    memcpy(msg+8, &(methodID->java_method), 4);				      \
    memcpy(msg+12, &nargs, 4);						      \
    ptr = &(msg[16]);							      \
    VALIST_TO_JVALS(ss, args, ptr);					      \
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      \
    handle_response(env);                              \
    get_msg(env, &result, sizeof(ResultType));	      \
    free(msg);								      \
    va_end(args);							      \
    return result;							      \
}									      \
extern "C" ResultType 							      \
jni_CallStatic##Result##MethodV(RemoteJNIEnv *env, jclass obj, jmethodID methodID,       \
			 va_list args) {				      \
    ResultType result;							      \
    int code = baseNum;							      \
    int nargs;								      \
    char * ss, *msg;							      \
    char * ptr;								      \
    ss = GET_SIGNATURE(methodID);					      \
    nargs = slen(ss);							      \
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      \
    memcpy(msg, &code, 4);						      \
    memcpy(msg+4, &obj, 4);						      \
    memcpy(msg+8, &(methodID->java_method), 4);				      \
    memcpy(msg+12, &nargs, 4);						      \
    ptr = &(msg[16]);							      \
    VALIST_TO_JVALS(ss, args, ptr);					      \
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      \
    handle_response(env);                              \
    get_msg(env, &result, sizeof(ResultType));	      \
    free(msg);								      \
    return result;							      \
}									      \
									      \
extern "C" ResultType 							      \
jni_CallStatic##Result##MethodA(RemoteJNIEnv *env, jclass obj, jmethodID methodID,       \
			  jvalue *args) {				      \
    ResultType result;							      \
    int code = baseNum;							      \
    int nargs;								      \
    char * ss, *msg;							      \
    char * ptr;								      \
    ss = GET_SIGNATURE(methodID);					      \
    nargs = slen(ss);							      \
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      \
    memcpy(msg, &code, 4);						      \
    memcpy(msg+4, &obj, 4);						      \
    memcpy(msg+8, &(methodID->java_method), 4); 			      \
    memcpy(msg+12, &nargs, 4);						      \
    ptr = &(msg[16]);							      \
    memcpy(ptr, args, nargs*sizeof(jvalue));				      \
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      \
    handle_response(env);                              \
    get_msg(env, &result, sizeof(ResultType));	      \
    free(msg);								      \
    return result;							      \
}

DEFINE_CALLSTATICMETHOD(jobject, Object, JAVA_PLUGIN_CALL_STATIC_OBJECT_METHOD)
DEFINE_CALLSTATICMETHOD(jboolean, Boolean, JAVA_PLUGIN_CALL_STATIC_BOOLEAN_METHOD)
DEFINE_CALLSTATICMETHOD(jbyte, Byte, JAVA_PLUGIN_CALL_STATIC_BYTE_METHOD)
DEFINE_CALLSTATICMETHOD(jchar, Char, JAVA_PLUGIN_CALL_STATIC_CHAR_METHOD)
DEFINE_CALLSTATICMETHOD(jshort, Short, JAVA_PLUGIN_CALL_STATIC_SHORT_METHOD)
DEFINE_CALLSTATICMETHOD(jint, Int, JAVA_PLUGIN_CALL_STATIC_INT_METHOD)
DEFINE_CALLSTATICMETHOD(jlong, Long, JAVA_PLUGIN_CALL_STATIC_LONG_METHOD)
DEFINE_CALLSTATICMETHOD(jfloat, Float, JAVA_PLUGIN_CALL_STATIC_FLOAT_METHOD)
DEFINE_CALLSTATICMETHOD(jdouble, Double, JAVA_PLUGIN_CALL_STATIC_DOUBLE_METHOD)

    /* Object construction is from our point of view the same as calling a
       static method. This is to be contrasted with object allocation which is
       more like getting a value of a field. */
DEFINE_CALLSTATICMETHOD(jobject, NewObject, JAVA_PLUGIN_NEW_OBJECT_METHOD)

extern "C" void 							      
jni_CallStaticVoidMethod(RemoteJNIEnv *env, jclass obj, jmethodID methodID, ...) { 
    int code = JAVA_PLUGIN_CALL_STATIC_VOID_METHOD;	
    int nargs;								      
    char * ss, *msg;							      
    char * ptr;								      
    va_list args;							      
    va_start(args, methodID);						      
    									      
    ss = GET_SIGNATURE(methodID);					      
    nargs = slen(ss);							      
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);		    

    memcpy(msg, &code, 4);						      
    memcpy(msg+4, &obj, 4);						      
    memcpy(msg+8, &(methodID->java_method), 4);

    memcpy(msg+12, &nargs, 4);
    ptr = &(msg[16]);							      
    VALIST_TO_JVALS(ss, args, ptr);					      
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      
    handle_response(env);                              
    free(msg);								      
    va_end(args);							      
}									      
extern "C" void 							      
jni_CallStaticVoidMethodV(RemoteJNIEnv *env, jclass obj, jmethodID methodID,  
			 va_list args) {				      
    int code = JAVA_PLUGIN_CALL_STATIC_VOID_METHOD;	
    int nargs;								      
    char * ss, *msg;							      
    char * ptr;								      
    ss = GET_SIGNATURE(methodID);					      
    nargs = slen(ss);							      
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      
    memcpy(msg, &code, 4);						      
    memcpy(msg+4, &obj, 4);						      
    memcpy(msg+8, &(methodID->java_method), 4);				      
    memcpy(msg+12, &nargs, 4);						      
    ptr = &(msg[16]);							      
    VALIST_TO_JVALS(ss, args, ptr);					      
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      
    free(msg);
    handle_response(env);                              
}									      
									      
extern "C" void 							      
jni_CallStaticVoidMethodA(RemoteJNIEnv *env, jclass obj, jmethodID methodID,       
			  jvalue *args) {				      
    int code = JAVA_PLUGIN_CALL_STATIC_VOID_METHOD;	
    int nargs;								      
    char * ss, *msg;						      
    char * ptr;								      
    ss = GET_SIGNATURE(methodID);					      
    nargs = slen(ss);							      
    msg = (char *) checked_malloc(nargs*sizeof(jvalue) + 16);				      
    memcpy(msg, &code, 4);						      
    memcpy(msg+4, &obj, 4);						      
    memcpy(msg+8, &(methodID->java_method), 4);				      
    memcpy(msg+12, &nargs, 4);						      
    ptr = &(msg[16]);							      
    memcpy(ptr, args, nargs*sizeof(jvalue));				      
    send_msg(env, msg, nargs*sizeof(jvalue) + 16);	      
    handle_response(env);                              
    free(msg);								      
}

/* arrays. Since we're dealing with an out of proc memory, almost necessarily,
   we have to copy arrays. JNI does have some nice features supporting that.
   In this naming convention jni_Capture<modifier>ArrayElements corresponds to
   (*env)->Get<modifier>ArrayElements, and jni_Get<modifier>ArrayElements
   corresponds to (*env)->Get<modifier>ArrayElementsRegion */

/* We're going to save ourselves some of the ipc. Calling GetArrayElements
   causes the elements to be sent to the JNI client, but NOT retained on the
   server side. thus, when releease is called with JNI_ABORT, we don't need to
   make a ipc call. It also makes the design of the server a bit easier: if we
   do want to commit the changes, the server gets the array elements, and
   overwrites them with a copy and puts them back to java. */


#define DEFINE_CAPTURESCALARARRAYELEMENTS(ElementType, ArrayType, Result, base)	\
extern "C" ElementType* JNICALL						\
jni_Capture##Result##ArrayElements(RemoteJNIEnv *env, ArrayType array, 		\
				   jboolean *isCopy) {			\
    int code = base;							\
    char msg[8];							\
    ElementType *retval;\
    int *tmp;\
    int nElements;							\
    if (isCopy)								\
	*isCopy = JNI_TRUE;						\
    memcpy(msg, &code, 4);						\
    memcpy(msg+4, &array, 4);						\
    send_msg(env, msg, 8);						\
    get_msg(env, &nElements, 4);			\
    tmp = (int*) checked_malloc(nElements * sizeof(ElementType)+sizeof(int));	\
    *tmp = nElements; retval= (ElementType *) (tmp + 1);                \
    get_msg(env, retval,				\
	 nElements * sizeof(ElementType));				\
    return retval;							\
}

DEFINE_CAPTURESCALARARRAYELEMENTS(jboolean, jbooleanArray, Boolean, JAVA_PLUGIN_CAP_BOOL_AREL)
DEFINE_CAPTURESCALARARRAYELEMENTS(jbyte, jbyteArray, Byte, JAVA_PLUGIN_CAP_BYTE_AREL)
DEFINE_CAPTURESCALARARRAYELEMENTS(jchar, jcharArray, Char, JAVA_PLUGIN_CAP_CHAR_AREL)
DEFINE_CAPTURESCALARARRAYELEMENTS(jshort, jshortArray, Short, JAVA_PLUGIN_CAP_SHORT_AREL)
DEFINE_CAPTURESCALARARRAYELEMENTS(jint, jintArray, Int, JAVA_PLUGIN_CAP_INT_AREL)
DEFINE_CAPTURESCALARARRAYELEMENTS(jlong, jlongArray, Long, JAVA_PLUGIN_CAP_LONG_AREL)
DEFINE_CAPTURESCALARARRAYELEMENTS(jfloat, jfloatArray, Float, JAVA_PLUGIN_CAP_FLOAT_AREL)
DEFINE_CAPTURESCALARARRAYELEMENTS(jdouble, jdoubleArray, Double, JAVA_PLUGIN_CAP_DOUBLE_AREL)

#define DEFINE_RELEASESCALARARRAYELEMENTS(ElementType, ArrayType, Result, base)	\
extern "C" void JNICALL							\
jni_Release##Result##ArrayElements(RemoteJNIEnv *env, ArrayType array, 		\
				   ElementType *buf, jint mode) {	\
    int code = base;							\
    char *msg;								\
    int nElements;							\
    int *tmp = (int *)buf;\
    if (mode == JNI_ABORT) {						\
	free(&(tmp[-1]));						\
	/* no IPC communication is necessary, since 			\
	   we don't keep this on the client side */			\
	return;								\
    }									\
    nElements = tmp[-1];					\
    msg = (char *) checked_malloc(16 + nElements *sizeof(ElementType));			\
    memcpy(msg, &code, 4);						\
    memcpy(msg+4, &array, 4);						\
    memcpy(msg+8, &mode, 4);						\
    memcpy(msg+12, &nElements, 4);					\
    memcpy(msg+16, buf, nElements *sizeof(ElementType));		\
    send_msg(env, msg, 16 + nElements *sizeof(ElementType));		\
    if (mode == 0)							\
	free(&(tmp[-1]));						\
    free(msg);								\
    return;								\
}

DEFINE_RELEASESCALARARRAYELEMENTS(jboolean, jbooleanArray, Boolean, JAVA_PLUGIN_REL_BOOL_AREL)
DEFINE_RELEASESCALARARRAYELEMENTS(jbyte, jbyteArray, Byte, JAVA_PLUGIN_REL_BYTE_AREL)
DEFINE_RELEASESCALARARRAYELEMENTS(jchar, jcharArray, Char, JAVA_PLUGIN_REL_CHAR_AREL)
DEFINE_RELEASESCALARARRAYELEMENTS(jshort, jshortArray, Short, JAVA_PLUGIN_REL_SHORT_AREL)
DEFINE_RELEASESCALARARRAYELEMENTS(jint, jintArray, Int, JAVA_PLUGIN_REL_INT_AREL)
DEFINE_RELEASESCALARARRAYELEMENTS(jlong, jlongArray, Long, JAVA_PLUGIN_REL_LONG_AREL)
DEFINE_RELEASESCALARARRAYELEMENTS(jfloat, jfloatArray, Float, JAVA_PLUGIN_REL_FLOAT_AREL)
DEFINE_RELEASESCALARARRAYELEMENTS(jdouble, jdoubleArray, Double, JAVA_PLUGIN_REL_DOUBLE_AREL)

#define DEFINE_GETSCALARARRAYELEMENTS(ElementType, ArrayType, Result, base)	\
extern "C" void JNICALL							\
jni_Get##Result##ArrayElements(RemoteJNIEnv *env, ArrayType array, jsize start,	\
			       jsize len, ElementType *buf) {		\
    int code = base;							\
    int retval;								\
    char msg[16];							\
    memcpy(msg, &code, 4);						\
    memcpy(msg+4, &array, 4);						\
    memcpy(msg+8, &start, 4);						\
    memcpy(msg+12, &len, 4);						\
    send_msg(env, msg, 16);						\
    get_msg(env, &retval, 4);			\
    if (retval != JNI_OK) { /*Exception has occured. no data for us. */	\
	fprintf(stderr, "remotejni: Retval not ok. No return value\n"); \
	return; 							\
     };\
    get_msg(env, buf, len * sizeof(ElementType));	\
    fprintf(stderr, "rem: Got result %X\n", (int) buf[0]); \
    return;								\
}

DEFINE_GETSCALARARRAYELEMENTS(jboolean, jbooleanArray, Boolean, JAVA_PLUGIN_GET_BOOL_AREL)
DEFINE_GETSCALARARRAYELEMENTS(jbyte, jbyteArray, Byte, JAVA_PLUGIN_GET_BYTE_AREL)
DEFINE_GETSCALARARRAYELEMENTS(jchar, jcharArray, Char, JAVA_PLUGIN_GET_CHAR_AREL)
DEFINE_GETSCALARARRAYELEMENTS(jshort, jshortArray, Short, JAVA_PLUGIN_GET_SHORT_AREL)
DEFINE_GETSCALARARRAYELEMENTS(jint, jintArray, Int, JAVA_PLUGIN_GET_INT_AREL)
DEFINE_GETSCALARARRAYELEMENTS(jlong, jlongArray, Long, JAVA_PLUGIN_GET_LONG_AREL)
DEFINE_GETSCALARARRAYELEMENTS(jfloat, jfloatArray, Float, JAVA_PLUGIN_GET_FLOAT_AREL)
DEFINE_GETSCALARARRAYELEMENTS(jdouble, jdoubleArray, Double, JAVA_PLUGIN_GET_DOUBLE_AREL)

#define DEFINE_SETSCALARARRAYELEMENTS(ElementType, ArrayType, Result, base)	\
extern "C" void JNICALL							\
jni_Set##Result##ArrayElements(RemoteJNIEnv *env, ArrayType array, jsize start,	\
			       jsize len, ElementType *buf) {		\
    int code = base;							\
    char *msg;								\
    msg = (char *) checked_malloc(16 + len *sizeof(ElementType));			\
    memcpy(msg, &code, 4);						\
    fprintf(stderr, "rem:Set array arr=%X start=%d len=%d buf[0]=%d\n", \
	       (int) array, (int) start, (int) len, (int) buf[0]);  \
    memcpy(msg+4, &array, 4);						\
    memcpy(msg+8, &start, 4);						\
    memcpy(msg+12, &len, 4);						\
    memcpy(msg+16, buf, len *sizeof(ElementType));			\
    send_msg(env, msg, 16 + len *sizeof(ElementType));			\
    free(msg);								\
    /* No reason to wait for the response code; if exception 		\
       has occured, we'll find out. Completion notification 		\
       is irrelevant, since IPC calls are serialized. But do so for better \
       performance. See also ExceptionClear etc. */			\
    handle_response(env);    \
    return;								\
}

DEFINE_SETSCALARARRAYELEMENTS(jboolean, jbooleanArray, Boolean, JAVA_PLUGIN_SET_BOOL_AREL)
DEFINE_SETSCALARARRAYELEMENTS(jbyte, jbyteArray, Byte, JAVA_PLUGIN_SET_BYTE_AREL)
DEFINE_SETSCALARARRAYELEMENTS(jchar, jcharArray, Char, JAVA_PLUGIN_SET_CHAR_AREL)
DEFINE_SETSCALARARRAYELEMENTS(jshort, jshortArray, Short, JAVA_PLUGIN_SET_SHORT_AREL)
DEFINE_SETSCALARARRAYELEMENTS(jint, jintArray, Int, JAVA_PLUGIN_SET_INT_AREL)
DEFINE_SETSCALARARRAYELEMENTS(jlong, jlongArray, Long, JAVA_PLUGIN_SET_LONG_AREL)
DEFINE_SETSCALARARRAYELEMENTS(jfloat, jfloatArray, Float, JAVA_PLUGIN_SET_FLOAT_AREL)
DEFINE_SETSCALARARRAYELEMENTS(jdouble, jdoubleArray, Double, JAVA_PLUGIN_SET_DOUBLE_AREL)

#define DEFINE_NEWSCALARARRAY(ResultType, Result, base)	\
extern "C" ResultType JNICALL					\
jni_New##Result##Array(RemoteJNIEnv *env, jsize len) {	\
    ResultType retval;					\
    char msg[8];					\
    int code = base;					\
    memcpy(msg, &code, 4);				\
    memcpy(msg+4, &len, 4);				\
    send_msg(env, msg, 8);				\
    get_msg(env, &retval, 4);	\
    return retval;					\
}

DEFINE_NEWSCALARARRAY(jbooleanArray, Boolean, JAVA_PLUGIN_NEW_BOOL_ARRAY)
DEFINE_NEWSCALARARRAY(jbyteArray, Byte, JAVA_PLUGIN_NEW_BYTE_ARRAY)
DEFINE_NEWSCALARARRAY(jcharArray, Char, JAVA_PLUGIN_NEW_CHAR_ARRAY)
DEFINE_NEWSCALARARRAY(jshortArray, Short, JAVA_PLUGIN_NEW_SHORT_ARRAY)
DEFINE_NEWSCALARARRAY(jintArray, Int, JAVA_PLUGIN_NEW_INT_ARRAY)
DEFINE_NEWSCALARARRAY(jlongArray, Long, JAVA_PLUGIN_NEW_LONG_ARRAY)
DEFINE_NEWSCALARARRAY(jfloatArray, Float, JAVA_PLUGIN_NEW_FLOAT_ARRAY)
DEFINE_NEWSCALARARRAY(jdoubleArray, Double, JAVA_PLUGIN_NEW_DOUBLE_ARRAY)


extern "C" jobjectArray JNICALL
jni_NewObjectArray(RemoteJNIEnv *env, jsize len, jclass elementClass,
                   jobject initialElement)
{ 
    jobjectArray retval;
    char msg[16];
    int code = JAVA_PLUGIN_NEW_OBJECT_ARRAY;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &len, 4);
    memcpy(msg+8, &elementClass, 4);
    memcpy(msg+12, &initialElement, 4);
    send_msg(env, msg, 16);
    get_msg(env, &retval, 4);
    return retval;
}


extern "C" jobject JNICALL
jni_GetObjectArrayElement(RemoteJNIEnv *env, jobjectArray array, jsize index) {
    jobject retval;
    char msg[12];
    int code = JAVA_PLUGIN_GET_OBJECT_ARRAY_ELEMENT;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &array, 4);
    memcpy(msg+8, &index, 4);
    send_msg(env, msg, 12);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" void JNICALL
jni_SetObjectArrayElement(RemoteJNIEnv *env, jobjectArray array, jsize index,
                          jobject value) {
    char msg[16];
    int code = JAVA_PLUGIN_SET_OBJECT_ARRAY_ELEMENT;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &array, 4);
    memcpy(msg+8, &index, 4);
    memcpy(msg+12, &value, 4);
    send_msg(env, msg, 16);
}

extern "C" jsize JNICALL
jni_GetArrayLength(RemoteJNIEnv *env, jarray array) {    
    jsize retval;
    char msg[8];
    int code = JAVA_PLUGIN_GET_ARRAY_LENGTH;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &array, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" jstring JNICALL
jni_NewString(RemoteJNIEnv *env, const jchar *unicodeChars, jsize len) {
    jstring retval;
    char *msg;
    int code = JAVA_PLUGIN_NEW_STRING;
    msg = (char *) checked_malloc(8 +len *sizeof(jchar));
    memcpy(msg, &code, 4);
    memcpy(msg+4, &len, 4);
    memcpy(msg+8, unicodeChars, len *sizeof(jchar));
    send_msg(env, msg, 8+len *sizeof(jchar));
    free(msg);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" jsize JNICALL
jni_GetStringLength(RemoteJNIEnv *env, jstring string) {    
    jsize retval;
    char msg[8];
    int code = JAVA_PLUGIN_GET_STRING_SIZE;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &string, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" const jchar* JNICALL
jni_GetStringChars(RemoteJNIEnv *env, jstring string, 
		    jboolean *isCopy) {
    int code = JAVA_PLUGIN_GET_STRING_CHARS;
    char msg[8];
    jchar *retval;
    int nElements;
    if (isCopy)
	*isCopy = JNI_TRUE;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &string, 4);
    send_msg(env, msg, 8);
    get_msg(env, &nElements, 4);
    retval = (jchar* )checked_malloc(nElements * sizeof(jchar));
    get_msg(env, retval, nElements * sizeof(jchar));
    return retval;
}

extern "C" void JNICALL
jni_ReleaseStringChars(RemoteJNIEnv *env, jstring str, const jchar *chars)
{
    /* do nothing. The calling process has a copy of the string characters,
       the object on the java side is not pinned, and strings are constant */
    free((void *)chars);
    UNUSED(env);
    UNUSED(str);
    return;
}


extern "C" jstring JNICALL
jni_NewStringUTF(RemoteJNIEnv *env, const char *UTFChars) {
    jstring retval;
    char *msg;
    int code = JAVA_PLUGIN_NEW_STRING_UTF;
    int len = slenUTF(UTFChars);
    msg = (char *) checked_malloc(8 +len *sizeof(char));
    memcpy(msg, &code, 4);
    memcpy(msg+4, &len, 4);
    memcpy(msg+8, UTFChars, len *sizeof(char));
    send_msg(env, msg, 8 +len *sizeof(char));
    free(msg);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" jsize JNICALL
jni_GetStringUTFLength(RemoteJNIEnv *env, jstring string) {    
    jsize retval;
    char msg[8];
    int code = JAVA_PLUGIN_GET_STRING_UTF_SIZE;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &string, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;
}

extern "C" const char* JNICALL
jni_GetStringUTFChars(RemoteJNIEnv *env, jstring string, 
		      jboolean *isCopy) {
    int code = JAVA_PLUGIN_GET_STRING_UTF_CHARS;
    char msg[8];
    char *retval;
    int nElements;
    if (isCopy)
	*isCopy = JNI_TRUE;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &string, 4);
    send_msg(env, msg, 8);
    get_msg(env, &nElements, 4);
    retval = (char *) checked_malloc((nElements+1) * sizeof(char));
    retval[nElements] = 0;
    get_msg(env, retval, nElements * sizeof(char));
    return retval;
}

/* spec: Informs the VM that the native code no longer needs access to 
   utf. The utf argument is a poniter derived from jstring using
   GetStringUTFChars.

   This matters in the in-process jni version, where a single version
   of the UTF chars may be shared by multiple callers, and releasing
   the UTF chars would then do a refdelete. However, in our case we
   always we don't have that optimization: we always have a copy of
   the chars anyhow, so all we need to do is free that copy.  
*/
extern "C" void JNICALL
jni_ReleaseStringUTFChars(RemoteJNIEnv *env, jstring str, const char *chars)
{
    /* do nothing. The calling process has a copy of the string characters,
       the object on the java side is not pinned, and strings are constant */
    free((void *)chars);
    UNUSED(env);
    UNUSED(str);
    return;
}

/* 
 * Register natives currently just sends the relevant information 
 * across. There is no provision made for receiving a native method
 */
extern "C" jint JNICALL
jni_RegisterNatives(RemoteJNIEnv *env, jclass clazz,
                    const RemoteJNINativeMethod *methods, jint nMethods)
{
    int i;
    short *lengths = (short *) checked_malloc(2 *nMethods *sizeof(short));
    int total = 0;
    jint retval;
    int code = JAVA_PLUGIN_REGISTER_NATIVES;
    char *msg, *ptr;
    /* Create array of lengths of the method names 
     * and method signatures. 
     * For the ith input method, length of its name is stored at length[i]
     *  and length of its signature is stored at length[2*i+1];
     */
    for (i = 0; i < nMethods; i ++) {
	total += lengths[2*i] = slen(methods[i].name);
	total += lengths[2*i+1] = slen(methods[i].signature);
	total += 4 +sizeof(void *); /* For the function pointer */
    }
    total += 12;		/* For the header info  */
    msg = (char *) checked_malloc(total);
    memcpy(msg, &code, 4);
    memcpy(msg+4, &clazz, 4);
    memcpy(msg+8, &nMethods, 4);
    ptr = msg + 12;
    for (i = 0; i < nMethods; i ++) {
	/* Store the length of the method name */
	memcpy(ptr, &(lengths[2*i]), 2);
	ptr += 2;
	/* Store the method name */
	memcpy(ptr, &(methods[i].name), lengths[2*i]);
	ptr += lengths[2*i];
	/* Store the length of the method signature */
	memcpy(ptr, &(lengths[2*i+1]), 2);
	ptr += 2;
	/* Store the method signature */
	memcpy(ptr, &(methods[i].signature), lengths[2*i+1]);
	ptr += lengths[2*i+1];
	/* Store the function pointer */
	memcpy(ptr, &(methods[i].fnPtr), sizeof(void *));
	ptr += sizeof(void *);
    }
    send_msg(env, msg, total);
    free(msg);
    free(lengths);
    get_msg(env, &retval, 4);
    return retval;
}

/* Currently just sends the message across. No provision is made
 * for actually unregistering the native method as yet 
 */
extern "C" jint JNICALL
jni_UnregisterNatives(RemoteJNIEnv *env, jclass clazz)
{
    jint retval;
    char msg[8];
    int code = JAVA_PLUGIN_UNREGISTER_NATIVES;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &clazz, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;    
}

extern "C" jint JNICALL
jni_MonitorEnter(RemoteJNIEnv *env, jobject obj)
{
    jint retval;
    char msg[8];
    int code = JAVA_PLUGIN_MONITOR_ENTER;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &obj, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;    
}

extern "C" jint JNICALL
jni_MonitorExit(RemoteJNIEnv *env, jobject obj)
{
    jint retval;
    char msg[8];
    int code = JAVA_PLUGIN_MONITOR_EXIT;
    memcpy(msg, &code, 4);
    memcpy(msg+4, &obj, 4);
    send_msg(env, msg, 8);
    get_msg(env, &retval, 4);
    return retval;    
}

/* The remote JNI has no invocation interface */
extern "C" jint JNICALL
jni_GetJavaVM(RemoteJNIEnv *env, void **vm)
{
    *vm = NULL;
    trace("remotejni::Calling GetJavaVM for env: %d\n", (int) env);
    return JNI_EVERSION;
}




/* Write a value into the pipe */
int
pack_value_of_type(RemoteJNIEnv* env, jd_jni_type value_type, jvalue* value,
		    char* msg) {
    
    trace("remotejni::pack_value_of_type env=%X type=%d\n", (int) env, (int) value_type);

    switch (value_type) {
    case jd_jboolean_type:  
	memcpy(msg, value, sizeof(jboolean)); 
	return sizeof(jboolean);
    case jd_jbyte_type:  
	memcpy(msg, value, sizeof(jbyte)); 
	return sizeof(jbyte);
    case jd_jchar_type:  
	memcpy(msg, value, sizeof(jchar)); 
	return sizeof(jchar);
    case jd_jshort_type:  
	memcpy(msg, value, sizeof(jshort)); 
	return sizeof(jshort);
    case jd_jint_type:  
	memcpy(msg, value, sizeof(jint)); 
	return sizeof(jint);
    case jd_jlong_type: 
	memcpy(msg, value, sizeof(jlong)); 
	return sizeof(jlong);
    case jd_jfloat_type:  
	memcpy(msg, value, sizeof(jfloat)); 
	return sizeof(jfloat);
    case jd_jdouble_type:  
	memcpy(msg, value, sizeof(jdouble)); 
	return sizeof(jdouble);
    case jd_jobject_type:  
	memcpy(msg, value, sizeof(jobject)); 
	return sizeof(jobject);
    default: 
	plugin_error("Unknown type in creating message");
	return 0;
    }

}

/* Secure interface */
extern "C" int 
jni_SecureNewObject(RemoteJNIEnv *env, jclass clazz, jmethodID methodID,
		    jvalue* args, jobject* result, ISecurityContext *ctx) {
    int nargs;
    char* msg;
    char* ss;
    char* ptr;
    int len;

    trace("remotejni:Entering jni_SecureNewObject()\n");

    if (env == NULL || clazz == NULL || methodID == NULL) {
	trace("remotejni:Exiting jni_SecureNewObject(), due to NULL value\n");
        return JD_ERROR_NULL_POINTER; 
    }

    if (ctx) ctx->AddRef();

    int code = JAVA_PLUGIN_SECURE_NEW_OBJECT;

    ss = GET_SIGNATURE(methodID);
    nargs = slen(ss);

    trace("jni_SecureNewObject Cls=%X sig=%s meth=%X narg=%d ct=%X\n",
	   (int) clazz, STR_OR_NULL(ss),
	   (int) (methodID->java_method), nargs, (int) ctx);

    void * buff = getAndPackSecurityInfo(ctx, &len); 

    int msg_size = nargs * sizeof(jvalue) + nargs + 20 + len;

    msg = (char *) checked_malloc(msg_size);
    memcpy(msg, &code, 4);
    memcpy(msg + 4, &clazz, 4);
    memcpy(msg + 8, &(methodID->java_method), 4);
    memcpy(msg + 12, &nargs, 4);
    memcpy(msg + 16, &ctx, 4);
    memcpy(msg + 20, buff, len);
    if (nargs > 0) {
	memcpy(msg + 20 + len, ss, nargs);
	ptr = &(msg[20 + len + nargs]);
	argarr_to_jvals(args, nargs, ptr);
    }

    free(buff);

    if (ctx) ctx->Release();
    send_msg(env, msg, msg_size);
    free(msg);

    handle_response(env);

    get_msg(env, result, sizeof(jobject));

    trace("remotejni:Exiting jni_SecureNewObject()\n");
    return JD_OK;
} 

extern "C" int
jni_SecureCallMethod(RemoteJNIEnv *env, jd_jni_type result_type,
		     jobject obj, 
		     jmethodID methodID, jvalue* args, jvalue* result,
		     ISecurityContext *ctx) {
    int msg_size;
    char* msg; 
    char* ss;
    char* ptr;
    int nargs;
    int len;
    int code = JAVA_PLUGIN_SECURE_CALL;

    trace("remotejni:Entering jni_SecureCallMethod()\n");

    if (env == NULL ) {
        trace("remotejni:Exiting jni_SecureCallMethod(), due to NULL value\n");
        return JD_ERROR_NULL_POINTER; 
    }
    
    trace("jni_SecureCallMethod(): env=%X type=%s obj=%X\n\t method=%X args=%X ctx=%X\n",
		       (int) env, get_jni_name(result_type), (int) obj, 
		       (int) methodID, (int) args, (int) ctx);

    void * buff = getAndPackSecurityInfo(ctx, &len); 

    ss = GET_SIGNATURE(methodID);
    nargs = slen(ss);
    msg_size = nargs * sizeof(jvalue) + nargs + 24 + len;
    msg = (char *) checked_malloc(msg_size);
    
    memcpy(msg, &code, 4);
    memcpy(msg + 4, &obj, 4);
    memcpy(msg + 8, &(methodID->java_method), 4);
    memcpy(msg + 12, &nargs, 4);
    memcpy(msg + 16, &ctx, 4);
    memcpy(msg + 20, &result_type, 4);
    memcpy(msg + 24, buff, len);
    if (nargs > 0) {
	memcpy(msg + 24 + len, ss, nargs);
	ptr = &(msg[24 + len + nargs]);
	argarr_to_jvals(args, nargs, ptr);
    }
    
    free(buff);

    send_msg(env, msg, msg_size);
    free(msg);

    handle_response(env);

    get_result_of_type(env, result_type, result);

    trace("remotejni:Exiting jni_SecureCallMethod()");

    return JD_OK;
}


extern "C" int
jni_SecureCallNonvirtualMethod(RemoteJNIEnv* env, jd_jni_type result_type,
			 jobject obj, jclass clazz,
			 jmethodID methodID,  jvalue *args, 
			 jvalue* result, ISecurityContext* ctx)
{
    if (ctx) ctx->AddRef();

    int msg_size;
    char* msg; 
    char* ss;
    char* ptr;
    int nargs;
    int len;

    int code = JAVA_PLUGIN_SECURE_CALL_NONVIRTUAL;

    trace("remotejni:Entering jni_SecureCallNonvirtualMethod()");

    if (env == NULL ) {
        return JD_ERROR_NULL_POINTER; 
    }
    
    /*
    if (tracing) trace("jni_SecureCallNonVirtualMethod env=%X type=%d "
		       "obj=%X class=%X method=%X value=%X res=%X ctx=%X\n",
		       (int) env, (int) result_type, (int) obj, (int) clazz,
		       (int) methodID, (int) args, (int) result, (int) ctx);
    */

    void * buff = getAndPackSecurityInfo(ctx, &len); 

    ss = GET_SIGNATURE(methodID);
    nargs = slen(ss);
    msg_size = nargs * sizeof(jvalue) + nargs + 28 + len;
    msg = (char *) checked_malloc(msg_size);
    
    memcpy(msg, &code, 4);
    memcpy(msg + 4, &obj, 4);
    memcpy(msg + 8, &clazz, 4);
    memcpy(msg + 12, &(methodID->java_method), 4);
    memcpy(msg + 16, &nargs, 4);
    memcpy(msg + 20, &ctx, 4);
    memcpy(msg + 24, &result_type, 4);
    memcpy(msg + 28, buff, len);
    if (nargs > 0) {
	memcpy(msg + 28 + len, ss, nargs);
	ptr = &(msg[28 + len + nargs]);
	argarr_to_jvals(args, nargs, ptr);
    }

    free(buff);

    if (ctx) ctx->Release();
    send_msg(env, msg, msg_size);
    free(msg);

    handle_response(env);

    get_result_of_type(env, result_type, result);
    return JD_OK;
}

extern "C" int
jni_SecureGetField(RemoteJNIEnv* env,
		   jd_jni_type type,
		   jobject obj, 
		   jfieldID fieldID,
		   jvalue* result,
		   ISecurityContext* ctx)
{
    trace("remotejni:Entering jni_SecureGetField()");

    if (ctx) ctx->AddRef();

    char * msg;
    int msg_size = 20;
    int len;

    int code = JAVA_PLUGIN_SECURE_GET_FIELD;

    trace("SECURE_GET_FIELD sending obj=%X fieldID=%X type=%d ctx=%X\n",
	  (int) obj, (int) fieldID, (int) type, (int) ctx);

    void * buff = getAndPackSecurityInfo(ctx, &len); 
    msg_size += len;

    msg = (char *) checked_malloc(msg_size);
    memcpy(msg, &code, 4);
    memcpy(msg + 4, &obj, 4);
    memcpy(msg + 8, &fieldID, 4);
    memcpy(msg + 12, &type, 4);
    memcpy(msg + 16, &ctx, 4);
    memcpy(msg + 20, buff, len);

    free(buff);
    if (ctx) ctx->Release();
    send_msg(env, msg, msg_size);
    free(msg);

    get_result_of_type(env, type, result);
    print_jvalue(type, *result, "Result of SECURE_GET_FIELD");
    return JD_OK;
}

extern "C" int 
jni_SecureSetField(RemoteJNIEnv* env,
		   jd_jni_type type,
		   jobject obj, 
		   jfieldID fieldID,
		   jvalue val,
		   ISecurityContext* ctx)
{
    int len;
    int msg_size = 28;

    trace("remotejni:Entering jni_SecureSetField()");

    if (ctx) ctx->AddRef();

    int code = JAVA_PLUGIN_SECURE_SET_FIELD;


    void * buff = getAndPackSecurityInfo(ctx, &len); 
    msg_size += len;

    char *  msg = checked_malloc(msg_size);    
    memcpy(msg, &code, 4);
    memcpy(msg + 4, &obj, 4);
    memcpy(msg + 8, &fieldID, 4);
    memcpy(msg + 12, &type, 4);
    memcpy(msg + 16, &ctx, 4);
    memcpy(msg + 20, buff, len);

    int sz = pack_value_of_type(env, type, &val, &(msg[20 + len]));
    if (tracing) trace("jni_SecureSetField env=%X type=%d sz=%d\n",
		       (int) env, (int) type, sz);

    free(buff);

    if (ctx) ctx->Release();
    send_msg(env, msg, 20 + sz + len);
    free(msg);
    /* No response necessary */
    return JD_OK;
}


extern "C" int 
jni_SecureCallStaticMethod(RemoteJNIEnv* env,
			   jd_jni_type result_type,
			   jclass clazz,
			   jmethodID methodID,
			   jvalue *args, 
			   jvalue* result,
			   ISecurityContext* ctx)
{
    trace("remotejni:Entering jni_SecureCallStaticMethod()");

    if (ctx) ctx->AddRef();

    trace("jni_SecureCallStaticMethod env=%X type=%d \nclazz=%X methodID=%X args=%X ctx=%X\n",(int) env, get_jni_name( result_type), (int) clazz, (int) methodID, (int) args, (int) ctx);

    int msg_size;
    char* msg; 
    char* ss;
    char* ptr;
    int nargs;
    int len;
    int code = JAVA_PLUGIN_SECURE_CALL_STATIC;
    if (env == NULL )   return JD_ERROR_NULL_POINTER; 
    ss = GET_SIGNATURE(methodID);
    nargs = slen(ss);

    void * buff = getAndPackSecurityInfo(ctx, &len); 
    msg_size = nargs * sizeof(jvalue) + nargs + 24 + len;

    msg = (char *) checked_malloc(msg_size);
    
    memcpy(msg, &code, 4);
    memcpy(msg + 4, &clazz, 4);
    memcpy(msg + 8, &(methodID->java_method), 4);
    memcpy(msg + 12, &nargs, 4);
    memcpy(msg + 16, &ctx, 4);
    memcpy(msg + 20, &result_type, 4);
    memcpy(msg + 24, buff, len);
    if (nargs > 0) {
	memcpy(msg + len + 24, ss, nargs);
	ptr = &(msg[24 + len + nargs]);
	argarr_to_jvals(args, nargs, ptr);
    }

    free(buff);
    if (ctx) ctx->Release();
    send_msg(env, msg, msg_size);
    free(msg);

    handle_response(env);

    get_result_of_type(env, result_type, result);

    trace("remotejni:Exiting jni_SecureCallStaticMethod()");
    return JD_OK;
}


extern "C" int 
jni_SecureGetStaticField(RemoteJNIEnv* env,
			 jd_jni_type type,
			 jclass clazz, 
			 jfieldID fieldID,
			 jvalue* result,
			 ISecurityContext* ctx)
{
    trace("remotejni:Entering jni_SecureGetStaticField()");

    trace("jni_SecureGetStaticField env=%X type=%s \n\tclazz=%X fieldID=%X ctx=%X\n",(int) env, get_jni_name(type), (int) clazz, (int) fieldID, (int) ctx);

    if (ctx) ctx->AddRef();
    int len;
    int msg_size = 20;
    char *msg;
    int code = JAVA_PLUGIN_SECURE_GET_STATIC_FIELD;

    void * buff = getAndPackSecurityInfo(ctx, &len); 
    msg_size += len;

    msg = checked_malloc(msg_size);
    memcpy(msg, &code, 4);
    memcpy(msg + 4, &clazz, 4);
    memcpy(msg + 8, &fieldID, 4);
    memcpy(msg + 12, &type, 4);
    memcpy(msg + 16, &ctx, 4);
    memcpy(msg + 20, buff, len);

    free(buff);
    if (ctx) ctx->Release();
    send_msg(env, msg, msg_size);
    free(msg);
    get_result_of_type(env, type, result);
    if (tracing)
	print_jvalue(type, *result, "Result of SECURE_GET_STATIC_FIELD");
    return JD_OK;
}


extern "C" int 
jni_SecureSetStaticField(RemoteJNIEnv* env,
			 jd_jni_type type,
			 jclass clazz, 
			 jfieldID fieldID,
			 jvalue val,
			 ISecurityContext* ctx)
{
    trace("remotejni:Entering jni_SecureSetStaticField()");

    if (ctx) ctx->AddRef();
    int len;
    int msg_size = 28;
    int code = JAVA_PLUGIN_SECURE_SET_STATIC_FIELD;

    void * buff = getAndPackSecurityInfo(ctx, &len); 
    msg_size += len;

    char * msg = checked_malloc(msg_size);    
    memcpy(msg, &code, 4);
    memcpy(msg + 4, &clazz, 4);
    memcpy(msg + 8, &fieldID, 4);
    memcpy(msg + 12, &type, 4);
    memcpy(msg + 16, &ctx, 4);
    memcpy(msg + 20, buff, len);
    if (tracing) trace("jni_SecureSetStaticField env=%X type=%s \n\tclazz=%X "
		       " fieldID=%X ctx=%X\n",
		       (int) env, get_jni_name(type), (int) clazz, (int) fieldID,
		       (int) ctx);

    int sz = pack_value_of_type(env, type, &val, &(msg[20 + len]));

    free(buff);
    if (ctx) ctx->Release();
    send_msg(env, msg, 20 + sz + len);
    free(msg);
    /* No response necessary */
    return JD_OK;
}

  
/* For a call from Java to JS, a protection check in CSecurityContext
   calls back into Java. Context is the access control context that is
   passed in during the Java->JS call. */
extern "C" int
jni_CSecurityContextImplies(RemoteJNIEnv* env,
			    jobject context,
			    const char* target, 
			    const char* action)
{
    int target_len;
    int action_len;
    int mesg_len;
    char* mesg;
    int jobject_sz;
    int len_sz;
    int ci;
    jboolean result;

    trace("remotejni:Entering jni_CSecurityContextImplies()");

    if (target == NULL) {
	plugin_error("Bad target or action allowed in security check");
        return JNI_FALSE;
    }

    jobject_sz = sizeof(jobject);
    target_len = slen(target);
    action_len = slen(action);
    len_sz = 4;

    mesg_len = 4 + jobject_sz + len_sz + target_len + len_sz + action_len;
    mesg = (char *) checked_malloc(mesg_len);

    ci = 0;
    int code = JAVA_PLUGIN_CSECURITYCONTEXT_IMPLIES;
    memcpy(mesg + ci, &code, 4); ci += 4;
    memcpy(mesg + ci, &context, jobject_sz); ci += jobject_sz;
    memcpy(mesg + ci, &target_len, len_sz);  ci += len_sz;
    memcpy(mesg + ci, target, target_len); ci += target_len;
    memcpy(mesg + ci, &action_len, len_sz);  ci += len_sz;
    memcpy(mesg + ci, action, action_len); ci += action_len;

    send_msg(env, mesg, mesg_len);
    free(mesg);

    handle_response(env);
    
    get_msg(env, &result, sizeof(jboolean));

    if (result == JNI_TRUE) 
	return JD_TRUE;
    else
	return JD_FALSE;
}

struct RemoteJNINativeInterface_ remotejni_NativeInterface = {
    NULL,
    NULL,
    NULL,

    NULL,

    jni_GetVersion,

    jni_DefineClass,
    jni_FindClass,

    NULL,
    NULL,
    NULL,

    jni_GetSuperclass,
    jni_IsSubclassOf,
    NULL,

    jni_Throw,
    jni_ThrowNew,
    jni_ExceptionOccurred,
    jni_ExceptionDescribe,
    jni_ExceptionClear,
    jni_FatalError,

    NULL,
    NULL,

    jni_NewGlobalRef,
    jni_DeleteGlobalRef,
    jni_DeleteLocalRef,
    jni_IsSameObject,

    NULL,
    NULL,

    jni_AllocObject,
    jni_CallStaticNewObjectMethod,
    jni_CallStaticNewObjectMethodV,
    jni_CallStaticNewObjectMethodA,

    jni_GetObjectClass,
    jni_IsInstanceOf,

    jni_GetMethodID,

    jni_CallObjectMethod,
    jni_CallObjectMethodV,
    jni_CallObjectMethodA,
    jni_CallBooleanMethod,
    jni_CallBooleanMethodV,
    jni_CallBooleanMethodA,
    jni_CallByteMethod,
    jni_CallByteMethodV,
    jni_CallByteMethodA,
    jni_CallCharMethod,
    jni_CallCharMethodV,
    jni_CallCharMethodA,
    jni_CallShortMethod,
    jni_CallShortMethodV,
    jni_CallShortMethodA,
    jni_CallIntMethod,
    jni_CallIntMethodV,
    jni_CallIntMethodA,
    jni_CallLongMethod,
    jni_CallLongMethodV,
    jni_CallLongMethodA,
    jni_CallFloatMethod,
    jni_CallFloatMethodV,
    jni_CallFloatMethodA,
    jni_CallDoubleMethod,
    jni_CallDoubleMethodV,
    jni_CallDoubleMethodA,
    jni_CallVoidMethod,
    jni_CallVoidMethodV,
    jni_CallVoidMethodA,

    jni_CallNonvirtualObjectMethod,
    jni_CallNonvirtualObjectMethodV,
    jni_CallNonvirtualObjectMethodA,
    jni_CallNonvirtualBooleanMethod,
    jni_CallNonvirtualBooleanMethodV,
    jni_CallNonvirtualBooleanMethodA,
    jni_CallNonvirtualByteMethod,
    jni_CallNonvirtualByteMethodV,
    jni_CallNonvirtualByteMethodA,
    jni_CallNonvirtualCharMethod,
    jni_CallNonvirtualCharMethodV,
    jni_CallNonvirtualCharMethodA,
    jni_CallNonvirtualShortMethod,
    jni_CallNonvirtualShortMethodV,
    jni_CallNonvirtualShortMethodA,
    jni_CallNonvirtualIntMethod,
    jni_CallNonvirtualIntMethodV,
    jni_CallNonvirtualIntMethodA,
    jni_CallNonvirtualLongMethod,
    jni_CallNonvirtualLongMethodV,
    jni_CallNonvirtualLongMethodA,
    jni_CallNonvirtualFloatMethod,
    jni_CallNonvirtualFloatMethodV,
    jni_CallNonvirtualFloatMethodA,
    jni_CallNonvirtualDoubleMethod,
    jni_CallNonvirtualDoubleMethodV,
    jni_CallNonvirtualDoubleMethodA,
    jni_CallNonvirtualVoidMethod,
    jni_CallNonvirtualVoidMethodV,
    jni_CallNonvirtualVoidMethodA,

    jni_GetFieldID,

    jni_GetObjectField,
    jni_GetBooleanField,
    jni_GetByteField,
    jni_GetCharField,
    jni_GetShortField,
    jni_GetIntField,
    jni_GetLongField,
    jni_GetFloatField,
    jni_GetDoubleField,
    jni_SetObjectField,
    jni_SetBooleanField,
    jni_SetByteField,
    jni_SetCharField,
    jni_SetShortField,
    jni_SetIntField,
    jni_SetLongField,
    jni_SetFloatField,
    jni_SetDoubleField,

    jni_GetStaticMethodID,

    jni_CallStaticObjectMethod,
    jni_CallStaticObjectMethodV,
    jni_CallStaticObjectMethodA,
    jni_CallStaticBooleanMethod,
    jni_CallStaticBooleanMethodV,
    jni_CallStaticBooleanMethodA,
    jni_CallStaticByteMethod,
    jni_CallStaticByteMethodV,
    jni_CallStaticByteMethodA,
    jni_CallStaticCharMethod,
    jni_CallStaticCharMethodV,
    jni_CallStaticCharMethodA,
    jni_CallStaticShortMethod,
    jni_CallStaticShortMethodV,
    jni_CallStaticShortMethodA,
    jni_CallStaticIntMethod,
    jni_CallStaticIntMethodV,
    jni_CallStaticIntMethodA,
    jni_CallStaticLongMethod,
    jni_CallStaticLongMethodV,
    jni_CallStaticLongMethodA,
    jni_CallStaticFloatMethod,
    jni_CallStaticFloatMethodV,
    jni_CallStaticFloatMethodA,
    jni_CallStaticDoubleMethod,
    jni_CallStaticDoubleMethodV,
    jni_CallStaticDoubleMethodA,
    jni_CallStaticVoidMethod,
    jni_CallStaticVoidMethodV,
    jni_CallStaticVoidMethodA,

    jni_GetStaticFieldID,

    jni_GetStaticObjectField,
    jni_GetStaticBooleanField,
    jni_GetStaticByteField,
    jni_GetStaticCharField,
    jni_GetStaticShortField,
    jni_GetStaticIntField,
    jni_GetStaticLongField,
    jni_GetStaticFloatField,
    jni_GetStaticDoubleField,

    jni_SetStaticObjectField,
    jni_SetStaticBooleanField,
    jni_SetStaticByteField,
    jni_SetStaticCharField,
    jni_SetStaticShortField,
    jni_SetStaticIntField,
    jni_SetStaticLongField,
    jni_SetStaticFloatField,
    jni_SetStaticDoubleField,

    jni_NewString,
    jni_GetStringLength,
    jni_GetStringChars,
    jni_ReleaseStringChars,

    jni_NewStringUTF,
    jni_GetStringUTFLength,
    jni_GetStringUTFChars,
    jni_ReleaseStringUTFChars,

    jni_GetArrayLength,
 
    jni_NewObjectArray,
    jni_GetObjectArrayElement,
    jni_SetObjectArrayElement,

    jni_NewBooleanArray,
    jni_NewByteArray,
    jni_NewCharArray,
    jni_NewShortArray,
    jni_NewIntArray,
    jni_NewLongArray,
    jni_NewFloatArray,
    jni_NewDoubleArray,

    jni_CaptureBooleanArrayElements,
    jni_CaptureByteArrayElements,
    jni_CaptureCharArrayElements,
    jni_CaptureShortArrayElements,
    jni_CaptureIntArrayElements,
    jni_CaptureLongArrayElements,
    jni_CaptureFloatArrayElements,
    jni_CaptureDoubleArrayElements,

    jni_ReleaseBooleanArrayElements,
    jni_ReleaseByteArrayElements,
    jni_ReleaseCharArrayElements,
    jni_ReleaseShortArrayElements,
    jni_ReleaseIntArrayElements,
    jni_ReleaseLongArrayElements,
    jni_ReleaseFloatArrayElements,
    jni_ReleaseDoubleArrayElements,

    jni_GetBooleanArrayElements,
    jni_GetByteArrayElements,
    jni_GetCharArrayElements,
    jni_GetShortArrayElements,
    jni_GetIntArrayElements,
    jni_GetLongArrayElements,
    jni_GetFloatArrayElements,
    jni_GetDoubleArrayElements,
    jni_SetBooleanArrayElements,
    jni_SetByteArrayElements,
    jni_SetCharArrayElements,
    jni_SetShortArrayElements,
    jni_SetIntArrayElements,
    jni_SetLongArrayElements,
    jni_SetFloatArrayElements,
    jni_SetDoubleArrayElements,

    jni_RegisterNatives,
    jni_UnregisterNatives,

    jni_MonitorEnter,
    jni_MonitorExit,

    jni_GetJavaVM,
    jni_SecureNewObject,
    jni_SecureCallMethod,
    jni_SecureCallNonvirtualMethod,
    jni_SecureGetField,
    jni_SecureSetField,
    jni_SecureCallStaticMethod,
    jni_SecureGetStaticField,
    jni_SecureSetStaticField,
    jni_CSecurityContextImplies,
};


/* INVOCATION INTERFACE */
/* Create a new RemoteJNIEnv */
RemoteJNIEnv* create_RemoteJNIEnv() {
    RemoteJNIEnv *env;
    struct RemoteJNINativeInterface_ * functions;
	
    env = new RemoteJNIEnv_();
    trace("remotejni::create_RemoteJNIEnv %d\n", (int) env);
    functions = (struct RemoteJNINativeInterface_ *) 
	malloc(sizeof(struct RemoteJNINativeInterface_));
    memcpy(functions, &remotejni_NativeInterface, 
	   sizeof(struct RemoteJNINativeInterface_));
    functions->environmentInfo = new environmentInfo_();
    env->functions = functions;
    return env;
}


void init_RemoteJNIEnv(RemoteJNIEnv* env, int env_index, void* pipe) {

    env->functions->environmentInfo->env_index = env_index;
    env->functions->environmentInfo->pipe = pipe;
    env->functions->environmentInfo->call_depth = 0;
}
	
void dispose_RemoteJNIEnv(RemoteJNIEnv* env) {
    delete(env->functions->environmentInfo);
    free((void *) env->functions);
    delete(env);
}

/*
 * Handle a response from the JVM. The response could either be an 'ok'
 * message or a call into javascript from java. A nesting of java->javascript
 * calls is thus supported.
 */ 
void handle_response(RemoteJNIEnv *env) {
    int ret_code;

    trace("remotejni:Entering handle_response()\n");
    while(1) {
	get_msg(env, &ret_code, 4);
	switch(ret_code) {
	case JAVA_PLUGIN_RETURN:
	    {
	      trace("%d remotejni:handle_response() PLUGIN_RETURN\n",message_counter);
		return;
	    }
	case JAVA_PLUGIN_REQUEST:
	    {
	      int cur_depth = env->functions->environmentInfo->call_depth; 
	      env->functions->environmentInfo->call_depth = cur_depth + 1;
	      trace("remotejni:handle_reponse() [depth=%d] %d PLUGIN_REQUEST\n", cur_depth, message_counter);
	      /* This is a JS Object request */
	      JSHandler(env);
	      trace("remotejni:handle_reponse() [depth=%d] %d after JSHandler in REQUEST\n", cur_depth,  message_counter);
	      env->functions->environmentInfo->call_depth = cur_depth;
	      break;
	    }
	default: 
	    {
	      plugin_error("handle_response :Protocol error: %d %X\n",
			   ret_code, ret_code);
		exit(-1);
	    }
	}
    }
}

void * getAndPackSecurityInfo(ISecurityContext * ctx, int * len) {

/* This function allocates memory to hold the information it gets back from the
   security context.  The caller MUST free the memory to prevent leaks!!
*/

    char origin[256];
    JDBool ubr = JD_FALSE;
    JDBool ujp = JD_FALSE;
    char * mem = NULL;
    short originlength = 0;

    trace("remotejni:Entering getAndPackSecurityInfo");

    if(ctx != NULL) {
        ctx->GetOrigin(origin,256);
        ctx->Implies("UniversalBrowserRead" ,"*",&ubr);
        ctx->Implies("UniversalJavaPermission","*",&ujp);
        originlength = strlen(origin);
    }

    if (tracing) trace("getAndPackSecurityInfo"
		       "\n\t ctx=%X"
                       "\n\t origin=%s"
                       "\n\t UniversalBrowserRead=%d UniversalJavaPerm=%d\n",
                       (int) ctx, STR_OR_NULL(origin),ubr,ujp);

    int ubronpipe = ubr;
    int ujponpipe = ujp;

    *len = sizeof(originlength) + originlength + sizeof(ubronpipe) + sizeof(ujponpipe);

    mem = (char *) malloc(*len);
    memcpy(mem,&originlength,sizeof(originlength));
    memcpy(mem+sizeof(originlength),origin,originlength);
    memcpy(mem+sizeof(originlength)+originlength,&ubr,sizeof(ubronpipe));
    memcpy(mem+sizeof(originlength)+originlength+sizeof(ubronpipe),&ujp,sizeof(ujponpipe));

    trace("remotejni:Exiting getAndPackSecurityInfo");

    return ((void *) mem);
}
