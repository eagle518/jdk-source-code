/*
 * Copyright (c) 1997, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007 Red Hat, Inc.
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

  // Generation of Interpreter
  //
  friend class AbstractInterpreterGenerator;

 private:
  address generate_normal_entry(bool synchronized);
  address generate_native_entry(bool synchronized);
  address generate_abstract_entry();
  address generate_math_entry(AbstractInterpreter::MethodKind kind);
  address generate_empty_entry();
  address generate_accessor_entry();
  address generate_method_handle_entry();
