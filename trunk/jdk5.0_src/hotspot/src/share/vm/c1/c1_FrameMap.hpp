#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_FrameMap.hpp	1.47 04/03/31 18:13:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class ciMethod;
class CallingConvention;


//--------------------------------------------------------
//               FrameMap
//--------------------------------------------------------

//  This class is responsible of mapping items (locals, monitors, spill
//  slots and registers to their frame location
//
//  The locals are specified by an index, same as in bytecodes (index is low endian).
//  The local index is 0.._size_locals-1.
//  The monitors are specified by a consecutive index, although each monitor entry
//  occupies two words. The monitor_index is 0.._size_monitors/2-1
//  The spill index is similar to local index; it is in range 0..(open)
//
//  The CPU registers are mapped using a fixed table; register with number 0
//  is the most used one.


//   stack grow direction -->                                                      SP
//  +----------+---+--------------+----------+-------+------------------------+-----+
//  |arguments | x | other locals | monitors | spill | reserved argument area | ABI |
//  +----------+---+--------------+----------+-------+------------------------+-----+
//  
//  locals = arguments + other locals
//  x =  ABI area (SPARC) or  return adress and link (i486)
//  ABI  = ABI area (SPARC) or nothing (i486)


class LIR_OprDesc;
typedef LIR_OprDesc* LIR_Opr;


class FrameMap VALUE_OBJ_CLASS_SPEC {
 public:
  enum {
    nof_cpu_regs = pd_nof_cpu_regs_frame_map,
    nof_fpu_regs = pd_nof_fpu_regs_frame_map,
    nof_caller_save_cpu_regs = pd_nof_caller_save_cpu_regs_frame_map
  };

# include "incls/_c1_FrameMap_pd.hpp.incl"  // platform dependent declarations

  friend class RInfo;
  friend class c1_AllocTable;

 public:
  enum {
    stack0 = STACK0
  };

 private:
  static bool         _init_done;
  static Register     _cpu_regs [nof_cpu_regs];

  static LIR_Opr      _caller_save_cpu_regs [nof_caller_save_cpu_regs];

  debug_only(bool     _is_java_method;)
  int                 _size_locals;
  int                 _size_monitors;
  int                 _size_arguments;
  int                 _size_spills;
  int                 _size_scratch_spills;
  int                 _reserved_argument_area_size;

  WordSizeList*       _local_name_to_offset_map;

  void check_local_index   (int local_index)   const { assert(local_index   >= 0 && local_index   < _size_locals, "bad index"); }
  void check_spill_index   (int spill_index)   const { assert(spill_index   >= 0, "bad index"); }
  void check_monitor_index (int monitor_index) const { assert(monitor_index >= 0 &&
                                                              monitor_index < _size_monitors, "bad index"); }

  static Register cpu_rnr2reg (int rnr);
  static int cpu_reg2rnr (Register reg);

 protected:
#ifndef PRODUCT
  static void cpu_range_check (int rnr)          { assert(0 <= rnr && rnr < nof_cpu_regs, "cpu register number is too big"); }
  static void fpu_range_check (int rnr)          { assert(0 <= rnr && rnr < nof_fpu_regs, "fpu register number is too big"); }
#endif

 public:
  FrameMap(int max_spills);

  static BasicTypeList*     signature_type_array_for(const ciMethod* method);
  static CallingConvention* calling_convention (bool is_static, const BasicTypeArray& signature, intArray* reg_args = NULL);
  static CallingConvention* calling_convention (const ciMethod* method, intArray* reg_args = NULL);

  int   framesize () const;
  int   size_locals                 () const     { assert(_size_locals >= 0, "not set"); return _size_locals; }
  int   size_arguments              () const     { assert(_size_arguments >= 0, "not set"); return _size_arguments; }
  int   size_monitors               () const     { assert(_size_monitors >= 0, "not set"); return _size_monitors; }
  int   size_spills                 () const     { assert(_size_spills >= 0, "not set"); return _size_spills; }
  int   reserved_argument_area_size () const     { return _reserved_argument_area_size; }

  // Debugging support
  debug_only(void set_is_java_method());
  // Maps names for local instructions to frame offsets
  void set_local_name_to_offset_map(WordSizeList* list);
  // Indicates how many local names have been allocated in all scopes
  int  num_local_names() const;
  // Indicates how many real frame slots are used by locals in all scopes
  void set_size_locals    (int size)             { assert(size >= 0, "check"); _size_locals    = size; }
  void set_size_arguments (int size)             { assert(size >= 0, "check"); _size_arguments = size; }
  void set_num_monitors   (int num)              { assert(num >= 0, "check"); set_size_monitors(num * sizeof(BasicObjectLock)/wordSize); }
  void set_size_monitors  (int size)             { assert(size >= 0, "check"); _size_monitors  = size; }
  void set_reserved_argument_area_size (int size) { assert(size >= 0, "check"); _reserved_argument_area_size = size; }

  // Fetch names for certain classes of values. Names for locals are
  // computed in IRScope::allocate_locals(). Indices for these various
  // values go from 0..max; the names span all types of values in the
  // FrameMap.
  // NOTE: we always reference doubleword spills using the name of the
  // low word. Whether we are really talking about the high word is
  // handled in address_for_name.
  int spill_name(int spill_index) const;
  // Abstracts away names for local arguments
  static int name_for_argument(int arg_no);

  // mapping
  Address address_for_name       (int name, bool is_two_word = false, bool for_hi_word = false) const;

  // Provide additional error checking in debug mode
  Address address_for_local_name(int name, bool is_two_word = false, bool for_hi_word = false) const {
    assert(name < num_local_names(), "not name of local");
    return address_for_name(name, is_two_word, for_hi_word);
  }
  Address address_for_spill_name(int name, bool is_two_word = false, bool for_hi_word = false) const {
    assert(name >= num_local_names() && name < num_local_names() + size_spills(), "not name of spill");
    return address_for_name(name, is_two_word, for_hi_word);
  }

  // convenience routines
  Address address_for_spill_index(int spill_index, bool is_two_word, bool for_hi_word = false) const {
    assert(spill_index >= 0 && spill_index < (size_spills() - _size_scratch_spills), "out of range");
    return address_for_spill_name(spill_name(spill_index), is_two_word, for_hi_word);
  }
  Address address_for_monitor_lock_index  (int monitor_index) const;
  Address address_for_monitor_object_index(int monitor_index) const;

  Address address_for_scratch_index(int scratch_index, bool is_two_word, bool for_hi_word = false) const {
    assert(scratch_index >= 0 && scratch_index < _size_scratch_spills, "out of range");
    assert(!is_two_word || scratch_index + 1 < _size_scratch_spills, "out of range");
    return address_for_spill_name(spill_name(size_spills() - _size_scratch_spills), is_two_word, for_hi_word);
  }

  // Used by LIR oop map generator
  bool is_spill_pos     (LIR_Opr opr)   const;

  // Creates Location describing desired slot and returns it via pointer
  // to Location object. Returns true if the stack frame offset was legal
  // (as defined by Location::legal_offset_in_bytes()), false otherwise.
  // Do not use the returned location if this returns false.
  bool location_for_monitor_lock_index  (int monitor_index, Location* loc) const;
  bool location_for_monitor_object_index(int monitor_index, Location* loc) const;

  // This takes the name of either a local or a spill slot.
  bool location_for_name                (int name, Location::Type loc_type, Location* loc,
                                         bool is_two_word = false, bool for_hi_word = false) const;

  // This is needed only for debug information generation because we
  // do not currently maintain enough liveness and type information
  // for locals to be able to generate debug info from the local
  // collection. Can go away once the new register allocator is in
  // place.
  bool location_for_local_offset        (int local_offset, Location::Type loc_type, Location* loc) const;

  OptoReg::Name single_word_regname     (int name) const;
  OptoReg::Name monitor_object_regname(int monitor_index) const;

  static OptoReg::Name cpu_regname (int n)     { return OptoReg::Name(n); }
  static OptoReg::Name cpu_regname (RInfo reg) { assert(reg.is_single_cpu(), "use cpu_regname(int) on each reg in pair");
                                                 return cpu_regname(reg.as_register()->encoding()); }

  int oop_map_arg_count();

  // return true if real-indeces of specified java-local-indeces are adjacent
  bool are_adjacent_indeces (int jix1, int jix2) const;

  void add_spill_slots(int nof_slots);

  static LIR_Opr caller_save_cpu_reg_at(int i);
  static RInfoCollection* caller_save_registers();
  static void init();
  static bool is_byte_rinfo(RInfo reg);

};


//--------------------------------------------------------
//               ArgumentLocation
//--------------------------------------------------------

class ArgumentLocation VALUE_OBJ_CLASS_SPEC {
  friend class CallingConvention;
 private:
  int _n;         // either a register number 0-5 (I0-I5 or O0-O5), or an offset (in words) from SP/FP

  ArgumentLocation (int n) : _n(n) {}
  ArgumentLocation (char k, int n) {
    switch (k) {
    case 'r':   set_register_location(n); break;
    case 's':   set_stack_location(n);    break;
    default:    ShouldNotReachHere();     break;
    }
  }

  int number () const { return _n; }

  void set_stack_location (int offset);
  void set_register_location (int number);

 public:
  ArgumentLocation () : _n(-1) {}

  bool is_illegal ()      const { return _n == -1; }
  bool is_register_arg () const;
  bool is_stack_arg ()    const;

  static ArgumentLocation new_stack_arg (WordSize offset) {
    return ArgumentLocation('s', in_words(offset));
  }

  static ArgumentLocation new_reg_arg (int n) {
    return ArgumentLocation('r', n);
  }

  int stack_offset_in_words () const;

  Address incoming_stack_location () const;
  Address outgoing_stack_location () const;

  RInfo incoming_reg_location () const;
  RInfo outgoing_reg_location () const;

  void print (bool incoming = true) const  PRODUCT_RETURN;
};


//--------------------------------------------------------
//               CallingConvention
//--------------------------------------------------------

class CallingConvention: public ResourceObj {
  friend class FrameMap;

 private:
  intStack* _args;

  CallingConvention () : _args(NULL)             {}
  CallingConvention (int size)                   { _args = new intStack(size, 0); }

  void map (int arg, ArgumentLocation where)     { _args->at_put_grow(arg, where.number(), 0); }

 public:
  ArgumentLocation arg_at(int i) const           { return ArgumentLocation(_args->at(i)); }
  int length() const                             { return _args->length(); }

#ifndef PRODUCT
  void print (bool incoming = true) const {
    for (int i = 0; i < length(); i++) {
      arg_at(i).print(incoming);
    }
  }
#endif // PRODUCT
};
