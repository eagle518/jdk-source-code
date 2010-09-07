/*
 * @(#)JPLISAgent.c	1.9 04/06/11
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

#include    <jni.h>
#include    <jvmti.h>
#include    <stdlib.h>
#include    <string.h>

#include    "JPLISAgent.h"
#include    "JPLISAssert.h"
#include    "Utilities.h"
#include    "Reentrancy.h"
#include    "JavaExceptions.h"
#include    "sun_instrument_InstrumentationImpl.h"

/*
 * Copyright 2003 Wily Technology, Inc.
 */


/*
 *  The JPLISAgent manages the initialization all of the Java programming language Agents.
 *  It also supports the native method bridge between the JPLIS and the JVMTI.
 *  It maintains a single JVMTI Env that all JPL agents share.
 *  It parses command line requests and creates individual Java agents.
 */


/*
 *  private prototypes
 */
 
/*  Creates a new JPLIS agent. Used by the singleton management code to make the singleton.
 *  Returns error if the agent cannot be created and initialized.
 *  The JPLISAgent* pointed to by agent_ptr is set to the new singleton broker,
 *  or NULL if an error has occurred.
 */
JPLISInitializationError
createNewJPLISAgent(JavaVM * vm, JPLISAgent **agent_ptr);

/* Allocates an unformatted JPLIS agent data structure. Returns NULL if allocation fails. */
JPLISAgent *
allocateJPLISAgent(jvmtiEnv *       jvmtiEnv);

/* Initializes an already-allocated JPLIS agent data structure. */
JPLISInitializationError
initializeJPLISAgent(   JPLISAgent *    agent,
                        JavaVM *        vm,
                        jvmtiEnv *      jvmtienv);
/* De-allocates a JPLIS agent data structure. Only used in partial-failure cases at startup;
 * in normal usage the JPLIS agent lives forever
 */                     
void
deallocateJPLISAgent(   jvmtiEnv *      jvmtienv,
                        JPLISAgent *    agent);

/* Does one-time work to interrogate the JVM about capabilities and cache the answers. */
void
checkCapabilities(JPLISAgent * agent);

/* Adds can_redefine_classes capability */
void
addRedefineClassesCapability(JPLISAgent * agent);
 

/* Our JPLIS agent is paralleled by a singleton Java InstrumentationImpl instance.
 * This routine uses JNI to create and initialized the Java singleton.
 * Returns true if it succeeds, false otherwise.
 */
jboolean
initializeJavaSingleton(    JPLISAgent *    agent,
                            JNIEnv *        jnienv);

/* Swaps the start phase event handlers out and the live phase event handlers in.
 * Returns true if it succeeds, false otherwise.
 */
jboolean
setLivePhaseEventHandlers(  JPLISAgent * agent);

/* Takes the elements of the command string (agent class name and options string) and
 * create java strins for them.
 * Returns true if a classname was found. Makes no promises beyond the textual; says nothing about whether
 * the class exists or can be loaded.
 * If return value is true, sets outputClassname to a non-NULL local JNI reference.
 * If return value is true, sets outputOptionsString either to NULL or to a non-NULL local JNI reference.
 * If return value is false, neither output parameter is set.
 */
jboolean
commandStringIntoJavaStrings(  jvmtiEnv *      jvmtiEnv,
                               JNIEnv *        jnienv,
                               const char *    classname,
			       const char *    optionsString, 
                               jstring *       outputClassname,
                               jstring *       outputOptionsString);

/* Loads each Java agent according to the already processed command lines. For each,
 * loads the Java agent class, then calls the premain method.
 * Returns true if all Java agent classes are loaded and all premain methods complete with no exceptions,
 * false otherwise.
 */
jboolean
startAllJavaAgents( JPLISAgent *    agent,
                    JNIEnv *        jnienv);

/* Start one Java agent from the supplied parameters.
 * Most of the logic lives in a helper function that lives over in Java code--
 * we pass parameters out to Java and use our own Java helper to actually
 * load the agent and call the premain.
 * Returns true if the Java agent class is loaded and the premain method completes with no exceptions,
 * false otherwise.
 */
jboolean
startJavaAgent( JNIEnv *    jnienv,
                jobject     instrumentationImpl,
                jmethodID   premainCaller,
                jstring     className,
                jstring     optionsString);     

/* Once we have loaded all the Java agents and called all the premains,
 * we can release all the copies we have been keeping of the command line string buffers.
 */
void
untrackAllJavaAgentCommandLines(JPLISAgent * agent);

/*
 *  Common support for various class list fetchers.
 */
typedef jvmtiError (*ClassListFetcher)
    (   jvmtiEnv *  jvmtiEnv,
        jobject     classLoader,
        jint *      classCount,
        jclass **   classes);

/* Fetcher that ignores the class loader parameter, and uses the JVMTI to get a list of all classes.
 * Returns a jvmtiError according to the underlying JVMTI service.
 */
jvmtiError
getAllLoadedClassesClassListFetcher(    jvmtiEnv *  jvmtiEnv,
                                        jobject     classLoader,
                                        jint *      classCount,
                                        jclass **   classes);

/* Fetcher that uses the class loader parameter, and uses the JVMTI to get a list of all classes
 * for which the supplied loader is the initiating loader.
 * Returns a jvmtiError according to the underlying JVMTI service.
 */
jvmtiError
getInitiatedClassesClassListFetcher(    jvmtiEnv *  jvmtiEnv,
                                        jobject     classLoader,
                                        jint *      classCount,
                                        jclass **   classes);

/*
 * Common guts for two native methods, which are the same except for the policy for fetching
 * the list of classes.
 * Either returns a local JNI reference to an array of references to java.lang.Class.
 * Can throw, if it does will alter the JNIEnv with an outstanding exception.
 */
jobjectArray
commonGetClassList( JNIEnv *            jnienv,
                    JPLISAgent *        agent,
                    jobject             classLoader,
                    ClassListFetcher    fetcher);


/*
 *  Misc. utilities.
 */

/* Checked exception mapper used by the redefine classes implementation.
 * Allows ClassNotFoundException, maps others to InternalError. Can return NULL in an error case. */
jthrowable
redefineClassMapper(    JNIEnv *    jnienv,
                        jthrowable  throwableToMap);

/* Turns a buffer of jclass * into a Java array whose elements are java.lang.Class.
 * Can throw, if it does will alter the JNIEnv with an outstanding exception.
 */
jobjectArray
getObjectArrayFromClasses(JNIEnv* jnienv, jclass* classes, jint classCount);


/*
 *  Workarounds. These will be gone before we are finished (waiting on JVMTI changes)
 */


/*
 *  Manages the singleton JPLISAgent instance.
 *  Stores NULL if the singleton cannot be created.
 *
 */
static JPLISAgent * sSingletonBroker        = NULL;
 

JPLISInitializationError
insureSingletonJPLISAgent(JavaVM * vm, JPLISAgent **agent_ptr) {
    JPLISInitializationError initerror = JPLIS_INIT_ERROR_NONE;
    if ( sSingletonBroker == NULL ) {
        initerror = createNewJPLISAgent(vm, &sSingletonBroker);
    }
    *agent_ptr = sSingletonBroker;
    return initerror;
}

JPLISAgent *
getSingletonJPLISAgent(jvmtiEnv * jvmtienv) {
    JPLISAgent *    agent        = NULL;
    jvmtiError      jvmtierror   = JVMTI_ERROR_NONE;

    jvmtierror = (*jvmtienv)->GetEnvironmentLocalStorage(
                                            jvmtienv,
                                            (void**)&agent);
    jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
    
    if (jvmtierror == JVMTI_ERROR_NONE) {
        jplis_assert(agent != NULL);
        jplis_assert(agent == sSingletonBroker);
        jplis_assert(agent->mJVMTIEnv == jvmtienv);
    } else {
        agent = NULL;
    }
    return agent;   
}

/*
 *  OnLoad processing code.
 */

/*
 *  Creates a new JPLISAgent.
 *  Returns error if the agent cannot be created and initialized.
 *  The JPLISAgent* pointed to by agent_ptr is set to the new broker,
 *  or NULL if an error has occurred.
 */
JPLISInitializationError
createNewJPLISAgent(JavaVM * vm, JPLISAgent **agent_ptr) {
    JPLISInitializationError initerror       = JPLIS_INIT_ERROR_NONE;
    jvmtiEnv *               jvmtienv        = NULL;    
    jint                     jnierror        = JNI_OK;

    *agent_ptr = NULL;
    jnierror = (*vm)->GetEnv(  vm,
                               (void **) &jvmtienv,
                               JVMTI_VERSION);
    if ( jnierror != JNI_OK ) {
        initerror = JPLIS_INIT_ERROR_CANNOT_CREATE_NATIVE_AGENT;
    } else {
        JPLISAgent * agent = allocateJPLISAgent(jvmtienv);
        if ( agent == NULL ) {
            initerror = JPLIS_INIT_ERROR_ALLOCATION_FAILURE;
        } else {
            initerror = initializeJPLISAgent(  agent,
                                               vm,
                                               jvmtienv);
            if ( initerror == JPLIS_INIT_ERROR_NONE ) {
                *agent_ptr = agent;
            } else {
                deallocateJPLISAgent(jvmtienv, agent);
            }
        }
            
        /* don't leak envs */   
        if ( initerror != JPLIS_INIT_ERROR_NONE ) {
            jvmtiError jvmtierror = (*jvmtienv)->DisposeEnvironment(jvmtienv);
            jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
        }
    }
    
    return initerror;
}

/*
 *  Allocates a JPLISAgent. Returns NULL if it cannot be allocated
 */
JPLISAgent *
allocateJPLISAgent(jvmtiEnv * jvmtienv) {
    return (JPLISAgent *) allocate( jvmtienv,
                                    sizeof(JPLISAgent));
}

JPLISInitializationError
initializeJPLISAgent(   JPLISAgent *    agent,
                        JavaVM *        vm,
                        jvmtiEnv *      jvmtienv) {
    jvmtiError           jvmtierror = JVMTI_ERROR_NONE;

    agent->mJVM                     = vm;
    agent->mJVMTIEnv                = jvmtienv;
    agent->mInstrumentationImpl     = NULL;
    agent->mPremainCaller           = NULL;
    agent->mTransform               = NULL;
    agent->mRedefineAvailable       = JNI_FALSE;    /* assume no until we learn otherwise */    
    agent->mRedefineAdded           = JNI_FALSE;    
    agent->mCommandLineStringCount  = 0;
    agent->mAgentClassList          = NULL;
    agent->mOptionsStringList       = NULL;

    /* make sure we can recover either handle in either direction.
     * the agent has a ref to the jvmti; make it mutual
     */
    jvmtierror = (*jvmtienv)->SetEnvironmentLocalStorage(
                                            jvmtienv,
                                            agent);
    jplis_assert(jvmtierror == JVMTI_ERROR_NONE);

    /* check what capabilities are available */
    checkCapabilities(agent);
    
    /* now turn on the VMInit event */
    if ( jvmtierror == JVMTI_ERROR_NONE ) {
        jvmtiEventCallbacks callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.VMInit = &eventHandlerVMInit;
        
        jvmtierror = (*jvmtienv)->SetEventCallbacks( jvmtienv,
                                                     &callbacks,
                                                     sizeof(callbacks));
        jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
    }
    
    if ( jvmtierror == JVMTI_ERROR_NONE ) {
        jvmtierror = (*jvmtienv)->SetEventNotificationMode(
                                                jvmtienv,
                                                JVMTI_ENABLE,
                                                JVMTI_EVENT_VM_INIT,
                                                NULL /* all threads */);
        jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
    }
    
    return (jvmtierror == JVMTI_ERROR_NONE)? JPLIS_INIT_ERROR_NONE : JPLIS_INIT_ERROR_FAILURE;
}
                            
void
deallocateJPLISAgent(jvmtiEnv * jvmtienv, JPLISAgent * agent) {
    deallocate(jvmtienv, agent);
}


JPLISInitializationError
trackJavaAgentCommandLine(  JPLISAgent *    agent,
                            const char *    agentClass,
			    const char *    optionsString,
			    jboolean	    redefineClasses) {
    JPLISInitializationError    initerror   = JPLIS_INIT_ERROR_NONE;
    jint        currentSize                 = agent->mCommandLineStringCount;
    jint        newSize                     = currentSize + 1;
    char *      ourCopyOfAgentClass  = NULL;
    char *      ourCopyOfOptionsString = NULL;
    
    /* if no actual params, bail out now */
    if ( (agentClass == NULL) || (*agentClass == 0) ) {
        initerror = JPLIS_INIT_ERROR_AGENT_CLASS_NOT_SPECIFIED;
    } else {
	if (redefineClasses) {
	    addRedefineClassesCapability(agent);
        }

 	ourCopyOfAgentClass = allocate(  agent->mJVMTIEnv, strlen(agentClass)+1 );
        if ( ourCopyOfAgentClass == NULL ) { 
            initerror = JPLIS_INIT_ERROR_ALLOCATION_FAILURE;
	} else {
	    if ( optionsString != NULL ) {
		ourCopyOfOptionsString = allocate( agent->mJVMTIEnv, strlen(optionsString)+1 );
		if ( ourCopyOfOptionsString == NULL ) {
		    initerror = JPLIS_INIT_ERROR_ALLOCATION_FAILURE;
		}
	    } else {
		ourCopyOfOptionsString = NULL;
	    }
	}

	if ( initerror == JPLIS_INIT_ERROR_NONE) {
            char const ** buffer    = NULL;

	    strcpy( ourCopyOfAgentClass, agentClass );
	    if ( optionsString != NULL ) {
		strcpy( ourCopyOfOptionsString, optionsString );
	    }

            buffer = allocate( agent->mJVMTIEnv,
                               sizeof(const char *) * newSize);
	    if (buffer != NULL) {
                if ( currentSize > 0 ) {
                    memcpy(buffer, agent->mAgentClassList, sizeof(const char *) * currentSize);
                    deallocate( agent->mJVMTIEnv,
                                agent->mAgentClassList );
                }
                buffer[currentSize] = ourCopyOfAgentClass;
                agent->mAgentClassList = buffer;

                buffer = allocate( agent->mJVMTIEnv,
                                   sizeof(const char *) * newSize);
                if (buffer != NULL) {
                    if ( currentSize > 0 ) {
                        memcpy(buffer, agent->mOptionsStringList, sizeof(const char *) * currentSize);
                        deallocate( agent->mJVMTIEnv,
                                    agent->mOptionsStringList );
                    }
                    buffer[currentSize] = ourCopyOfOptionsString;
                    agent->mOptionsStringList = buffer;
		}
            }

	    if ( buffer == NULL ) {
		initerror = JPLIS_INIT_ERROR_ALLOCATION_FAILURE;
	    } else {
                agent->mCommandLineStringCount++;
            }
        }
    }
    return initerror;
}

/*
 *  VMInit processing code.
 */


/*
 * If this call fails, the JVM launch will ultimately be aborted,
 * so we don't have to be super-careful to clean up in partial failure
 * cases.
 */ 
jboolean
processJavaStart(   JPLISAgent *    agent,
                    JNIEnv *        jnienv) {
    jboolean    result;

    /*
     *  OK, Java is up now. We can start everything that needs Java.
     */

    /*
     *  First make our emergency fallback InternalError throwable.
     */
    result = initializeFallbackError(jnienv);
    jplis_assert(result);
    
    /*
     *  Now make the singleton InstrumentationImpl instance.
     */
    if ( result ) {
        result = initializeJavaSingleton(agent, jnienv);
        jplis_assert(result);
    }

    
    /*
     *  Then turn off the VMInit handler and turn on the ClassFileLoadHook.
     *  This way it is on before anyone registers a transformer.
     */
    if ( result ) {
        result = setLivePhaseEventHandlers(agent);
        jplis_assert(result);
    }
    
    /*
     *  Load all the Java agents, and call all the premains.
     */
    if ( result ) {
        result = startAllJavaAgents(agent,
                                    jnienv);
    }
    
    /*
     * Finally surrender all of the tracking data that we don't need any more.
     * If something is wrong, skip it, we will be aborting the JVM anyway.
     */
    if ( result ) {
        untrackAllJavaAgentCommandLines(agent);
    }
    
    return result;
}

jboolean
startAllJavaAgents( JPLISAgent *    agent,
                    JNIEnv *        jnienv) {
    int         x;
        
    for ( x = 0; x < agent->mCommandLineStringCount; x++ ) {
        jboolean    success = JNI_FALSE;
	const char * agentClass = agent->mAgentClassList[x];
        const char * optionsString = agent->mOptionsStringList[x];
        
        jstring classNameObject = NULL;
        jstring optionsStringObject = NULL;
        
        success = commandStringIntoJavaStrings(    agent->mJVMTIEnv,
                                                   jnienv,
                                                   agentClass,
						   optionsString,
                                                   &classNameObject,
                                                   &optionsStringObject);
        
        if ( success ) {
            success = startJavaAgent(   jnienv,
                                        agent->mInstrumentationImpl,
                                        agent->mPremainCaller,
                                        classNameObject,
                                        optionsStringObject);       
        }
        
        /* if any one agent fails, short-circuit bail; this will cause us to abort the JVM */
        if ( !success ) {
            return JNI_FALSE;
        }
    }
    
    /* if we make it through the whole loop, return happy */
    return JNI_TRUE;
}

void
untrackAllJavaAgentCommandLines(    JPLISAgent * agent) {
    int x = 0;
    
    if ( agent->mCommandLineStringCount > 0 ) {
        /* release all the copied command line strings */
        for ( x = 0; x < agent->mCommandLineStringCount; x++ ) {
            deallocate( agent->mJVMTIEnv,
                        (void*)agent->mAgentClassList[x]);
	    if ( agent->mOptionsStringList[x] != NULL) {
	        deallocate( agent->mJVMTIEnv, agent->mOptionsStringList[x] );
	    }
        }
        
        /* release the tracking buffer */
        deallocate( agent->mJVMTIEnv,
                    agent->mAgentClassList );
	deallocate( agent->mJVMTIEnv,
		    agent->mOptionsStringList );
        
        /* zero things out so it is easier to see what is going on */
	agent->mAgentClassList = NULL;
	agent->mOptionsStringList = NULL;
        agent->mCommandLineStringCount = 0;
    }
}


jboolean
initializeJavaSingleton(    JPLISAgent *    agent,
                            JNIEnv *        jnienv) {
    jclass      implClass               = NULL;
    jboolean    errorOutstanding        = JNI_FALSE;
    jobject     resultImpl              = NULL;
    jmethodID   premainCallerMethodID   = NULL;
    jmethodID   transformMethodID       = NULL;
    jmethodID   constructorID           = NULL;
    jobject     localReference          = NULL;

    /* First find the class of our implementation */        
    implClass = (*jnienv)->FindClass(   jnienv,
                                        JPLIS_INSTRUMENTIMPL_CLASSNAME);
    errorOutstanding = checkForAndClearThrowable(jnienv);
    errorOutstanding = errorOutstanding || (implClass == NULL);
    jplis_assert_msg(!errorOutstanding, "find class on InstrumentationImpl failed");
    
    /* Use the class to create the singleton instance */
    if ( !errorOutstanding ) {
        constructorID = (*jnienv)->GetMethodID( jnienv,
                                                implClass,
                                                JPLIS_INSTRUMENTIMPL_CONSTRUCTOR_METHODNAME,
                                                JPLIS_INSTRUMENTIMPL_CONSTRUCTOR_METHODSIGNATURE);
        errorOutstanding = checkForAndClearThrowable(jnienv);
        errorOutstanding = errorOutstanding || (constructorID == NULL);
        jplis_assert_msg(!errorOutstanding, "find constructor on InstrumentationImpl failed");
        }
        
    if ( !errorOutstanding ) {
        jlong   peerReferenceAsScalar = (jlong) agent;
        localReference = (*jnienv)->NewObject(  jnienv,
                                                implClass,
                                                constructorID,
                                                peerReferenceAsScalar,
                                                agent->mRedefineAdded); 
        errorOutstanding = checkForAndClearThrowable(jnienv);
        errorOutstanding = errorOutstanding || (localReference == NULL);
        jplis_assert_msg(!errorOutstanding, "call constructor on InstrumentationImpl failed");
    }
    
    if ( !errorOutstanding ) {
        resultImpl = (*jnienv)->NewGlobalRef(jnienv, localReference);
        errorOutstanding = checkForAndClearThrowable(jnienv);
        jplis_assert_msg(!errorOutstanding, "copy local ref to global ref");
    }

    /* Now look up the method ID for the pre-main caller (we will need this more than once) */
    if ( !errorOutstanding ) {
        premainCallerMethodID = (*jnienv)->GetMethodID( jnienv,
                                                        implClass,
                                                        JPLIS_INSTRUMENTIMPL_PREMAININVOKER_METHODNAME,
                                                        JPLIS_INSTRUMENTIMPL_PREMAININVOKER_METHODSIGNATURE);
        errorOutstanding = checkForAndClearThrowable(jnienv);
        errorOutstanding = errorOutstanding || (premainCallerMethodID == NULL);
        jplis_assert_msg(!errorOutstanding, "can't find premain invoker methodID");
    }
    
    /* Now look up the method ID for the transform method (we will need this constantly) */
    if ( !errorOutstanding ) {
        transformMethodID = (*jnienv)->GetMethodID( jnienv,
                                                    implClass,
                                                    JPLIS_INSTRUMENTIMPL_TRANSFORM_METHODNAME,
                                                    JPLIS_INSTRUMENTIMPL_TRANSFORM_METHODSIGNATURE);
        errorOutstanding = checkForAndClearThrowable(jnienv);
        errorOutstanding = errorOutstanding || (transformMethodID == NULL);
        jplis_assert_msg(!errorOutstanding, "can't find transform methodID");
    }
    
    if ( !errorOutstanding ) {
        agent->mInstrumentationImpl = resultImpl;
        agent->mPremainCaller       = premainCallerMethodID;
        agent->mTransform           = transformMethodID;
    }
        
    return !errorOutstanding;
}

jboolean
commandStringIntoJavaStrings(  jvmtiEnv *      jvmtienv,
                               JNIEnv *        jnienv,
                               const char *    classname,
			       const char *    optionsString,
                               jstring *       outputClassname,
                               jstring *       outputOptionsString) {
    jstring     classnameJavaString     = NULL;
    jstring     optionsJavaString       = NULL;
    jboolean    errorOutstanding        = JNI_TRUE;
    
    classnameJavaString = (*jnienv)->NewStringUTF(jnienv, classname);
    errorOutstanding = checkForAndClearThrowable(jnienv);
    jplis_assert_msg(!errorOutstanding, "can't create class name java string");
        
    if ( !errorOutstanding ) {
	if ( optionsString != NULL) {
            optionsJavaString = (*jnienv)->NewStringUTF(jnienv, optionsString);
            errorOutstanding = checkForAndClearThrowable(jnienv);
            jplis_assert_msg(!errorOutstanding, "can't create options java string");
        }
                        
        if ( !errorOutstanding ) {
            *outputClassname        = classnameJavaString;
            *outputOptionsString    = optionsJavaString;
        }
    }
        
    return !errorOutstanding;   
}


jboolean
startJavaAgent( JNIEnv *    jnienv,
                jobject     instrumentationImpl,
                jmethodID   premainCallingMethod,
                jstring     className,
                jstring     optionsString) {
    jboolean errorOutstanding = JNI_FALSE;

    jplis_assert(premainCallingMethod != NULL);
    if ( premainCallingMethod != NULL ) {
        (*jnienv)->CallVoidMethod(  jnienv,
                                    instrumentationImpl,
                                    premainCallingMethod,
                                    className,
                                    optionsString);
        errorOutstanding = checkForThrowable(jnienv);
        if ( errorOutstanding ) {
            logThrowable(jnienv);
        }
        checkForAndClearThrowable(jnienv);
    }
    return !errorOutstanding;
}

jboolean
setLivePhaseEventHandlers(  JPLISAgent * agent) {
    jvmtiEventCallbacks callbacks;
    jvmtiEnv *          jvmtienv = agent->mJVMTIEnv;
    jvmtiError          jvmtierror;

    /* first swap out the handlers (switch from the VMInit handler, which we do not need,
     * to the ClassFileLoadHook handler, which is what the agents need from now on)
     */
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.ClassFileLoadHook = &eventHandlerClassFileLoadHook;
        
    jvmtierror = (*jvmtienv)->SetEventCallbacks( jvmtienv,
                                                 &callbacks,
                                                 sizeof(callbacks));
    jplis_assert(jvmtierror == JVMTI_ERROR_NONE);


    if ( jvmtierror == JVMTI_ERROR_NONE ) {
        /* turn off VMInit */
        jvmtierror = (*jvmtienv)->SetEventNotificationMode(
                                                    jvmtienv,
                                                    JVMTI_DISABLE,
                                                    JVMTI_EVENT_VM_INIT,
                                                    NULL /* all threads */);
        jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
    }

    if ( jvmtierror == JVMTI_ERROR_NONE ) {
        /* turn on ClassFileLoadHook */
        jvmtierror = (*jvmtienv)->SetEventNotificationMode(
                                                    jvmtienv,
                                                    JVMTI_ENABLE,
                                                    JVMTI_EVENT_CLASS_FILE_LOAD_HOOK,
                                                    NULL /* all threads */);
        jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
    }
        
    return (jvmtierror == JVMTI_ERROR_NONE);
}

/** 
 *  Check if the can_redefine_classes capability is available.
 */
void
checkCapabilities(JPLISAgent * agent) {
    jvmtiEnv *          jvmtienv = agent->mJVMTIEnv;
    jvmtiCapabilities   potentialCapabilities;
    jvmtiError          jvmtierror;
    
    memset(&potentialCapabilities, 0, sizeof(potentialCapabilities));
    
    jvmtierror = (*jvmtienv)->GetPotentialCapabilities(jvmtienv, &potentialCapabilities);
    jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
    
    if ( jvmtierror == JVMTI_ERROR_NONE ) {
        if ( potentialCapabilities.can_redefine_classes == 1 ) {
            agent->mRedefineAvailable = JNI_TRUE;
        }
    }
}

/**
 * Add the can_redefine_classes capability
 */
void
addRedefineClassesCapability(JPLISAgent * agent) {
    jvmtiEnv *          jvmtienv = agent->mJVMTIEnv;
    jvmtiCapabilities   desiredCapabilities;
    jvmtiError          jvmtierror;

    if (agent->mRedefineAvailable && !agent->mRedefineAdded) {
        jvmtierror = (*jvmtienv)->GetCapabilities(jvmtienv, &desiredCapabilities);
        jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
        desiredCapabilities.can_redefine_classes = 1;
        jvmtierror = (*jvmtienv)->AddCapabilities(jvmtienv, &desiredCapabilities);
        jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
	agent->mRedefineAdded = JNI_TRUE;
    }
}


/*
 *  Support for the JVMTI callbacks
 */
 
void
transformClassFile(             JPLISAgent *            agent,
                                JNIEnv *                jnienv,
                                jobject                 loaderObject, 
                                const char*             name, 
                                jclass                  classBeingRedefined,
                                jobject                 protectionDomain,
                                jint                    class_data_len, 
                                const unsigned char*    class_data, 
                                jint*                   new_class_data_len, 
                                unsigned char**         new_class_data) {
    jboolean        errorOutstanding        = JNI_FALSE;
    jstring         classNameStringObject   = NULL;
    jarray          classFileBufferObject   = NULL;
    jarray          transformedBufferObject = NULL;
    jsize           transformedBufferSize   = 0;
    unsigned char * resultBuffer            = NULL;
    jboolean        shouldRun               = JNI_FALSE;
    
    /* only do this if we aren't already in the middle of processing a class on this thread */
    shouldRun = tryToAcquireReentrancyToken(
                                agent->mJVMTIEnv,
                                NULL);  /* this thread */

    if ( shouldRun ) {
        /* first marshall all the parameters */
        classNameStringObject = (*jnienv)->NewStringUTF(jnienv,
                                                        name);
        errorOutstanding = checkForAndClearThrowable(jnienv);
        jplis_assert_msg(!errorOutstanding, "can't create name string");
    
        if ( !errorOutstanding ) {
            classFileBufferObject = (*jnienv)->NewByteArray(jnienv,
                                                            class_data_len);
            errorOutstanding = checkForAndClearThrowable(jnienv);
            jplis_assert_msg(!errorOutstanding, "can't create byte arrau");
        }
        
        if ( !errorOutstanding ) {
            jbyte * typedBuffer = (jbyte *) class_data; /* nasty cast, dumb JNI interface, const missing */
                                                        /* The sign cast is safe. The const cast is dumb. */
            (*jnienv)->SetByteArrayRegion(  jnienv,
                                            classFileBufferObject,
                                            0,
                                            class_data_len,
                                            typedBuffer);
            errorOutstanding = checkForAndClearThrowable(jnienv);
            jplis_assert_msg(!errorOutstanding, "can't set byte array region");
        }
    
        /*  now call the JPL agents to do the transforming */
        /*  potential future optimization: may want to skip this if there are none */
        if ( !errorOutstanding ) {
            jplis_assert(agent->mInstrumentationImpl != NULL);
            jplis_assert(agent->mTransform != NULL);
            transformedBufferObject = (*jnienv)->CallObjectMethod(
                                                jnienv,
                                                agent->mInstrumentationImpl,
                                                agent->mTransform,
                                                loaderObject,
                                                classNameStringObject,
                                                classBeingRedefined,    
                                                protectionDomain,
                                                classFileBufferObject);
            errorOutstanding = checkForAndClearThrowable(jnienv);
            jplis_assert_msg(!errorOutstanding, "transform method call failed");
        }
    
        /* Finally, unmarshall the parameters (if someone touched the buffer, tell the JVM) */
        if ( !errorOutstanding ) {
            if ( transformedBufferObject != NULL ) {
                transformedBufferSize = (*jnienv)->GetArrayLength(  jnienv,
                                                                    transformedBufferObject);
                errorOutstanding = checkForAndClearThrowable(jnienv);
                jplis_assert_msg(!errorOutstanding, "can't get array length");
                
                if ( !errorOutstanding ) {
                    /* allocate the response buffer with the JVMTI allocate call.
                     *  This is what the JVMTI spec says to do for Class File Load hook responses
                     */
                    jvmtiError  allocError = (*(agent->mJVMTIEnv))->Allocate(agent->mJVMTIEnv,
                                                                             transformedBufferSize,
                                                                             &resultBuffer);
                    errorOutstanding = (allocError != JVMTI_ERROR_NONE);
                    jplis_assert_msg(!errorOutstanding, "can't allocate result buffer");
                }
    
                if ( !errorOutstanding ) {
                    (*jnienv)->GetByteArrayRegion(  jnienv,
                                                    transformedBufferObject,
                                                    0,
                                                    transformedBufferSize,
                                                    (jbyte *) resultBuffer);
                    errorOutstanding = checkForAndClearThrowable(jnienv);
                    jplis_assert_msg(!errorOutstanding, "can't get byte array region");
                
                    /* in this case, we will not return the buffer to the JVMTI,
                     * so we need to deallocate it ourselves
                     */
                    if ( errorOutstanding ) {
                        deallocate(agent->mJVMTIEnv,
                                   resultBuffer);
                    }
                }
            
                if ( !errorOutstanding ) {
                    *new_class_data_len = (transformedBufferSize);
                    *new_class_data     = resultBuffer;
                }
            }
        }
        
        /* release the token */
        releaseReentrancyToken( agent->mJVMTIEnv,
                                NULL);      /* this thread */   
        
    }

    return;
}

/*
 *  Misc. internal utilities.
 */

/*
 *  The only checked exception we can throw is ClassNotFoundException.
 *  All others map to InternalError.
 */
jthrowable
redefineClassMapper(    JNIEnv *    jnienv,
                        jthrowable  throwableToMap) {
    jthrowable  mappedThrowable = NULL;
    
    jplis_assert(isSafeForJNICalls(jnienv));
    jplis_assert(!isUnchecked(jnienv, throwableToMap));

    if ( isInstanceofClassName( jnienv,
                                throwableToMap,
                                "java/lang/ClassNotFoundException") ) {
        mappedThrowable = throwableToMap;
    }
    else {
        jstring message = NULL;
        
        message = getMessageFromThrowable(jnienv, throwableToMap);
        mappedThrowable = createInternalError(jnienv, message);
    }

    jplis_assert(isSafeForJNICalls(jnienv));
    return mappedThrowable;
}


    
jobjectArray
getObjectArrayFromClasses(JNIEnv* jnienv, jclass* classes, jint classCount) {
    jclass          classArrayClass = NULL;
    jobjectArray    localArray      = NULL;
    jint            classIndex      = 0;
    jboolean        errorOccurred   = JNI_FALSE;
    
    /* get the class array class */
    classArrayClass = (*jnienv)->FindClass(jnienv, "java/lang/Class");
    errorOccurred = checkForThrowable(jnienv);

    if (!errorOccurred) {
        jplis_assert_msg(classArrayClass != NULL, "FindClass returned null class");
        
        /* create the array for the classes */
        localArray = (*jnienv)->NewObjectArray(jnienv, classCount, classArrayClass, NULL);
        errorOccurred = checkForThrowable(jnienv);
        
        if (!errorOccurred) {
            jplis_assert_msg(localArray != NULL, "NewObjectArray returned null array");
                
            /* now copy refs to all the classes and put them into the array */
            for (classIndex = 0; classIndex < classCount; classIndex++) {
                /* put class into array */
                (*jnienv)->SetObjectArrayElement(jnienv, localArray, classIndex, classes[classIndex]);
                errorOccurred = checkForThrowable(jnienv);

                if (errorOccurred) {
                    localArray = NULL;
                    break;
                }
            }
        }
    }
    
    return localArray;
}



/*
 *  Underpinnings for native methods
 */

/*
 *  Java code must not call this with a null list or a zero-length list.
 */
void
redefineClasses(JNIEnv * jnienv, JPLISAgent * agent, jobjectArray classDefinitions) {
    jvmtiEnv*   jvmtienv                        = agent->mJVMTIEnv;
    jboolean    errorOccurred                   = JNI_FALSE;
    jclass      classDefClass                   = NULL;
    jmethodID   getDefinitionClassMethodID      = NULL;
    jmethodID   getDefinitionClassFileMethodID  = NULL;
    jvmtiClassDefinition* classDefs             = NULL;
    jsize       numDefs                         = 0;
    
    jplis_assert(classDefinitions != NULL);
    
    numDefs = (*jnienv)->GetArrayLength(jnienv, classDefinitions);
    errorOccurred = checkForThrowable(jnienv);
    jplis_assert(!errorOccurred);
    
    if (!errorOccurred) {
        jplis_assert(numDefs > 0);
        /* get method IDs for methods to call on class definitions */
        classDefClass = (*jnienv)->FindClass(jnienv, "java/lang/instrument/ClassDefinition");
        errorOccurred = checkForThrowable(jnienv);
        jplis_assert(!errorOccurred);
    }
    
    if (!errorOccurred) {
        getDefinitionClassMethodID = (*jnienv)->GetMethodID(    jnienv,
                                                classDefClass,
                                                "getDefinitionClass",
                                                "()Ljava/lang/Class;");
        errorOccurred = checkForThrowable(jnienv);
        jplis_assert(!errorOccurred);
    }
    
    if (!errorOccurred) {
        getDefinitionClassFileMethodID = (*jnienv)->GetMethodID(    jnienv,
                                                    classDefClass,
                                                    "getDefinitionClassFile",
                                                    "()[B");
        errorOccurred = checkForThrowable(jnienv);
        jplis_assert(!errorOccurred);
    }
        
    if (!errorOccurred) {
        classDefs = (jvmtiClassDefinition *) allocate( 
                                                jvmtienv,
                                                numDefs * sizeof(jvmtiClassDefinition));
        errorOccurred = (classDefs == NULL);
        jplis_assert(!errorOccurred);
        if ( errorOccurred ) {
            createAndThrowThrowableFromJVMTIErrorCode(jnienv, JVMTI_ERROR_OUT_OF_MEMORY);
        }
        else {  
            jint i;
            for (i = 0; i < numDefs; i++) {
                jclass      classDef    = NULL;
                jbyteArray  targetFile  = NULL;
    
                classDef = (*jnienv)->GetObjectArrayElement(jnienv, classDefinitions, i);
                errorOccurred = checkForThrowable(jnienv);
                jplis_assert(!errorOccurred);
                if (errorOccurred) {
                    break;
                }
                
                classDefs[i].klass = (*jnienv)->CallObjectMethod(jnienv, classDef, getDefinitionClassMethodID);
                errorOccurred = checkForThrowable(jnienv);
                jplis_assert(!errorOccurred);
                if (errorOccurred) {
                    break;
                }
                
                targetFile = (*jnienv)->CallObjectMethod(jnienv, classDef, getDefinitionClassFileMethodID);
                errorOccurred = checkForThrowable(jnienv);
                jplis_assert(!errorOccurred);
                if (errorOccurred) {
                    break;
                }
                
                classDefs[i].class_bytes = (unsigned char*)(*jnienv)->GetByteArrayElements(jnienv, targetFile, NULL);
                errorOccurred = checkForThrowable(jnienv);
                jplis_assert(!errorOccurred);
                if (errorOccurred) {
                    break;
                }
                
                classDefs[i].class_byte_count = (*jnienv)->GetArrayLength(jnienv, targetFile);
                errorOccurred = checkForThrowable(jnienv);
                jplis_assert(!errorOccurred);
                if (errorOccurred) {
                    break;
                }
            }
            
            if (!errorOccurred) {
                jvmtiError  errorCode = JVMTI_ERROR_NONE;
                errorCode = (*jvmtienv)->RedefineClasses(jvmtienv, numDefs, classDefs);
                errorOccurred = (errorCode != JVMTI_ERROR_NONE);
                if ( errorOccurred ) {
                    createAndThrowThrowableFromJVMTIErrorCode(jnienv, errorCode);
                } 
            }
    
            /* Give back the buffer if we allocated it.
             */
            deallocate(jvmtienv, classDefs);
        }
    }
    
    mapThrownThrowableIfNecessary(jnienv, redefineClassMapper);
}

/* Cheesy sharing. ClassLoader may be null. */
jobjectArray
commonGetClassList( JNIEnv *            jnienv,
                    JPLISAgent *        agent,
                    jobject             classLoader,
                    ClassListFetcher    fetcher) {
    jvmtiEnv *      jvmtienv        = agent->mJVMTIEnv;
    jboolean        errorOccurred   = JNI_FALSE;
    jvmtiError      jvmtierror      = JVMTI_ERROR_NONE;
    jint            classCount      = 0;
    jclass *        classes         = NULL;
    jobjectArray    localArray      = NULL;
    
    /* retrieve the classes from the JVMTI agent */
    jvmtierror = (*fetcher)( jvmtienv,
                        classLoader,
                        &classCount,
                        &classes);
    errorOccurred = (jvmtierror != JVMTI_ERROR_NONE);
    jplis_assert(!errorOccurred);
    
    if ( errorOccurred ) {
        createAndThrowThrowableFromJVMTIErrorCode(jnienv, jvmtierror);
    } else {
        localArray = getObjectArrayFromClasses( jnienv,
                                                classes,
                                                classCount);
        errorOccurred = checkForThrowable(jnienv);
        jplis_assert(!errorOccurred);
        
        /* do this whether or not we saw a problem */
        deallocate(jvmtienv, classes);
    }
        
    mapThrownThrowableIfNecessary(jnienv, mapAllCheckedToInternalErrorMapper);
    return localArray;

}

jvmtiError
getAllLoadedClassesClassListFetcher(    jvmtiEnv *  jvmtienv,
                                        jobject     classLoader,
                                        jint *      classCount,
                                        jclass **   classes) {
    return (*jvmtienv)->GetLoadedClasses(jvmtienv, classCount, classes);
}

jobjectArray
getAllLoadedClasses(JNIEnv * jnienv, JPLISAgent * agent) {
    return commonGetClassList(  jnienv,
                                agent,
                                NULL,
                                getAllLoadedClassesClassListFetcher);
}

jvmtiError
getInitiatedClassesClassListFetcher(    jvmtiEnv *  jvmtienv,
                                        jobject     classLoader,
                                        jint *      classCount,
                                        jclass **   classes) {
    return (*jvmtienv)->GetClassLoaderClasses(jvmtienv, classLoader, classCount, classes);
}


jobjectArray
getInitiatedClasses(JNIEnv * jnienv, JPLISAgent * agent, jobject classLoader) {
    return commonGetClassList(  jnienv,
                                agent,
                                classLoader,
                                getInitiatedClassesClassListFetcher);
}

jlong
getObjectSize(JNIEnv * jnienv, JPLISAgent * agent, jobject objectToSize) {
    jvmtiEnv *  jvmtienv    = agent->mJVMTIEnv;
    jlong       objectSize  = -1;
    jvmtiError  jvmtierror  = JVMTI_ERROR_NONE;
    
    jvmtierror = (*jvmtienv)->GetObjectSize(jvmtienv, objectToSize, &objectSize);
    jplis_assert(jvmtierror == JVMTI_ERROR_NONE);
    if ( jvmtierror != JVMTI_ERROR_NONE ) {
        createAndThrowThrowableFromJVMTIErrorCode(jnienv, jvmtierror);
    }
    
    mapThrownThrowableIfNecessary(jnienv, mapAllCheckedToInternalErrorMapper);
    return objectSize;
}


