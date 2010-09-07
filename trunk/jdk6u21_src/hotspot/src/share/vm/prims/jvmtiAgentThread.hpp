/*
 * Copyright (c) 2003, 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
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
  const void *_start_arg;

public:
  JvmtiAgentThread(JvmtiEnv* env, jvmtiStartFunction start_fn, const void *start_arg);

  bool is_jvmti_agent_thread() const    { return true; }

  static void start_function_wrapper(JavaThread *thread, TRAPS);
  void call_start_function();
};
