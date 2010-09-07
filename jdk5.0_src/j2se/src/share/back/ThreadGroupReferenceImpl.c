/*
 * @(#)ThreadGroupReferenceImpl.c	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "ThreadGroupReferenceImpl.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
name(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env;
    jthreadGroup group;
    
    group = inStream_readThreadGroupRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {

	jvmtiThreadGroupInfo info;

	(void)memset(&info, 0, sizeof(info));
	threadGroupInfo(group, &info);
	(void)outStream_writeString(out, info.name);
        if ( info.name != NULL )
	    jvmtiDeallocate(info.name);
    
    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

static jboolean 
parent(PacketInputStream *in, PacketOutputStream *out) 
{
    JNIEnv *env;
    jthreadGroup group;
    
    group = inStream_readThreadGroupRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {

        jvmtiThreadGroupInfo info;
	
	(void)memset(&info, 0, sizeof(info));
	threadGroupInfo(group, &info);
	(void)outStream_writeObjectRef(out, info.parent);
        if ( info.name != NULL )
	    jvmtiDeallocate(info.name);

    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

static jboolean 
children(PacketInputStream *in, PacketOutputStream *out) 
{
     JNIEnv *env;
     jthreadGroup group;
     
     group = inStream_readThreadGroupRef(in);
     if (inStream_error(in)) {
         return JNI_TRUE;
     }
 
     env = getEnv();
    
     WITH_LOCAL_REFS(env, 1) {
     
	 jvmtiError error;
	 jint threadCount;
	 jint groupCount;
	 jthread *theThreads;
	 jthread *theGroups;
	 
	 error = JVMTI_FUNC_PTR(gdata->jvmti,GetThreadGroupChildren)(gdata->jvmti, group,
					      &threadCount,&theThreads,
					      &groupCount, &theGroups);
	 if (error != JVMTI_ERROR_NONE) {
	     outStream_setError(out, map2jdwpError(error));
	 } else {

	     int i;
	     
	     /* Squish out all of the debugger-spawned threads */
	     threadCount = filterDebugThreads(theThreads, threadCount);
	  
	     (void)outStream_writeInt(out, threadCount);
	     for (i = 0; i < threadCount; i++) {
		 (void)outStream_writeObjectRef(out, theThreads[i]);
	     }
	     (void)outStream_writeInt(out, groupCount);
	     for (i = 0; i < groupCount; i++) {
		 (void)outStream_writeObjectRef(out, theGroups[i]);
	     }

	     jvmtiDeallocate(theGroups);
	     jvmtiDeallocate(theThreads);
	 }

     } END_WITH_LOCAL_REFS(env);

     return JNI_TRUE;
}

void *ThreadGroupReference_Cmds[] = { (void *)3,
                                      (void *)name,
                                      (void *)parent,
                                      (void *)children };

