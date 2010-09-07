/*
 * Copyright (c) 1997, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008, 2009 Red Hat, Inc.
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

// In normal, CPU-specific ports of HotSpot these two classes are used
// for generating assembly language.  We don't do any of this in zero,
// of course, but we do sneak entry points around in CodeBuffers so we
// generate those here.

class Assembler : public AbstractAssembler {
 public:
  Assembler(CodeBuffer* code) : AbstractAssembler(code) {}

 public:
  void pd_patch_instruction(address branch, address target);
#ifndef PRODUCT
  static void pd_print_patched_instruction(address branch);
#endif // PRODUCT
};

class MacroAssembler : public Assembler {
 public:
  MacroAssembler(CodeBuffer* code) : Assembler(code) {}

 public:
  void align(int modulus);
  void bang_stack_with_offset(int offset);
  bool needs_explicit_null_check(intptr_t offset);
  RegisterOrConstant delayed_value_impl(intptr_t* delayed_value_addr,
                                        Register tmp, int offset);
 public:
  void advance(int bytes);
  void store_oop(jobject obj);
};

#ifdef ASSERT
inline bool AbstractAssembler::pd_check_instruction_mark() {
  ShouldNotCallThis();
}
#endif

address ShouldNotCallThisStub();
address ShouldNotCallThisEntry();
