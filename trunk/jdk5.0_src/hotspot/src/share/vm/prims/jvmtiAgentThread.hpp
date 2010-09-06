#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiAgentThread.hpp	1.6 04/05/27 11:19:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// class JvmtiAgentThread
//
// JavaThread used to wrap a thread started by an agent
// using the JVMTI method RunAgentThread.
//
class JvmtiAgentThread : public JavaThread {

  jvmtiStartFunction _start_fn;
  JvmtiEnv* _env;
  JVMDI_StartFunction _jvmdi_start_fn;
  const void *_start_arg;

public:
  JvmtiAgentThread(JvmtiEnv* env, jvmtiStartFunction start_fn, const void *start_arg);
  JvmtiAgentThread(JVMDI_StartFunction jvmdi_start_fn, const void *start_arg);

  bool is_jvmti_agent_thread() const	{ return true; }

  static void start_function_wrapper(JavaThread *thread, TRAPS);
  void call_start_function();
};
