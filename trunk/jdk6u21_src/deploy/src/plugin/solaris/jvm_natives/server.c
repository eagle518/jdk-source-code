/*
 * @(#)server.c	1.51 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stropts.h>
#include <errno.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/un.h>

#include "pipe_interface.h"

#include "sun_plugin_navig_motif_AThread.h"
#include "sun_plugin_javascript_navig_JSObject.h"
#include "sun_plugin_javascript_navig5_JSObject.h"
#include "sun_plugin_services_MNetscape6BrowserAuthenticator.h"
#include "sun_plugin_viewer_MNetscapePluginObject.h"

/*
  A strange conflict was arising when this header
  file was included with cc (but not gcc), 
  so we don't include this. Since this is a C file
  anyhow, just the function definitions are needed
#include "sun_plugin_navig_motif_OJIPlugin.h"
*/

#include "protocol.h"
#include "jni.h"
#include "Debug.h"
#include "plugin_defs.h"
#include "pluginversion.h"

#ifdef DO_TRACE
#define TRACE_CONVERT(s) 
#define CHECK_EXC(msg, env) check_exception(msg, env)
#define CHECK_EXC_RET(msg, env, res) check_exception(msg, env) ; check_result(msg, res);
#define REGISTER_NAME(msg, name, id) register_name(msg, name, id)
#else

#define TRACE_CONVERT(s) 
#define CHECK_EXC(p, e) 
#define CHECK_EXC_RET(msg, env, res) 
#define REGISTER_NAME(msg, name, id) register_name(msg, name, id)
#endif


/* Number of local refs in a jni call stack frame */
#define LOCALS_FOR_CALL 50
#define STR_OR_NULL(s) ((s==NULL)?"NULL":s)
#define ENTER_MONITOR(env, jobject) if(jobject != NULL) \
	(*env)->MonitorEnter(env, jobject);
#define EXIT_MONITOR(env, jobject) if (jobject != NULL) \
	(*env)->MonitorExit(env, jobject);

static int approx_call_count = 0;


/************************************************************************
 *              Tracing and Error Reporting 
 *
 ************************************************************************/


static int tracing_on = 0;

void native_trace(char *format,...) {
    va_list ap;
    va_start(ap, format);
    if (tracing_on) {
      fprintf(stdout, "Server: "); 
      vfprintf(stdout, format, ap);
    }
    va_end(ap);
}

void 
native_error(char *format,...) {
    va_list(args);
    va_start(args, format);
    fprintf(stderr, "\n**************** SERVER ERROR **************\n");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n**************** ************ **************\n");
    va_end(args);
}

void stop_fn() {
  if (tracing_on)
	fprintf(stderr, "call:%d\n", approx_call_count);
}

int
bp_fn(int after_call) {
  /* Dummy function to use as a breakpoint */
  if (approx_call_count > after_call) {
    stop_fn();
    return 1;
  } else {
    return 0;
  }


}


/* These method and class tables are not really safe, but really
   useful for debugging.  Needs a lock to protect it */
char *name_arr[10000];
int id_arr[10000];
static int cur_name_ind = 0;

void
register_name(char *msg, char *name, int id) {
  char *name_copy;

  if (tracing_on == 0 || cur_name_ind >= 10000)
	return;

  if (name == NULL) return;

  if (id != 0) 
    native_trace("Register %s [%s] = %X\n", msg, name, id);

  name_copy = (char *) malloc(strlen(name) + 1);
  sprintf(name_copy, "%s", name);

  name_arr[cur_name_ind] = name_copy;
  id_arr[cur_name_ind] = id;
  cur_name_ind++;

}


char*
lookup_name(int id) {
  int i;
  static char* lookup_result[2] = {"", "Unnamed"};
  
  if (tracing_on == 0)
	return lookup_result[0];

  for(i = 0; i < cur_name_ind; i++) {
    if (id_arr[i] == id) 
      return name_arr[i];
  }
  return lookup_result[1];
}
 
/* Write all data to the pipe. Make noises if that fails */
int
write_fully(int pipe, void* data, int len) {
    int nbytes_written;
    int first_word = -1;
    if (len >= 4) {
      memcpy(&first_word, data, 4);
    } else {
      memcpy(&first_word, data, len);
    }
/*
    native_trace("write_fully(): [%d] %d bytes on pipe %d. Start hex=%X dec=%d \n", 
	    approx_call_count,
	    len,
	    pipe, first_word, first_word);
*/
    nbytes_written = write(pipe, data, len);
    if (nbytes_written != len) {
        native_error("write_fully: Did not write everything pipe=%d %d %d",
		     pipe, len, nbytes_written);
        return -1;
    }

    return 0;

}



/************************************************************************
 * Some necessary (repeated) data structures . SHould be moved into a shared
 * header file
 *
 ************************************************************************/

/* Repeat this enum definition from nsISecureJNI */
typedef enum jni_type {
    jobject_type = 0,
    jboolean_type,
    jbyte_type,
    jchar_type,
    jshort_type,
    jint_type,
    jlong_type,
    jfloat_type,
    jdouble_type,
    jvoid_type
} jni_type;



void 
trace_jvalue(jni_type type, jvalue v, const char* str)
{
    char temp[256];
    switch (type)
    {
    case jboolean_type: sprintf(temp, " Boolean %s = %s", str, 
				(v.z ? "true" : "false")); break;
    case jbyte_type: sprintf(temp, "Byte %s = %d", str, v.b); break;
    case jchar_type: sprintf(temp, "Char %s = %c", str, v.c); break;
    case jshort_type: sprintf(temp, "Short %s = %d", str, v.s); break;
    case jint_type: sprintf(temp, "Int %s = %d", str, v.i); break;
	
    case jlong_type: 
	sprintf(temp, "Long %s = %d ", str, (int) v.j);
	break;
    case jfloat_type: sprintf(temp, "Float %s = %f", str, v.f); break;
    case jdouble_type: sprintf(temp, "Double %s = %g", str, v.d); break;
    case jvoid_type: sprintf(temp, "Void %s = void", str); break;
    case jobject_type: sprintf(temp, "Object %s = Object", str); break;
    default:    sprintf(temp, "Error type"); 
    }
}



/************************************************************************
 *              Reading and Writing data 
 *
 ************************************************************************/
const char*
get_jni_name(jni_type t) {
    switch(t) {
    case jboolean_type: return "boolean";
    case jbyte_type: return "byte";
    case jchar_type: return "char";
    case jshort_type: return "short";
    case jint_type: return "int";
    case jlong_type: return "jlong";
    case jfloat_type: return "jfloat";
    case jdouble_type: return "jdouble";
    case jobject_type: return "jobject";
    case jvoid_type: return "jvoid";
    default:
	native_error("Unknown type - cannot get name");
    }
}

/* Return the size in bytes of a given jni_type */
int 
get_jni_type_size(jni_type t) {
    switch(t) {
    case jboolean_type: return (sizeof(jboolean));
    case jbyte_type: return (sizeof(jbyte));
    case jchar_type: return (sizeof(jchar));
    case jshort_type: return (sizeof(jshort));
    case jint_type: return (sizeof(jint));
    case jlong_type: return(sizeof(jlong));
    case jfloat_type: return(sizeof(jfloat));
    case jdouble_type: return(sizeof(jdouble));
    case jobject_type: return(sizeof(jobject));
    case jvoid_type: return(0);
    default:
	native_error("Unknown type - cannot get size");
    }
}



void 
get_args(JNIEnv* env, int pipe, int nargs, char* *sig, jvalue* *args) {
    if (nargs > 0) {
	/* Get the signature, and null terminate it */
	*sig = (char*) malloc(nargs + 1);
	get_bytes(pipe, *sig, nargs);
	(*sig)[nargs] = '\0';
	
	/* Get the arguments */
	*args = malloc(nargs * sizeof(jvalue));
	get_bytes(pipe, *args, nargs * sizeof(jvalue));
    } else {
	*sig = NULL;
	*args = NULL;
    }
}

void 
free_args(JNIEnv* env, int nargs, char* *sig, jvalue* *args) {
    if (nargs > 0) {
	free(*sig);
	free(*args);

	*sig = NULL;
	*args = NULL;
    }
}

void 
trace_call(int pipe, JNIEnv* env, int nargs, jni_type res, int objorclass,
		int method, char* sig, int ctx) 
{
    native_trace("[%d] Nargs=%d \n\tReturn Type=%s "
		 "\n\tObj/Clss= %X %s\n\tMet/Fld=%X %s"
		 "\n\tsig=%s ctx=%X\n",
		 pipe,
		 nargs,  get_jni_name(res), 
		 objorclass, lookup_name(objorclass),
		 method, lookup_name(method),
		 STR_OR_NULL(sig), ctx);
}

void 
trace_call_secure(int pipe, JNIEnv* env, int nargs, jni_type res, int objorclass,
		int method, char* sig, int ctx, char * origin, int ubr, int ujp) 
{
    native_trace("trace_call_secure(): [pipe= %d] Nargs=%d \n\tReturn Type=%s "
		 "\n\tObj/Clss= %X %s\n\tMet/Fld=%X %s"
		 "\n\tsig=%s ctx=%X"
		 "\n\torigin='%s'"
		 "\n\tUniversalBrowserRead=%d UniversalJavaPerm=%d\n",
		 pipe,
		 nargs,  get_jni_name(res), 
		 objorclass, lookup_name(objorclass),
		 method, lookup_name(method),
		 STR_OR_NULL(sig), ctx, STR_OR_NULL(origin),ubr,ujp);

}

void 
trace_call2(FILE* out, char* msg, int pipe, JNIEnv* env, int nargs, jni_type res, int objorclass,
		int method, char* sig, int ctx) 
{
    native_trace("%s [pipe= %d] Nargs=%d \n\tReturn Type=%s "
		 "\n\tObj/Clss= %X %s\n\tMet/Fld=%X %s"
		 "\n\tsig=%s ctx=%X\n",
	         msg,
		 pipe,
		 nargs,  get_jni_name(res), 
		 objorclass, lookup_name(objorclass),
		 method, lookup_name(method),
		 STR_OR_NULL(sig), ctx);
}

void 
trace_call2_secure(FILE* out, char* msg, int pipe, JNIEnv* env, int nargs, jni_type res, int objorclass,
		int method, char* sig, int ctx, char * origin, int ubr, int ujp) 
{
    native_trace("%s [pipe= %d] Nargs=%d \n\tReturn Type=%s "
		 "\n\tObj/Clss= %X %s\n\tMet/Fld=%X %s"
		 "\n\tsig=%s ctx=%X"
                 "\n\torigin=%s"
                 "\n\tUniversalBrowserRead=%d UniversalJavaPerm=%d\n",
	         msg,
		 pipe,
		 nargs,  get_jni_name(res), 
		 objorclass, lookup_name(objorclass),
		 method, lookup_name(method),
		 STR_OR_NULL(sig), ctx, STR_OR_NULL(origin),ubr,ujp);
}

/* Send a response on the env pipe */
void 
send_response(JNIEnv* env, int pipe, void *response, int len) {

    int nbytes_written;
    
    /* Write to the env pipe */
    
    write_fully(pipe, response, len);

}

void
send_val_of_type(JNIEnv* env, int pipe, jni_type t, jvalue* val) {

    switch(t) {
    case jboolean_type:  
	send_response(env, pipe, &(val->z), sizeof(jboolean)); 
	break;
    case jbyte_type:  
	send_response(env,pipe, &(val->b), sizeof(jbyte)); 
	break;
    case jchar_type:  
	send_response(env,pipe, &(val->c), sizeof(jchar)); 
	break;
    case jshort_type:  
	send_response(env,pipe, &(val->s), sizeof(jshort)); 
	break;
    case jint_type:  
	send_response(env,pipe, &(val->i), sizeof(jint)); 
	break;
    case jlong_type:  
	send_response(env,pipe, &(val->j), sizeof(jlong)); 
	break;
    case jfloat_type: 
	send_response(env,pipe, &(val->f), sizeof(jfloat)); 
	break;
    case jdouble_type:  
	send_response(env,pipe, &(val->d), sizeof(jdouble)); 
	break;
    case jobject_type:  
	send_response(env,pipe, &(val->l), sizeof(jobject)); 
	break;
    case jvoid_type:
	/* Send nothing */
	break;
    default: 
	native_error("Unknown val type in send_val_of_type");
    }

}

/* Send an ack on the env pipe */
void send_OK(JNIEnv* env, int pipe, void *response, int len) {
    int code = JAVA_PLUGIN_RETURN;

    if ((len != 0) && (response != NULL)) {
      char *msg = (char *) malloc(len + 4);
      memcpy(msg, &code, 4);
      memcpy(msg + 4, response, len);

      write_fully(pipe, msg, len + 4);

      free(msg);

    } else {
      write_fully(pipe, &code, 4);
    }
}



/* Send an ack on the env pipe */
void 
send_OK_val_of_type(JNIEnv* env, int pipe, jni_type t, jvalue *val){

    int code = JAVA_PLUGIN_RETURN;
    int jni_type_size = get_jni_type_size(t);
    int msg_size = jni_type_size + 4;
    char *msg = (char *) malloc(msg_size);
    
    /*
    fprintf(stdout, "server - returning val of type: %s msg_size=%d\n", 
	    get_jni_name(t), msg_size);
    */

    /* First send the OK to indicate that this is 
       not recursive */
    memcpy(msg, &code, 4);

    /* Then send the data */
    switch(t) {
    case jboolean_type:  
      memcpy(msg + 4, &(val->z), sizeof(jboolean)); 
      break;
    case jbyte_type:  
      memcpy(msg + 4, &(val->b), sizeof(jbyte)); 
      break;
    case jchar_type:  
      memcpy(msg + 4, &(val->c), sizeof(jchar)); 
      break;
    case jshort_type:  
      memcpy(msg + 4, &(val->s), sizeof(jshort)); 
      break;
    case jint_type:  
      memcpy(msg + 4, &(val->i), sizeof(jint)); 
      break;
    case jlong_type:  
      memcpy(msg + 4, &(val->j), sizeof(jlong)); 
      break;
    case jfloat_type: 
      memcpy(msg + 4, &(val->f), sizeof(jfloat)); 
      break;
    case jdouble_type:  
      memcpy(msg + 4, &(val->d), sizeof(jdouble)); 
      break;
    case jobject_type: 
      {
	jobject final = val->l;
	native_trace("Returning object: %X\n", (int) final);
	memcpy(msg + 4, &final, sizeof(jobject)); 
	break;
      }
    case jvoid_type:
      /* Send nothing */
      break;
    default: 
      native_error("Unknown val type in send_val_of_type");
    }
    send_response(env, pipe, msg, msg_size);

    /*    send_val_of_type(env, pipe, t, response); */

    free(msg);
}

/* Read a result from the env's pipe that is of a specified type
   into the right field of the jvalue union */
void
get_val_of_type(JNIEnv* env, int pipe,	int val_type, jvalue* val) 
{
   switch (val_type) {
    case jboolean_type:  
	get_bytes(pipe, &(val->z), sizeof(jboolean)); 
	break;
    case jbyte_type:  
	get_bytes(pipe, &(val->b), sizeof(jbyte)); 
	break;
    case jchar_type: 
	get_bytes(pipe, &(val->c), sizeof(jchar)); 
	break;
    case jshort_type:  
	get_bytes(pipe, &(val->s), sizeof(jshort)); 
	break;
    case jint_type:  
	get_bytes(pipe, &(val->i), sizeof(jint)); 
	break;
    case jlong_type:  
	get_bytes(pipe, &(val->j), sizeof(jlong)); 
	break;
    case jfloat_type:  
	get_bytes(pipe, &(val->f), sizeof(jfloat)); 
	break;
    case jdouble_type:  
	get_bytes(pipe, &(val->d), sizeof(jdouble)); 
	break;
    case jobject_type:  
      {
	get_bytes(pipe, &(val->l), sizeof(jobject)); 
	native_trace("remote:Get val of type %X\n", (int) val->l);
	break;
      }
    default: 
	native_error("[%d] get_val_of_type bad type=%d", pipe, val_type);
    }

}



/************************************************************************
 *              Wrapped JNI calls for checking, convenience
 *
 ************************************************************************/

void 
check_exception(char* msg, JNIEnv* env) {
  
    if (env == NULL) 
	native_trace("Env is NULL!");
    if ((*env)->ExceptionOccurred(env)) {
	native_trace("An exception on this env: %d %s\n", (int) env, msg);
	(*env)->ExceptionDescribe(env);
    } else {
      /* native_trace("Env is ok. %d %s\n", (int) env, msg);*/
    }

}

void 
check_result(char* msg, int val) {
    if (val == (int)NULL) 
      native_error(" %s Null result!");
}


/* Preliminary support for remote jni native calls */
void*
wrap_remote(int index) {
    static void * handle = NULL;
    char symname[50];
    void *retval;

    if (handle == NULL)
	handle = dlopen("libtemplate.so", RTLD_LAZY);

    sprintf(symname, "template_sym_%d", index);

    retval = dlsym(handle, symname);
    return retval;
}


/* Find a class in a local frame and make it a global ref */
jclass
wrapFindClassGlobal(JNIEnv* env, char* nm) {
    jclass cl, res;

    (*env)->PushLocalFrame(env, 10);


    cl = (*env)->FindClass(env, nm);

    /* Make it a global ref only if it is valid */
    if (cl != NULL) 
      res = (jclass) (*env)->NewGlobalRef(env, cl);
    else {
      res = cl;
    }

    (*env)->PopLocalFrame(env, NULL);

    return res;
}


/* Get the class for an object. Trace call */
jclass
wrapGetObjectClass(JNIEnv* env, jobject obj) {
    jclass res;
    res = (*env)->GetObjectClass(env, obj);
    return res;
}


jmethodID
wrapGetMethodID(JNIEnv* env, jclass clz, char* method, char* sig) {
    jmethodID methodid = (jmethodID) (*env)->GetMethodID(env, clz, method, 
							 sig);

    CHECK_EXC_RET("GetMethodID", env, (int) methodid);

    return methodid;
}

jmethodID
wrapGetStaticMethodID(JNIEnv* env, jclass clz, char* method, char* sig) {
    jmethodID methodid = (jmethodID) (*env)->GetStaticMethodID(env, clz, 
							       method, sig);
    CHECK_EXC_RET("GetStaticMethodID", env, (int) methodid);
    return methodid;
}

jobject 
wrapToReflectedMethod(JNIEnv* env, jclass clz, jmethodID methodID, 
		      jboolean isStatic) {
    jobject obj = (*env)->ToReflectedMethod(env, clz, methodID, isStatic);

    CHECK_EXC_RET("ToReflectedMethod", env, (int) obj);
    return obj;
}

jobject 
wrapToReflectedField(JNIEnv* env, jclass clz, jfieldID fieldID, 
		     jboolean isStatic) {
    jobject obj =  (*env)->ToReflectedField(env, clz, fieldID, isStatic);

    CHECK_EXC_RET("ToReflectedField", env, (int) obj);
    return obj;
}

jint
getVersion(JNIEnv* env) {
    return (*env)->GetVersion(env);
}

jboolean
wrapIsInstanceOf(JNIEnv* env, jobject obj, jclass clazz) {
    return (*env)->IsInstanceOf(env, obj, clazz);
}


void
wrapExceptionClear(JNIEnv* env) {
    (*env)->ExceptionClear(env);
}

/* A lightweight way to check whether
   Exception is thrown or not 
*/
jboolean
wrapExceptionCheck(JNIEnv* env) {
  if ((*env)->ExceptionCheck(env) == JNI_TRUE)
  {
    native_trace("Exception occurred");
    return JNI_TRUE;
  }
  else
    return JNI_FALSE;
}

/************************************************************************
 *              Initialization of some static globals for efficiency 
 *
 ************************************************************************/
static int are_globals_initialized = 0;

static jclass      g_jclass_Class = NULL;
static jclass      g_jclass_Object = NULL;
static jclass      g_jclass_Boolean = NULL;
static jclass      g_jclass_Byte = NULL;
static jclass      g_jclass_Character = NULL;
static jclass      g_jclass_Short = NULL;
static jclass      g_jclass_Integer = NULL;
static jclass      g_jclass_Long = NULL;
static jclass      g_jclass_Float = NULL;
static jclass      g_jclass_Double = NULL;
static jclass      g_jclass_Void = NULL;
static jmethodID   g_jmethod_Object_init = NULL;
static jmethodID   g_jmethod_Boolean_booleanValue = NULL;
static jmethodID   g_jmethod_Byte_byteValue = NULL;
static jmethodID   g_jmethod_Character_charValue = NULL;
static jmethodID   g_jmethod_Short_shortValue = NULL;
static jmethodID   g_jmethod_Integer_intValue = NULL;
static jmethodID   g_jmethod_Long_longValue = NULL;
static jmethodID   g_jmethod_Float_floatValue = NULL;
static jmethodID   g_jmethod_Double_doubleValue = NULL;
static jmethodID   g_jmethod_Boolean_init = NULL;
static jmethodID   g_jmethod_Byte_init = NULL;
static jmethodID   g_jmethod_Character_init = NULL;
static jmethodID   g_jmethod_Short_init = NULL;
static jmethodID   g_jmethod_Integer_init = NULL;
static jmethodID   g_jmethod_Long_init = NULL;
static jmethodID   g_jmethod_Float_init = NULL;
static jmethodID   g_jmethod_Double_init = NULL;

static jclass      g_jclass_SecureInvocation = NULL;
static jmethodID   g_jmethod_SecureInvocation_ConstructObject = NULL;
static jmethodID   g_jmethod_SecureInvocation_CallMethod = NULL;
static jmethodID   g_jmethod_SecureInvocation_GetField = NULL;
static jmethodID   g_jmethod_SecureInvocation_SetField = NULL;

/* For handling the monitor on the spontaneous pipe */
static jclass      g_ojiplugin_class;
static jmethodID   g_ojiplugin_acquireThreadPipe;
static jobject     g_jobject_queue_lock = NULL;

void
initialize_globals(JNIEnv* m_env) {
  jobject    jlobject = NULL;

  if (are_globals_initialized) return;

  if(getenv("JAVA_PLUGIN_TRACE")) {
      tracing_on = 1;
  }
  g_ojiplugin_class = wrapFindClassGlobal(m_env, 
										  "sun/plugin/navig/motif/OJIPlugin");
  g_ojiplugin_acquireThreadPipe = 
	wrapGetStaticMethodID(m_env, g_ojiplugin_class, 
						  "acquirePipeForCurrentThread", "()I");

  g_jclass_Object     = wrapFindClassGlobal(m_env,"java/lang/Object");
  g_jclass_Boolean    = wrapFindClassGlobal(m_env,"java/lang/Boolean");
  g_jclass_Byte       = wrapFindClassGlobal(m_env,"java/lang/Byte");
  g_jclass_Character  = wrapFindClassGlobal(m_env,"java/lang/Character");
  g_jclass_Short      = wrapFindClassGlobal(m_env,"java/lang/Short");
  g_jclass_Integer    = wrapFindClassGlobal(m_env,"java/lang/Integer");
  g_jclass_Long       = wrapFindClassGlobal(m_env,"java/lang/Long");
  g_jclass_Float      = wrapFindClassGlobal(m_env,"java/lang/Float");
  g_jclass_Double     = wrapFindClassGlobal(m_env,"java/lang/Double");
   

  g_jmethod_Object_init           = wrapGetMethodID(m_env, g_jclass_Object, "<init>", "()V");
  g_jmethod_Boolean_booleanValue  = wrapGetMethodID(m_env,g_jclass_Boolean, "booleanValue", "()Z");
  g_jmethod_Byte_byteValue        = wrapGetMethodID(m_env,g_jclass_Byte, "byteValue", "()B");
  g_jmethod_Character_charValue   = wrapGetMethodID(m_env,g_jclass_Character, "charValue", "()C");
  g_jmethod_Short_shortValue      = wrapGetMethodID(m_env,g_jclass_Short, "shortValue", "()S");
  g_jmethod_Integer_intValue      = wrapGetMethodID(m_env,g_jclass_Integer, "intValue", "()I");
  g_jmethod_Long_longValue        = wrapGetMethodID(m_env,g_jclass_Long, "longValue", "()J");
  g_jmethod_Float_floatValue      = wrapGetMethodID(m_env,g_jclass_Float, "floatValue", "()F");
  g_jmethod_Double_doubleValue    = wrapGetMethodID(m_env,g_jclass_Double, "doubleValue", "()D");

  g_jmethod_Boolean_init      = wrapGetMethodID(m_env,g_jclass_Boolean, "<init>", "(Z)V");
  g_jmethod_Byte_init         = wrapGetMethodID(m_env,g_jclass_Byte, "<init>", "(B)V");
  g_jmethod_Character_init    = wrapGetMethodID(m_env,g_jclass_Character, "<init>", "(C)V");
  g_jmethod_Short_init        = wrapGetMethodID(m_env,g_jclass_Short, "<init>", "(S)V");
  g_jmethod_Integer_init      = wrapGetMethodID(m_env,g_jclass_Integer, "<init>", "(I)V");
  g_jmethod_Long_init         = wrapGetMethodID(m_env,g_jclass_Long, "<init>", "(J)V");
  g_jmethod_Float_init        = wrapGetMethodID(m_env,g_jclass_Float, "<init>", "(F)V");
  g_jmethod_Double_init       = wrapGetMethodID(m_env,g_jclass_Double, "<init>", "(D)V");

  g_jclass_SecureInvocation   = wrapFindClassGlobal(m_env,"sun/plugin/liveconnect/SecureInvocation");

  g_jmethod_SecureInvocation_ConstructObject = wrapGetStaticMethodID(m_env, g_jclass_SecureInvocation,
																	 "ConstructObject",
																	 "(Ljava/lang/Class;Ljava/lang/reflect/Constructor;[Ljava/lang/Object;Ljava/lang/String;ZZ)Ljava/lang/Object;");

  g_jmethod_SecureInvocation_CallMethod = wrapGetStaticMethodID(m_env, g_jclass_SecureInvocation,
																"CallMethod",
																"(Ljava/lang/Class;Ljava/lang/Object;Ljava/lang/reflect/Method;[Ljava/lang/Object;Ljava/lang/String;ZZ)Ljava/lang/Object;");

  g_jmethod_SecureInvocation_GetField = wrapGetStaticMethodID(m_env,g_jclass_SecureInvocation,
															  "GetField",
															  "(Ljava/lang/Class;Ljava/lang/Object;Ljava/lang/reflect/Field;Ljava/lang/String;ZZ)Ljava/lang/Object;");

  g_jmethod_SecureInvocation_SetField = wrapGetStaticMethodID(m_env,g_jclass_SecureInvocation,
															  "SetField",
															  "(Ljava/lang/Class;Ljava/lang/Object;Ljava/lang/reflect/Field;Ljava/lang/Object;Ljava/lang/String;ZZ)V");

  jlobject                      = (*m_env)->NewObject(m_env, g_jclass_Object, g_jmethod_Object_init);
  g_jobject_queue_lock          = (*m_env)->NewGlobalRef(m_env, jlobject);
  (*m_env)->DeleteLocalRef(m_env, jlobject);

  are_globals_initialized = 1;
}



/************************************************************************
 *              Secure JNI Methods
 *
 ************************************************************************/
/* Convert a Jvalue to a java object for use in the reflection calls
   to set the object field. The jni_type indicates the type of 
   the jvalue enum */
int ConvertJValueToJava(JNIEnv *m_env, jvalue val, jni_type type,
			 jobject* result)
{
    switch(type) {
    case jobject_type:
        *result = val.l;
	break;
    case jboolean_type: 
        *result = (*m_env)->NewObject(m_env, g_jclass_Boolean, 
				   g_jmethod_Boolean_init, val.z);
	break;
    case jbyte_type:
        *result = (*m_env)->NewObject(m_env, g_jclass_Byte, 
				   g_jmethod_Byte_init, val.b);
	break;
    case jchar_type:
        *result = (*m_env)->NewObject(m_env,
				      g_jclass_Character, 
				      g_jmethod_Character_init, val.c);
	break;
    case jshort_type:
        *result = (*m_env)->NewObject(m_env,
				      g_jclass_Short, 
				      g_jmethod_Short_init, val.s);
	break;
    case jint_type:
        *result = (*m_env)->NewObject(m_env,
				      g_jclass_Integer, 
				      g_jmethod_Integer_init, val.i);
	break;
    case jlong_type:
        *result = (*m_env)->NewObject(m_env, g_jclass_Long, 
				      g_jmethod_Long_init, val.j);
	break;
    case jfloat_type:
        *result = (*m_env)->NewObject(m_env, g_jclass_Float, 
				      g_jmethod_Float_init, val.f);
	break;
    case jdouble_type:
        *result = (*m_env)->NewObject(m_env, g_jclass_Double, 
				      g_jmethod_Double_init, val.d);
	break;
    default:
	native_error("Unknown type for conversion of JValueToJava obj");
	return 0;
    }

    return 1;
}

/* Convert the character type used to describe signatures in remotejni
   to the enum type */
jni_type GetEnumTypeFromCharType(char char_type) {
    switch(char_type) {
    case 'Z': return jboolean_type; 
    case 'B': return jbyte_type; 
    case 'C': return jchar_type; 
    case 'S': return jshort_type;
    case 'I': return jint_type; 
    case 'J': return jlong_type; 
    case 'F': return jfloat_type;
    case 'D': return jdouble_type; 
    case 'L': return jobject_type; 
    default:
	native_error("Unknown type for conversion of Char to Enum Type %c\n",
		     char_type);
	return 0;
    }
}

/* Acquire the request ID before dispatching the JS call from Java.
   Reuse the ID when it reaches the 2^32-1.
*/
int getJSRequestID() {
	static unsigned int request_id = 0;

	return request_id++;
}


/* Convert a java object to a jvalue.  If 'obj' is an actual java object 
   and not a primitive type, it is turned into a global ref after 
   being converted into a jobject */
int ConvertJavaToJValue(JNIEnv *m_env,
			jni_type type, 
			jobject obj, 
			jvalue* result)
{
    jclass clazz = NULL;
    jmethodID m = NULL;

    if (type == jvoid_type) {
	result->l = 0;
	return 1;
    } 
    native_trace("ConvertJavaToJValue type=%s\n", get_jni_name(type));;


    switch(type)
    {
        case jboolean_type: {
            result->z = (*m_env)->CallBooleanMethod(m_env, obj, 
				       g_jmethod_Boolean_booleanValue);
	    
            break;
        }
        
        case jbyte_type:    {
            result->b = (*m_env)->CallByteMethod(m_env, obj, 
						 g_jmethod_Byte_byteValue);
            break;
        }

        case jchar_type:    {
            result->c = (*m_env)->CallCharMethod(m_env, obj, 
			 g_jmethod_Character_charValue);
            break;
        }

        case jshort_type:   {
            result->s = (*m_env)->CallShortMethod(m_env,
				  obj, g_jmethod_Short_shortValue);        
            break;
        }

        case jint_type:     {
            result->i = (*m_env)->CallIntMethod(m_env,
				obj, g_jmethod_Integer_intValue);            
            break;
        }

        case jlong_type:   {
            result->j = (*m_env)->CallLongMethod(m_env,
				 obj, g_jmethod_Long_longValue); 
            break;
        }

        case jfloat_type:  {
            result->f = (*m_env)->CallFloatMethod(m_env,
						  obj, 
						  g_jmethod_Float_floatValue);
            break;
        }

        case jdouble_type: {
            result->d = (*m_env)->CallDoubleMethod(m_env,
					   obj, 
					   g_jmethod_Double_doubleValue); 
	    native_trace("Double result: %f\n", (float) result->d);
            break;
        }


        case jvoid_type:
        {
            break;
        }
        case jobject_type:
	  {
	    result->l = obj;
	    break;
	  }
      
        default:
        {
	  native_trace("WIERD RETURN TYPE!\n\n");
	  return 0;
        }
    }
   return 1;
}

int
slen(const char *s) {
    if (s == NULL) 
	return 0;
    return strlen(s);
}

int ConvertJValueArrayToJavaArray(JNIEnv *m_env,
				  jobject method,
				  char* sig,
				  jvalue* args, 
				  jobjectArray* result)
{
    int len, i;
    char* sigi;

    TRACE_CONVERT("ConvertJValueArrayToJavaArray");

    if (method == NULL) {
	TRACE_CONVERT("ConvertJValueArrayToJavaArray: Null method"); 
	return 0;
    }

    if (args != NULL && result == NULL) {
	TRACE_CONVERT("ConvertJValueArrayToJavaArray: null result"); 
	return 0;
    }

    if (args == NULL) {
	TRACE_CONVERT("ConvertJValueArrayToJavaArray: null object , result");
	return 1;
    }

    wrapExceptionClear(m_env); 

    if (!sig) {
	TRACE_CONVERT("ConvertJValueArrayToJavaArray: No parameters ");
	return 1;
    }

    len = slen(sig);
    /* Create an array of dummy objects for the arguments */
    if (len > 0) {
	jclass obj_clz;
	TRACE_CONVERT("ConvertJValueArrayToJavaArray: parameters exist");
	obj_clz = (*m_env)->FindClass(m_env, "java/lang/Object");
	*result = (*m_env)->NewObjectArray(m_env, len, obj_clz, NULL);
    }

    /* Fill the array of dummy arguments with objects, including
       wrapping any primitives */
    sigi = sig;
    for (i=0; i < len; i++, sigi++) {
	jobject val = NULL;
	jni_type enum_type = GetEnumTypeFromCharType(*sigi);
	ConvertJValueToJava(m_env, args[i], enum_type, &val);
	TRACE_CONVERT("Setting next object array element");
	(*m_env)->SetObjectArrayElement(m_env, *result, i, val);
    }
    TRACE_CONVERT("ConvertJValueArrayToJavaArray: success");
    return 1;
}

/* Construct a java object with the given constructor and signature
   The result is turned into a global ref */
int ConstructJavaObject(JNIEnv* m_env,
                        jclass clazz,
			jobject constructor, 
			char* sig,
			jvalue* arg, 
			int ctx, char * origin, int ubr, int ujp, 
			jobject* result) {
    jobjectArray jarg = NULL;
    jstring utforigin = NULL;
    
    /* if (ctx)     ctx->AddRef(); */
    wrapExceptionClear(m_env); 
    
    if (!ConvertJValueArrayToJavaArray(m_env, constructor, sig, arg, &jarg)) {
	native_error("ConstructJavaObject failed. Could not convert args");
	return 0;
    }
    
    utforigin = (*m_env)->NewStringUTF(m_env,origin);
    *result = (*m_env)->CallStaticObjectMethod(m_env,
					       g_jclass_SecureInvocation,
					       g_jmethod_SecureInvocation_ConstructObject,
                                               clazz, constructor, jarg, utforigin,
                                               ubr, ujp);


    if (utforigin)
        (*m_env)->DeleteLocalRef(m_env, utforigin);

    if (jarg)
        (*m_env)->DeleteLocalRef(m_env, jarg);

    /* Turn the result into a global ref */
    if (*result) {
	return 1;
    } else {
	native_error("ConstructJavaObject failed");
	return 0;
    }
}

/* The result is turned into a global ref if it is an object type */
int CallJavaMethod(JNIEnv *m_env, jclass clazz, jni_type type, jobject obj, jobject method, 
		   char* sig, jvalue* arg,  int ctx,char * origin, int ubr, int ujp,  jvalue* result)
{
    jobjectArray jarg = NULL;
    jobject ret;
    jstring utforigin = NULL;
    int error = 0;

    wrapExceptionClear(m_env); 
    
    if (!(ConvertJValueArrayToJavaArray(m_env, method, sig, arg, &jarg)))
	return 0;


    CHECK_EXC("call_java_method", m_env);

    utforigin = (*m_env)->NewStringUTF(m_env,origin);
    /* ConvertJavaToJValue creates a global reference for ret as well,
       if it is a non-primitive object */
    ret = (*m_env)->CallStaticObjectMethod(m_env,
					   g_jclass_SecureInvocation,
					   g_jmethod_SecureInvocation_CallMethod,
					   clazz, obj, method, jarg, utforigin,
					   ubr, ujp);

    //reset the result to handle exception cases
    memset(result, 0, sizeof(jvalue));

    /* Fix bug 4796147, if exceptions were thrown in the last call,
       print out the exception and return right away rather than calling
       ConvertJavaToJValue, otherwise JVM will crash or hung. It is up to
       caller to clear the exception
    */
    if (wrapExceptionCheck(m_env) == JNI_TRUE)
        error = 0;
    else
	error = ConvertJavaToJValue(m_env, type, ret, result);

    if (utforigin)
	(*m_env)->DeleteLocalRef(m_env, utforigin);

    if (jarg)
	(*m_env)->DeleteLocalRef(m_env, jarg);

    return error;
}



int GetJavaField(JNIEnv *m_env,
                 jclass clazz,
		 jni_type type, 
		 jobject obj, 
		 jobject field, 
		 int ctx,char * origin, int ubr, int ujp, 
		 jvalue* result)
{
    jobject ret;
    jstring utforigin = NULL;

    CHECK_EXC("GetJavaField", m_env);

    wrapExceptionClear(m_env); 

    utforigin = (*m_env)->NewStringUTF(m_env,origin);
    ret = (*m_env)->CallStaticObjectMethod(m_env,
				   g_jclass_SecureInvocation,
				   g_jmethod_SecureInvocation_GetField,
                                   clazz, obj, field, utforigin,
                                   ubr, ujp);

    //reset the result to handle exception cases
    memset(result, 0, sizeof(jvalue));

     /* If exception thrown, function should return right 
       away rather than continuing since the return value 
       may be a bogus value */
    if (wrapExceptionCheck(m_env))
      return 0;
    
    if (utforigin)
        (*m_env)->DeleteLocalRef(m_env, utforigin);

    return ConvertJavaToJValue(m_env, type, ret, result);

}

int SetJavaField(JNIEnv *m_env,
		 jclass clazz,
		 jni_type type,
		 jobject obj, 
		 jobject field, 
		 jvalue val, 
		 int ctx,char * origin, int ubr, int ujp)
{
    jmethodID class_getType_methodID;
    jclass fieldClazz = NULL;
    jobject jval = NULL;
    jclass field_type;
    jstring utforigin = NULL;


    wrapExceptionClear(m_env); 

    fieldClazz = wrapGetObjectClass(m_env, field);

    if (!fieldClazz) {
	native_error("SetJavaField: fieldclazz was null");
	return 0;
    }
    
    if (!(ConvertJValueToJava(m_env, val, type, &jval))) {
	native_error("SetJavaField: Could not convert Jvalue to Java");
	return 0;
    }

    utforigin = (*m_env)->NewStringUTF(m_env,origin);
    (*m_env)->CallStaticVoidMethod(m_env,
				   g_jclass_SecureInvocation,
				   g_jmethod_SecureInvocation_SetField, 
                                   clazz, obj, field, jval, utforigin,
                                   ubr, ujp);

    if (jval)
      (*m_env)->DeleteLocalRef(m_env, jval);
    
    if (utforigin)
      (*m_env)->DeleteLocalRef(m_env, utforigin);

    if (fieldClazz)
      (*m_env)->DeleteLocalRef(m_env, fieldClazz);

    return 1;

}



/* Create a new object. The result is turned into a global ref */
void CSecureJNI2_NewObject(JNIEnv *m_env,
			   jclass clazz, 
			   jmethodID methodID, 
			   char* sig,
			   jvalue* args, 
			   jobject* result,
			   int ctx,char * origin, int ubr, int ujp)
{
    jobject constructor = NULL;

    if (m_env == NULL || clazz == NULL || methodID == NULL || result == NULL)
	native_error("CSecureJNI2_NewObject: Bad parameters");

    if (getVersion(m_env) == JNI_VERSION_1_1)
	native_error("CSecureJNI2_NewObject on a  1.1 VM?");


    constructor = wrapToReflectedMethod(m_env,clazz, methodID, JNI_FALSE); 

    if (constructor == NULL)
	native_error("CSecureJNI2_NewObject null constructor");

    ConstructJavaObject(m_env, clazz, constructor, sig, args, ctx, origin, ubr, ujp, result);

    if (constructor)
      (*m_env)->DeleteLocalRef(m_env, constructor);

}

/* Call the method. If the result is an object.
 * It is made into a global  ref 
 */
void CSecureJNI2_CallMethod(JNIEnv *m_env,jni_type type, jobject obj, 
			    jmethodID methodID, char* sig, jvalue *args, 
			    jvalue* result, int ctx, char * origin, int ubr, int ujp)
{
    jobject method = NULL;
    jclass clazz = NULL;
    
    if (m_env == NULL || obj == NULL || methodID == NULL)
	native_error("CSecureJNI2_CallMethod: Bad parameters");


    CHECK_EXC("CSecureJNI_CallMethod", m_env);
    
    clazz = wrapGetObjectClass(m_env, obj); 

    if (clazz == NULL)
	native_error("Bad class in SecureCallMethod");

    method = wrapToReflectedMethod(m_env,clazz, methodID, JNI_FALSE); 

    if (method == NULL)
	native_error("CSecureJNI2_CallMethod: Bad java method");
    else
        CallJavaMethod(m_env, clazz, type, obj, method, sig, args, ctx, origin, ubr, ujp, result);
 
    if (method)
        (*m_env)->DeleteLocalRef(m_env, method);
	
    if (clazz)
        (*m_env)->DeleteLocalRef(m_env, clazz);

}

void CSecureJNI2_CallNonvirtualMethod(JNIEnv *m_env,
				      jni_type type,
				      jobject obj, 
				      jclass clazz,
				      jmethodID methodID,
				      char* sig,
				      jvalue* args, 
				      jvalue* result,
				      int ctx, char * origin, int ubr, int ujp)
{
    jobject method = NULL;

    if (m_env == NULL || obj == NULL || clazz == NULL || methodID == NULL)
	native_error("CSecureJNI2_CallNonVirtualMethod: Bad parameters");

    if (wrapIsInstanceOf(m_env, obj, clazz) == JNI_FALSE)
	native_error("CSecureJNI2_CallNonVirtualMethod: Bad object type");

    method = wrapToReflectedMethod(m_env,clazz, methodID, JNI_FALSE); 

    if (method == NULL)
	native_error("CSecureJNI2_CallNonVirtualMethod: Bad object type");
    else
        CallJavaMethod(m_env, clazz, type, obj, method, sig, args, ctx, origin, ubr, ujp, result);

    if (method)
        (*m_env)->DeleteLocalRef(m_env, method);

}

void CSecureJNI2_GetField(JNIEnv *m_env,
			  jni_type type,
			  jobject obj, 
			  jfieldID fieldID,
			  jvalue* result,
			  int ctx, char * origin, int ubr, int ujp)
{
    jni_type fieldType;
    jobject field = NULL;
    jclass clazz = NULL;

    if (m_env == NULL || obj == NULL || fieldID == NULL)
      native_error("CSecureJNI2_GetField: Bad parameters");


    clazz = wrapGetObjectClass(m_env, obj);

    if (clazz == NULL)
      native_error("CSecureJNI2_GetField: Bad class");

    field = wrapToReflectedField(m_env,clazz, fieldID, JNI_FALSE); 

    if (field == NULL)
      native_error("CSecureJNI2_GetField: Bad field");
    else
      GetJavaField(m_env, clazz, type, obj, field, ctx, origin, ubr, ujp, result);

    if (field)
        (*m_env)->DeleteLocalRef(m_env, field);

    if (clazz)
        (*m_env)->DeleteLocalRef(m_env, clazz);

}

void CSecureJNI2_SetField(JNIEnv *m_env,
			   jni_type type,
			   jobject obj, 
			   jfieldID fieldID,
			   jvalue val,
			   int ctx,char * origin, int ubr, int ujp)
{
    jclass clazz = NULL;
    jobject field = NULL;

    if (m_env == NULL || obj == NULL || fieldID == NULL)
        native_error("CSecureJNI2_SetField: bad parameters");

    if (getVersion(m_env) == JNI_VERSION_1_1)
        native_error("CSecureJNI2_SetField: bad version");

    clazz = wrapGetObjectClass(m_env, obj);

    if (clazz == NULL)
      native_error("CSecureJNI2_SetField: bad class");

    field = wrapToReflectedField(m_env,clazz, fieldID, JNI_FALSE); 

    if (field == NULL)
	native_error("CSecureJNI2_SetField: bad field");
    else
        SetJavaField(m_env, clazz, type, obj, field, val, ctx, origin, ubr, ujp);

    if (field)
        (*m_env)->DeleteLocalRef(m_env, field);
	
    if (clazz)
        (*m_env)->DeleteLocalRef(m_env, clazz);

}


void CSecureJNI2_CallStaticMethod(JNIEnv *m_env,
				  jni_type type,
				  jclass clazz,
				  jmethodID methodID,
				  char* sig,
				  jvalue *args, 
				  jvalue* result,
				  int ctx,char * origin, int ubr, int ujp)
{
    jobject method = NULL;

    if (m_env == NULL || clazz == NULL || methodID == NULL)
      native_error("CSecureJNI2_CallStaticMethod: bad parameters");

    method = wrapToReflectedMethod(m_env,clazz, methodID, JNI_TRUE); 

    if (method == NULL)
	native_error("SecureCallStaticMethod: bad method");

    CallJavaMethod(m_env, clazz, type, NULL, method, sig, args, ctx, origin, ubr, ujp, result);
    
    if (method)
        (*m_env)->DeleteLocalRef(m_env, method);

}


void CSecureJNI2_GetStaticField(JNIEnv *m_env,
				jni_type type,
				jclass clazz, 
				jfieldID fieldID,
				jvalue* result,
				int ctx,char * origin, int ubr, int ujp)
{
    jobject field = NULL;

    if (m_env == NULL || clazz == NULL || fieldID == NULL)
	native_error("SecureGetStaticField: Bad parameters");

    field = wrapToReflectedField(m_env,clazz, fieldID, JNI_TRUE); 

    if (field == NULL)
	native_error("SecureGetStaticField: Bad field");
    else
        GetJavaField(m_env, clazz, type, NULL, field, ctx, origin, ubr, ujp, result);

    if (field)
        (*m_env)->DeleteLocalRef(m_env, field);
}

void CSecureJNI2_SetStaticField(JNIEnv *m_env,
				 jni_type type,
				 jclass clazz, 
				 jfieldID fieldID,
				 jvalue val,
				 int ctx,char * origin, int ubr, int ujp)
{
    jobject field = NULL;

    if (m_env == NULL || clazz == NULL || fieldID == NULL)
	native_error("SecureSetStaticField: bad parameters");

    field = wrapToReflectedField(m_env,clazz, fieldID, JNI_TRUE); 

    if (field == NULL)
	native_error("SecureSetStaticField: Bad field");
    else
        SetJavaField(m_env, clazz, type, NULL, field, val, ctx, origin, ubr, ujp);
    
    if (field)
        (*m_env)->DeleteLocalRef(m_env, field);
}


JNIEXPORT void JNICALL
Java_sun_plugin_navig_motif_AThread_initGlobals(JNIEnv *env, 
						 jclass thr_clz) {
    if (!are_globals_initialized)
	initialize_globals(env);


}

/************************************************************************
 *              Main event handler code 
 *
 ************************************************************************/


void
handle_single_request(JNIEnv* env, jclass thr_clz, jint pipe);

void
handle_code(int code, JNIEnv* env, jclass thr_clz, jint pipe);

/* Main event loop that handles requests for a single thread */
JNIEXPORT void JNICALL
Java_sun_plugin_navig_motif_AThread_handleRequest(JNIEnv *env, 
						  jclass thr_clz, jint pipe) {
    int thread_start_ack = 5050;
    int nbytes = 0;

    /* Send an ack to the JavaVM5 that the thread has started */
    write_fully(pipe, &thread_start_ack, 4);
    
    /* Read ack from Browser */
    nbytes = read(pipe, &thread_start_ack, 4);

    /* Flags are 2 in the fast case */

    {
      int flags = fcntl(pipe, F_GETFL);
      int newflags = flags & ~O_NONBLOCK;
      int finalflags;
      /*      fcntl(pipe, F_SETFL, newflags); */
      finalflags = fcntl(pipe, F_GETFL);

    }


    for(;;) {
	handle_single_request(env, thr_clz, pipe);
    }

}


void
handle_single_request(JNIEnv* env, jclass thr_clz, jint pipe) 
{
    /* Indicate that we are ready to the parent process */

    int code;

    read_message(pipe);

    code = get_bits32(pipe);
    handle_code(code, env, thr_clz, pipe);
}
 
void 
handle_code(int code, JNIEnv* env, jclass thr_clz, jint pipe) {

    approx_call_count++;

    native_trace("handle_code(): [%d %d]**** >>>> %s     \n", 
	    approx_call_count,
	    pipe,
	    protocol_descriptor_to_str(code));

    if (code != JAVA_PLUGIN_EXCEPTION_OCCURED && 
        code != JAVA_PLUGIN_EXCEPTION_DESCRIBE)
        (*env)->ExceptionClear(env);
    switch (code) {
    case JAVA_PLUGIN_JNI_VERSION: 
	{
	    jint ret = (*env)->GetVersion(env);
	    send_response(env, pipe, &ret, 4);
	    break;
	}
    case JAVA_PLUGIN_DEFINE_CLASS: 
	{
	    char *name;
	    jbyte *buf;
	    jobject lref;
	    jsize bufLen;
	    jclass ret;
	    name = get_string(pipe);
	    lref = (jobject) get_bits32(pipe);
	    bufLen = (jsize) get_bits32(pipe);
	    buf = malloc(bufLen *sizeof(bufLen));
	    get_bytes(pipe, buf, bufLen *sizeof(bufLen));
	    ret = (*env)->DefineClass(env, name, lref, buf, bufLen);
	    send_response(env, pipe, &ret, 4);
	    free(name);
	    free(buf);
	    break;
	}
    case JAVA_PLUGIN_FIND_CLASS:
	{
	    char *name;
	    jclass ret; 
	    jclass final_ret;
	    name = get_string(pipe);
	    ret =(*env)->FindClass(env, name);
	    final_ret = ret;
	    send_OK(env, pipe, &final_ret, 4);
	    REGISTER_NAME("Class", name, (int) final_ret);
	    free(name);
	    break;
	}
    case JAVA_PLUGIN_GET_SUPER_CLASS:
	{
	    jclass sub = (jclass)get_bits32(pipe);
	    jclass ret = (*env)->GetSuperclass(env, sub);
	    send_response(env, pipe, &ret, 4);
	    break;
	}
    case JAVA_PLUGIN_IS_SUBCLASS_OF:
	{
	    jclass sub = (jclass)get_bits32(pipe);
	    jclass sup = (jclass)get_bits32(pipe);
	    jboolean ret = (*env)->IsAssignableFrom(env, sub, sup);
	    send_response(env, pipe, &ret, sizeof(jboolean));
	    break;
	}
    case JAVA_PLUGIN_GET_OBJECT_CLASS:
	{
	    jclass clz;
	    jobject obj  = (jobject) get_bits32(pipe);
	    clz =  (*env)->GetObjectClass(env, obj);
	    send_response(env, pipe, &clz, 4);
	    break;
	}
    case JAVA_PLUGIN_IS_INSTANCE_OF: 
	{
	    jobject obj = (jobject) get_bits32(pipe);
	    jclass clz = (jclass) get_bits32(pipe);
	    jboolean ret = (*env)->IsInstanceOf(env, obj, clz);
	    send_response(env, pipe, &ret, sizeof(jboolean));
	    break;
	}
    case JAVA_PLUGIN_IS_SAME_OBJECT: 
	{
	    jobject obj = (jobject) get_bits32(pipe);
	    jobject obj2 = (jobject) get_bits32(pipe);
	    jboolean ret = (*env)->IsSameObject(env, obj, obj2);
	    send_response(env, pipe, &ret, sizeof(jboolean));
	    break;
	}

    /* I'm going to dalay the question of handling global references. At this
       point I see these possiblilities:
       1. All objects used out of process are Global Refs; NewGlobalRef and
       RefeleaseGlobalRef are nops.
       2. we local refs inside this function. These are valid while the
       function is running; the function exits when DetachThread is called.
       Another thought is that GlobalRef operations might have something to do
       with the reference counting. (Oh, god, I'm becoming COM-indoctrinated)
       */
    case JAVA_PLUGIN_NEW_GLOBAL_REF:
	  {
	    jobject obj = (jobject) get_bits32(pipe);
	    jobject ref = (*env)->NewGlobalRef(env, obj);  
	    send_response(env, pipe, &ref, 4);
	    break;
	}
    case JAVA_PLUGIN_RELEASE_GLOBAL_REF:
	  {
	    jobject obj = (jobject) get_bits32(pipe);
	    (*env)->DeleteGlobalRef(env, obj);
	    send_OK(env, pipe, NULL, 0);
	    break;
	}
    case JAVA_PLUGIN_RELEASE_LOCAL_REF:
	{
	    jobject obj = (jobject) get_bits32(pipe);
	    (*env)->DeleteLocalRef(env, obj);
	    send_OK(env, pipe, NULL, 0);
	    break;
	}
    case JAVA_PLUGIN_THROW:
	{
	    jobject exc = (jobject)get_bits32(pipe);
	    jint ret = (*env)->Throw(env, exc);
	    send_response(env, pipe, &ret, 4);
	    break;
	}
    case JAVA_PLUGIN_THROW_NEW:
	{
	    jclass clz = (jclass) get_bits32(pipe);
	    const char * mesg = get_string(pipe);
	    jint ret = (*env)->ThrowNew(env, clz, mesg);
	    free((void *)mesg);
	    send_response(env, pipe, &ret, 4);
	    break;
	}
    case JAVA_PLUGIN_EXCEPTION_OCCURED:
	{
	    jobject exc = (*env)->ExceptionOccurred(env);
	    char retmsg[8];
	    int test = 1051;
	    memcpy(retmsg, &exc, 4);
	    memcpy(retmsg + 4, &test, 4);
	    send_response(env, pipe, retmsg, 8);
	    if (exc != NULL) 
	      native_trace("Exception occured %d\n", (int) exc);
	    else 
	      native_trace("No Exception\n");
	    /*     send_response(env, pipe, &exc, 4); */
	    break;
	}
    case JAVA_PLUGIN_EXCEPTION_DESCRIBE:
	{
	    (*env)->ExceptionDescribe(env);
	    break;
	}
    case JAVA_PLUGIN_EXCEPTION_CLEAR:
	{
	    (*env)->ExceptionClear(env);
	    send_OK(env, pipe, NULL, 0); 
	    break;
	}
    case JAVA_PLUGIN_FATAL_ERROR:
	{
	    const char *mesg = get_string(pipe);
	    (*env)->FatalError(env, mesg);
	    free((void *)mesg);
	    break;
	}
    case JAVA_PLUGIN_GET_FIELD_ID:
	{
	    jclass clz = (jclass) get_bits32(pipe);
	    char * sig;
	    char * name;
	    jfieldID ret;
	    name = get_string(pipe);
	    sig = get_string(pipe);
	    ret = (*env)->GetFieldID(env, clz, name, sig);
	    send_response(env, pipe, &ret, 4);
	    REGISTER_NAME("GetFieldID", name,  (int) ret);
	    free(sig); free(name);
	    break;
	}
    case JAVA_PLUGIN_GET_STATIC_FIELD_ID:
	{
	    jclass clz = (jclass) get_bits32(pipe);
	    char * sig;
	    char * name;
	    jfieldID ret;
	    name = get_string(pipe);
	    sig = get_string(pipe);
	    ret = (*env)->GetStaticFieldID(env, clz, name, sig);
	    REGISTER_NAME("GetStaticFieldID", name, (int) ret);
	    send_response(env, pipe, &ret, 4);
	    free(sig); free(name);
	    break;
	}
    /* It may be the case that we need to handle object fields
       differently. Most likely we will have to convert objects to global
       references. */
#define DEFINE_GETFIELD(type, Result, tag)			\
    case tag:							\
	{							\
	    jobject obj = (jobject) get_bits32(pipe);		\
	    type ret;						\
	    jfieldID fid= (jfieldID) get_bits32(pipe);		\
	    ret = (*env)->Get##Result##Field(env, obj, fid);	\
            send_response(env, pipe, &ret, sizeof(type));			\
	    break;						\
	}

#define DEFINE_GETSTATICFIELD(type, Result, tag)			\
    case tag:							\
	{							\
	    jclass cls = (jclass) get_bits32(pipe);		\
	    type ret;						\
	    jfieldID fid= (jfieldID) get_bits32(pipe);		\
	    ret = (*env)->GetStatic##Result##Field(env, cls, fid);	\
            send_response(env, pipe, &ret, sizeof(type));			\
	    break;						\
	}
    
    case  JAVA_PLUGIN_GET_OBJECT_FIELD: 
	{ 
	    jobject obj = (jobject) get_bits32(pipe); 
	    jobject ret;
	    jfieldID fid= (jfieldID) get_bits32(pipe); 
	    ret = (*env)->GetObjectField(env, obj, fid);
	    send_response(env, pipe, &ret, sizeof(jobject));
	    break; 
	}
    case  JAVA_PLUGIN_GET_STATIC_OBJECT_FIELD: 
	{ 
	    jclass cls = (jclass) get_bits32(pipe); 
	    jobject ret;
	    jfieldID fid= (jfieldID) get_bits32(pipe); 
	    ret = (*env)->GetStaticObjectField(env, cls, fid);
	    send_response(env, pipe, &ret, sizeof(jobject));
	    break; 
	}
        DEFINE_GETFIELD(jboolean, Boolean, JAVA_PLUGIN_GET_BOOLEAN_FIELD)
	DEFINE_GETFIELD(jbyte, Byte, JAVA_PLUGIN_GET_BYTE_FIELD)
	DEFINE_GETFIELD(jchar, Char, JAVA_PLUGIN_GET_CHAR_FIELD)
	DEFINE_GETFIELD(jshort, Short, JAVA_PLUGIN_GET_SHORT_FIELD)
	DEFINE_GETFIELD(jint, Int, JAVA_PLUGIN_GET_INT_FIELD)
	DEFINE_GETFIELD(jlong, Long, JAVA_PLUGIN_GET_LONG_FIELD)
	DEFINE_GETFIELD(jfloat, Float, JAVA_PLUGIN_GET_FLOAT_FIELD)
	DEFINE_GETFIELD(jdouble, Double, JAVA_PLUGIN_GET_DOUBLE_FIELD)
    
	DEFINE_GETSTATICFIELD(jboolean, Boolean, JAVA_PLUGIN_GET_STATIC_BOOLEAN_FIELD)
	DEFINE_GETSTATICFIELD(jbyte, Byte, JAVA_PLUGIN_GET_STATIC_BYTE_FIELD)
	DEFINE_GETSTATICFIELD(jchar, Char, JAVA_PLUGIN_GET_STATIC_CHAR_FIELD)
	DEFINE_GETSTATICFIELD(jshort, Short, JAVA_PLUGIN_GET_STATIC_SHORT_FIELD)
	DEFINE_GETSTATICFIELD(jint, Int, JAVA_PLUGIN_GET_STATIC_INT_FIELD)
	DEFINE_GETSTATICFIELD(jlong, Long, JAVA_PLUGIN_GET_STATIC_LONG_FIELD)
	DEFINE_GETSTATICFIELD(jfloat, Float, JAVA_PLUGIN_GET_STATIC_FLOAT_FIELD)
	DEFINE_GETSTATICFIELD(jdouble, Double, JAVA_PLUGIN_GET_STATIC_DOUBLE_FIELD)

	case JAVA_PLUGIN_ALLOC_OBJECT:
	  {
	    jclass cls = (jclass) get_bits32(pipe); 
	    jobject ret;
	    int resp=JNI_OK;
	    ret= (*env)->AllocObject(env, cls);
	    
	    if ( ret != NULL ){
		send_response(env, pipe, &resp, 4);
		send_response(env, pipe, &ret,sizeof(jobject)); 
	      }
	    else {
		resp =JNI_ERR;
		send_response(env, pipe,&resp, 4);
	      }
	    break;
	  }
#define DEFINE_SETFIELD(type, Result, tag) 			\
    case tag:							\
	{							\
	    jobject obj = (jobject) get_bits32(pipe);		\
	    jfieldID fid = (jfieldID) get_bits32(pipe);		\
	    type val;						\
	    get_bytes(pipe, &val, sizeof(type));		\
	    (*env)->Set##Result##Field(env, obj, fid, val);	\
	    break;						\
	}

#define DEFINE_SETSTATICFIELD(type, Result, tag) 	       \
    case tag:							\
	{							\
	    jclass cls = (jclass) get_bits32(pipe);		\
	    jfieldID fid = (jfieldID) get_bits32(pipe);		\
	    type val;						\
	    get_bytes(pipe, &val, sizeof(type));		\
	    (*env)->SetStatic##Result##Field(env, cls, fid, val);	\
	    break;						\
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

	DEFINE_SETSTATICFIELD(jobject, Object, JAVA_PLUGIN_SET_STATIC_OBJECT_FIELD)
	DEFINE_SETSTATICFIELD(jboolean, Boolean, JAVA_PLUGIN_SET_STATIC_BOOLEAN_FIELD)
	DEFINE_SETSTATICFIELD(jbyte, Byte, JAVA_PLUGIN_SET_STATIC_BYTE_FIELD)
	DEFINE_SETSTATICFIELD(jchar, Char, JAVA_PLUGIN_SET_STATIC_CHAR_FIELD)
	DEFINE_SETSTATICFIELD(jshort, Short, JAVA_PLUGIN_SET_STATIC_SHORT_FIELD)
	DEFINE_SETSTATICFIELD(jint, Int, JAVA_PLUGIN_SET_STATIC_INT_FIELD)
	DEFINE_SETSTATICFIELD(jlong, Long, JAVA_PLUGIN_SET_STATIC_LONG_FIELD)
	DEFINE_SETSTATICFIELD(jfloat, Float, JAVA_PLUGIN_SET_STATIC_FLOAT_FIELD)
	DEFINE_SETSTATICFIELD(jdouble, Double, JAVA_PLUGIN_SET_STATIC_DOUBLE_FIELD)
    
	case JAVA_PLUGIN_GET_METHOD_ID:
	    {
		jclass clz = (jclass) get_bits32(pipe);
		char * sig;
		char * name;
		jmethodID ret;
		name = get_string(pipe);
		sig = get_string(pipe);
		ret = (*env)->GetMethodID(env, clz, name, sig);
		send_response(env, pipe, &ret, 4);
		REGISTER_NAME("GetMethodID", name, (int) ret);
		free(sig); free(name);
		break;
	    }
    case JAVA_PLUGIN_GET_STATIC_METHOD_ID:
	{
	    jclass clz = (jclass) get_bits32(pipe);
	    char * sig;
	    char * name;
	    jmethodID ret;
	    name = get_string(pipe);
	    sig = get_string(pipe);
	    ret = (*env)->GetStaticMethodID(env, clz, name, sig);
	    if (ret == NULL) {
		native_error("Did not find static method");
		native_error(name);
	    }
	    REGISTER_NAME("GetStaticMethodID", name, (int) ret);
	    send_response(env, pipe, &ret, 4);
	    free(sig); free(name);
	    break;
	}

    /*Call method can be macroized, with 2 exceptions: void and object. For
      void we cannot assignthe result, for object we need to create a global
      ref. */
    
#define DEFINE_CALLMETHOD(type, Result, tag)				\
    case tag:								\
	{								\
        jobject obj = (jobject) get_bits32(pipe);			\
	jmethodID meth = (jmethodID) get_bits32(pipe);		\
	int nargs = get_bits32(pipe);				\
	type ret;							\
	jvalue *args;						\
	args = malloc(nargs * sizeof(jvalue));			\
	get_bytes(pipe, args, nargs*sizeof(jvalue)); \
	ret = (*env)->Call##Result##MethodA(env, obj, meth, args);	\
	free(args);							\
	send_OK(env, pipe, &ret, sizeof(type));				\
	break;							\
	}
    DEFINE_CALLMETHOD(jboolean, Boolean, JAVA_PLUGIN_CALL_BOOLEAN_METHOD)
	DEFINE_CALLMETHOD(jbyte, Byte, JAVA_PLUGIN_CALL_BYTE_METHOD)
	DEFINE_CALLMETHOD(jchar, Char, JAVA_PLUGIN_CALL_CHAR_METHOD)
	DEFINE_CALLMETHOD(jshort, Short, JAVA_PLUGIN_CALL_SHORT_METHOD)
	DEFINE_CALLMETHOD(jint, Int, JAVA_PLUGIN_CALL_INT_METHOD)
	DEFINE_CALLMETHOD(jlong, Long, JAVA_PLUGIN_CALL_LONG_METHOD)
	DEFINE_CALLMETHOD(jfloat, Float, JAVA_PLUGIN_CALL_FLOAT_METHOD)
	DEFINE_CALLMETHOD(jdouble, Double, JAVA_PLUGIN_CALL_DOUBLE_METHOD)

	DEFINE_CALLMETHOD(jboolean, StaticBoolean, JAVA_PLUGIN_CALL_STATIC_BOOLEAN_METHOD)
	DEFINE_CALLMETHOD(jbyte, StaticByte, JAVA_PLUGIN_CALL_STATIC_BYTE_METHOD)
	DEFINE_CALLMETHOD(jchar, StaticChar, JAVA_PLUGIN_CALL_STATIC_CHAR_METHOD)
	DEFINE_CALLMETHOD(jshort, StaticShort, JAVA_PLUGIN_CALL_STATIC_SHORT_METHOD)
	DEFINE_CALLMETHOD(jint, StaticInt, JAVA_PLUGIN_CALL_STATIC_INT_METHOD)
	DEFINE_CALLMETHOD(jlong, StaticLong, JAVA_PLUGIN_CALL_STATIC_LONG_METHOD)
	DEFINE_CALLMETHOD(jfloat, StaticFloat, JAVA_PLUGIN_CALL_STATIC_FLOAT_METHOD)
	DEFINE_CALLMETHOD(jdouble, StaticDouble, JAVA_PLUGIN_CALL_STATIC_DOUBLE_METHOD)
	case JAVA_PLUGIN_CALL_VOID_METHOD:
	    {
		jobject obj = (jobject) get_bits32(pipe);
		jmethodID meth = (jmethodID) get_bits32(pipe);
		int i;
		int nargs = get_bits32(pipe);
		jvalue *args;
		args = malloc(nargs * sizeof(jvalue));
		get_bytes(pipe, args, nargs*sizeof(jvalue));
		(*env)->CallVoidMethodA(env, obj, meth, args);
		free(args);
		send_OK(env, pipe, NULL, 0);
		break;
	    }
    case JAVA_PLUGIN_CALL_STATIC_VOID_METHOD:
	{
	    jobject obj = (jobject) get_bits32(pipe);
	    jmethodID meth = (jmethodID) get_bits32(pipe);
	    int nargs = get_bits32(pipe);
	    jvalue *args;
	    args = malloc(nargs * sizeof(jvalue));
	    get_bytes(pipe, args, nargs*sizeof(jvalue));
	    (*env)->CallStaticVoidMethodA(env, obj, meth, args);
	    free(args);
	    send_OK(env, pipe, NULL, 0);
	    break;
	}

    case JAVA_PLUGIN_CALL_OBJECT_METHOD:
	{
	    jobject obj = (jobject) get_bits32(pipe);
	    jmethodID meth = (jmethodID) get_bits32(pipe);
	    int nargs = get_bits32(pipe);
	    jvalue *args;
	    jobject ret;
	    args = malloc(nargs * sizeof(jvalue));
	    get_bytes(pipe, args, nargs*sizeof(jvalue));
	    ret = (*env)->CallObjectMethodA(env, obj, meth, args);
	    free(args);
	    send_OK(env, pipe, &ret, sizeof(jobject));
	    break;
	}
    case JAVA_PLUGIN_CALL_STATIC_OBJECT_METHOD:
	{
	    jobject obj = (jobject) get_bits32(pipe);
	    jmethodID meth = (jmethodID) get_bits32(pipe);
	    int nargs = get_bits32(pipe);
	    jvalue *args;
	    jobject ret;
	    args = malloc(nargs * sizeof(jvalue));
	    get_bytes(pipe, args, nargs*sizeof(jvalue));
	    ret = (*env)->CallStaticObjectMethodA(env, obj, meth, args);
	    free(args);
	    send_OK(env, pipe, &ret, sizeof(jobject));
	    break;
	}
    case JAVA_PLUGIN_NEW_OBJECT_METHOD:
	{
	    jclass obj = (jclass) get_bits32(pipe);
	    jmethodID meth = (jmethodID) get_bits32(pipe);
	    int nargs = get_bits32(pipe);
	    jvalue *args;
	    jobject ret;
	    const int n_locals = 15;
	    /* jint res = (*env)->PushLocalFrame(env, n_locals); */
	    args = malloc(nargs * sizeof(jvalue));
	    get_bytes(pipe, args, nargs*sizeof(jvalue));
	    ret = (*env)->NewObjectA(env, obj, meth, args);
	    free(args);
	    send_OK(env, pipe, &ret, sizeof(jobject));
	    /*   (*env)->PopLocalFrame(env, NULL); */
	    break;
	}
#define DEFINE_CALLNVMETHOD(type, Result, tag)				\
    case tag:								\
	{								\
	    jobject obj = (jobject) get_bits32(pipe);			\
	    jclass clz = (jclass) get_bits32(pipe);			\
	    jmethodID meth = (jmethodID) get_bits32(pipe);		\
	    int nargs = get_bits32(pipe);				\
	    type ret;							\
	    jvalue *args;						\
	    args = malloc(nargs * sizeof(jvalue));			\
	    get_bytes(pipe, args, nargs*sizeof(jvalue)); ret = (*env)->CallNonvirtual##Result##MethodA(env, obj, clz, meth, args);	\
	    free(args);							\
	    send_OK(env, pipe, &ret, sizeof(type));				\
	    break;							\
	}

    
    DEFINE_CALLNVMETHOD(jboolean, Boolean, JAVA_PLUGIN_CALL_NV_BOOLEAN_METHOD)
	DEFINE_CALLNVMETHOD(jbyte, Byte, JAVA_PLUGIN_CALL_NV_BYTE_METHOD)
	DEFINE_CALLNVMETHOD(jchar, Char, JAVA_PLUGIN_CALL_NV_CHAR_METHOD)
	DEFINE_CALLNVMETHOD(jshort, Short, JAVA_PLUGIN_CALL_NV_SHORT_METHOD)
	DEFINE_CALLNVMETHOD(jint, Int, JAVA_PLUGIN_CALL_NV_INT_METHOD)
	DEFINE_CALLNVMETHOD(jlong, Long, JAVA_PLUGIN_CALL_NV_LONG_METHOD)
	DEFINE_CALLNVMETHOD(jfloat, Float, JAVA_PLUGIN_CALL_NV_FLOAT_METHOD)
	DEFINE_CALLNVMETHOD(jdouble, Double, JAVA_PLUGIN_CALL_NV_DOUBLE_METHOD)
	case JAVA_PLUGIN_CALL_NV_OBJECT_METHOD:
	    {
		jobject obj = (jobject) get_bits32(pipe);
		jclass clz = (jclass) get_bits32(pipe);
		jmethodID meth = (jmethodID) get_bits32(pipe);
		int nargs = get_bits32(pipe);
		jvalue *args;
		jobject ret;
		args = malloc(nargs * sizeof(jvalue));
		get_bytes(pipe, args, nargs*sizeof(jvalue));
		ret = (*env)->CallNonvirtualObjectMethodA(env, obj, clz, meth, args);
		free(args);
		send_OK(env, pipe, &ret, sizeof(jobject));
		break;
	    }
    
    case JAVA_PLUGIN_CALL_NV_VOID_METHOD:
	{
	    jobject obj = (jobject) get_bits32(pipe);
	    jclass clz = (jclass) get_bits32(pipe);
	    jmethodID meth = (jmethodID) get_bits32(pipe);
	    int nargs = get_bits32(pipe);
	    jvalue *args;
	    args = malloc(nargs * sizeof(jvalue));
	    get_bytes(pipe, args, nargs*sizeof(jvalue));
	    (*env)->CallNonvirtualVoidMethodA(env, obj, clz, meth, args);
	    free(args);
	    send_OK(env, pipe, NULL, 0);
	    break;
	}
    
#define DEFINE_CAPTURESCALARARRAYELEMENTS(type, Result, tag)		 \
    case tag:						 \
	{							 \
	     jarray arr = (jarray) get_bits32(pipe);		 \
	     type *arrEl;					 \
	     jboolean copy = JNI_TRUE;			 \
	     int arrSize;					 \
	     arrSize = (*env)->GetArrayLength(env, arr);	 \
	     arrEl = (*env)->Get##Result##ArrayElements(env, arr, &copy); \
	     send_response(env, pipe, &arrSize, 4);		 \
	     send_response(env, pipe, arrEl, arrSize *sizeof(type)); \
	     (*env)->Release##Result##ArrayElements(env, arr, arrEl,0);\
	     break;					 \
	}
    DEFINE_CAPTURESCALARARRAYELEMENTS(jboolean, Boolean, JAVA_PLUGIN_CAP_BOOL_AREL)
	DEFINE_CAPTURESCALARARRAYELEMENTS(jbyte, Byte, JAVA_PLUGIN_CAP_BYTE_AREL)
	DEFINE_CAPTURESCALARARRAYELEMENTS(jchar, Char, JAVA_PLUGIN_CAP_CHAR_AREL)
	DEFINE_CAPTURESCALARARRAYELEMENTS(jshort, Short, JAVA_PLUGIN_CAP_SHORT_AREL)
	DEFINE_CAPTURESCALARARRAYELEMENTS(jint, Int, JAVA_PLUGIN_CAP_INT_AREL)
	DEFINE_CAPTURESCALARARRAYELEMENTS(jlong, Long, JAVA_PLUGIN_CAP_LONG_AREL)
	DEFINE_CAPTURESCALARARRAYELEMENTS(jfloat, Float, JAVA_PLUGIN_CAP_FLOAT_AREL)
	DEFINE_CAPTURESCALARARRAYELEMENTS(jdouble, Double, JAVA_PLUGIN_CAP_DOUBLE_AREL)

#define DEFINE_RELEASESCALARARRAYELEMENTS(type, Result, tag)		   \
	case tag:								   \
	    {								   \
	       jarray arr = (jarray) get_bits32(pipe);			   \
	       int mode = (int) get_bits32(pipe);				   \
	       int size = (int) get_bits32(pipe);				   \
	       type *arrEl = (*env)->Get##Result##ArrayElements(env, arr, 0); \
	       get_bytes(pipe, arrEl, size * sizeof(type));				   \
	       (*env)->Release##Result##ArrayElements(env, arr, arrEl, 0);	   \
	       break;							   \
	    }

    DEFINE_RELEASESCALARARRAYELEMENTS(jboolean, Boolean, JAVA_PLUGIN_REL_BOOL_AREL)
	DEFINE_RELEASESCALARARRAYELEMENTS(jbyte, Byte, JAVA_PLUGIN_REL_BYTE_AREL)
	DEFINE_RELEASESCALARARRAYELEMENTS(jchar, Char, JAVA_PLUGIN_REL_CHAR_AREL)
	DEFINE_RELEASESCALARARRAYELEMENTS(jshort, Short, JAVA_PLUGIN_REL_SHORT_AREL)
	DEFINE_RELEASESCALARARRAYELEMENTS(jint, Int, JAVA_PLUGIN_REL_INT_AREL)
	DEFINE_RELEASESCALARARRAYELEMENTS(jlong, Long, JAVA_PLUGIN_REL_LONG_AREL)
	DEFINE_RELEASESCALARARRAYELEMENTS(jfloat, Float, JAVA_PLUGIN_REL_FLOAT_AREL)
	DEFINE_RELEASESCALARARRAYELEMENTS(jdouble, Double, JAVA_PLUGIN_REL_DOUBLE_AREL)

	/*XXX: do error handling!!!!*/
#define DEFINE_GETSCALARARRAYELEMENTS(type, Result, tag)		     \
	case tag:								     \
	    {								     \
	 jarray arr = (jarray) get_bits32(pipe);			     \
         int start = (int) get_bits32(pipe);				     \
	 int size = (int) get_bits32(pipe);				     \
         int code = JNI_OK;   \
	 type *arrReg, *realArr;						     \
         arrReg = malloc(4 + size * sizeof(type));			     \
         realArr = (type *) ((char *) arrReg + 4); \
	 memcpy(arrReg, &code, 4); 	\
	 (*env)->Get##Result##ArrayRegion(env, arr, start, size, realArr); \
	 send_response(env, pipe, arrReg, 4 + size * sizeof(type));     \
	 free(arrReg);						     \
	 break;							     \
	    }
    DEFINE_GETSCALARARRAYELEMENTS(jboolean, Boolean, JAVA_PLUGIN_GET_BOOL_AREL)
	DEFINE_GETSCALARARRAYELEMENTS(jbyte, Byte, JAVA_PLUGIN_GET_BYTE_AREL)
	DEFINE_GETSCALARARRAYELEMENTS(jchar, Char, JAVA_PLUGIN_GET_CHAR_AREL)
	DEFINE_GETSCALARARRAYELEMENTS(jshort, Short, JAVA_PLUGIN_GET_SHORT_AREL)
	DEFINE_GETSCALARARRAYELEMENTS(jint, Int, JAVA_PLUGIN_GET_INT_AREL)
	DEFINE_GETSCALARARRAYELEMENTS(jlong, Long, JAVA_PLUGIN_GET_LONG_AREL)
	DEFINE_GETSCALARARRAYELEMENTS(jfloat, Float, JAVA_PLUGIN_GET_FLOAT_AREL)
	DEFINE_GETSCALARARRAYELEMENTS(jdouble, Double, JAVA_PLUGIN_GET_DOUBLE_AREL)

#define DEFINE_SETSCALARARRAYELEMENTS(type, Result, tag)		     \
	case tag:			     \
	    {						     \
            jarray arr = (jarray) get_bits32(pipe);			     \
	    int start = (int) get_bits32(pipe);				     \
	    int len = (int) get_bits32(pipe);				     \
	    type *arrReg;						     \
	    arrReg = malloc(len * sizeof(type));			     \
	    get_bytes(pipe, arrReg, len * sizeof(type));		     \
	    (*env)->Set##Result##ArrayRegion(env, arr, start, len, arrReg); \
	    send_OK(env, pipe, NULL, 0); \
	    free(arrReg);						     \
	    break;							     \
	    }
    DEFINE_SETSCALARARRAYELEMENTS(jboolean, Boolean, JAVA_PLUGIN_SET_BOOL_AREL)
	DEFINE_SETSCALARARRAYELEMENTS(jbyte, Byte, JAVA_PLUGIN_SET_BYTE_AREL)
	DEFINE_SETSCALARARRAYELEMENTS(jchar, Char, JAVA_PLUGIN_SET_CHAR_AREL)
	DEFINE_SETSCALARARRAYELEMENTS(jshort, Short, JAVA_PLUGIN_SET_SHORT_AREL)
	DEFINE_SETSCALARARRAYELEMENTS(jint, Int, JAVA_PLUGIN_SET_INT_AREL)
	DEFINE_SETSCALARARRAYELEMENTS(jlong, Long, JAVA_PLUGIN_SET_LONG_AREL)
	DEFINE_SETSCALARARRAYELEMENTS(jfloat, Float, JAVA_PLUGIN_SET_FLOAT_AREL)
	DEFINE_SETSCALARARRAYELEMENTS(jdouble, Double, JAVA_PLUGIN_SET_DOUBLE_AREL)


#define DEFINE_NEWSCALARARRAY(Result, tag)				\
	case tag:							\
	    {							\
	    int size = (int) get_bits32(pipe);			\
	    jarray arr = (*env)->New##Result##Array(env, size);	\
	    send_response(env, pipe, &arr, sizeof(jarray));		\
	    break;							\
	    }
    DEFINE_NEWSCALARARRAY(Boolean, JAVA_PLUGIN_NEW_BOOL_ARRAY)
	DEFINE_NEWSCALARARRAY(Byte, JAVA_PLUGIN_NEW_BYTE_ARRAY)
	DEFINE_NEWSCALARARRAY(Char, JAVA_PLUGIN_NEW_CHAR_ARRAY)
	DEFINE_NEWSCALARARRAY(Short, JAVA_PLUGIN_NEW_SHORT_ARRAY)
	DEFINE_NEWSCALARARRAY(Int, JAVA_PLUGIN_NEW_INT_ARRAY)
	DEFINE_NEWSCALARARRAY(Long, JAVA_PLUGIN_NEW_LONG_ARRAY)
	DEFINE_NEWSCALARARRAY(Float, JAVA_PLUGIN_NEW_FLOAT_ARRAY)
	DEFINE_NEWSCALARARRAY(Double, JAVA_PLUGIN_NEW_DOUBLE_ARRAY)

	case JAVA_PLUGIN_NEW_OBJECT_ARRAY:
	    {
		int size = (int) get_bits32(pipe);
		jclass clz = (jclass) get_bits32(pipe);
		jobject initObj = (jobject) get_bits32(pipe);
		jarray arr = (*env)->NewObjectArray(env, size, clz, initObj);
		send_response(env, pipe, &arr, 4);
		break;
	    }
    case JAVA_PLUGIN_GET_OBJECT_ARRAY_ELEMENT:
	{
	    jobject ret;
	    jarray arr = (jarray) get_bits32(pipe);
	    int index = (int) get_bits32(pipe);
	    ret = (*env)->GetObjectArrayElement(env, arr, index);
	    send_response(env, pipe, &ret, 4);
	    break;
	}
    case JAVA_PLUGIN_SET_OBJECT_ARRAY_ELEMENT:
	{
	    jarray arr = (jarray) get_bits32(pipe);
	    int index = (int) get_bits32(pipe);
	    jobject ret = (jobject) get_bits32(pipe);
	    (*env)->SetObjectArrayElement(env, arr, index, ret);
	    break;
	}
    case JAVA_PLUGIN_GET_ARRAY_LENGTH:
	{
	    jarray arr = (jarray) get_bits32(pipe);
	    jsize size = (*env)->GetArrayLength(env, arr);
	    send_response(env, pipe, &size, 4);
	    break;
	}
    case JAVA_PLUGIN_NEW_STRING:
	{
	    jint len = (jint) get_bits32(pipe);
	    jchar *unicodeChars;
	    jstring str; 
	    unicodeChars = (jchar *) malloc(len * sizeof(jchar));
	    get_bytes(pipe, unicodeChars, len*sizeof(jchar));
	    str = (*env)->NewString(env, unicodeChars, len);
#ifndef NO_DEBUG
	    {
	    const char* debugStr = (*env)->GetStringUTFChars(env, str, NULL);
            native_trace("Created a new string=%d str=%s\n", (int) str, debugStr);
	    (*env)->ReleaseStringUTFChars(env,str,debugStr);
	    }
#endif	    
	    send_response(env, pipe, &str, 4);
	    free(unicodeChars);
	    break;
	}
    case JAVA_PLUGIN_GET_STRING_SIZE:
	{
	    jstring string = (jstring)get_bits32(pipe);
	    jint ret; 
	    ret = (*env)->GetStringLength(env, string);
	    send_response(env, pipe, &ret, 4);
	    break;
	}
    case JAVA_PLUGIN_GET_STRING_CHARS:
	{
	    jstring string = (jstring) get_bits32(pipe);
	    const jchar *chars;
	    /* Copy len and data into a single array so that
	       it can be sent in a single write. Multiple writes
	       are much slower */
	    char *store_len_and_data;
	    int msg_nbytes;

	    jboolean copy = JNI_TRUE;
	    int len;

	    len = (*env)->GetStringLength(env, string);
	    chars = (*env)->GetStringChars(env, string, &copy);
	    msg_nbytes = len * sizeof(jchar);
	    store_len_and_data = (char *) malloc(4 + msg_nbytes);

	    memcpy(store_len_and_data, &len, 4);
	    memcpy(store_len_and_data + 4, chars, msg_nbytes);

	    send_response(env, pipe, store_len_and_data, 4 + msg_nbytes);

	    /*
	    send_response(env, pipe, &len, 4);
	    send_response(env, pipe, (void *) chars, len*sizeof(jchar));
	    */
	    if (copy == JNI_TRUE)
	      (*env)->ReleaseStringChars(env, string, chars);
	    free(store_len_and_data);
	    break;
	}
    case JAVA_PLUGIN_NEW_STRING_UTF:
	{
	    jsize len = (jsize) get_bits32(pipe);
	    char * UTFChars =(char*) malloc(len+1);
	    jstring ret;
	    get_bytes(pipe, UTFChars, len);
	    UTFChars[len] = '\0';
	    ret = (*env)->NewStringUTF(env, UTFChars);
	    /* XXX Is UTFChars copied? Can/should UTFChars be freed? */
	    send_response(env, pipe, &ret, 4);
            free(UTFChars);
	    break;
	}
    case JAVA_PLUGIN_GET_STRING_UTF_SIZE:
	{
	    jstring ret = (jstring) get_bits32(pipe);
	    jsize size = (*env)->GetStringUTFLength(env, ret);
	    send_response(env, pipe, &size, 4);
	    break;
	}
    case JAVA_PLUGIN_GET_STRING_UTF_CHARS:
	{
	    jboolean copy = JNI_TRUE;
	    jstring str = (jstring) get_bits32(pipe);
	    char *len_and_data;
	    jsize len = (*env)->GetStringUTFLength(env, str);
	    const char * UTFChars = (*env)->GetStringUTFChars(env, str, &copy);
	    len_and_data = (char *) malloc(4 + len);
	    memcpy(len_and_data, &len, 4);
	    memcpy(len_and_data + 4, UTFChars, len);
	    send_response(env, pipe, (void *) len_and_data, len + 4);
	    /*
	    send_response(env, pipe, &len, 4);
	    send_response(env, pipe,(void *) UTFChars, len);
	    */
	    if(copy == JNI_TRUE)
	      (*env)->ReleaseStringUTFChars(env, str, UTFChars);
	    free(len_and_data);
	    break;
	}
    case JAVA_PLUGIN_REGISTER_NATIVES:
	{
	    jclass clz = (jclass) get_bits32(pipe);
	    int nMethods = (int) get_bits32(pipe);
	    int i;
	    jint retcode;
	    JNINativeMethod *methods;
	    methods = malloc(nMethods * sizeof(JNINativeMethod));
	    for (i = 0; i < nMethods; i++) {
		int len1, len2, idx;
		get_bytes(pipe, &len1, 2);
		methods[i].name = malloc(len1);
		get_bytes(pipe, methods[i].name, len1);
		get_bytes(pipe, &len2, 2);
		methods[i].signature = malloc(len2);
		get_bytes(pipe, methods[i].signature, len2);
		get_bytes(pipe, &(idx), sizeof(void*));
		methods[i].fnPtr = wrap_remote(idx);
	    }
	    retcode = (*env)->RegisterNatives(env, clz, methods,
					      nMethods);
	    send_response(env, pipe, &retcode, 4);
	    for (i = 0; i < nMethods; i++) {
		free(methods[i].name);
		free(methods[i].signature);
	    }
	    free(methods);
	    break;
	}
    case JAVA_PLUGIN_UNREGISTER_NATIVES:
	{
	    jclass clz = (jclass) get_bits32(pipe);
	    jint retcode = (*env)->UnregisterNatives(env, clz);
	    send_response(env, pipe, &retcode, 4);
	    break;
	}
    case JAVA_PLUGIN_MONITOR_ENTER:
	{
	    jobject obj= (jobject) get_bits32(pipe);
	    jint retcode = (*env)->MonitorEnter(env, obj);
	    send_response(env, pipe, &retcode, 4);
	    break;
	}
    case JAVA_PLUGIN_MONITOR_EXIT:
	{
	    jobject obj= (jobject) get_bits32(pipe);
	    jint retcode = (*env)->MonitorExit(env, obj);
	    send_response(env, pipe, &retcode, 4);
	    break;
	}
    case JAVA_PLUGIN_RETURN: 
	return;
    case JAVA_PLUGIN_SECURE_NEW_OBJECT:
	{
	    jclass clazz = (jclass) get_bits32(pipe);
	    jmethodID methodID = (jmethodID) get_bits32(pipe);
	    int nargs = get_bits32(pipe);
	    int ctx = get_bits32(pipe);
	    jobject res;
	    jvalue* args;
	    char* sig;
            char* origin = get_string(pipe);
	    int ubr = get_bits32(pipe);
            int ujp = get_bits32(pipe);

	    /* jint lframe = (*env)->PushLocalFrame(env, LOCALS_FOR_CALL); */
	    
	    get_args(env, pipe, nargs, &sig, &args);
	    trace_call2_secure(stderr, "SECURE_NEW_OBJECT", pipe, env, nargs, jobject_type, (int) clazz, 
			   (int) methodID, sig, ctx, origin, ubr, ujp);
	    /* Perform the call */
	    CSecureJNI2_NewObject(env, clazz, methodID, sig, args, &res, ctx, origin, ubr, ujp);

	    free_args(env, nargs, &sig, &args);
            free(origin);
	    send_OK(env, pipe, &res, sizeof(jobject));
	    /*  (*env)->PopLocalFrame(env, NULL); */
	    break;
	}
    case JAVA_PLUGIN_SECURE_CALL:
	{
	    jvalue result;
	    char* sig = NULL;
	    jobject jobj = (jobject) get_bits32(pipe);
	    jmethodID methodID = (jmethodID) get_bits32(pipe);
	    int nargs = get_bits32(pipe);
	    int ctx = get_bits32(pipe);
	    jni_type result_type = (jni_type) get_bits32(pipe);
        char* origin = get_string(pipe);
	    int ubr = get_bits32(pipe);
        int ujp = get_bits32(pipe);

	    jvalue* args = NULL;
	    get_args(env, pipe, nargs, &sig, &args);
            
		trace_call_secure(pipe, env, nargs, result_type, (int) jobj,
						  (int) methodID, sig, ctx, origin, ubr, ujp);

	    CSecureJNI2_CallMethod(env, result_type, jobj, methodID, 
				   sig, args, &result, ctx, origin, ubr, ujp);
	    
		free_args(env, nargs, &sig, &args);
		free(origin);
        send_OK_val_of_type(env, pipe, result_type, &result);
	    break;
	}
    case JAVA_PLUGIN_SECURE_CALL_NONVIRTUAL:
	{
	    jvalue result;
	    char* sig = NULL;
	    jobject jobj = (jobject) get_bits32(pipe);
	    jclass clazz = (jclass) get_bits32(pipe);
	    jmethodID methodID = (jmethodID) get_bits32(pipe);
	    int nargs = get_bits32(pipe);
	    int ctx = get_bits32(pipe);
	    jni_type result_type = (jni_type) get_bits32(pipe);
            char* origin = get_string(pipe);
	    int ubr = get_bits32(pipe);
            int ujp = get_bits32(pipe);
	    jvalue* args = NULL;
	    const int n_locals = 50;
	    get_args(env, pipe, nargs, &sig, &args);
	    trace_call_secure(pipe, env, nargs, result_type, (int) clazz, 
			 (int) methodID, sig, ctx, origin, ubr, ujp);
	    CSecureJNI2_CallNonvirtualMethod(env, result_type, jobj, clazz,
				methodID, sig, args, &result, ctx, origin, ubr, ujp);
	    free_args(env, nargs, &sig, &args);
            free(origin);

	    send_OK_val_of_type(env, pipe, result_type, &result);
	    break;
	}
    case JAVA_PLUGIN_SECURE_GET_FIELD:
	{
	    jobject obj = (jobject) get_bits32(pipe);
	    jfieldID fieldID = (jfieldID) get_bits32(pipe);
	    jni_type result_type = (jni_type) get_bits32(pipe);
	    int ctx = get_bits32(pipe);
            char* origin = get_string(pipe);
	    int ubr = get_bits32(pipe);
            int ujp = get_bits32(pipe);
	    jvalue result;
	    if (tracing_on) 
            trace_call_secure(pipe, env, 1, result_type, (int) obj, (int) fieldID, "GET_FIELD", ctx, origin, ubr, ujp);

	    CSecureJNI2_GetField(env, result_type, obj, fieldID, &result, 
				 ctx, origin, ubr, ujp);
	    send_val_of_type(env, pipe, result_type, &result);
            free(origin);

	    /* (*env)->PopLocalFrame(env, NULL); */
	    break;
	}
    case JAVA_PLUGIN_SECURE_SET_FIELD:
	{
	    jobject obj = (jobject) get_bits32(pipe);
	    jfieldID fieldID = (jfieldID) get_bits32(pipe);
	    jni_type arg_type = (jni_type) get_bits32(pipe);
	    int ctx =  get_bits32(pipe);
            char* origin = get_string(pipe);
	    int ubr = get_bits32(pipe);
            int ujp = get_bits32(pipe);
	    jvalue val;
            trace_call_secure(pipe, env, 1, arg_type, (int) obj, (int) fieldID, 
			   "SET_FIELD", ctx, origin, ubr, ujp);
	    get_val_of_type(env, pipe, arg_type, &val);
	    CSecureJNI2_SetField(env, arg_type, obj, fieldID, val, ctx, origin, ubr, ujp);
            free(origin);

	    break;
	}
    case JAVA_PLUGIN_SECURE_CALL_STATIC:
	{
	    jvalue result;
	    char* sig = NULL;
	    jclass clazz = (jclass) get_bits32(pipe);
	    jmethodID methodID = (jmethodID) get_bits32(pipe);
	    int nargs = get_bits32(pipe);
	    int ctx = get_bits32(pipe);
	    jni_type result_type = (jni_type) get_bits32(pipe);
        char* origin = get_string(pipe);
	    int ubr = get_bits32(pipe);
        int ujp = get_bits32(pipe);
	    jvalue* args = NULL;
	    /* jint res = (*env)->PushLocalFrame(env, LOCALS_FOR_CALL); */

	     bp_fn(9129);

	    get_args(env, pipe, nargs, &sig, &args);

        trace_call_secure(pipe, env, nargs, result_type, (int) clazz,
			   (int) methodID, sig, ctx, origin, ubr, ujp);

	    CSecureJNI2_CallStaticMethod(env, result_type, clazz, methodID, 
				   sig, args, &result, ctx, origin, ubr, ujp);
	    
	    free_args(env, nargs, &sig, &args);
	    free(origin);

	    send_OK_val_of_type(env, pipe, result_type, &result);
	    /* (*env)->PopLocalFrame(env, NULL); */

	    break;
	}
    case JAVA_PLUGIN_SECURE_GET_STATIC_FIELD:
	{
	    jclass clazz = (jclass) get_bits32(pipe);
	    jfieldID fieldID = (jfieldID) get_bits32(pipe);
	    jni_type result_type = (jni_type) get_bits32(pipe);
	    int ctx = get_bits32(pipe);
            char* origin = get_string(pipe);
	    int ubr = get_bits32(pipe);
            int ujp = get_bits32(pipe);
	    jvalue result;
	    /* jint res = (*env)->PushLocalFrame(env, LOCALS_FOR_CALL); */
            trace_call_secure(pipe, env, 1, result_type, (int) clazz, (int) fieldID,
			   "GET_STATIC_FIELD", ctx, origin, ubr, ujp);

	    CSecureJNI2_GetStaticField(env, result_type, clazz, 
				       fieldID, &result, ctx, origin, ubr, ujp);
	    send_val_of_type(env, pipe, result_type, &result);
            free(origin);

	    /* (*env)->PopLocalFrame(env, NULL); */
	    break;
	}
    case JAVA_PLUGIN_SECURE_SET_STATIC_FIELD:
	{
	    jclass clazz = (jclass) get_bits32(pipe);
	    jfieldID fieldID = (jfieldID) get_bits32(pipe);
	    jni_type arg_type = (jni_type) get_bits32(pipe);
	    int ctx =  get_bits32(pipe);
            char* origin = get_string(pipe);
	    int ubr = get_bits32(pipe);
            int ujp = get_bits32(pipe);
	    jvalue val;
            trace_call_secure(pipe, env, 1, arg_type, (int) clazz, (int) fieldID, 
			   "SET_STATIC_FIELD", ctx, origin, ubr, ujp);
	    get_val_of_type(env, pipe, arg_type, &val);
	    CSecureJNI2_SetStaticField(env, arg_type, 
				       clazz, fieldID, val, ctx, origin, ubr, ujp);
            free(origin);

	    break;
	}
    case JAVA_PLUGIN_CSECURITYCONTEXT_IMPLIES:
	{
	    /* For a call from Java to Javascript. The implies calls back
	       to the VM for a verification */
	    int context = get_bits32(pipe);
	    char *target  = get_string(pipe); 
	    char *action = get_string(pipe);
	    jclass clazz;
	    jmethodID methodID;
	    jstring jTarget, jAction;
	    jboolean action_allowed;

	    /* There's a lot of junk created during this call which should not
	       be stored on the local stack. So we push and pop a local
	       stack frame. The only value that comes out of it is a boolean
	       which is copied back */
	    (*env)->PushLocalFrame(env, LOCALS_FOR_CALL); 

	    (*env)->ExceptionClear(env); 
	    
	    clazz = 
		(*env)->FindClass(env, 
				  "sun/plugin/liveconnect/SecurityContextHelper");
        
	    if (clazz == NULL)
		native_error("Could not find SecuritityContextHelper!");

	    methodID = (*env)->GetStaticMethodID(env,
						 clazz, 
						 "Implies", 
                         "(Ljava/security/AccessControlContext;Ljava/lang/String;Ljava/lang/String;)Z");

	    if (methodID == NULL)
		native_error("Could not find SecurityContextHelper.implies");
	    
	    jTarget = (*env)->NewStringUTF(env, target);
	    jAction = (*env)->NewStringUTF(env, action);

	    action_allowed = (*env)->CallStaticBooleanMethod(env,
							     clazz, methodID, 
							     context, 
							     jTarget, jAction); 
	    free(jTarget);
	    free(jAction);
	    send_OK(env, pipe, &action_allowed, sizeof(jboolean));
	    (*env)->PopLocalFrame(env, NULL);
	    break;
	}
    default:
	native_error("Unimplemented code %d %X\n", code, code);
    }
}



typedef enum ReturnType_enum {
    return_void,
    return_int,
    return_jstring,
    return_jobject
} ReturnType;


typedef union ReturnValue_Struct {
    jobject o;
    jstring s;
    int i;
} ReturnValue;



#if 0
int recvfd(int src) {
    int recfd;
    struct msghdr msg;
    struct iovec base[1];
    int rc;
    base[0].iov_base = (char *)0;
    base[0].iov_len = 0;
    msg.msg_iov = base;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_accrights = (caddr_t)&recfd;
    msg.msg_accrightslen = sizeof(int);
    
    rc = recvmsg(src, &msg, 0);
    if (rc < 0) {
	if (tracing_on) fprintf(stderr, "error: %d %d\n",rc, errno);
        perror("Receiving fd:");
	return -1;
    }
    return recfd;
}
#else
int recvfd(int src) {
    int rc;
    struct strrecvfd r;

    rc = ioctl(src, I_RECVFD, &r);
    if (rc < 0) {
	native_trace("Error in receiving FD: %d %d\n",rc, errno);
        perror("Receiving fd:");
	return -1;
    }
    return r.fd;
}
    
#endif

JNIEXPORT void JNICALL
Java_sun_plugin_navig_motif_Plugin_attachThread(JNIEnv *env, jclass clz) {
    int comm_sock;
    int port;
    int recfd;
    jclass thr_clz;
    jmethodID mid;
    jobject thr;
    struct  hostent *hostPtr;
    const char *serverHost;
    int i, j, bufferSize, clientSock, onOption;
#if defined(USE_TCP_SOCKET)
    struct sockaddr_in client;
#else
    struct sockaddr_un client;
#endif

#if defined(USE_TCP_SOCKET)
    comm_sock = socket(AF_INET, SOCK_STREAM, 0);
#else
    comm_sock = socket(AF_UNIX, SOCK_STREAM, 0);
#endif
    
    if (comm_sock <= 0) 
      native_error("Could not create a client socket");
    
    read(COMMAND_FD, &port, 4);

#if defined(USE_TCP_SOCKET)
    serverHost = "localhost";

    if ((hostPtr = gethostbyname(serverHost)) == NULL) {
        native_error("gethostbyname() failed, errno = %d\n", errno);
    }

    client.sin_family = AF_INET;
/*
This was the original code but I believe it is redundant since the ordering is correct 
to begin with.  Also this seemed to cause a problem on Linux and Solaris Intel
    client.sin_addr.s_addr = 
      inet_addr(inet_ntoa(hostPtr->h_addr_list[0]));
*/
    client.sin_addr.s_addr = *(int *)(hostPtr->h_addr_list[0]);
    client.sin_port = htons(port);

    native_trace("Using port: %d\n", port);

    if (connect(comm_sock, 
		(struct sockaddr *)&client, sizeof(client)) < 0)
      native_error("Could not connect to host");
#else
    bzero(&client,sizeof(client));
    client.sun_family = AF_UNIX;
    sprintf(client.sun_path,"%s.%s.%d",JAVA_PLUGIN_SOCKFILE,PLUGIN_NODOTVERSION,htonl(port));

    native_trace("Using file: %s\n", client.sun_path);

    if (connect(comm_sock,
                (struct sockaddr *)&client, sizeof(client)) < 0)
      native_error("Could not connect to host");
#endif
    
    /* Dup so that it the FD registerd with green threads */
    recfd = dup(comm_sock);

    native_trace("Performed connect and dupt %d %d\n", comm_sock, recfd);

    /* Create the  AThread object */
    thr_clz = (*env)->FindClass(env, "sun/plugin/navig/motif/AThread");
    mid = (*env)->GetMethodID(env, thr_clz, "<init>", "(I)V");
    thr = (*env)->NewObject(env, thr_clz, mid, recfd);

    /* Start up the new thread */
    mid = (*env)->GetMethodID(env, thr_clz, "start", "()V");
    native_trace("JVM:Starting the thread\n");

    (*env)->CallVoidMethod(env, thr, mid);
    native_trace("JVM:Thread started\n");
}


/* Acquire the right pipe for this env. If this is a recursive call,
   then this env will have been registered and we can find out its pipe
   from the AThread. Otherwise, a new AThread must be created with the
   spontaneous pipe.
   */
int
AcquireThreadPipe(JNIEnv* env) {
    jclass pluginclz;
    jint jres;
    int res;
    jres = (*env)->CallStaticIntMethod(env, g_ojiplugin_class, 
				      g_ojiplugin_acquireThreadPipe);
    res = (int) jres;
    native_trace("Result of call to acquire pipe: %d \n", res);
    return res;

}


typedef struct queue_node {
    int key;
    char* message;
    struct queue_node *next;
} message_queue_node;

static message_queue_node* queue_head = NULL;

static void put_message_to_queue(JNIEnv* env, int in_key, char* in_message, int length) {
    message_queue_node *newNode = NULL;

    if (env == NULL || in_message == NULL) return;

    ENTER_MONITOR(env, g_jobject_queue_lock);
   
    newNode = (message_queue_node*)malloc(sizeof(message_queue_node));
  
    if (newNode != NULL) {
		newNode->key = in_key;
		
		newNode->message = (char*)malloc(length * sizeof(char));
		if (newNode->message == NULL) {
			free(newNode);
			return;
		}
		memcpy(newNode->message, in_message, length);
		newNode->next = queue_head;
			
		queue_head = newNode;
    }

    EXIT_MONITOR(env, g_jobject_queue_lock);
}

static char* get_message_from_queue(JNIEnv* env, int in_key) {	
    message_queue_node *currNode = queue_head;
    message_queue_node *prevNode = queue_head;
    char* retMsg = NULL;

    if (env == NULL) return NULL;

    ENTER_MONITOR(env, g_jobject_queue_lock);
	
    while (currNode != NULL) {
		if (currNode->key == in_key) {
			prevNode->next = currNode->next;
			retMsg = currNode->message;
			if (currNode == queue_head) {
				queue_head = currNode->next;
			}
			/* free(currNode->message);*/
			free(currNode);
			break;
		}
		
		prevNode = currNode;
		currNode = currNode->next;
    }
	
    EXIT_MONITOR(env, g_jobject_queue_lock);

    return retMsg;
}
		
void
handleJNIJSResponse(JNIEnv* env, int pipe, int requestID,
					ReturnType ret_type, ReturnValue* retval) {
    /* Wait for a response, which is either a JAVA_PLUGIN_RETURN or
       a JAVA_PLUGIN_REQUEST  */
  
    int ret_code, replyID, rc;
    int cont = 1, length;
    char* message = NULL;
    struct pollfd fds[1];
    jobject read_lock = (jobject)get_pipelock(pipe, 1);

    fds[0].fd = pipe;
    fds[0].events = POLLRDNORM;
	
    native_trace("Entered handleJNIJSResponse with lock %d\n", read_lock);

    while(cont) {
		/* Retrive the message from the queue if requestID match. */
		if ((message = get_message_from_queue(env, requestID)) != NULL) {	
			cont = 0;
		}
		else {
			/* The poll call here is to preemptive the threads which is running all the time.
			   The 'poll' call is to simulate 'sleep' (actually a better version of sleep).
			*/
			poll(fds, 1, 1);
			ENTER_MONITOR(env, read_lock);
	 
			rc = poll(fds, 1, 0);
			if (rc > 0) {
				read_message(pipe);
			}
			else {
				EXIT_MONITOR(env, read_lock); 
				continue;
			}
			ret_code = get_bits32(pipe);
	
			switch(ret_code) {
			case JAVA_PLUGIN_RETURN:
				{
					native_trace("<<<<<<||||VM: JS call returned. type=%d \n", 
								 (int) ret_type);
					replyID = get_bits32(pipe);

					if (replyID == requestID) {
						cont = 0;
					}
					else {
						/* Message id does not match, put it into the queue. */
						native_trace("<<<<<<||||VM:Message ID mismatch>>>\n");
						message = get_message(pipe, &length);
						put_message_to_queue(env, replyID, message, length);
					}
					break;
				}
			default:
				{
					/* Make a jni request */
					/* clz is a dummy, normally AThread class, but arg not used */
					jclass clz;
					native_trace("Handling recursive call back to java \n ");
					handle_code(ret_code, env, clz, pipe);
				}
			}	 
			EXIT_MONITOR(env, read_lock);
		}
    }

    /* Read the response data and return */
    switch(ret_type) {
    case return_void:  
		native_trace("handleJNIJSResponse(): Void returned\n");
		break;
    case return_int:  
		{
			if (message != NULL) {
				memcpy(&(retval->i), message+8, 4);
			}
			else {
				retval->i = get_bits32(pipe);
			}
			native_trace("handleJNIJSResponse(): Returning an int %d\n", retval->i);
			break;
		}
    case return_jstring:
		{
			if (message != NULL) {
				memcpy(&(retval->s), message+8, sizeof(jstring));
			}
			else {
				get_bytes(pipe, &(retval->s), sizeof(jstring));
			}
			native_trace("handleJNIJSResponse(): Returning a jstring %d\n", retval->s);
			break;
		}
    case return_jobject:
		{
			if (message != NULL) {
				memcpy(&(retval->o), message+8, sizeof(jobject));
			}
			else {
				get_bytes(pipe, &(retval->o), sizeof(jobject));
			}
			native_trace("handleJNIJSResponse(): Returning an object %X\n", retval->o);
			break;
		}
    default:
		{
			native_error("handleJNIJSResponse(): Error in return type");
		}
		free(message);
    }
}

/*
    A helper method to get the slot index from jarray
    */
static int getSlotIndex(JNIEnv* env, jobjectArray jArgs)
{
    if (jArgs != NULL) {
        jobject arg = (*env)->GetObjectArrayElement(env, jArgs, 0);

        return (*env)->CallIntMethod(env, arg, g_jmethod_Integer_intValue);
    }

    return 0;
}

/*
  For sending messages we define a common "frame" with the following fields
  1] code
  2] index (nativejsobject or plugin_index)
  3] slotindex 
  4] a jstring as utf
  5] a jstring as regular chars (prefixed by its length and sz)
  6] an array of jobject
  7] a value of jobject (value to set usually)
  8] a ctx
*/
void SendJNIJSMessage(JNIEnv* env, 
					  jint code, jint pluginIndex,  jint nativeJSObject,
					  jstring jurlstr, 
					  jstring jmethodName, 
					  jobjectArray jarray, 
					  jboolean ctx,
					  jobject* ret)
{
    
    const jchar* methodName = NULL;
    int urlstr_len, methodName_len, methodName_sz;
    int header_code = JAVA_PLUGIN_REQUEST;
    const char* urlstr = NULL;
    int args_len;
    jint slotIndex = 0;
    int requestID, requestID_sz = 4;
    int value_sz, code_sz, index_sz, var_sz, arr_sz, ctx_sz;
    int mesg_sz;
    int mesg_size_sz;
    int non_header_sz;
    char* mesg;
    int ci;
    jobject jvalue = NULL;
    int pipe;
    ReturnValue returnObject;
    ReturnType returnType = return_jobject;
    jobject write_lock = NULL;

    requestID = getJSRequestID();
    /* These 3 global static variables are for get/setSlot and getWindow */
    if (g_jclass_Integer == NULL)
		g_jclass_Integer = wrapFindClassGlobal(env,"java/lang/Integer");
    if (g_jmethod_Integer_intValue == NULL)
		g_jmethod_Integer_intValue = wrapGetMethodID(env,g_jclass_Integer, "intValue", "()I");
    if (g_jmethod_Integer_init == NULL)
		g_jmethod_Integer_init = wrapGetMethodID(env,g_jclass_Integer, "<init>", "(I)V");
  
    /* Convert the first string argument into a unicode array */
    if (jmethodName) {
		methodName = (*env)->GetStringChars(env, jmethodName, NULL);
		methodName_len = (*env)->GetStringLength(env, jmethodName);
		methodName_sz = sizeof(jchar) * methodName_len;
    } else {
		methodName_sz = 0;
		methodName_len = 0;
    }
    /* Convert the second string argument into a UTF-8 array */
    if (jurlstr) {
		urlstr = (*env)->GetStringUTFChars(env, jurlstr, NULL);
		urlstr_len = (*env)->GetStringUTFLength(env, jurlstr);
		native_trace("utfstr='%s'\n", urlstr);
    } else {
		urlstr_len = 0;
    }

    /* A liitle bit tweak to get the slot index and jvalue from the object array 
       passed from Java */
    switch (code) {
    case JAVA_PLUGIN_JNIJS_GET_NATIVE:
		returnType = return_int;
		break;
    case JAVA_PLUGIN_JNIJS_SETMEMBER:
		if (jarray != NULL)
			jvalue = (*env)->GetObjectArrayElement(env, jarray, 0);
		break;
    case JAVA_PLUGIN_JNIJS_SETSLOT:
		slotIndex = getSlotIndex(env, jarray);
		if (jarray != NULL)
			jvalue = (*env)->GetObjectArrayElement(env, jarray, 1);
		break;
    case JAVA_PLUGIN_JNIJS_GETSLOT:
		slotIndex = getSlotIndex(env, jarray);
		break;
    }

    /* Determine the size of the message */
    mesg_size_sz = 4;		/* Header indicating the length of message */
    value_sz = sizeof(jobject);
    index_sz = 4;
    arr_sz = sizeof(jobjectArray);
    ctx_sz = 4;
    var_sz = 4;

    mesg_sz = mesg_size_sz + requestID_sz
		+ index_sz
		+ index_sz
		+ var_sz + urlstr_len 
		+ var_sz + var_sz + methodName_sz 	
		+ arr_sz
		+ value_sz 
		+ ctx_sz;

    mesg = (char*) malloc(mesg_sz);

    /* Create the message */
    ci = 0;

    /* The length of the actual request message. Message size without
       length header */
    non_header_sz = mesg_sz - mesg_size_sz;

    memcpy(mesg + ci, &non_header_sz, mesg_size_sz); ci += mesg_size_sz;
    memcpy(mesg + ci, &requestID, requestID_sz); ci += requestID_sz;

    /* Then the actual message */
    memcpy(mesg + ci, &nativeJSObject, index_sz); ci +=  index_sz;
    memcpy(mesg + ci, &slotIndex, index_sz); ci += index_sz;
    memcpy(mesg + ci, &urlstr_len, var_sz); ci += var_sz;
    if (urlstr_len > 0) {
		memcpy(mesg + ci, urlstr, urlstr_len );    
		ci += urlstr_len;
    }
    memcpy(mesg + ci, &methodName_len, var_sz); ci += var_sz;
    memcpy(mesg + ci, &methodName_sz, var_sz);  ci += var_sz;
    if (methodName_sz > 0) {
		memcpy(mesg + ci, methodName, methodName_sz);   ci += methodName_sz;
    }
    memcpy(mesg + ci, &jarray, arr_sz);   ci += arr_sz;
    memcpy(mesg + ci, &jvalue, value_sz);   ci += value_sz;
    memcpy(mesg + ci, &ctx, ctx_sz);     ci += ctx_sz;


    /* Acquire the appropriate pipe, get the associated write lock, if the lock exisits,
	   lock the pipe and write the message to the pipe.
	*/
    pipe = AcquireThreadPipe(env);
  
	write_lock = (jobject)get_pipelock(pipe, 0);
    
	ENTER_MONITOR(env, write_lock);

    native_trace("Pipe acquired %d, Entering write lock %d\n", pipe, write_lock);

    /* Write the message header, JAVA_PLUGIN_REQUEST */
    write_fully(pipe, &header_code, 4);
    
    /* Write the plugin index to the pipe */
    write_fully(pipe, &pluginIndex, 4);

    /* Write the code */
    write_fully(pipe, &code, 4);

    /* Send the message */
    write_fully(pipe, mesg, mesg_sz);

    native_trace(">>>>>|||||SendJNIJSMessage(): size=%d code=%X\n pluginIndex=%d"
				 "nativeJSObject=%d slotindex=%d strAsUTF_len=%d\n"
				 " methodName_sz=%d value=%d\n"
				 " jarray=%d ctx=%d, msgID=%d\n",
				 mesg_sz, code, pluginIndex, nativeJSObject,
				 slotIndex, urlstr_len,
				 methodName_sz, (int) jvalue,
				 (int) jarray, (int) ctx, requestID);

    EXIT_MONITOR(env, write_lock);

    /* Wait for a response */	
    handleJNIJSResponse(env, pipe, requestID, returnType, &returnObject);	 
				 
				 
    if (code == JAVA_PLUGIN_JNIJS_GET_NATIVE) {
		*ret = (*env)->NewObject(env, g_jclass_Integer, g_jmethod_Integer_init, returnObject.i);
    }
    else if (ret != NULL)
		*ret = returnObject.o;

    /* Preserve any exception for after the java call */
    {
		jobject exc  = (*env)->ExceptionOccurred(env);

		(*env)->ExceptionClear(env);

		if (exc != NULL) {
			native_trace("SendJNIJSMessage: Throwing an exception!!: %X\n", (int) exc);
			(*env)->Throw(env, exc);
		}

		native_trace("after release pipe\n");
    }


    free(mesg);

    if (urlstr) (*env)->ReleaseStringUTFChars(env, jurlstr, urlstr);
    if (methodName) (*env)->ReleaseStringChars(env, jmethodName, methodName);
}


/*
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT int JNICALL
Java_sun_plugin_viewer_MNetscapePluginObject_convertToGlobalRef
(JNIEnv *env, jobject jObject, jobject obj)
{
    jobject res = (*env)->NewGlobalRef(env, obj);
    int res_as_int = (int) res;
    native_trace("OJIPlugin_convertToGlobalRef(): global ref %d\n", 
		 res_as_int);
    return res_as_int;
}

/*
 * Signature: (I;)V
 */
JNIEXPORT void JNICALL
Java_sun_plugin_viewer_MNetscapePluginObject_releaseGlobalRef
	(JNIEnv* env, jobject jObject, jint globalRef)
{
    if (globalRef != 0)
      (*env)->DeleteGlobalRef(env, (jobject)globalRef);

    return;
}

/*
 * Signature: (Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL
Java_sun_plugin_navig_motif_OJIPlugin_nativeInitializePipe
(JNIEnv *env, jclass jClazz, jint pipe, jobject rlock, jobject wlock) 
{
    jobject read_lock = NULL;
    jobject write_lock = NULL;
    if (rlock != NULL)
      read_lock = (*env)->NewGlobalRef(env, rlock);
    if (wlock != NULL)
      write_lock = (*env)->NewGlobalRef(env, wlock);

    native_trace("OJIPlugin_nativeInitializePipe(): Tyring to initialize pipe: %d\n", pipe);
    init_pipe_interface(pipe, read_lock, write_lock);

    return;
}


static jobjectArray dummyArrArg;
static jobject dummyObjArg;

/*
 * Class:     sun_plugin_javascript_navig5_JSObject
 * Method:    JSObjectInvoke
 * Signature: (IIIILjava/lang/String;Ljava/lang/String;[Ljava/lang/Object;Z)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_sun_plugin_javascript_navig5_JSObject_JSObjectInvoke
  (JNIEnv *env, 
   jobject jobj,
   jint   invokeCode,
   jint   jsThreadID,
   jint   pPluginInstance,
   jint   nativeJSObject, 
   jstring url, 
   jstring methodName, 
   jobjectArray args,
   jboolean isAllPermission)
{
  jobject ret;
  
  SendJNIJSMessage(env,
		   JNIJS_BASE + invokeCode,
		   pPluginInstance, nativeJSObject,
		   url,
		   methodName,
		   args,
		   isAllPermission,
		   &ret);

  return ret;
}

/*
 * Class:     sun_plugin_javascript_navig5_JSObject
 * Method:    JSObjectCleanup
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_sun_plugin_javascript_navig5_JSObject_JSObjectCleanup
  (JNIEnv *env, jobject jobj, jint jsThreadID, jint pPluginInstance, jint nativeJSObject)
{

  jobject dummy;

  SendJNIJSMessage(env, JAVA_PLUGIN_JNIJS_FINALIZE,
		   pPluginInstance, nativeJSObject, NULL, NULL, NULL, NULL, &dummy);

}
/*
 * Class:     sun_plugin_javascript_navig5_JSObject
 * Method:    JSGetILiveConnect
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL 
Java_sun_plugin_javascript_navig5_JSObject_JSGetILiveConnect
(JNIEnv *env, jclass jsClass, jint inst)
{
    jint ret = (int)NULL;
    /* Don't bother. Use a global on the other end */
    return ret;
}

/*
 * Class:     sun_plugin_javascript_navig5_JSObject
 * Method:    JSGetThreadID
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_sun_plugin_javascript_navig5_JSObject_JSGetThreadID
  (JNIEnv *env,
   jclass jsClass,
   jint jnsIPluginInstancePeer)
{
    jint ret = NULL;
#if 0
    /* Find running Plug-in instance from the map to ensure the
     plug-in instance is still running.
    */
    nsIPluginInstancePeer* pPeer = reinterpret_cast<nsIPluginInstancePeer*>(jnsIPluginInstancePeer);

    CJavaPlugin* pPlugin = g_App.FindPluginInMap(pPeer);

    /* Check if plug-in instance still exist and valid */

    if (pPlugin != NULL)
    {
        CJavaPlugin* pPlugin = g_App.FindAnyPluginInMap();

        CJavaPluginView* pView = reinterpret_cast<CJavaPluginView*>(pPlugin->GetView());
        HWND hWnd = pView->GetHWND();

        /* Query the JS thread ID from the main thread */
        ret = ::SendMessage(hWnd, WM_MOZILLA_JS_QUERY_THREAD_ID, (WPARAM) pPeer, (LPARAM) NULL);
    }

#endif
    return ret;
}

/*
 * Class:     sun_plugin_services_Netscape6BrowserAuthenticator
 * Method:    getBrowserAuthentication
 * Signature: (Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)[C
 * 
 * To simpify the implementation and take advantage of current infrastructure, convert the call
 * to Javascript call() method semantic.
 * Current infrastructure of call over pipe is very badly written, we should do something about
 * it in tiger.
 */
JNIEXPORT jcharArray JNICALL Java_sun_plugin_services_MNetscape6BrowserAuthenticator_getBrowserAuthentication
(JNIEnv *env, jobject obj, jobjectArray args) {
  
  jobject ret;

  SendJNIJSMessage(env, JAVA_PLUGIN_GET_BROWSER_AUTHINFO, 0, 0, NULL, NULL, 
		   args, NULL, &ret);

  return ret;
}
