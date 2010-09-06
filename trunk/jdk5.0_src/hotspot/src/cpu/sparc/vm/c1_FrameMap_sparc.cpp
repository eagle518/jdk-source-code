#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_FrameMap_sparc.cpp	1.63 04/03/31 18:13:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_FrameMap_sparc.cpp.incl"



//--------------------------------------------------------
//               ArgumentLocation
//--------------------------------------------------------


void ArgumentLocation::set_stack_location (int offset) {
  _n = offset;
  assert(is_stack_arg(), "bad stack argument offset");
}


void ArgumentLocation::set_register_location (int number) {
  _n = number;
  assert(is_register_arg(), "bad register number");
}


bool ArgumentLocation::is_register_arg () const {
  return _n >= 0 && _n < FrameMap::nof_reg_args;
}


bool ArgumentLocation::is_stack_arg () const {
  return _n >= frame::memory_parameter_word_sp_offset;
}


int ArgumentLocation::stack_offset_in_words () const {
  assert(is_stack_arg(), "argument is not on stack");
  return _n;
}


Address ArgumentLocation::incoming_stack_location () const {
  assert(is_stack_arg(), "argument is not on stack");
  return Address(FP, 0, ((_n) * wordSize) + STACK_BIAS);
}


Address ArgumentLocation::outgoing_stack_location () const {
  assert(is_stack_arg(), "argument is not on stack");
  return Address(SP, 0, ((_n) * wordSize) + STACK_BIAS);
}


RInfo ArgumentLocation::incoming_reg_location () const {
  assert(is_register_arg(), "argument is not in a register");
  return as_RInfo(as_iRegister(_n));
}


RInfo ArgumentLocation::outgoing_reg_location () const {
  assert(is_register_arg(), "argument is not in a register");
  return as_RInfo(as_oRegister(_n));
}


#ifndef PRODUCT

void ArgumentLocation::print (bool incoming) const {
  if (is_register_arg()) {
    RInfo r = incoming ? incoming_reg_location() : outgoing_reg_location();
    tty->print(" Register parameter: "); r.print(); tty->cr();
  } else if (is_stack_arg()) {
    tty->print_cr(" Stack parameter: %s + %d", (incoming ? "FP" : "SP"), stack_offset_in_words);
  } else if (is_illegal()) {
    tty->print_cr(" Illegal parameter location");
  } else {
    ShouldNotReachHere();
  }
}

#endif


//--------------------------------------------------------
//               FrameMap
//--------------------------------------------------------

FloatRegister FrameMap::_fpu_regs [FrameMap::nof_fpu_regs];
c1_RegMask    FrameMap::_callee_save_regs;
c1_RegMask    FrameMap::_caller_save_regs;


// some useful constant RInfo's:
RInfo FrameMap::_I0_I1_RInfo;
RInfo FrameMap::_O0_O1_RInfo;
RInfo FrameMap::_O2_O3_RInfo;
RInfo FrameMap::_F0_RInfo;
RInfo FrameMap::_F1_RInfo;
RInfo FrameMap::_F3_RInfo;
RInfo FrameMap::_F0_double_RInfo;
RInfo FrameMap::_F2_double_RInfo;

RInfo FrameMap::_G0_RInfo;
RInfo FrameMap::_G1_RInfo;
RInfo FrameMap::_G2_RInfo;
RInfo FrameMap::_G3_RInfo;
RInfo FrameMap::_G4_RInfo;
RInfo FrameMap::_G5_RInfo;
RInfo FrameMap::_G6_RInfo;
RInfo FrameMap::_G7_RInfo;
RInfo FrameMap::_O0_RInfo;
RInfo FrameMap::_O1_RInfo;
RInfo FrameMap::_O2_RInfo;
RInfo FrameMap::_O3_RInfo;
RInfo FrameMap::_O4_RInfo;
RInfo FrameMap::_O5_RInfo;
RInfo FrameMap::_O6_RInfo;
RInfo FrameMap::_O7_RInfo;
RInfo FrameMap::_L0_RInfo;
RInfo FrameMap::_L1_RInfo;
RInfo FrameMap::_L2_RInfo;
RInfo FrameMap::_L3_RInfo;
RInfo FrameMap::_L4_RInfo;
RInfo FrameMap::_L5_RInfo;
RInfo FrameMap::_L6_RInfo;
RInfo FrameMap::_L7_RInfo;
RInfo FrameMap::_I0_RInfo;
RInfo FrameMap::_I1_RInfo;
RInfo FrameMap::_I2_RInfo;
RInfo FrameMap::_I3_RInfo;
RInfo FrameMap::_I4_RInfo;
RInfo FrameMap::_I5_RInfo;
RInfo FrameMap::_I6_RInfo;
RInfo FrameMap::_I7_RInfo;

RInfo FrameMap::_SP_RInfo;
RInfo FrameMap::_FP_RInfo;

RInfo FrameMap::_Oexception_RInfo;
RInfo FrameMap::_Oissuing_pc_RInfo;

RInfo FrameMap::_GIC_RInfo;

LIR_Opr FrameMap::_caller_save_cpu_regs[] = { 0, };


FloatRegister FrameMap::nr2floatreg (int rnr) {
  assert(_init_done, "tables not initialized");
  debug_only(fpu_range_check(rnr);)
  return _fpu_regs[rnr];
}


int FrameMap::framesize () const {
  return round_to(_size_locals - _size_arguments + _size_monitors + _size_spills +
                  _reserved_argument_area_size + frame::memory_parameter_word_sp_offset, 2);
}

  
// returns true if reg could be smashed by a callee.
bool FrameMap::is_caller_save_register (RInfo reg) {
  if (reg.is_float() || reg.is_double()) { return true; }
  if (reg.is_long()) {
    return is_caller_save_register(reg.as_register_lo()) ||
           is_caller_save_register(reg.as_register_hi());
  }
  return is_caller_save_register(reg.as_register());
}


NEEDS_CLEANUP   // once the new calling convention is enabled, we no
                // longer need to treat I5, I4 and L0 specially
// Because the interpreter destroys caller's I5, I4 and L0,
// we must spill them before doing a Java call as we may land in
// interpreter.
bool FrameMap::is_caller_save_register (Register r) {
  return (r->is_global() && (r != G0)) || r->is_out();
}


RInfoCollection* FrameMap::caller_save_registers () {
  RInfoCollection* regs = new RInfoCollection();
  int i;
  for (i = 0; i < RegAlloc::nof_cpu_regs; i++) {
    if (is_caller_save_register(_cpu_regs[i])) {
      regs->append(as_RInfo(_cpu_regs[i]));
    }
  }
  for (i = 0; i < RegAlloc::nof_fpu_regs; i++) {
    regs->append(as_RInfo(_fpu_regs[i], false));
  }
  return regs;
}


FrameMap::FrameMap(int size_spill) {
  _size_locals = 0;
  _size_monitors = 0;
  _size_arguments = 0;
  _size_spills = size_spill;
  _size_scratch_spills = 0;
  _reserved_argument_area_size = 0;
  debug_only(_is_java_method = false);
  _local_name_to_offset_map = NULL;
  if (!_init_done) init();
}


void FrameMap::init () {
  if (_init_done) return;
  c1_RegMask::init_masks(nof_cpu_regs);

  int i=0;
  // Register usage:
  //  O6: sp
  //  I6: fp
  //  I7: return address
  //  G0: zero
  //  G2: thread
  //  G7: not available
  //  G6: not available
  /*  0 */ _cpu_regs[i++] = L0;
  /*  1 */ _cpu_regs[i++] = L1;
  /*  2 */ _cpu_regs[i++] = L2;
  /*  3 */ _cpu_regs[i++] = L3;
  /*  4 */ _cpu_regs[i++] = L4;
  /*  5 */ _cpu_regs[i++] = L5;
  /*  6 */ _cpu_regs[i++] = L6;
  /*  7 */ _cpu_regs[i++] = L7;
  
  /*  8 */ _cpu_regs[i++] = I5;
  /*  9 */ _cpu_regs[i++] = I4;
  /* 10 */ _cpu_regs[i++] = I3;
  /* 11 */ _cpu_regs[i++] = I2;
  /* 12 */ _cpu_regs[i++] = I1;
  /* 13 */ _cpu_regs[i++] = I0;
  /* 14 */ _cpu_regs[i++] = O0;
  /* 15 */ _cpu_regs[i++] = O1;
  /* 16 */ _cpu_regs[i++] = O2;
  /* 17 */ _cpu_regs[i++] = O3;
  /* 18 */ _cpu_regs[i++] = O4;
  /* 19 */ _cpu_regs[i++] = O5; // <- last register visible in RegAlloc (RegAlloc::nof+cpu_regs)
  /* 20 */ _cpu_regs[i++] = G1;
  /* 21 */ _cpu_regs[i++] = G3;
  /* 22 */ _cpu_regs[i++] = G4;
  /* 23 */ _cpu_regs[i++] = G5;
  /* 24 */ _cpu_regs[i++] = G0;

  // the following registers are not normally available
  /* 25 */ _cpu_regs[i++] = G2;
  /* 26 */ _cpu_regs[i++] = O6;
  /* 27 */ _cpu_regs[i++] = O7;
  /* 28 */ _cpu_regs[i++] = I6;
  /* 29 */ _cpu_regs[i++] = I7;
  /* 30 */ _cpu_regs[i++] = G6;
  /* 31 */ _cpu_regs[i++] = G7;
  assert(i == nof_cpu_regs, "number of CPU registers");

  for (i = 0; i < nof_fpu_regs; i++) {
    _fpu_regs[i] = as_FloatRegister(i);
  }

  _init_done = true;

  _I0_I1_RInfo.set_long_reg(I1, I0);
  _O0_O1_RInfo.set_long_reg(O1, O0);
  _O2_O3_RInfo.set_long_reg(O3, O2);

  _G0_RInfo.set_word_reg(G0);
  _G1_RInfo.set_word_reg(G1);
  _G2_RInfo.set_word_reg(G2);
  _G3_RInfo.set_word_reg(G3);
  _G4_RInfo.set_word_reg(G4);
  _G5_RInfo.set_word_reg(G5);
  _G6_RInfo.set_word_reg(G6);
  _G7_RInfo.set_word_reg(G7);
  _O0_RInfo.set_word_reg(O0);
  _O1_RInfo.set_word_reg(O1);
  _O2_RInfo.set_word_reg(O2);
  _O3_RInfo.set_word_reg(O3);
  _O4_RInfo.set_word_reg(O4);
  _O5_RInfo.set_word_reg(O5);
  _O6_RInfo.set_word_reg(O6);
  _O7_RInfo.set_word_reg(O7);
  _L0_RInfo.set_word_reg(L0);
  _L1_RInfo.set_word_reg(L1);
  _L2_RInfo.set_word_reg(L2);
  _L3_RInfo.set_word_reg(L3);
  _L4_RInfo.set_word_reg(L4);
  _L5_RInfo.set_word_reg(L5);
  _L6_RInfo.set_word_reg(L6);
  _L7_RInfo.set_word_reg(L7);
  _I0_RInfo.set_word_reg(I0);
  _I1_RInfo.set_word_reg(I1);
  _I2_RInfo.set_word_reg(I2);
  _I3_RInfo.set_word_reg(I3);
  _I4_RInfo.set_word_reg(I4);
  _I5_RInfo.set_word_reg(I5);
  _I6_RInfo.set_word_reg(I6);
  _I7_RInfo.set_word_reg(I7);

  _FP_RInfo.set_word_reg(FP);
  _SP_RInfo.set_word_reg(SP);

  _F0_RInfo.set_float_reg(F0);
  _F1_RInfo.set_float_reg(F1);
  _F3_RInfo.set_float_reg(F3);
  _F0_double_RInfo.set_double_reg(F0);
  _F2_double_RInfo.set_double_reg(F2);

  _Oexception_RInfo.set_word_reg(Oexception);
  _Oissuing_pc_RInfo.set_word_reg(Oissuing_pc);
  _GIC_RInfo.set_word_reg(G5_inline_cache_reg);

  // Add only registers that are visible to Register allocator
  for (i = 0; i < RegAlloc::nof_cpu_regs; i++) {
    if (is_caller_save_register(cpu_rnr2reg(i))) {
      _caller_save_regs.add_reg(i);
    } else {
      _callee_save_regs.add_reg(i);
    }      
  }

  _caller_save_cpu_regs[0] = LIR_OprFact::rinfo(FrameMap::_O0_RInfo);
  _caller_save_cpu_regs[1] = LIR_OprFact::rinfo(FrameMap::_O1_RInfo);
  _caller_save_cpu_regs[2] = LIR_OprFact::rinfo(FrameMap::_O2_RInfo);
  _caller_save_cpu_regs[3] = LIR_OprFact::rinfo(FrameMap::_O3_RInfo);
  _caller_save_cpu_regs[4] = LIR_OprFact::rinfo(FrameMap::_O4_RInfo);
  _caller_save_cpu_regs[5] = LIR_OprFact::rinfo(FrameMap::_O5_RInfo);
}

// ------------ argument locations --------------
//
// Maps argument locals to their locations on entry to a C1 method.
//
// The first 6 non-floating-point locals are passed in registers I0 through I5.
// Floating point args and those not-floats that those non-floats that don't make it into
// an I register are passed on the stack.
//
// There is stack space reserved for all arguments wether they are actually passed on
// registers or not. This stack sparc follows the same layout and conventions as the
// interpreter.
//
// For each local arg there is an int describing its location. If it's less than six it
// refers to an I register, otherwise it must be greater than
// frame::memory_parameter_word_sp_offset (23) and it refers to the arg's offset from fp
// in words.
//
// reg_args points to an array of six ints. reg_args[n] gives the local arg index passed in
// the nth I register or -1 if no argument is passed in that register.
//
// Note that double word arguments are referred as two consecutive locals.

CallingConvention* FrameMap::calling_convention(bool is_static, const BasicTypeArray& signature, intArray* reg_args) {
  CallingConvention* arg_location = new CallingConvention(signature.length());
  FrameMap fm(0);
  fm.set_size_arguments(signature.length());
  fm.set_size_locals   (signature.length());

  if (reg_args != NULL) {
    for (int reg = 0; reg < nof_reg_args; reg++) {
      reg_args->at_put(reg, -1);
    }
  }

  int idx = 0;
  while (idx < signature.length()) {
    BasicType t = signature.at(idx);
    int size = type2size[t];                       // either one word or two words
    assert(size == 1 || (size == 2 && t == signature.at(idx+1)), "sanity check");
    switch (t) {
    case T_BOOLEAN:
    case T_CHAR:
    case T_BYTE:
    case T_SHORT:
    case T_INT:
    case T_OBJECT:
    case T_ARRAY:
      {
        if (idx < nof_reg_args) {
          if (reg_args != NULL) reg_args->at_put(idx, idx);
          arg_location->map(idx, ArgumentLocation::new_reg_arg(idx));
        } else {
          arg_location->map(idx, ArgumentLocation::new_stack_arg(fm.fp_offset_for_name(name_for_argument(idx), false, false)));
        }
        break;
      }
    case T_LONG:
      {
        // both halfs on stack
        arg_location->map(idx,   ArgumentLocation::new_stack_arg(fm.fp_offset_for_name(name_for_argument(idx), true, true )));
        arg_location->map(idx+1, ArgumentLocation::new_stack_arg(fm.fp_offset_for_name(name_for_argument(idx), true, false)));
      break;
      }
    default:
      {
        if (size == 1) {
          arg_location->map(idx,   ArgumentLocation::new_stack_arg(fm.fp_offset_for_name(name_for_argument(idx), false, false)));
        } else {
          arg_location->map(idx,   ArgumentLocation::new_stack_arg(fm.fp_offset_for_name(name_for_argument(idx), true, true )));
          arg_location->map(idx+1, ArgumentLocation::new_stack_arg(fm.fp_offset_for_name(name_for_argument(idx), true, false)));
        }
        break;
      }
    }
    idx += size;
  }
  return arg_location;
}



// ----------------mapping-----------------------
// all mapping is based on FP addressing
bool FrameMap::are_adjacent_indeces(int jix1, int jix2) const {
  if (jix1 + 1 == jix2)
    return true;
  ShouldNotReachHere();
  return false;
}


///////////////////////////////////////////
// Internal accessors (FrameMap-private) //
///////////////////////////////////////////


WordSize FrameMap::fp_offset_for_slot(int slot) const {
  int res = size_arguments() - slot - 1;
  if (slot < size_arguments()) {
    res += frame::memory_parameter_word_sp_offset;
  }
  return in_WordSize(res);
}


int FrameMap::local_to_slot(int local_name, bool is_two_word) const {
#ifdef ASSERT
  if (_is_java_method) {
    assert(_local_name_to_offset_map != NULL, "Must set name-to-offset map for Java methods");
  }
#endif // ASSERT

  if (_local_name_to_offset_map == NULL) {
    // Compiled library intrinsic or compiled native method
    return local_name;
  }

  int word_offset = in_words(_local_name_to_offset_map->at(local_name));
#ifdef ASSERT
  // We should never attempt to fetch the offset for the hi word of a
  // doubleword local. This is handled in fp_offset_for_name.
  assert(word_offset != -1, "Should never fetch offset for hi word of doubleword local");
  if (is_two_word) {
    int hi_word_offset = in_words(_local_name_to_offset_map->at(1 + local_name));
    // Hi word should never have an offset allocated
    assert(hi_word_offset == -1, "Hi word for doubleword local should never have offset");
  }
#endif // ASSERT
  return word_offset;
}


WordSize FrameMap::fp_offset_for_monitor_lock(int monitor_index) const {
  assert(sizeof(BasicObjectLock)/wordSize == 2, "not tested for this BasicObjectLock");
  check_monitor_index(monitor_index);
  int index = monitor_index * (sizeof(BasicObjectLock)/wordSize) + 1 + size_locals();
  assert(fp_offset_for_slot(index) + in_WordSize(1) == fp_offset_for_monitor_object(monitor_index), "BasicObjectLocks aren't laid out correctly");
  return fp_offset_for_slot(index);
}


WordSize FrameMap::fp_offset_for_monitor_object(int monitor_index) const {
  assert(sizeof(BasicObjectLock)/wordSize == 2, "not tested for this BasicObjectLock");
  check_monitor_index(monitor_index);
  int index = monitor_index * (sizeof(BasicObjectLock)/wordSize) + size_locals();
  return fp_offset_for_slot(index);
}


// This only needs to be different than make_new_address() below because of
// the broken 64-bit data model
Address FrameMap::make_new_address_for_name(WordSize fp_offset, bool for_hi_word) const {
#ifdef LP64
  // In 64 bit Mode Longs are packed in the lower addressed stack slot
  // This is the higher indexed stack offset or top of stack as the stack
  // grows down. The other stack slot is unused.
  // This code should work for 32 bit mode as well but I'll ifdef it
  // for now.
  return Address(FP, 0, (in_words(fp_offset) * wordSize) + STACK_BIAS + (for_hi_word ? 0 : (longSize / 2)));
#else
  return make_new_address(fp_offset);
#endif
}


Address FrameMap::make_new_address(WordSize fp_offset) const {
  return Address(FP, 0, (in_words(fp_offset) * wordSize) + STACK_BIAS);
}


bool FrameMap::location_for_fp_offset(WordSize word_offset_from_fp,
                                      Location::Type loc_type,
                                      Location* loc) const {
  WordSize word_offset_from_sp = fp2sp_offset(word_offset_from_fp);
  int byte_offset_from_sp = in_words(word_offset_from_sp) * BytesPerWord;
  assert(byte_offset_from_sp >= 0, "incorrect offset");
  if (!Location::legal_offset_in_bytes(byte_offset_from_sp)) {
    return false;
  }
  Location tmp_loc = Location::new_stk_loc(loc_type, byte_offset_from_sp);
  *loc = tmp_loc;
  return true;
}


WordSize FrameMap::fp2sp_offset(WordSize fp_offset) const {
  return fp_offset + in_WordSize(framesize());
}


//////////////////////
// Public accessors //
//////////////////////


WordSize FrameMap::fp_offset_for_name(int name, bool is_two_word, bool for_hi_word) const {
  int index;
  if (name < num_local_names()) {
    // Local
    index = local_to_slot(name, is_two_word);
    check_local_index(index);
  } else {
    // Spill
    int spill_ix = name - num_local_names();
    check_spill_index(spill_ix);
    index = size_locals() + size_monitors() + spill_ix;
  }
  
  if (is_two_word && for_hi_word) index++;
  return fp_offset_for_slot(index);
}


int FrameMap::spill_name(int spill_index) const {
  return num_local_names() + spill_index;
}


int FrameMap::name_for_argument(int arg_no) {
  // We know that in the IR we allocate local names starting at 0 and
  // increasing
  return arg_no;
}


Address FrameMap::address_for_name(int name, bool is_two_word, bool for_hi_word) const {
  return make_new_address_for_name(fp_offset_for_name(name, is_two_word, for_hi_word), for_hi_word);
}


Address FrameMap::address_for_monitor_lock_index(int monitor_index) const {
  return make_new_address(fp_offset_for_monitor_lock(monitor_index));
}


Address FrameMap::address_for_monitor_object_index(int monitor_index) const {
  return make_new_address(fp_offset_for_monitor_object(monitor_index));
}


bool FrameMap::location_for_monitor_lock_index(int monitor_index, Location* loc) const {
  return location_for_fp_offset(fp_offset_for_monitor_lock(monitor_index),
                                Location::normal,
                                loc);
}


bool FrameMap::location_for_monitor_object_index(int monitor_index, Location* loc) const {
  return location_for_fp_offset(fp_offset_for_monitor_object(monitor_index),
                                Location::oop,
                                loc);
}


bool FrameMap::location_for_name(int name, Location::Type loc_type, Location* loc,
                                 bool is_two_word, bool for_hi_word) const {
  return location_for_fp_offset(fp_offset_for_name(name, is_two_word, for_hi_word),
                                loc_type,
                                loc);
}


bool FrameMap::location_for_local_offset(int local_offset, Location::Type loc_type, Location* loc) const {
  return location_for_fp_offset(fp_offset_for_slot(local_offset),
                                loc_type,
                                loc);
}


// For OopMaps, map a local variable or spill index to an OptoReg name.
// This is the offset from sp() in the frame of the slot for the index,
// skewed by SharedInfo::stack0 to indicate a stack location (vs.a register.)
//
//         C ABI size +
//         framesize +     framesize + 
//         stack0          stack0         stack0          0 <- OptoReg::Name
//            |              |              | <registers> |
//  ..........|..............|..............|.............| 
//    0 1 2 3 | <C ABI area> | 4 5 6 ...... |               <- local indices
//    ^                        ^          sp()
//    |                        |
//  arguments            non-argument locals

OptoReg::Name FrameMap::single_word_regname(int local_name) const {
  return SharedInfo::stack2reg(in_words(fp2sp_offset(fp_offset_for_name(local_name, false, false))));
}


OptoReg::Name FrameMap::monitor_object_regname(int monitor_index) const {
  return SharedInfo::stack2reg(in_words(fp2sp_offset(fp_offset_for_monitor_object(monitor_index))));
}


OptoReg::Name FrameMap::fpu_regname (int n) {
  return OptoReg::Name(n + nof_cpu_regs);
}


int FrameMap::oop_map_arg_count() {
  // include the C ABI area in the caller's frame between the arguments' locations and this frame
  return size_arguments() + frame::memory_parameter_word_sp_offset;
}


bool FrameMap::is_byte_rinfo(RInfo reg) {
  return true;
}
