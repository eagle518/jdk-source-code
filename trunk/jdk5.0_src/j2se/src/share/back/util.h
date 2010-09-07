/*
 * @(#)util.h	1.89 04/02/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_UTIL_H
#define JDWP_UTIL_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef DEBUG
    /* Just to make sure these interfaces are not used here. */
    #undef free
    #define free(p) Do not use this interface.
    #undef malloc
    #define malloc(p) Do not use this interface.
    #undef calloc
    #define calloc(p) Do not use this interface.
    #undef realloc
    #define realloc(p) Do not use this interface.
    #undef strdup
    #define strdup(p) Do not use this interface.
#endif

#include "log_messages.h"
#include "vm_interface.h"
#include "JDWP.h"
#include "util_md.h"
#include "error_messages.h"
#include "debugInit.h"

/*
 * Globals used throughout the back end
 */

typedef jint FrameNumber;

typedef struct {
    jvmtiEnv *jvmti;
    JavaVM   *jvm;
    volatile jboolean vmDead; /* Once VM is dead it stays that way - don't put in init */
    jboolean assertOn;
    jboolean assertFatal;
    jboolean doerrorexit;

    char * options;

    jclass		classClass;
    jclass		threadClass;
    jclass		threadGroupClass;
    jclass		classLoaderClass;
    jclass		stringClass;
    jclass		systemClass;
    jmethodID		threadConstructor;
    jmethodID		threadSetDaemon;
    jmethodID		threadResume;
    jmethodID		systemGetProperty;
    jthreadGroup	systemThreadGroup;
    jmethodID		stringGetBytes;
    
    jint		cachedJvmtiVersion;
    jvmtiCapabilities	cachedJvmtiCapabilities;
    jboolean		haveCachedJvmtiCapabilities;
    jvmtiEventCallbacks callbacks;

    /* Various property values we should grab on initialization */
    char* property_java_version;          /* UTF8 java.version */
    char* property_java_vm_name;          /* UTF8 java.vm.name */
    char* property_java_vm_info;          /* UTF8 java.vm.info */
    char* property_java_class_path;       /* UTF8 java.class.path */
    char* property_sun_boot_class_path;   /* UTF8 sun.boot.class.path */
    char* property_sun_boot_library_path; /* NOT UTF8! sun.boot.library.path */
    char* property_path_separator;        /* UTF8 path.separator */
    char* property_user_dir;              /* UTF8 user.dir */

    unsigned log_flags;

} BackendGlobalData;

extern BackendGlobalData * gdata;

/*
 * Event Index for handlers
 */

typedef enum {
	EI_min			=  1,
	
	EI_SINGLE_STEP		=  1,
	EI_BREAKPOINT		=  2,
	EI_FRAME_POP		=  3,
	EI_EXCEPTION		=  4,
	EI_THREAD_START		=  5,
	EI_THREAD_END		=  6,
	EI_CLASS_PREPARE	=  7,
	EI_GC_FINISH		=  8,
	EI_CLASS_LOAD		=  9,
	EI_FIELD_ACCESS		= 10,
	EI_FIELD_MODIFICATION	= 11,
	EI_EXCEPTION_CATCH	= 12,
	EI_METHOD_ENTRY		= 13,
	EI_METHOD_EXIT		= 14,
	EI_VM_INIT		= 15,
	EI_VM_DEATH		= 16,
	
	EI_max			= 16
} EventIndex;

/* Combined event information */

typedef struct {
    
    EventIndex 	ei;
    jthread 	thread;
    jclass 	clazz;
    jmethodID 	method;
    jlocation 	location;
    jobject 	object; /* possibly an exception or user object */
    
    union {
    
	/* ei = EI_FIELD_ACCESS */
	struct {
	    jclass 	field_clazz;
	    jfieldID 	field;
	} field_access;

	/* ei = EI_FIELD_MODIFICATION */
	struct {
	    jclass 	field_clazz;
	    jfieldID 	field;
	    char 	signature_type;
	    jvalue 	new_value;
	} field_modification;

	/* ei = EI_EXCEPTION */
	struct {
	    jclass 	catch_clazz;
	    jmethodID 	catch_method;
	    jlocation 	catch_location;
	} exception;

    } u;

} EventInfo;

/*
 * JNI signature constants, beyond those defined in JDWP_TAG(*)
 */
#define SIGNATURE_BEGIN_ARGS    '('
#define SIGNATURE_END_ARGS      ')'
#define SIGNATURE_END_CLASS     ';'

/*
 * Modifier flags for classes, fields, methods
 */
#define MOD_PUBLIC       0x0001     /* visible to everyone */
#define MOD_PRIVATE      0x0002     /* visible only to the defining class */
#define MOD_PROTECTED    0x0004     /* visible to subclasses */
#define MOD_STATIC       0x0008     /* instance variable is static */
#define MOD_FINAL        0x0010     /* no further subclassing, overriding */
#define MOD_SYNCHRONIZED 0x0020     /* wrap method call in monitor lock */
#define MOD_VOLATILE     0x0040     /* can cache in registers */
#define MOD_TRANSIENT    0x0080     /* not persistant */
#define MOD_NATIVE       0x0100     /* implemented in C */
#define MOD_INTERFACE    0x0200     /* class is an interface */
#define MOD_ABSTRACT     0x0400     /* no definition provided */
/*
 * Additional modifiers not defined as such in the JVM spec
 */
#define MOD_SYNTHETIC    0xf0000000  /* not in source code */

/*
 * jlong conversion macros
 */
#define jlong_zero	 ((jlong) 0)
#define jlong_one	 ((jlong) 1)

#define jlong_to_ptr(a)  ((void*)(a))
#define ptr_to_jlong(a)  ((jlong)(a))
#define jint_to_jlong(a) ((jlong)(a))
#define jlong_to_jint(a) ((jint)(a))


/*
 * util funcs
 */
void util_initialize(void);   
void util_reset(void);

struct PacketInputStream;
struct PacketOutputStream;

jint uniqueID(void); 
jbyte referenceTypeTag(jclass clazz);
jbyte specificTypeKey(jobject object);
jboolean isObjectTag(jbyte tag);
jvmtiError spawnNewThread(jvmtiStartFunction func, void *arg, char *name);
void convertSignatureToClassname(char *convert);
void writeCodeLocation(struct PacketOutputStream *out, jclass clazz, 
                       jmethodID method, jlocation location);

/*
 * Command handling helpers shared among multiple command sets
 */
int filterDebugThreads(jthread *threads, int count);


void sharedGetFieldValues(struct PacketInputStream *in, 
                          struct PacketOutputStream *out, 
                          jboolean isStatic);
jboolean sharedInvoke(struct PacketInputStream *in,
                      struct PacketOutputStream *out);

jvmtiError fieldSignature(jclass, jfieldID, char **, char **, char **);
jvmtiError fieldModifiers(jclass, jfieldID, jint *);
jvmtiError methodSignature(jmethodID, char **, char **, char **);
jvmtiError methodModifiers(jmethodID, jint *);
jvmtiError methodClass(jmethodID, jclass *);
jvmtiError methodLocation(jmethodID, jlocation*, jlocation*);
jvmtiError classLoader(jclass, jobject *);

/*
 * Thin wrappers on top of JNI
 */
JNIEnv *getEnv(void);
jboolean isClass(jobject object);
jboolean isThread(jobject object);
jboolean isThreadGroup(jobject object);
jboolean isString(jobject object);
jboolean isClassLoader(jobject object);
jboolean isArray(jobject object);

/*
 * Thin wrappers on top of JVMTI
 */
jvmtiError jvmtiGetCapabilities(jvmtiCapabilities *caps);
jint jvmtiMajorVersion(void);
jint jvmtiMinorVersion(void);
jint jvmtiMicroVersion(void);
jvmtiError getSourceDebugExtension(jclass clazz, char **extensionPtr);
jboolean canSuspendResumeThreadLists(void);

jrawMonitorID debugMonitorCreate(char *name);
void debugMonitorEnter(jrawMonitorID theLock);
void debugMonitorExit(jrawMonitorID theLock);
void debugMonitorWait(jrawMonitorID theLock);
void debugMonitorTimedWait(jrawMonitorID theLock, jlong millis);
void debugMonitorNotify(jrawMonitorID theLock);
void debugMonitorNotifyAll(jrawMonitorID theLock);
void debugMonitorDestroy(jrawMonitorID theLock);

jthread *allThreads(jint *count);

void threadGroupInfo(jthreadGroup, jvmtiThreadGroupInfo *info);

char *getClassname(jclass);
jvmtiError classSignature(jclass, char**, char**);
jint classStatus(jclass);
void writeGenericSignature(struct PacketOutputStream *, char *);
jboolean isMethodNative(jmethodID);
jboolean isMethodObsolete(jmethodID);
jvmtiError isMethodSynthetic(jmethodID, jboolean*);
jvmtiError isFieldSynthetic(jclass, jfieldID, jboolean*);

jboolean isSameObject(JNIEnv *env, jobject o1, jobject o2);

jint objectHashCode(jobject);

jvmtiError allInterfaces(jclass clazz, jclass **ppinterfaces, jint *count);
jvmtiError allLoadedClasses(jclass **ppclasses, jint *count);
jvmtiError allClassLoaderClasses(jobject loader, jclass **ppclasses, jint *count);
jvmtiError allNestedClasses(jclass clazz, jclass **ppnested, jint *pcount);

void *jvmtiAllocate(jint numBytes);
void jvmtiDeallocate(void *buffer);

void             eventIndexInit(void);
jdwpEvent        eventIndex2jdwp(EventIndex i);
jvmtiEvent       eventIndex2jvmti(EventIndex i);
EventIndex       jdwp2EventIndex(jdwpEvent eventType);
EventIndex       jvmti2EventIndex(jvmtiEvent kind);

jvmtiError       map2jvmtiError(jdwpError);
jdwpError        map2jdwpError(jvmtiError);
jdwpThreadStatus map2jdwpThreadStatus(jint state);
jint             map2jdwpSuspendStatus(jint state);
jint             map2jdwpClassStatus(jint);

void log_debugee_location(const char *func, 
		jthread thread, jmethodID method, jlocation location);

/*
 * Local Reference management. The two macros below are used 
 * throughout the back end whenever space for JNI local references
 * is needed in the current frame. 
 */

void createLocalRefSpace(JNIEnv *env, jint capacity);

#define WITH_LOCAL_REFS(env, number) \
    createLocalRefSpace(env, number); \
    { /* BEGINNING OF WITH SCOPE */

#define END_WITH_LOCAL_REFS(env) \
	JNI_FUNC_PTR(env,PopLocalFrame)(env, NULL); \
    } /* END OF WITH SCOPE */

void saveGlobalRef(JNIEnv *env, jobject obj, jobject *pobj);
void tossGlobalRef(JNIEnv *env, jobject *pobj);

#endif

