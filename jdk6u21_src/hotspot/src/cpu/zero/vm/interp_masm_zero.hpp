/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
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

// This file specializes the assember with interpreter-specific macros

class InterpreterMacroAssembler : public MacroAssembler {
 public:
  InterpreterMacroAssembler(CodeBuffer* code) : MacroAssembler(code) {}

 public:
  RegisterOrConstant delayed_value_impl(intptr_t* delayed_value_addr,
                                        Register  tmp,
                                        int       offset) {
    ShouldNotCallThis();
  }
};
