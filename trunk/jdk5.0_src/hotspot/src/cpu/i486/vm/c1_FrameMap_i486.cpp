#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_FrameMap_i486.cpp	1.74 04/03/31 18:13:24 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_FrameMap_i486.cpp.incl"



// some useful constant RInfo's:
RInfo FrameMap::_esiRInfo;
RInfo FrameMap::_ediRInfo;
RInfo FrameMap::_ebxRInfo;
RInfo FrameMap::_eaxRInfo;
RInfo FrameMap::_edxRInfo;
RInfo FrameMap::_ecxRInfo;
RInfo FrameMap::_espRInfo;
RInfo FrameMap::_ebpRInfo;
RInfo FrameMap::_eax_edxRInfo;
RInfo FrameMap::_f0RInfo;
RInfo FrameMap::_d0RInfo;

c1_RegMask FrameMap::_byte_reg_mask;

LIR_Opr FrameMap::_caller_save_cpu_regs[] = { 0, };


//--------------------------------------------------------
//               FrameMap::FpuStackSim
//--------------------------------------------------------

// This class maps the FPU registers to their stack locations; it computes
// the offsets between individual registers and simulates the FPU stack.
//
// Each stack entry can contain several registers. An fpu_dup operation
// does not move the registers on stack, but marks that a certain stack 
// location represents two different registers

FrameMap::FpuStackSim::FpuStackSim() {
  for (int i = 0; i < nof_fpu_regs; i++) _regs.append(new intStack());
  _stack_size = 0;
}


void FrameMap::FpuStackSim::dup(int from_rnr, int to_rnr) {
  if (TraceFPUStack) { tty->print("FPU-dup %d -> %d", from_rnr, to_rnr); print(); tty->cr(); }
  for (int i = tos_index(); i >= 0; i--) {
    if (_regs.at(i)->contains(from_rnr)) {
      _regs.at(i)->append(to_rnr);
    }
  }
}


bool FrameMap::FpuStackSim::pop(int rnr) {
  if (TraceFPUStack) { tty->print("FPU-pop %d", rnr); print(); tty->cr(); }
  intStack* top_regs = regs_at_tos();
  assert(top_regs->contains(rnr), "rnr is not on TOS");
  top_regs->remove(rnr);
  if (top_regs->length() == 0) {
    dec_stack_size();
    return true;
  } else {
    return false;
  }
}


void FrameMap::FpuStackSim::push(int rnr) {
  if (TraceFPUStack) { tty->print("FPU-push %d", rnr); print(); tty->cr(); }
  assert(_regs.at(stack_size())->length() == 0, "should be empty");
  _regs.at(stack_size())->push(rnr);
  inc_stack_size();
}

void FrameMap::FpuStackSim::swap() {
  if (TraceFPUStack) { tty->print("FPU-swap"); print(); tty->cr(); }
  exchange_with_tos(1);
}


int FrameMap::FpuStackSim::offset_from_tos(int rnr) const {
  for (int i = tos_index(); i >= 0; i--) {
    if (_regs.at(i)->contains(rnr)) {
      return tos_index() - i;
    }
  }
  ShouldNotReachHere(); // register not found
}


void FrameMap::FpuStackSim::exchange_with_tos(int offset) {
  intStack* temp_0 = _regs.at(tos_index());
  intStack* temp_offset = _regs.at(tos_index() - offset);
  _regs.at_put(tos_index(), temp_offset);
  _regs.at_put(tos_index() - offset, temp_0);
}

// returns the offset from TOS that is needed to move rnr to TOS
// position and moves the register to TOS
int FrameMap::FpuStackSim::move_on_tos(int rnr) {
  int offset = offset_from_tos(rnr);
  if (TraceFPUStack) { tty->print("FPU-move-on-tos %d (offs %d)", rnr, offset); print(); tty->cr(); }
  exchange_with_tos(offset);
  return offset;
}


bool FrameMap::FpuStackSim::is_empty() {
#ifdef ASSERT
  if (stack_size() == 0) {
    for (int i = 0; i < nof_fpu_regs; i++) {
      assert(_regs.at(i)->length() == 0, "must be empty");
    }
  }
#endif
  return stack_size() == 0;
}


void FrameMap::FpuStackSim::clear() {
  if (TraceFPUStack) { tty->print("FPU-clear"); print(); tty->cr(); }
  for (int i = tos_index(); i >= 0; i--) {
    _regs.at(i)->clear();
  }
  _stack_size = 0;
}


#ifndef PRODUCT
void FrameMap::FpuStackSim::print() {
  tty->print(" N=%d[", stack_size());\
  for (int i = 0; i < stack_size(); i++) {
    tty->print("(");
    for (int m = 0; m < _regs.at(i)->length(); m++) {
      if (m!=0) tty->print(", ");
      tty->print("%d", _regs.at(i)->at(m));
    }
    tty->print(")");
  };
  tty->print(" ]");
}
#endif

//--------------------------------------------------------
//               FrameMap
//--------------------------------------------------------


FrameMap::FrameMap(int size_spill) {
  _size_locals = -1;
  _size_monitors = -1;
  _size_arguments = -1;
  _size_spills = size_spill;
  _size_scratch_spills = 0;
  _reserved_argument_area_size = 0;
  debug_only(_is_java_method = false);
  _local_name_to_offset_map = NULL;
  if (!_init_done) init();
}


void FrameMap::init() {
  if (_init_done) return;

  assert(nof_cpu_regs == 8, "wrong number of CPU registers");
  c1_RegMask::init_masks(nof_cpu_regs);

  _cpu_regs[0] = esi;  _esiRInfo.set_word_reg(0);
  _cpu_regs[1] = edi;  _ediRInfo.set_word_reg(1);
  _cpu_regs[2] = ebx;  _ebxRInfo.set_word_reg(2);
  _cpu_regs[3] = eax;  _eaxRInfo.set_word_reg(3);
  _cpu_regs[4] = edx;  _edxRInfo.set_word_reg(4);
  _cpu_regs[5] = ecx;  _ecxRInfo.set_word_reg(5);

  _cpu_regs[6] = esp;  _espRInfo.set_word_reg(6);
  _cpu_regs[7] = ebp;  _ebpRInfo.set_word_reg(7);

  _eax_edxRInfo.set_long_reg(3 /*eax*/, 4 /*edx*/);
  _f0RInfo.set_float_reg(0);
  _d0RInfo.set_double_reg(0);

  _byte_reg_mask.add_reg(_eaxRInfo);
  _byte_reg_mask.add_reg(_ecxRInfo);
  _byte_reg_mask.add_reg(_edxRInfo);
  _byte_reg_mask.add_reg(_ebxRInfo);

  _caller_save_cpu_regs[0] = LIR_OprFact::rinfo(FrameMap::_esiRInfo);
  _caller_save_cpu_regs[1] = LIR_OprFact::rinfo(FrameMap::_ediRInfo);
  _caller_save_cpu_regs[2] = LIR_OprFact::rinfo(FrameMap::_ebxRInfo);
  _caller_save_cpu_regs[3] = LIR_OprFact::rinfo(FrameMap::_eaxRInfo);
  _caller_save_cpu_regs[4] = LIR_OprFact::rinfo(FrameMap::_edxRInfo);
  _caller_save_cpu_regs[5] = LIR_OprFact::rinfo(FrameMap::_ecxRInfo);

  _init_done = true;
}


CallingConvention* FrameMap::calling_convention(bool is_static, const BasicTypeArray& signature, intArray*) {

  CallingConvention* arg_location = new CallingConvention(signature.length());
  FrameMap fm(0);
  fm.set_size_arguments(signature.length());
  fm.set_size_locals   (signature.length());

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
        arg_location->map(idx, ArgumentLocation::new_stack_arg(fm.fp_offset_for_name(name_for_argument(idx), false, false)));
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


int FrameMap::framesize () const {
  // add 2 for return address and link
  int size = size_locals() + size_spills() + size_monitors() + 2 - size_arguments() + _reserved_argument_area_size;
  if (ForceStackAlignment) {
    size = round_to(size, 2);
  }
  return size;
}


bool FrameMap::is_byte_rinfo (RInfo reg) {
  return reg.is_word() && cpu_rnr2reg(reg.reg())->has_byte_register();
}


// ----------------mapping-----------------------
// all mapping is based on ebp addressing, except for simple leaf methods where we access 
// the locals esp based (and no frame is built)


// Frame for simple leaf methods (quick entries)
//
//   +----------+
//   | ret addr |   <- TOS
//   +----------+
//   | args     |   
//   | ......   |

// Frame for standard methods
// 
//   | .........|  <- TOS
//   | locals   |
//   +----------+
//   | old ebp  |  <- EBP
//   +----------+
//   | ret addr |
//   +----------+
//   |  args    |
//   | .........|


///////////////////////////////////////////
// Internal accessors (FrameMap-private) //
///////////////////////////////////////////


// Maps java_index to an ebp offset
WordSize FrameMap::fp_offset_for_slot(int slot) const {
  return in_WordSize(size_arguments() - slot + (slot < size_arguments() ? 1 : -1));
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
  
  if (is_two_word && !for_hi_word) index++;
  return fp_offset_for_slot(index);
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


Address FrameMap::make_new_address(WordSize fp_offset) const {
  return Address(ebp, in_words(fp_offset) * wordSize);
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
  // subtract two for return address and link
  WordSize sp_offset = fp_offset + in_WordSize(framesize() - 2);
  assert(sp_offset > in_WordSize(0), "incorrect offset");
  return sp_offset;
}


//////////////////////
// Public accessors //
//////////////////////


int FrameMap::spill_name(int spill_index) const {
  return num_local_names() + spill_index;
}


int FrameMap::name_for_argument(int arg_no) {
  // We know that in the IR we allocate local names starting at 0 and
  // increasing
  return arg_no;
}


Address FrameMap::address_for_name(int name, bool is_two_word, bool for_hi_word) const {
  return make_new_address(fp_offset_for_name(name, is_two_word, for_hi_word));
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


bool FrameMap::are_adjacent_indeces(int jix1, int jix2) const {
  assert(jix2 == jix1 + 1, "can't compare arbitrary indices");
  return abs(in_words(fp_offset_for_name(jix1, true, false)) -
             in_words(fp_offset_for_name(jix1, true, true))) <= 1;
}


//------------------------------------------------------------------------



// For OopMaps, map a local variable or spill index to an OptoReg name.
// This is the offset from sp() in the frame of the slot for the index,
// skewed by SharedInfo::stack0 to indicate a stack location (vs.a register.)
//
//           framesize +
//           stack0         stack0          0  <- OptoReg::Name
//             |              | <registers> |
//  ...........|..............|.............|  
//      0 1 2 3 x x 4 5 6 ... |                <- local indices
//      ^           ^        sp()                 ( x x indicate link
//      |           |                               and return addr)
//  arguments   non-argument locals

OptoReg::Name FrameMap::single_word_regname(int local_name) const {
  return SharedInfo::stack2reg(in_words(fp2sp_offset(fp_offset_for_name(local_name, false, false))));
}


OptoReg::Name FrameMap::monitor_object_regname(int monitor_index) const {
  return SharedInfo::stack2reg(in_words(fp2sp_offset(fp_offset_for_monitor_object(monitor_index))));
}

OptoReg::Name FrameMap::fpu_stack_regname (int n) {
  // Return the OptoReg name for the fpu stack slot "n" spilled in a stub frame.
  // A spilled fpu stack slot comprises to two single-word OptoReg's.
  return OptoReg::Name(2*n + nof_cpu_regs);
}


OptoReg::Name FrameMap::fpu_regname (int n) {
  // Map virtual fpu register "n" to an OptoReg name for debug info.
  return fpu_stack_regname(fpu_stack()->offset_from_tos(n));
}


int FrameMap::oop_map_arg_count() {
  return size_arguments();
}


Register FrameMap::first_register() { 
  init();
  return _cpu_regs[0]; 
}


Register FrameMap::first_byte_register() {
  init();
  return _cpu_regs[2]; 
}



//--------------------------------------------------------
//               ArgumentLocation
//--------------------------------------------------------


void ArgumentLocation::set_stack_location (int offset) {
  _n = offset;
  assert(is_stack_arg(), "bad stack argument offset");
}


void ArgumentLocation::set_register_location (int number) {
  // we don't do register args on Intel
  ShouldNotReachHere();
}


bool ArgumentLocation::is_register_arg () const {
  return false;
}


bool ArgumentLocation::is_stack_arg () const {
  return true;
}


int ArgumentLocation::stack_offset_in_words () const {
  assert(is_stack_arg(), "argument is not on stack");
  return _n;
}


Address ArgumentLocation::incoming_stack_location () const {
  assert(is_stack_arg(), "argument is not on stack");
  return Address(ebp, (_n) * wordSize);
}


Address ArgumentLocation::outgoing_stack_location () const {
  assert(is_stack_arg(), "argument is not on stack");
  return Address(ebp, (_n) * wordSize);
}


RInfo ArgumentLocation::incoming_reg_location () const {
  ShouldNotReachHere();
  return norinfo;
}


RInfo ArgumentLocation::outgoing_reg_location () const {
  ShouldNotReachHere();
  return norinfo;
}


#ifndef PRODUCT

void ArgumentLocation::print (bool incoming) const {
  if (is_register_arg()) {
    RInfo r = incoming ? incoming_reg_location() : outgoing_reg_location();
    tty->print(" Register parameter: "); r.print(); tty->cr();
  } else if (is_stack_arg()) {
    tty->print_cr(" Stack parameter: %s + %d", (incoming ? "FP" : "SP"), stack_offset_in_words());
  } else if (is_illegal()) {
    tty->print_cr(" Illegal parameter location");
  } else {
    ShouldNotReachHere();
  }
}

#endif


RInfoCollection* FrameMap::caller_save_registers () {
  Unimplemented();
  return NULL;
}

