/*
 * Copyright (c) 2003, 2006, Oracle and/or its affiliates. All rights reserved.
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

// We have interfaces for the following instructions:
// - NativeInstruction
// - - NativeCall
// - - NativeMovConstReg
// - - NativeMovConstRegPatching
// - - NativeJump
// - - NativeIllegalOpCode
// - - NativeReturn
// - - NativeReturnX (return with argument)
// - - NativePushConst
// - - NativeTstRegMem

// The base class for different kinds of native instruction abstractions.
// Provides the primitive operations to manipulate code relative to this.

class NativeInstruction VALUE_OBJ_CLASS_SPEC {
 public:
  bool is_jump() {
    ShouldNotCallThis();
  }

  bool is_safepoint_poll() {
    ShouldNotCallThis();
  }
};

inline NativeInstruction* nativeInstruction_at(address address) {
  ShouldNotCallThis();
}

class NativeCall : public NativeInstruction {
 public:
  enum zero_specific_constants {
    instruction_size = 0 // not used within the interpreter
  };

  address instruction_address() const {
    ShouldNotCallThis();
  }

  address next_instruction_address() const {
    ShouldNotCallThis();
  }

  address return_address() const {
    ShouldNotCallThis();
  }

  address destination() const {
    ShouldNotCallThis();
  }

  void set_destination_mt_safe(address dest) {
    ShouldNotCallThis();
  }

  void verify_alignment() {
    ShouldNotCallThis();
  }

  void verify() {
    ShouldNotCallThis();
  }

  static bool is_call_before(address return_address) {
    ShouldNotCallThis();
  }
};

inline NativeCall* nativeCall_before(address return_address) {
  ShouldNotCallThis();
}

inline NativeCall* nativeCall_at(address address) {
  ShouldNotCallThis();
}

class NativeMovConstReg : public NativeInstruction {
 public:
  address next_instruction_address() const {
    ShouldNotCallThis();
  }

  intptr_t data() const {
    ShouldNotCallThis();
  }

  void set_data(intptr_t x) {
    ShouldNotCallThis();
  }
};

inline NativeMovConstReg* nativeMovConstReg_at(address address) {
  ShouldNotCallThis();
}

class NativeMovRegMem : public NativeInstruction {
 public:
  int offset() const {
    ShouldNotCallThis();
  }

  void set_offset(intptr_t x) {
    ShouldNotCallThis();
  }

  void add_offset_in_bytes(int add_offset) {
    ShouldNotCallThis();
  }
};

inline NativeMovRegMem* nativeMovRegMem_at(address address) {
  ShouldNotCallThis();
}

class NativeJump : public NativeInstruction {
 public:
  enum zero_specific_constants {
    instruction_size = 0 // not used within the interpreter
  };

  address jump_destination() const {
    ShouldNotCallThis();
  }

  void set_jump_destination(address dest) {
    ShouldNotCallThis();
  }

  static void check_verified_entry_alignment(address entry,
                                             address verified_entry) {
  }

  static void patch_verified_entry(address entry,
                                   address verified_entry,
                                   address dest);
};

inline NativeJump* nativeJump_at(address address) {
  ShouldNotCallThis();
}

class NativeGeneralJump : public NativeInstruction {
 public:
  address jump_destination() const {
    ShouldNotCallThis();
  }

  static void insert_unconditional(address code_pos, address entry) {
    ShouldNotCallThis();
  }

  static void replace_mt_safe(address instr_addr, address code_buffer) {
    ShouldNotCallThis();
  }
};

inline NativeGeneralJump* nativeGeneralJump_at(address address) {
  ShouldNotCallThis();
}
