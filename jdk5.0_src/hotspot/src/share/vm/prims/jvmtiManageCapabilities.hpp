#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiManageCapabilities.hpp	1.3 03/12/23 16:43:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _JAVA_JVMTI_MANAGE_CAPABILITIES_H_
#define _JAVA_JVMTI_MANAGE_CAPABILITIES_H_



class JvmtiManageCapabilities : public AllStatic {

private:

  // these four capabilities sets represent all potentially
  // available capabilities.  They are disjoint, covering
  // the four cases: (OnLoad vs OnLoad+live phase) X 
  // (one environment vs any environment).
  static jvmtiCapabilities always_capabilities;
  static jvmtiCapabilities onload_capabilities;
  static jvmtiCapabilities always_solo_capabilities;
  static jvmtiCapabilities onload_solo_capabilities;

  // solo capabilities that have not been grabbed
  static jvmtiCapabilities always_solo_remaining_capabilities;
  static jvmtiCapabilities onload_solo_remaining_capabilities;

  // all capabilities ever acquired
  static jvmtiCapabilities acquired_capabilities;

  // basic intenal operations
  static jvmtiCapabilities *either(const jvmtiCapabilities *a, const jvmtiCapabilities *b, jvmtiCapabilities *result);
  static jvmtiCapabilities *both(const jvmtiCapabilities *a, const jvmtiCapabilities *b, jvmtiCapabilities *result);
  static jvmtiCapabilities *exclude(const jvmtiCapabilities *a, const jvmtiCapabilities *b, jvmtiCapabilities *result);
  static bool has_some(const jvmtiCapabilities *a);
  static void update();

  // init functions
  static jvmtiCapabilities init_always_capabilities();
  static jvmtiCapabilities init_onload_capabilities();
  static jvmtiCapabilities init_always_solo_capabilities();
  static jvmtiCapabilities init_onload_solo_capabilities();

public:
  
  static void initialize();

  // queries and actions
  static void get_potential_capabilities(const jvmtiCapabilities *current, jvmtiCapabilities *result);
  static jvmtiError add_capabilities(const jvmtiCapabilities *current, const jvmtiCapabilities *desired, 
                                     jvmtiCapabilities *result);
  static void relinquish_capabilities(const jvmtiCapabilities *current, const jvmtiCapabilities *unwanted, 
                                      jvmtiCapabilities *result);
  static jvmtiError estimate_cost_of_capabilities(const jvmtiCapabilities *current,
                                                  const jvmtiCapabilities *desired,
                                                  jint* time_impact_ptr, jint* space_impact_ptr);
  static void copy_capabilities(const jvmtiCapabilities *from, jvmtiCapabilities *to);
};

#endif   /* _JAVA_JVMTI_MANAGE_CAPABILITIES_H_ */
