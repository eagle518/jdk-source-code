/*
 * Copyright (c) 1997, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008, 2010 Red Hat, Inc.
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

 protected:
  // Size of interpreter code
  const static int InterpreterCodeSize = 6 * K;

 public:
  // Method entries
  static void normal_entry(methodOop method, intptr_t UNUSED, TRAPS);
  static void native_entry(methodOop method, intptr_t UNUSED, TRAPS);
  static void accessor_entry(methodOop method, intptr_t UNUSED, TRAPS);
  static void empty_entry(methodOop method, intptr_t UNUSED, TRAPS);

 public:
  // Main loop of normal_entry
  static void main_loop(int recurse, TRAPS);

 private:
  // Stack overflow checks
  static bool stack_overflow_imminent(JavaThread *thread);

 private:
  // Fast result type determination
  static BasicType result_type_of(methodOop method);
