#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)nativeInst_ia64.hpp	1.14 04/03/17 14:08:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


//-----------------------------------------------------------------------------
// NativeInstruction
//
// The base class for different kinds of native instruction abstractions.
// Provides the primitive operations to manipulate code relative to this.
// - NativeInstruction
// - - NativeCall
// - - NativeMovConstReg
// - - NativeJump
// - - NativeIllegalInstruction
//
//-----------------------------------------------------------------------------
class NativeInstruction VALUE_OBJ_CLASS_SPEC {
  friend class Relocation;

 public:

  address  addr_at(int offset) const   { return address(this) + offset; }

  void  verify() const;

  bool is_call() const;

  bool is_illegal() const;

  bool is_movl() const;

  bool is_nop() const;

  // The signal handler uses the safepoint address as validation.
  // This is only used in debug builds.
  bool is_safepoint_poll() { return true; }

  inline friend NativeInstruction* nativeInstruction_at(address address);
};

inline NativeInstruction* nativeInstruction_at(address address) {
  NativeInstruction* inst = (NativeInstruction*)address;
  #ifdef ASSERT
    inst->verify();
  #endif
  return inst;
}


//-----------------------------------------------------------------------------
// NativeCall
// The NativeCall is an abstraction for accessing/manipulating native 
//  call instructions.  // (used to manipulate inline caches, primitive & 
//  dll calls, etc.)
//
// This is the first pass at implementing the NativeCall interface
// for Itanium processors.  Eventually we will support both
// short and long sequences of call instructions.  Currently we only
// support the long form since it works in all situations.
//
//  call imm21;;     [FUTURE USE]
//  
//   or
//  
//  movl dst, imm64;;		Bundle 0
//  movi br,dst			Bundle 1
//  call [br];;			
//-----------------------------------------------------------------------------
class NativeCall: public NativeInstruction {

 public:

  enum ia64_specific_constants {
    instruction_size 		       =  (2 * sizeof (IPF_Bundle)),
    return_address_offset              =  instruction_size
  };

  address return_address() const       { return addr_at(return_address_offset); }

  address instruction_address() const  { return addr_at(0); }

  void  verify() const;

  address destination() const { 
    IPF_Bundle *bundle = (IPF_Bundle *)addr_at(0);
    return (address)X2::inv_imm(bundle->get_slot2(), bundle->get_slot1()); 
  }

  void  set_destination(address dest)  { 
    verify();
    uint41_t new_X;
    uint41_t new_L;
    IPF_Bundle *bundle = (IPF_Bundle *)addr_at(0);
    X2::set_imm((uint64_t)dest, bundle->get_slot2(), new_X, new_L);
    bundle->set_slot1( new_L );
    bundle->set_slot2( new_X );
  }

  void  set_destination_mt_safe(address dest);

  // Creation
  inline friend NativeCall* nativeCall_at(address instr);

  inline friend NativeCall* nativeCall_before(address return_address);

  // Tester
  static bool is_call_before(address return_address) {
    return nativeInstruction_at(return_address - NativeCall::return_address_offset)->is_call();
  }
};

inline NativeCall* nativeCall_at(address instr) {
  NativeCall* call = (NativeCall*)instr;
  #ifdef ASSERT
    call->verify();
  #endif
  return call;
}

inline NativeCall* nativeCall_before(address return_address) {
  NativeCall* call = (NativeCall*)(return_address - NativeCall::return_address_offset);
  #ifdef ASSERT
    call->verify();
  #endif
  return call;
}


//-----------------------------------------------------------------------------
// NativeMovConstReg
// An interface for accessing/manipulating native set_oop reg=imm64 instructions.
// (used to manipulate inlined data references, etc.)
//     set_oop reg=imm64
// 
// On Itanium we use 
//
//     movl reg=imm64
//
//-----------------------------------------------------------------------------
class NativeMovConstReg: public NativeInstruction {

 public:

  enum ia64_specific_constants {
    instruction_size       =  sizeof (IPF_Bundle)
  };

  address instruction_address() const  { return addr_at(0); }

  address next_instruction_address() const  { return addr_at(instruction_size); }

  intptr_t data() const { 
    IPF_Bundle *bundle = (IPF_Bundle *)addr_at(0);
    return X2::inv_imm(bundle->get_slot2(), bundle->get_slot1()); 
  }

  void set_data(intptr_t x);

  void  verify() const;

  // Creation
  inline friend NativeMovConstReg* nativeMovConstReg_at(address address);
};

inline NativeMovConstReg* nativeMovConstReg_at(address address) {
  NativeMovConstReg* test = (NativeMovConstReg*)address;
  #ifdef ASSERT
    test->verify();
  #endif
  return test;
}


//-----------------------------------------------------------------------------
// NativeJump
// An interface for accessing/manipulating native jumps
//      jump_to addr
//
//      movl dst, addr;;            Bundle 0
//      movi br,dst                 Bundle 1
//      br [br];;
//
// [TODO]  Check to see if long branch would be faster 
//
//-----------------------------------------------------------------------------
class NativeJump: public NativeInstruction {

 public:
  enum ia64_specific_constants {
    instruction_size                   =  (2 * sizeof (IPF_Bundle)),
  };

  address jump_destination() const { 
    IPF_Bundle *bundle = (IPF_Bundle *)addr_at(0);
    return (address)X2::inv_imm(bundle->get_slot2(), bundle->get_slot1());
  }

  void set_jump_destination(address dest) { 
    uint41_t new_X;
    uint41_t new_L;
    IPF_Bundle *bundle = (IPF_Bundle *)addr_at(0);
    X2::set_imm((uint64_t)dest, bundle->get_slot2(), new_X, new_L);
    bundle->set_slot1( new_L );
    bundle->set_slot2( new_X );
  }

  void  verify() const;

  // Creation
  inline friend NativeJump* nativeJump_at(address address);

  static void check_verified_entry_alignment(address entry, address verified_entry) {
    // nothing to do for ia64.
  }
  static void patch_verified_entry(address entry, address verified_entry, address dest);
};

inline NativeJump* nativeJump_at(address address) {
  NativeJump* jump = (NativeJump*)address;
  #ifdef ASSERT
    jump->verify();
  #endif
  return jump;
}


//-----------------------------------------------------------------------------
// NativeIllegalInstruction
//
// Used to insert an illegal instruction in the code stream.
// This is used in the safepoint handler to patch calls and
// when handling the safepoint type of relocation entry.
//
//-----------------------------------------------------------------------------
class NativeIllegalInstruction: public NativeInstruction {
 public:
  enum ia64_specific_constants {
    instruction_size	        =    0
  };

  // Insert illegal opcode as specific address
  static void insert(address code_pos) { assert(0, "Time to implement insert()"); }

};
