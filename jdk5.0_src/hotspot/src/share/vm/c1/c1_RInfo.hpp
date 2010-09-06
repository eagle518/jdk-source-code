#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_RInfo.hpp	1.35 03/12/23 16:39:17 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Abstract register representation

// This class represents a register that is either in CPU or in FPU.
// It also represents "long-registers", which are composed of two CPU
// registers. The type and the registers are stored in the _number.

class RInfo VALUE_OBJ_CLASS_SPEC {
  #include "incls/_c1_RInfo_pd.hpp.incl"

  // The underlying representation of RInfo is an integer stored in
  // _number. We want to keep _number private as well as any other
  // conversions between ints and RInfos. There are a few classes that
  // need to convert from int to RInfo, those are the friend classes
  // below.
  friend class RInfoCollection;
  friend class c1_RegMask;
  friend class RegAlloc;
  friend class RInfo2Reg;
  friend class Item;
  friend class FrameMap;
  friend class LIR_OprFact;
  friend class LIR_OprDesc;
  friend class RegisterManager;
  
 public:

  //     3                   2                   1
  //   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
  //  +-----------------------+-----------------------+---------------+
  //  |         reg2          |         reg1          |     type      |
  //  +-----------------------+-----------------------+---------------+

  // make it compatible with LIR_OprDesc
  enum {
    type_bits    = 10, // includes kind and type of LIR_OprDesc
    value_bits   = BitsPerInt - type_bits,
    reg_bits     = value_bits / 2
  };
 private:
  enum OprKind {
      pointer_value      = 0
    , stack_value        = 1
    , cpu_register       = 3
    , fpu_register       = 5
    , illegal_value      = 7
  };

  enum OprBits {
      pointer_bits   = 1
    , kind_bits      = 3
    , opr_type_bits  = 4
    , size_bits      = 2
    , destroys_bits  = 1
    , non_data_bits  = kind_bits + opr_type_bits + size_bits + destroys_bits
    , data_bits      = BitsPerInt - non_data_bits
    , opr_reg_bits   = data_bits / 2      // for two registers in one value encoding
  };

  enum OprShift {
      kind_shift     = 0
    , opr_type_shift = kind_shift     + kind_bits
    , size_shift     = opr_type_shift + opr_type_bits
    , destroys_shift = size_shift     + size_bits
    , data_shift     = destroys_shift + destroys_bits
  };

  enum OprRegSize {
      single_size = 0 << size_shift
    , double_size = 1 << size_shift
  };

  enum RegType {
    word_reg_type   = cpu_register | single_size,   // one cpu register
    long_reg_type   = cpu_register | double_size,   // a pair of cpu registers
    float_reg_type  = fpu_register | single_size,
    double_reg_type = fpu_register | double_size
  };
  enum {
    type_shift = 0,
    reg1_shift = type_bits,
    reg2_shift = reg1_shift + reg_bits
  };
  enum {
    type_mask          = right_n_bits(type_bits),
    type_mask_in_place = type_mask << type_shift,
    reg1_mask          = right_n_bits(reg_bits),
    reg1_mask_in_place = reg1_mask << reg1_shift,
    reg2_mask          = right_n_bits(reg_bits),
    reg2_mask_in_place = (int)(reg2_mask << reg2_shift)
  };
  enum {
    illegal_reg_number = -1
  };

 private:
  int _number;

  RInfo (int n, char dummy) : _number(n) {}

  RegType type () const {
    assert(type_shift == 0, "assumption");
    return (RegType)mask_bits(number(), type_mask_in_place);
  }

  int number () const { return _number; }
  int data   () const { return _number & ~type_mask_in_place; }
  int reg1   () const { return mask_bits(number() >> reg1_shift, reg1_mask); }
  int reg2   () const { return mask_bits(number() >> reg2_shift, reg2_mask); }

  void set_word_reg   (int reg)            { _number = (reg  << reg1_shift) + word_reg_type; }
  void set_long_reg   (int reg1, int reg2) { _number = (reg1 << reg1_shift) + (reg2 << reg2_shift) + long_reg_type; }
  void set_word_reg   (const Register r);
  void set_long_reg   (const Register lo, const Register hi);
  void set_float_reg  (int reg)            { _number = (reg  << reg1_shift) + float_reg_type; }
  // set_double_reg is machine dependent

  int reg ()        const { assert(is_word(),   "type_check"); return reg1(); }
  int reg_lo ()     const { assert(is_long(),   "type_check"); return reg1(); }
  int reg_hi ()     const { assert(is_long(),   "type_check"); return reg2(); }
  int float_reg ()  const { assert(is_float(),  "type check"); return reg1(); }
  int double_reg () const { assert(is_double(), "type check"); return reg2(); }

  static RInfo word_reg (int rnr) { RInfo r; r.set_word_reg(rnr); return r; }
  
 public:
  // constructors:
  RInfo () : _number(illegal_reg_number) {}

  friend RInfo as_RInfo(const Register r)                       { RInfo rinfo; rinfo.set_word_reg(r);      return rinfo; }
  friend RInfo as_RInfo(const Register lo, const Register hi)   { RInfo rinfo; rinfo.set_long_reg(lo, hi); return rinfo; }

  // accessors and tests:
  void set_no_rinfo ()                        { _number = illegal_reg_number; }
  bool is_same_type (const RInfo other) const { return type() == other.type(); }
  bool is_same  (const RInfo other) const     { return _number == other._number; }
  bool overlaps (const RInfo other) const;

  // these are deprecated
  bool is_word()      const { return type() == word_reg_type; }
  bool is_long()      const { return type() == long_reg_type; }
  bool is_float()     const { return type() == float_reg_type; }
  bool is_double()    const { return type() == double_reg_type; }

  bool is_virtual()   const { return reg1() >= pd_REG_COUNT; }


  // these names match the names in LIR_OprDesc
  bool is_single_cpu() const { return type() == word_reg_type; }
  bool is_double_cpu() const { return type() == long_reg_type; }
  bool is_single_fpu() const { return type() == float_reg_type; }
  bool is_double_fpu() const { return type() == double_reg_type; }
  bool is_float_kind() const { return is_single_fpu() || is_double_fpu(); }
  bool is_illegal()    const { return _number == illegal_reg_number; }
  bool is_valid()      const { return _number != illegal_reg_number; }

  int cpu_regnr()      const { assert(is_single_cpu(), "type check");   return (int)reg1(); }
  int cpu_regnrLo()    const { assert(is_double_cpu(), "type check");   return (int)reg1(); }
  int cpu_regnrHi()    const { assert(is_double_cpu(), "type check");   return (int)reg2(); }
  int fpu_regnr()      const { assert(is_single_fpu(), "type check");   return (int)reg1(); }
  int fpu_regnrLo()    const { assert(is_double_fpu(), "type check");   return (int)reg1(); }
  int fpu_regnrHi()    const { assert(is_double_fpu(), "type check");   return (int)reg2(); }

  Register as_register   () const;       // for integers
  Register as_register_lo() const;       // for longs
  Register as_register_hi() const;       // for longs

  RInfo as_rinfo_lo() const; // for longs and doubles
  RInfo as_rinfo_hi() const; // for longs and doubles

  // debugging:
  void print ()          const PRODUCT_RETURN;
  void print_raw ()      const PRODUCT_RETURN;
  void print (char* msg) const PRODUCT_RETURN;
};

const RInfo norinfo = ::RInfo();

class RInfoCollection : public CompilationResourceObj {
 private:
  intStack  _regs;                       // register as RInfo numbers

 public:
  RInfoCollection (): _regs(4) { }

  int length () const                    { return _regs.length(); }
  RInfo at(int index) const              { return (index >= length()) ? norinfo : RInfo(_regs.at(index), 'a'); }

  void append(const RInfo r)             { _regs.append(r.number()); }
  void at_put(int index, const RInfo r)  { _regs.at_put_grow(index, r.number(), RInfo::illegal_reg_number);}
  bool contains(RInfo r) const           { return _regs.contains(r.number()); }
  void remove(RInfo r)                   { _regs.remove(r.number()); }
  void print() const                     PRODUCT_RETURN;
};


//-------------------------------------------------------
//                 c1_RegMask
//-------------------------------------------------------

// Represents a set of registers. Implemented as an int mask, a register
// is a member of the set if the bit corresponding to its rnr is set in
// the mask.

class c1_RegMask VALUE_OBJ_CLASS_SPEC {

  friend class c1_AllocTable;
  friend class FrameMap;
  friend class RegAlloc;

 private:
  intx _mask;

  static int _size;
#ifdef CONST_SDM_BUG
  // This should be const but the "const" provokes a gcc bug id c++/1983
  static c1_RegMask _empty_set;
#else
  static const c1_RegMask _empty_set;
#endif

  c1_RegMask add_reg    (int rnr)               { check_rnr(rnr); set_nth_bit  (_mask, rnr); return *this; }
  c1_RegMask remove_reg (int rnr)               { check_rnr(rnr); clear_nth_bit(_mask, rnr); return *this; }
  bool       contains   (int rnr) const         { check_rnr(rnr); return is_set_nth_bit(_mask, rnr); }
  int        get_first () const;

  static void check_rnr (int rnr)               { assert(rnr >= 0 && rnr < _size, "wrong index"); }

 public:
  c1_RegMask () : _mask(0) {}
  c1_RegMask (RInfo r, char dummy) : _mask(0)   { assert(r.is_word(), "only cpu regs"); add_reg(r.reg()); }

  c1_RegMask add_reg    (RInfo r)               { assert(r.is_word(), "only cpu regs"); return add_reg   (r.reg()); }
  c1_RegMask remove_reg (RInfo r)               { assert(r.is_word(), "only cpu regs"); return remove_reg(r.reg()); }
  bool       contains   (RInfo r) const         { assert(r.is_word(), "only cpu regs"); return contains  (r.reg()); }
  bool       is_empty() const                   { return _mask == 0; }
  RInfo      get_first_reg() const              { return RInfo::word_reg(get_first()); }

  static void init_masks(int size);
  static const c1_RegMask empty_set()           { return _empty_set; }

  // debugging:
  void print () const PRODUCT_RETURN;
};

