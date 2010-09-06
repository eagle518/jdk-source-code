#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiCodeBlobEvents.hpp	1.5 03/12/23 16:43:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
 
#ifndef _JVMTI_CODE_BLOB_EVENTS_H_
#define _JVMTI_CODE_BLOB_EVENTS_H_

// forward declaration
class JvmtiEnv;


// JVMTI code blob event support
// -- used by GenerateEvents to generate CompiledMethodLoad and
//    DynamicCodeGenerated events
// -- also provide utility function build_jvmti_addr_location_map to create
//    a jvmtiAddrLocationMap list for a nmethod.

class JvmtiCodeBlobEvents : public AllStatic {
 public:

  // generate a DYNAMIC_CODE_GENERATED_EVENT event for each non-nmethod 
  // code blob in the code cache.
  static jvmtiError generate_dynamic_code_events(JvmtiEnv* env);

  // generate a COMPILED_METHOD_LOAD event for each nmethod 
  // code blob in the code cache.
  static jvmtiError generate_compiled_method_load_events(JvmtiEnv* env);

  // create a C-heap allocated address location map for an nmethod
  static void build_jvmti_addr_location_map(nmethod *nm, jvmtiAddrLocationMap** map, 
					    jint *map_length);
}; 

#endif
