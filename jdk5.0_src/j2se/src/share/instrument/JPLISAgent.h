/*
 * @(#)JPLISAgent.h	1.5 04/06/08
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#ifndef _JPLISAGENT_H_
#define _JPLISAGENT_H_

#include    <jni.h>
#include    <jvmti.h>

/*
 * Copyright 2003 Wily Technology, Inc.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  The JPLISAgent manages the initialization all of the Java programming language Agents.
 *  It also supports the native method bridge between the JPLIS and the JVMTI.
 *  It maintains a single JVMTI Env that all JPL agents share.
 *  It parses command line requests and creates individual Java agents.
 */


/*
 *  Forward definitions
 */
struct  _JPLISAgent;

typedef struct _JPLISAgent  JPLISAgent;


/* constants for class names and methods names and such 
    these all must stay in sync with Java code & interfaces
*/
#define JPLIS_INSTRUMENTIMPL_CLASSNAME                      "sun/instrument/InstrumentationImpl"
#define JPLIS_INSTRUMENTIMPL_GETNATIVEAGENT_METHODNAME      "getNativeAgent"
#define JPLIS_INSTRUMENTIMPL_GETNATIVEAGENT_METHODSIGNATURE "()J"
#define JPLIS_INSTRUMENTIMPL_CONSTRUCTOR_METHODNAME         "<init>"
#define JPLIS_INSTRUMENTIMPL_CONSTRUCTOR_METHODSIGNATURE    "(JZ)V"
#define JPLIS_INSTRUMENTIMPL_PREMAININVOKER_METHODNAME      "loadClassAndCallPremain"
#define JPLIS_INSTRUMENTIMPL_PREMAININVOKER_METHODSIGNATURE "(Ljava/lang/String;Ljava/lang/String;)V"
#define JPLIS_INSTRUMENTIMPL_TRANSFORM_METHODNAME           "transform"
#define JPLIS_INSTRUMENTIMPL_TRANSFORM_METHODSIGNATURE      "(Ljava/lang/ClassLoader;Ljava/lang/String;Ljava/lang/Class;Ljava/security/ProtectionDomain;[B)[B"


/*
 *  Error messages
 */
#define JPLIS_ERRORMESSAGE_CANNOTSTART              "processing of -javaagent failed"


/*
 *  Our initialization errors
 */
typedef enum {
  JPLIS_INIT_ERROR_NONE,
  JPLIS_INIT_ERROR_CANNOT_CREATE_NATIVE_AGENT,
  JPLIS_INIT_ERROR_FAILURE,
  JPLIS_INIT_ERROR_ALLOCATION_FAILURE,
  JPLIS_INIT_ERROR_AGENT_CLASS_NOT_SPECIFIED
} JPLISInitializationError;



struct _JPLISAgent {
    JavaVM *                mJVM;                   /* handle to the JVM */
    jvmtiEnv *              mJVMTIEnv;              /* all JPL agents share one jvmtiEnv */
    jobject                 mInstrumentationImpl;   /* handle to the shared Instrumentation instance */
    jmethodID               mPremainCaller;         /* method on the InstrumentationImpl that does the premain stuff (cached to save lots of lookups) */
    jmethodID               mTransform;             /* method on the InstrumentationImpl that does the class file transform */
    jboolean                mRedefineAvailable;     /* cached answer to "does this JVM support redefine" */
    jboolean		    mRedefineAdded;	    /* indicates if can_redefine_classes capability has been added */
    jint                    mCommandLineStringCount;/* current number of -javaagent command lines */    
    char const **           mAgentClassList;        /* list of agent class names */
    char const **	    mOptionsStringList;	    /* list of options strings */
};

/*
 * JVMTI event handlers
 */
 
/* VMInit event handler. Installed during OnLoad, then removed during VMInit. */ 
extern void JNICALL
eventHandlerVMInit( jvmtiEnv *      jvmtienv,
                    JNIEnv *        jnienv,
                    jthread         thread);

/* ClassFileLoadHook event handler. Installed during VMInit, then left in place forever. */ 
extern void JNICALL
eventHandlerClassFileLoadHook(  jvmtiEnv *              jvmtienv,
                                JNIEnv *                jnienv,
                                jclass                  class_being_redefined,
                                jobject                 loader, 
                                const char*             name, 
                                jobject                 protectionDomain,
                                jint                    class_data_len, 
                                const unsigned char*    class_data, 
                                jint*                   new_class_data_len, 
                                unsigned char**         new_class_data);

/*
 * Main entry points for the JPLIS JVMTI agent code
 */

/* looks up the singleton JVMTI agent instance. returns null if there isn't one */
extern JPLISAgent *
getSingletonJPLISAgent(jvmtiEnv * jvmtienv);

/* looks up the singleton JVMTI agent instance. If there isn't one, tries to make one.
 * returns null if the singleton cannot be created.
 */
extern JPLISInitializationError
insureSingletonJPLISAgent(JavaVM * vm, JPLISAgent **agent_ptr);

/* during OnLoad phase (command line parsing)
 *  this is how the invocation engine tells us to track a single occurrence of -javaagent
 */
extern JPLISInitializationError
trackJavaAgentCommandLine(  JPLISAgent *    agent,
                            const char *    agentClass,
			    const char *    optionsString,
			    jboolean	    redefineClasses );

/* during VMInit processing
 *  this is how the invocation engine (callback wrapper) tells us to start up all the javaagents
 */
extern jboolean
processJavaStart(   JPLISAgent *    agent,
                    JNIEnv *        jnienv);

/* on an ongoing basis,
 *  this is how the invocation engine (callback wrapper) tells us to process a class file
 */
extern void
transformClassFile(             JPLISAgent *            agent,
                                JNIEnv *                jnienv,
                                jobject                 loader, 
                                const char*             name, 
                                jclass                  classBeingRedefined,
                                jobject                 protectionDomain,
                                jint                    class_data_len, 
                                const unsigned char*    class_data, 
                                jint*                   new_class_data_len, 
                                unsigned char**         new_class_data);

/* on an ongoing basis,
 *  these are implementations of the Instrumentation services.
 *  Most are simple covers for JVMTI access services. These are the guts of the InstrumentationImpl
 *  native methods.
 */
extern void
redefineClasses(JNIEnv * jnienv, JPLISAgent * agent, jobjectArray classDefinitions);

extern jobjectArray
getAllLoadedClasses(JNIEnv * jnienv, JPLISAgent * agent);

extern jobjectArray
getInitiatedClasses(JNIEnv * jnienv, JPLISAgent * agent, jobject classLoader);

extern jlong
getObjectSize(JNIEnv * jnienv, JPLISAgent * agent, jobject objectToSize);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif
