#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_LIRAssembler_sparc.hpp	1.24 04/04/07 14:44:59 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

 private:

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // 
  // Sparc load/store emission
  //
  // The sparc ld/st instructions cannot accomodate displacements > 13 bits long.
  // The following "pseudo" sparc instructions (load/store) make it easier to use the indexed addressing mode
  // by allowing 32 bit displacements:
  //
  //    When disp <= 13 bits long, a single load or store instruction is emitted with (disp + [d]).
  //    When disp >  13 bits long, code is emitted to set the displacement into the O7 register, 
  //       and then a load or store is emitted with ([O7] + [d]).
  //

  // some load/store variants return the code_offset for proper positioning of debug info for null checks

  // load/store with 32 bit displacement
  int load(Register s, int disp, Register d, BasicType ld_type, CodeEmitInfo* info = NULL);
  void store(Register value, Register base, int offset, BasicType type, CodeEmitInfo *info = NULL);

  // loadf/storef with 32 bit displacement
  void load(Register s, int disp, FloatRegister d, BasicType ld_type, CodeEmitInfo* info = NULL);
  void store(FloatRegister d, Register s1, int disp, BasicType st_type, CodeEmitInfo* info = NULL);

  // convienence methods for calling load/store with an Address
  void load(const Address& a, Register d, BasicType ld_type, CodeEmitInfo* info = NULL, int offset = 0);
  void store(Register d, const Address& a, BasicType st_type, CodeEmitInfo* info = NULL, int offset = 0);
  void load(const Address& a, FloatRegister d, BasicType ld_type, CodeEmitInfo* info = NULL, int offset = 0);
  void store(FloatRegister d, const Address& a, BasicType st_type, CodeEmitInfo* info = NULL, int offset = 0);

  int store(RInfo from_reg, Register base, int offset, BasicType type);
  int store(RInfo from_reg, Register base, Register disp, BasicType type);
  int store(RInfo from_reg, Address addr, BasicType type) { return store(from_reg, addr.base(), addr.disp(), type); }

  int load(Register base, int offset, RInfo to_reg, BasicType type);
  int load(Register base, Register disp, RInfo to_reg, BasicType type);
  int load(Address addr, RInfo to_reg, BasicType type) { return load(addr.base(), addr.disp(), to_reg, type); }

  BasicType BasicType_from_RInfo(RInfo r);

  void monitorenter(Register obj_reg, Register lock_reg, Register hdr, Register scratch, int monitor_no, CodeEmitInfo* info);
  void monitorexit(Register obj_reg, Register lock_reg, Register hdr, int monitor_no);

  void setup_locals(CallingConvention* args, BasicTypeArray* sig_types);
  void reg2local(int local_index, RInfo reg, BasicType t);
  void local2reg(RInfo reg, int local_index, BasicType t);

  void interpreter_to_compiler_calling_convention(ciMethod* method);

  int shift_amount(BasicType t);
  
  Address stack_Address(BasicType type, Register r, int disp_bytes, bool for_hi_word = true);
  Address stack_AddressLO(BasicType type, Register r, int disp_bytes) {
    return stack_Address(type,r,disp_bytes,false);
  }
  Address stack_AddressHI(BasicType type, Register r, int disp_bytes) {
    return stack_Address(type,r,disp_bytes,true);
  }

  Address heap_Address(BasicType type, Register r, int disp_bytes, bool for_hi_word);
  Address heap_AddressLO(BasicType type, Register r, int disp_bytes)  { return heap_Address(type, r, disp_bytes, false); }
  Address heap_AddressHI(BasicType type, Register r, int disp_bytes)  { return heap_Address(type, r, disp_bytes, true); }

  Address as_Address_lo(LIR_Address* addr);
  Address as_Address_hi(LIR_Address* addr);

  void pass_oop_to_native(int java_arg_index, Address java_arg_addr, Argument jni_arg);

  void save_native_fp_result(BasicType return_type, Address float_spill_addr);
  void restore_native_fp_result(BasicType return_type, Address float_spill_addr);

 public:
  bool is_empty_fpu_stack() const                { return true; }

  void emit_delay(LIR_OpDelay* op);

  void pack64( Register rs, Register rd );
  void unpack64( Register rd );

