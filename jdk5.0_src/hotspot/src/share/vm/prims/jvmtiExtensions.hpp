#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiExtensions.hpp	1.2 03/12/23 16:43:20 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
 
#ifndef _JVMTI_EXTENSIONS_H_
#define _JVMTI_EXTENSIONS_H_


// JvmtiExtensions
//
// Maintains the list of extension functions and events in this JVMTI
// implementation. The list of functions and events can be obtained by 
// the profiler using the JVMTI GetExtensionFunctions and 
// GetExtensionEvents functions. 

class JvmtiExtensions : public AllStatic {
 private:
  static GrowableArray<jvmtiExtensionFunctionInfo*>* _ext_functions;
  static GrowableArray<jvmtiExtensionEventInfo*>* _ext_events;

 public:
  // register extensions function
  static void register_extensions();

  // returns the list of extension functions
  static jvmtiError get_functions(JvmtiEnv* env, jint* extension_count_ptr, 
				  jvmtiExtensionFunctionInfo** extensions);

  // returns the list of extension events
  static jvmtiError get_events(JvmtiEnv* env, jint* extension_count_ptr, 
			       jvmtiExtensionEventInfo** extensions);

  // sets the callback function for an extension event and enables the event
  static jvmtiError set_event_callback(JvmtiEnv* env, jint extension_event_index, 
				       jvmtiExtensionEvent callback);
};

#endif  /* _JVMTI_EXTENSIONS_H_ */
