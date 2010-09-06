#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_LIR.hpp	1.114 04/04/21 01:13:22 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 
// 

class BlockBegin;
class BlockList;
class LIR_AbstractAssembler;
class LIR_Assembler;
class CodeEmitInfo;
class CodeStub;
class CodeStubList;
class StaticCallStub;
class ArrayCopyStub;
class LIR_Op;
class ciType;
class ValueType;
class LIR_OpVisitState;


//---------------------------------------------------------------------
//                 LIR Operands
//  LIR_OprDesc
//    LIR_OprPtr
//      LIR_Const
//      LIR_Address
//---------------------------------------------------------------------
class LIR_OprDesc;
class LIR_OprPtr;
class LIR_Const;
class LIR_Address;
class LIR_OprVisitor;

typedef LIR_OprDesc* LIR_Opr;
typedef int          RegNr;

define_array(LIR_OprArray, LIR_Opr)
define_stack(LIR_OprList, LIR_OprArray)

define_array(LIR_OprRefArray, LIR_Opr*)
define_stack(LIR_OprRefList, LIR_OprRefArray)

define_array(CodeEmitInfoArray, CodeEmitInfo*)
define_stack(CodeEmitInfoList, CodeEmitInfoArray)

define_array(LIR_OpArray, LIR_Op*)
define_stack(LIR_OpList, LIR_OpArray)

// define LIR_OprPtr early so LIR_OprDesc can refer to it
class LIR_OprPtr: public CompilationResourceObj {
 public:
  bool is_oop_pointer() const                    { return (type() == T_OBJECT); }
  bool is_float_kind() const                     { BasicType t = type(); return (t == T_FLOAT) || (t == T_DOUBLE); }

  virtual LIR_Const*  as_constant()              { return NULL; }
  virtual LIR_Address* as_address()              { return NULL; }
  virtual BasicType type() const                 { ShouldNotReachHere(); return T_ILLEGAL; }
  virtual void print() = 0;
};



// LIR constants
class LIR_Const: public LIR_OprPtr {
 private:
  JavaValue _value;
 public:
  LIR_Const(jint i)                              { _value.set_type(T_INT);     _value.set_jint(i); }
  LIR_Const(jlong l)                             { _value.set_type(T_LONG);    _value.set_jlong(l); }
  LIR_Const(jfloat f)                            { _value.set_type(T_FLOAT);   _value.set_jfloat(f); }
  LIR_Const(jdouble d)                           { _value.set_type(T_DOUBLE);  _value.set_jdouble(d); }
  LIR_Const(jobject o)                           { _value.set_type(T_OBJECT);  _value.set_jobject(o); }

  virtual BasicType type()       const           { return _value.get_type(); }
  virtual LIR_Const* as_constant()               { return this; }

  jint      as_jint()    const                   { assert(type() == T_INT,    "type check"); return _value.get_jint(); }
  jlong     as_jlong()   const                   { assert(type() == T_LONG,   "type check"); return _value.get_jlong(); }
  jfloat    as_jfloat()  const                   { assert(type() == T_FLOAT,  "type check"); return _value.get_jfloat(); }
  jdouble   as_jdouble() const                   { assert(type() == T_DOUBLE, "type check"); return _value.get_jdouble(); }
  jobject   as_jobject() const                   { assert(type() == T_OBJECT, "type check"); return _value.get_jobject(); }
  jint      as_jint_lo() const {
    assert(type() == T_LONG || type() == T_DOUBLE, "type check");
    jlong l;
    if (type() == T_LONG) {
      l = _value.get_jlong();
    } else {
      jdouble d = _value.get_jdouble();
      l = jlong_cast(d);
    }
    return low(l);
  }

  jint      as_jint_hi() const {
    assert(type() == T_LONG || type() == T_DOUBLE, "type check");
    jlong l;
    if (type() == T_LONG) {
      l = _value.get_jlong();
    } else {
      jdouble d = _value.get_jdouble();
      l = jlong_cast(d);
    }
    return high(l);
  }

  virtual void print() PRODUCT_RETURN;


  bool is_zero_float() {
    jfloat f = as_jfloat();
    jfloat ok = 0.0f;
    return jint_cast(f) == jint_cast(ok);
  }
  
  bool is_one_float() {
    jfloat f = as_jfloat();
    return !g_isnan(f) && g_isfinite(f) && f == 1.0;
  }
  
  bool is_zero_double() {
    jdouble d = as_jdouble();
    jdouble ok = 0.0;
    return jlong_cast(d) == jlong_cast(ok);
  }
  
  bool is_one_double() {
    jdouble d = as_jdouble();
    return !g_isnan(d) && g_isfinite(d) && d == 1.0;
  }
};


//---------------------LIR Operand descriptor------------------------------------
//
// The class LIR_OprDesc represents a LIR instruction operand;
// it can be a register (ALU/FPU), stack location or a constant;
// Constants and addresses are represented as resource area allocated 
// structures (see above).
// Registers and stack locations are inlined into the this pointer
// (see value function).

class LIR_OprDesc: public CompilationResourceObj {
 public:
  // value structure:
  //     data       opr-type opr-kind
  // +--------------+-------+-------+
  // [max...........|7 6 5 4|3 2 1 0]
  //                             ^
  //                    is_pointer bit
  // 
  // lowest bit cleared, means it is a structure pointer
  // we need  4 bits to represent types
  //
  // Note that RInfo presentation is identical as the representation
  // for registers in LIR_OprDesc; this is verified with asserts.

 private:
  friend class LIR_OprFact;

  // Conversion
  intptr_t value() const                         { return (intptr_t) this; }

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
    , type_bits      = 4
    , size_bits      = 2
    , destroys_bits  = 1
    , non_data_bits  = kind_bits + type_bits + size_bits + destroys_bits
    , data_bits      = BitsPerInt - non_data_bits
    , reg_bits       = data_bits / 2      // for two registers in one value encoding
  };

  enum OprShift {
      kind_shift     = 0
    , type_shift     = kind_shift     + kind_bits
    , size_shift     = type_shift     + type_bits
    , destroys_shift = size_shift     + size_bits
    , data_shift     = destroys_shift + destroys_bits
  };

  enum OprSize {
      single_size = 0 << size_shift
    , double_size = 1 << size_shift
  };

  enum OprMask {
      kind_mask      = right_n_bits(kind_bits)
    , type_mask      = right_n_bits(type_bits) << type_shift
    , size_mask      = right_n_bits(size_bits) << size_shift
    , destroys_mask  = right_n_bits(destroys_bits) << destroys_shift
    , pointer_mask   = right_n_bits(pointer_bits)
    , lower_reg_mask = right_n_bits(reg_bits)
    , no_type_mask   = (int)(~(type_mask | destroys_mask))
  };

  enum {
    vreg_base = pd_REG_COUNT
  };

  intptr_t data() const                          { return value() >> data_shift; }
  int lo_reg_half() const                        { return data() & lower_reg_mask; }
  int hi_reg_half() const                        { return data() >> reg_bits; }
  OprKind kind_field() const                     { return (OprKind)(value() & kind_mask); }
  OprSize size_field() const                     { return (OprSize)(value() & size_mask); }

  intptr_t value_without_type() const            { return is_illegal() ? value() : value() & no_type_mask; }

  static char type_char(BasicType t);

 public:
  enum OprType {
      unknown_type = 0          << type_shift      // means: not set (catch uninitialized types)
    , boolean_type = T_BOOLEAN  << type_shift
    , byte_type =    T_BYTE     << type_shift
    , short_type =   T_SHORT    << type_shift
    , int_type =     T_INT      << type_shift
    , long_type =    T_LONG     << type_shift
    , char_type =    T_CHAR     << type_shift
    , object_type =  T_OBJECT   << type_shift
    , array_type =   T_ARRAY    << type_shift
    , float_type =   T_FLOAT    << type_shift
    , double_type =  T_DOUBLE   << type_shift
  };
  friend OprType as_OprType(BasicType t);
  friend BasicType as_BasicType(OprType t);
  OprType type_field() const                     { return is_illegal() ? unknown_type : (OprType)(value() & type_mask); }


  static OprSize size_for(BasicType t) {
    switch (t) {
      case T_LONG:
      case T_DOUBLE:
        return double_size;
        break;

      case T_FLOAT:
      case T_BOOLEAN:
      case T_CHAR:
      case T_BYTE:
      case T_SHORT:
      case T_INT:
      case T_OBJECT:
      case T_ARRAY:
      case T_ADDRESS:
        return single_size;
        break;
        
      default:
        ShouldNotReachHere();
      }
  }


  // returns a new LIR_Opr with the OprType of from and all other information the current LIR_Opr
  LIR_Opr with_type_of(LIR_Opr from) {
    assert(!is_pointer() && !from->is_pointer(), "only simple LIR_Oprs");
    return (LIR_Opr)(from->type_field() | value_without_type());
  }

  void validate_type() const { 
#ifndef PRODUCT
    if (!is_pointer()) {
      switch (as_BasicType(type_field())) {
      case T_LONG:
        assert((kind_field() == cpu_register || kind_field() == stack_value) && size_field() == double_size, "must match");
        break;
      case T_FLOAT:
        assert((kind_field() == fpu_register || kind_field() == stack_value) && size_field() == single_size, "must match");
        break;
      case T_DOUBLE:
        assert((kind_field() == fpu_register || kind_field() == stack_value) && size_field() == double_size, "must match");
        break;
      case T_BOOLEAN:
      case T_CHAR:
      case T_BYTE:
      case T_SHORT:
      case T_INT:
      case T_OBJECT:
      case T_ARRAY:
      case T_ADDRESS:
        assert((kind_field() == cpu_register || kind_field() == stack_value) && size_field() == single_size, "must match");
        break;
        
      case T_ILLEGAL:
        // XXX TKR also means unknown right now
        // assert(is_illegal(), "must match");
        break;
        
      default:
        ShouldNotReachHere();
      }
    }
#endif
  }

  BasicType type() const {
    if (is_pointer()) {
      return pointer()->type();
    }
    return as_BasicType(type_field());
  }


  ValueType* value_type() const                  { return as_ValueType(type()); }

  char type_char() const                         { return type_char((is_pointer()) ? pointer()->type() : type()); }

  bool is_same(LIR_Opr opr) const                { return this == opr; }
  bool overlaps(LIR_Opr opr) const               { return is_register() && opr->is_register() && rinfo().overlaps(opr->rinfo()); }
  // checks whether types are same or one type is unknown
  bool is_type_compatible(LIR_Opr opr) const     {
    return type_field() == unknown_type ||
      opr->type_field() == unknown_type ||
      type_field() == opr->type_field();
  }

  // is_type_compatible and equal
  bool is_equivalent(LIR_Opr opr) const {
    if (!is_pointer()) {
      if (is_type_compatible(opr) && data() == opr->data() && kind_field() == opr->kind_field()) {
        return true;
      }
    }
    // could also do checks for other pointers types
    return is_same(opr);
  }



  bool is_oop() const;
  bool is_register() const                       { return !(is_pointer() || is_stack() || is_illegal()); }
  bool is_float_kind() const                     { return is_pointer() ? pointer()->is_float_kind() : (kind_field() == fpu_register); }
  bool is_stack() const                          { return kind_field() == stack_value; }
  bool is_valid() const                          { return !is_illegal(); }

  bool is_constant() const                       { return is_pointer() && pointer()->as_constant() != NULL; }
  bool is_address() const                        { return is_pointer() && pointer()->as_address() != NULL; }
  bool is_pointer() const                        { return mask_bits(value(), pointer_mask) == pointer_value; }
  bool is_single_word() const                    { return size_field() == single_size && !is_pointer(); }
  bool is_double_word() const                    { return size_field() == double_size && !is_pointer(); }
  bool is_single_stack() const                   { return kind_field() == stack_value && size_field() == single_size; }
  bool is_double_stack() const                   { return kind_field() == stack_value && size_field() == double_size; }
  bool is_single_cpu() const                     { validate_type(); return (value() & (kind_mask | size_mask)) == (cpu_register | single_size); }
  bool is_double_cpu() const                     { validate_type(); return (value() & (kind_mask | size_mask)) == (cpu_register | double_size); }
  bool is_single_fpu() const                     { validate_type(); return (value() & (kind_mask | size_mask)) == (fpu_register | single_size); }
  bool is_double_fpu() const                     { validate_type(); return (value() & (kind_mask | size_mask)) == (fpu_register | double_size); }
  bool is_virtual() const                        { validate_type(); return (kind_field() == cpu_register || kind_field() == fpu_register) && data() >= vreg_base; }
  bool is_illegal() const                        { return kind_field() == illegal_value; }

  bool is_destroyed() const                      { assert(is_register(), "only works for registers"); return value() & destroys_mask; }
  LIR_Opr make_destroyed()                       { assert(is_register(), "only works for registers"); return (LIR_Opr)(value() | destroys_mask); }

  int single_stack_ix() const                    { assert(is_single_stack(), "type check"); return (int)data(); }
  int double_stack_ix() const                    { assert(is_double_stack(), "type check"); return (int)data(); }

  RegNr cpu_regnr() const                        { assert(is_single_cpu(), "type check");   return (RegNr)data(); }
  RegNr cpu_regnrLo() const                      { assert(is_double_cpu(), "type check");   return (RegNr)lo_reg_half(); }
  RegNr cpu_regnrHi() const                      { assert(is_double_cpu(), "type check");   return (RegNr)hi_reg_half(); }
  RegNr fpu_regnr() const                        { assert(is_single_fpu(), "type check");   return (RegNr)data(); }
  RegNr fpu_regnrLo() const                      { assert(is_double_fpu(), "type check");   return (RegNr)lo_reg_half(); }
  RegNr fpu_regnrHi() const                      { assert(is_double_fpu(), "type check");   return (RegNr)hi_reg_half(); }
  int   vreg_number() const                      { assert(is_virtual(), "type check");   return (RegNr)data() - vreg_base; }
  int   vreg_numberLo() const                    { assert(is_virtual(), "type check");   return (RegNr)lo_reg_half() - vreg_base; }
  int   vreg_numberHi() const                    { assert(is_virtual(), "type check");   return (RegNr)hi_reg_half() - vreg_base; }

  LIR_OprPtr* pointer()  const                   { assert(is_pointer(), "type check");      return (LIR_OprPtr*)this; }
  LIR_Const* as_constant_ptr() const             { return pointer()->as_constant(); }
  LIR_Address* as_address_ptr() const            { return pointer()->as_address(); }

  RInfo rinfo() const { 
    assert(!is_pointer() && !is_stack(), "type check"); 
    RInfo r = RInfo(value_without_type(), 'x');
    assert(matches(r), "conversion check");
    return r; 
  }

  Register as_register()    const                   { return rinfo().as_register();    }
  Register as_register_lo() const                   { return rinfo().as_register_lo(); }
  Register as_register_hi() const                   { return rinfo().as_register_hi(); }
  RInfo    as_rinfo()       const                   { return rinfo();                  }
  RInfo    as_rinfo_lo()    const                   { return rinfo().as_rinfo_lo();    }
  RInfo    as_rinfo_hi()    const                   { return rinfo().as_rinfo_hi();    }

  jint      as_jint()    const { return as_constant_ptr()->as_jint(); }
  jlong     as_jlong()   const { return as_constant_ptr()->as_jlong(); }
  jfloat    as_jfloat()  const { return as_constant_ptr()->as_jfloat(); }
  jdouble   as_jdouble() const { return as_constant_ptr()->as_jdouble(); }
  jobject   as_jobject() const { return as_constant_ptr()->as_jobject(); }

  void print() PRODUCT_RETURN;
#ifdef ASSERT
  bool matches(RInfo r) const {
    // Note that RInfo encoding scheme matches the LIR_Opr encoding scheme;
    // The following list of assertions verifies that
    assert(RInfo::type_bits == non_data_bits, "constants do not match");
    return 
         r.is_word()    == is_single_cpu()
      && r.is_long()    == is_double_cpu()
      && r.is_float()   == is_single_fpu()
      && r.is_double()  == is_double_fpu()
      && r.is_illegal() == is_illegal();
  }
#endif
};


inline LIR_OprDesc::OprType as_OprType(BasicType t) {
  if (t == T_ILLEGAL) {
    return LIR_OprDesc::unknown_type;
  } else {
    return (LIR_OprDesc::OprType)(t << LIR_OprDesc::type_shift);
  }
}

inline BasicType as_BasicType(LIR_OprDesc::OprType t) {
  if (t == LIR_OprDesc::unknown_type) {
    return T_ILLEGAL;
  } else {
    return (BasicType)(t >> LIR_OprDesc::type_shift);
  }
}


// LIR_Address
class LIR_Address: public LIR_OprPtr {
 public:
  // NOTE: currently these must be the log2 of the scale factor (and
  // must also be equivalent to the ScaleFactor enum in
  // assembler_i486.hpp)
  enum Scale {
    times_1  =  0,
    times_2  =  1,
    times_4  =  2,
    times_8  =  3
  };

 private:
  LIR_Opr   _base;
  LIR_Opr   _index;
  Scale     _scale;
  intx      _disp;

 public:
  LIR_Address(LIR_Opr base, LIR_Opr index): 
       _base(base)
     , _index(index)
     , _scale(times_1)
     , _disp(0) { assert(base->is_single_cpu(), "wrong base operand"); }

  LIR_Address(LIR_Opr base, LIR_Opr index, int disp): 
       _base(base)
     , _index(index)
     , _scale(times_1)
     , _disp(disp) { assert(base->is_single_cpu(), "wrong base operand"); }

  LIR_Address(LIR_Opr base, LIR_Opr index, Scale scale, int disp): 
       _base(base)
     , _index(index)
     , _scale(scale)
     , _disp(disp)
  {
#ifdef SPARC
    assert(scale == times_1, "Scaled addressing mode not available on SPARC and should not be used");
#endif
    assert(base->is_single_cpu(), "wrong base operand");
  }

  LIR_Opr base()  const                          { return _base;  }
  LIR_Opr index() const                          { return _index; }
  Scale   scale() const                          { return _scale; }
  intx    disp()  const                          { return _disp;  }

  bool equals(LIR_Address* other) const          { return base() == other->base() && index() == other->index() && disp() == other->disp() && scale() == other->scale(); }

  virtual LIR_Address* as_address()              { return this;   }
  virtual BasicType type() const                 { return T_ADDRESS; }
  virtual void print() PRODUCT_RETURN;

  static Scale scale(BasicType type);
};


// operand factory
class LIR_OprFact: public AllStatic {
 public:

  static LIR_Opr illegalOpr;
  static LIR_Opr rinfo(RInfo r) {
    BasicType t;
    if (r.is_word()) {
      t = T_INT;
    } else if (r.is_long()) {
      t = T_LONG;
    } else if (r.is_double()) {
      t = T_DOUBLE;
    } else if (r.is_float()) {
      t = T_FLOAT;
    } else if (r.is_illegal()) {
      t = T_ILLEGAL;
    } else {
      ShouldNotReachHere();
    }
    return rinfo(r, t);
  }

  static LIR_Opr rinfo(RInfo r, BasicType type) {
    LIR_OprDesc::OprType t = as_OprType(type);
    if (r.is_illegal()) return illegalOpr;
    LIR_Opr res = (LIR_Opr)(r.data() | t | ((type == T_FLOAT || type == T_DOUBLE) ? LIR_OprDesc::fpu_register : LIR_OprDesc::cpu_register) | LIR_OprDesc::size_for(type));
    // Note that RInfo encoding scheme matches the LIR_Opr encoding scheme;
    // The following list of assertions verifies that
    assert(res->matches(r), "conversion check");
    assert(res->type_field() == t, "type check");
    return res;
  }

  static LIR_Opr virtual_register(int index, BasicType type) {
    LIR_OprDesc::OprType t = as_OprType(type);
    int value = index + LIR_OprDesc::vreg_base;
    if (type == T_LONG || type == T_DOUBLE) {
      value |= (index + LIR_OprDesc::vreg_base + 1) << LIR_OprDesc::reg_bits;
    }
    LIR_Opr res = (LIR_Opr)((value << LIR_OprDesc::data_shift) | t | ((type == T_FLOAT || type == T_DOUBLE) ? LIR_OprDesc::fpu_register : LIR_OprDesc::cpu_register) | LIR_OprDesc::size_for(type));
    assert(res->type_field() == t, "type check");
    if (type == T_LONG || type == T_DOUBLE) {
      assert(res->vreg_numberLo() == index, "conversion check");
      assert(res->vreg_numberHi() == index + 1, "conversion check");
    } else {
      assert(res->vreg_number() == index, "conversion check");
    }
    return res;
  }

  // 'ix' is computed by FrameMap::local_stack_pos(index); do not use other parameters as
  // the index is platform independent; a double stack useing indeces 2 and 3 has always
  // index 2.

  static LIR_Opr stack(int ix, BasicType type) {
    assert( ix <= (max_jint >> LIR_OprDesc::data_shift), "index is too big");
    LIR_Opr res = (LIR_Opr)((ix << LIR_OprDesc::data_shift) | LIR_OprDesc::stack_value | as_OprType(type) | LIR_OprDesc::size_for(type));
    return res;
  }

  static LIR_Opr single_stack(int ix, BasicType type) {
    LIR_Opr res = stack(ix, type);
    assert(ix == res->single_stack_ix() && res->is_single_stack() && res->type_field() == as_OprType(type), "should match");
    return res;
  }

  static LIR_Opr double_stack(int ix, BasicType type) {
    LIR_Opr res = stack(ix, type);
    assert(ix == res->double_stack_ix() && res->is_double_stack() && res->type_field() == as_OprType(type), "should match");
    return res;
  }


  static LIR_Opr intConst(jint i)                { return (LIR_Opr)(new LIR_Const(i)); }
  static LIR_Opr longConst(jlong l)              { return (LIR_Opr)(new LIR_Const(l)); }
  static LIR_Opr floatConst(jfloat f)            { return (LIR_Opr)(new LIR_Const(f)); }
  static LIR_Opr doubleConst(jdouble d)          { return (LIR_Opr)(new LIR_Const(d)); }
  static LIR_Opr oopConst(jobject o)             { return (LIR_Opr)(new LIR_Const(o)); }
  static LIR_Opr address(LIR_Address* a)         { return (LIR_Opr)a; }
  static LIR_Opr illegal()                       { return (LIR_Opr)norinfo.number(); }

  static LIR_Opr value_type(ValueType* type);
  static LIR_Opr dummy_value_type(ValueType* type);
};


// Array that is indexed by a LIR_Opr; the array has a distinct type,
// but can accomodate double and single values
class LIR_OprRefCount: public CompilationResourceObj {
 public:
  enum LIR_OprRefCountType {
      cpu_reg_type
    , fpu_reg_type
    , stack_type
  };
 private:
  LIR_OprRefCountType _type;
  intStack* _ref_count_data; // mapped to LIR_Opr (maps only single values)
  LIR_OprList* _oprs;

  LIR_OprRefCountType type() const                  { return _type; }
  bool is_correct_type(LIR_Opr opr) const;
  void increment(int index, LIR_Opr opr, int count = 1);
  intStack* data() const                         { return _ref_count_data; }

 public:
  LIR_OprRefCount(LIR_OprRefCountType type);

  // these will fail if LIR_Opr is wrong type
  void incr_ref(LIR_Opr opr, int count = 1);
  int length() const { return data()->length(); }
  int ref_count(LIR_Opr opr) const;
  LIR_Opr opr_at(int index) const { return _oprs->at(index); }
  void merge(LIR_OprRefCount* count);

  void print() PRODUCT_RETURN;

};


//-------------------------------------------------------------------------------
//                   LIR Instructions
//-------------------------------------------------------------------------------
//
// Note: 
//  - every instruction has a result operand
//  - every instruction has an CodeEmitInfo operand (can be revisited later)
//  - every instruction has a LIR_OpCode operand
//  - LIR_OpN, means an instruction that has N input operands
//
// class hierarchy:
//
class  LIR_Op;
class    LIR_Op0;
class      LIR_OpLabel;
class    LIR_Op1;
class      LIR_OpBranch;
class      LIR_OpRTCall;
class      LIR_OpConvert;
class      LIR_OpAllocObj;
class    LIR_Op2;
class    LIR_OpDelay;
class    LIR_Op3;
class      LIR_OpAllocArray;
class    LIR_OpJavaCall;
class    LIR_OpArrayCopy;
class    LIR_OpLock;
class    LIR_OpTypeCheck;
class    LIR_OpCompareAndSwap;


// LIR operation codes
enum LIR_Code {
    lir_none
  , begin_op0
      , lir_word_align
      , lir_label
      , lir_nop
      , lir_backwardbranch_target
      , lir_align_entry
      , lir_verified_entry
      , lir_build_frame
      , lir_fpop_raw
      , lir_empty_fpu
      , lir_24bit_FPU
      , lir_reset_FPU
      , lir_breakpoint
      , lir_rtcall
      , lir_jvmpi_method_enter
      , lir_jvmpi_method_exit
      , lir_membar
      , lir_membar_acquire
      , lir_membar_release
      , lir_get_thread
  , end_op0
  , begin_op1
      , lir_fpu_push
      , lir_fpu_pop
      , lir_fpu_dup
      , lir_push
      , lir_pop
      , lir_null_check
      , lir_return
      , lir_leal
      , lir_neg
      , lir_branch
      , lir_cond_float_branch
      , lir_move
      , lir_volatile_move
      , lir_array_move
      , lir_convert
      , lir_fast_convert
      , lir_alloc_object
      , lir_monaddr
      // , lir_fast_hash
      , lir_new_multi
      , lir_round32
      , lir_safepoint
  , end_op1
  , begin_op2
      , lir_cmp
      , lir_cmp_l2i
      , lir_ucmp_fd2i
      , lir_cmp_fd2i
      , lir_add
      , lir_sub
      , lir_mul
      , lir_mul_strictfp
      , lir_div
      , lir_div_strictfp
      , lir_rem
      , lir_sqrt
      , lir_sin
      , lir_cos
      , lir_unverified_entry
      , lir_logic_and
      , lir_logic_or
      , lir_logic_orcc
      , lir_logic_xor
      , lir_shl
      , lir_shlx
      , lir_shr
      , lir_ushr
      , lir_alloc_array
      , lir_throw
      , lir_unwind
  , end_op2
  , begin_op3
      , lir_idiv
      , lir_irem
  , end_op3
  , begin_opJavaCall
      , lir_static_call
      , lir_optvirtual_call
      , lir_icvirtual_call
      , lir_virtual_call
  , end_opJavaCall
  , begin_opArrayCopy
      , lir_arraycopy
  , end_opArrayCopy
  , begin_opLock
    , lir_lock
    , lir_unlock
  , end_opLock
  , begin_delay_slot
    , lir_delay_slot
  , end_delay_slot
  , begin_opTypeCheck
    , lir_instanceof
    , lir_checkcast
    , lir_store_check
  , end_opTypeCheck
  , begin_opCompareAndSwap
    , lir_cas_long
    , lir_cas_obj
    , lir_cas_int
  , end_opCompareAndSwap
};


// --------------------------------------------------
// LIR_Op
// --------------------------------------------------
class LIR_Op: public CompilationResourceObj {
 friend class LIR_Optimizer;

 protected:
  LIR_Opr       _result;
  LIR_Code      _code;
  CodeEmitInfo* _info;


  static void print_code(LIR_Code code) PRODUCT_RETURN;

 protected:
  static bool is_in_range(LIR_Code test, LIR_Code start, LIR_Code end)  { return start < test && test < end; }

 public:
  LIR_Op() 
    : _result(LIR_OprFact::illegalOpr)
    , _code(lir_none)
    , _info(NULL)                             {}

  LIR_Op(LIR_Code code, LIR_Opr result, CodeEmitInfo* info)
    : _result(result)
    , _code(code) 
    , _info(info)                             {}

  CodeEmitInfo* info() const                  { return _info;   }
  LIR_Code code()      const                  { return _code;   }
  LIR_Opr result_opr() const                  { return _result; }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm) = 0;
  virtual void print_instr() const                    = 0;
  virtual void print_on(outputStream* st) const PRODUCT_RETURN;

  virtual LIR_OpJavaCall* as_OpJavaCall() { return NULL; }
  virtual LIR_OpLabel* as_OpLabel() { return NULL; }
  virtual LIR_OpDelay* as_OpDelay() { return NULL; }
  virtual LIR_OpLock* as_OpLock() { return NULL; }
  virtual LIR_OpAllocArray* as_OpAllocArray() { return NULL; }
  virtual LIR_OpAllocObj* as_OpAllocObj() { return NULL; }
  virtual LIR_OpBranch* as_OpBranch() { return NULL; }
  virtual LIR_OpRTCall* as_OpRTCall() { return NULL; }
  virtual LIR_Op0* as_Op0() { return NULL; }
  virtual LIR_Op1* as_Op1() { return NULL; }
  virtual LIR_Op2* as_Op2() { return NULL; }
  virtual LIR_Op3* as_Op3() { return NULL; }
  virtual LIR_OpArrayCopy* as_OpArrayCopy() { return NULL; }
  virtual LIR_OpTypeCheck* as_OpTypeCheck() { return NULL; }
  virtual LIR_OpCompareAndSwap* as_OpCompareAndSwap() { return NULL; }

};

// for calls
class LIR_OpCall: public LIR_Op {
 private:
  address _addr;
 protected:
  LIR_OpCall(LIR_Code code, address addr, LIR_Opr result = LIR_OprFact::illegalOpr, CodeEmitInfo* info = NULL)
    : LIR_Op(code, result, info)
    , _addr(addr) {}

 public:
  virtual void visit(LIR_OpVisitState* visitor);

  address addr() const                           { return _addr; }
};


// --------------------------------------------------
// LIR_OpJavaCall
// --------------------------------------------------
class LIR_OpJavaCall: public LIR_OpCall {
 private:
  StaticCallStub* _stub;
  RInfo           _receiver;
 public:
  LIR_OpJavaCall(LIR_Code code, RInfo receiver, LIR_Opr result, address addr, CodeEmitInfo* info, StaticCallStub* stub = NULL)
  : LIR_OpCall(code, addr, result, info)
  , _receiver(receiver)
  , _stub(stub)              { assert(is_in_range(code, begin_opJavaCall, end_opJavaCall), "code check"); }

LIR_OpJavaCall(LIR_Code code, RInfo receiver, LIR_Opr result, intptr_t vtable_offset, CodeEmitInfo* info)
  : LIR_OpCall(code, (address)vtable_offset, result, info)
  , _receiver(receiver)
  , _stub(NULL)              { assert(is_in_range(code, begin_opJavaCall, end_opJavaCall), "code check"); }

  StaticCallStub* stub() const                   { return _stub; }
  RInfo receiver() const                         { return _receiver; }

  intptr_t vtable_offset() const {
    assert(_code == lir_virtual_call, "only have vtable for real vcall");
    return (intptr_t) addr();
  }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpJavaCall* as_OpJavaCall() { return this; }
  virtual void print_instr() const PRODUCT_RETURN;
};

// --------------------------------------------------
// LIR_OpLabel
// --------------------------------------------------
// Location where a branch can continue
class LIR_OpLabel: public LIR_Op {
 private:
  Label* _label;
 public:
  LIR_OpLabel(Label* lbl)
   : LIR_Op(lir_label, LIR_OprFact::illegalOpr, NULL)
   , _label(lbl)                                 {}
  Label* label() const                           { return _label; }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpLabel* as_OpLabel() { return this; }
  virtual void print_instr() const PRODUCT_RETURN;
};

// LIR_OpArrayCopy
class LIR_OpArrayCopy: public LIR_Op {
 private:
  ArrayCopyStub* _stub;
  LIR_Opr   _src;
  LIR_Opr   _src_pos;
  LIR_Opr   _dst;
  LIR_Opr   _dst_pos;
  LIR_Opr   _length;
  LIR_Opr   _tmp;
  ciArrayKlass* _expected_type;
  int       _flags;

public:
  enum Flags {
    src_null_check         = 1 << 0,
    dst_null_check         = 1 << 1,
    src_pos_positive_check = 1 << 2,
    dst_pos_positive_check = 1 << 3,
    length_positive_check  = 1 << 4,
    src_range_check        = 1 << 5,
    dst_range_check        = 1 << 6,
    type_check             = 1 << 7,
    all_flags              = (1 << 8) - 1
  };

  LIR_OpArrayCopy(LIR_Opr src, LIR_Opr src_pos, LIR_Opr dst, LIR_Opr dst_pos, LIR_Opr length, LIR_Opr tmp,
                  ciArrayKlass* expected_type, int flags, CodeEmitInfo* info)
    : LIR_Op(lir_arraycopy, LIR_OprFact::illegalOpr, info)
    , _tmp(tmp)
    , _src(src)
    , _src_pos(src_pos)
    , _dst(dst)
    , _dst_pos(dst_pos)
    , _expected_type(expected_type)
    , _flags(flags)
    , _length(length)                            { }

  LIR_Opr src() const                            { return _src; }
  LIR_Opr src_pos() const                        { return _src_pos; }
  LIR_Opr dst() const                            { return _dst; }
  LIR_Opr dst_pos() const                        { return _dst_pos; }
  LIR_Opr length() const                         { return _length; }
  LIR_Opr tmp() const                            { return _tmp; }
  int flags() const                              { return _flags; }
  ciArrayKlass* expected_type() const            { return _expected_type; }
  ArrayCopyStub* stub() const                    { return _stub; }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpArrayCopy* as_OpArrayCopy() { return this; }
  void print_instr() const PRODUCT_RETURN;
};


// --------------------------------------------------
// LIR_Op0
// --------------------------------------------------
class LIR_Op0: public LIR_Op {
 public:
  LIR_Op0(LIR_Code code, CodeEmitInfo* info = NULL)
   : LIR_Op(code, LIR_OprFact::illegalOpr, info)  { assert(is_in_range(code, begin_op0, end_op0), "code check"); }
  LIR_Op0(LIR_Code code, RInfo result)
   : LIR_Op(code, LIR_OprFact::rinfo(result), NULL)  { assert(is_in_range(code, begin_op0, end_op0), "code check"); }

  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_Op0* as_Op0() { return this; }
  virtual void print_instr() const PRODUCT_RETURN;
};


// --------------------------------------------------
// LIR_Op1
// --------------------------------------------------

class LIR_Op1: public LIR_Op {
 friend class LIR_Optimizer;

 public:
  enum LIR_PatchCode { 
    patch_none, patch_low, patch_high, patch_normal
  };

 protected:
  LIR_Opr         _opr;   // input operand
  BasicType       _type;  // Operand types
  LIR_PatchCode   _patch; // only required with patchin (NEEDS_CLEANUP: do we want a special instruction for patching?)
  
  static void print_patch_code(LIR_PatchCode code);

 public:
  LIR_Op1(LIR_Code code, LIR_Opr opr, LIR_Opr result = LIR_OprFact::illegalOpr, BasicType type = T_ILLEGAL, LIR_PatchCode patch= patch_none, CodeEmitInfo* info = NULL)
    : LIR_Op(code, result, info)
    , _opr(opr)
    , _patch(patch)
    , _type(type)                                { assert(is_in_range(code, begin_op1, end_op1), "code check"); }

  LIR_Op1(LIR_Code code, LIR_Opr opr, CodeEmitInfo* info)
    : LIR_Op(code, LIR_OprFact::illegalOpr, info)
    , _opr(opr)
    , _patch(patch_none)
    , _type(T_ILLEGAL)                           { assert(is_in_range(code, begin_op1, end_op1), "code check"); }

  LIR_Opr in_opr()           const               { return _opr;   }
  LIR_PatchCode patch_code() const               { return _patch; }
  BasicType type()           const               { return _type;  }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_Op1* as_Op1() { return this; }
  
  virtual void print_instr() const PRODUCT_RETURN;
};


// for runtime calls
class LIR_OpRTCall: public LIR_OpCall {
 private:
  int     _size;
  int     _args;
  LIR_Opr _tmp;
 public:
  LIR_OpRTCall(address addr, LIR_Opr tmp, int size, int args)
    : LIR_OpCall(lir_rtcall, addr)
    , _size(size)
    , _args(args)
    , _tmp(tmp) {}

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void print_instr() const PRODUCT_RETURN;
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpRTCall* as_OpRTCall() { return this; }

  int size() const                               { return _size; }
  int args() const                               { return _args; }
  LIR_Opr tmp() const                            { return _tmp; }
};


class LIR_OpBranch: public LIR_Op1 {
 public:
  enum LIR_Condition {
      equal
    , notEqual
    , less
    , lessEqual
    , greaterEqual
    , greater
    , belowEqual
    , aboveEqual
    , always
    , intrinsicFailed
    , unknown = -1
  };

 private:
  LIR_Condition _cond;
  Label*        _label;
  Label*        _ulabel;
  BlockBegin*   _block;  // if this is a branch to a block, this is the block
  CodeStub*     _stub;   // if this is a branch to a stub, this is the stub
  
  static void print_condition(LIR_Condition cond) PRODUCT_RETURN;
 public:
  LIR_OpBranch(LIR_Condition cond, LIR_Opr cond_reg, Label* lbl, CodeEmitInfo* info)
    : LIR_Op1(lir_branch, cond_reg, info)
    , _cond(cond)
    , _label(lbl)
    , _block(NULL)
    , _stub(NULL)
    , _ulabel(NULL) {
    assert(info == NULL || cond == LIR_OpBranch::always, "shouldn't have infos on conditional branches");
  }

  // these are temporary constructors until we start using the conditional register  
  LIR_OpBranch(LIR_Condition cond, Label* lbl, CodeEmitInfo* info)
    : LIR_Op1(lir_branch, LIR_OprFact::illegalOpr, info)
    , _cond(cond)
    , _label(lbl)
    , _block(NULL)
    , _stub(NULL)
    , _ulabel(NULL) {
    assert(info == NULL || cond == LIR_OpBranch::always, "shouldn't have infos on conditional branches");
  }

  LIR_OpBranch(LIR_Condition cond, BlockBegin* block, CodeEmitInfo* info);
  LIR_OpBranch(LIR_Condition cond, CodeStub* stub, CodeEmitInfo* info);

  LIR_OpBranch(LIR_Condition cond, Label* lbl, Label* ulbl, CodeEmitInfo* info)
    : LIR_Op1(lir_cond_float_branch, LIR_OprFact::illegalOpr, info)
    , _cond(cond)
    , _label(lbl)
    , _block(NULL)
    , _stub(NULL)
    , _ulabel(ulbl) {
    assert(info == NULL || cond == LIR_OpBranch::always, "shouldn't have infos on conditional branches");
  }

  LIR_Condition cond()        const              { return _cond;        }
  LIR_Opr       cond_reg()    const              { return in_opr();     }
  Label*        label()       const              { return _label;       }
  Label*        ulabel()      const              { return _ulabel;      }
  BlockBegin*   block()       const              { return _block;       }
  CodeStub*     stub()        const              { return _stub;       }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpBranch* as_OpBranch() { return this; }
  virtual void print_instr() const PRODUCT_RETURN;
};


class LIR_OpConvert: public LIR_Op1 {
 private:
   bool            _is_32bit;
   Bytecodes::Code _bytecode;

 public:
   LIR_OpConvert(Bytecodes::Code code, LIR_Opr opr, LIR_Opr result, bool is_32bit = false)
     : LIR_Op1(lir_convert, opr, result)
     , _is_32bit(is_32bit)
     , _bytecode(code)                           {}

  Bytecodes::Code bytecode() const               { return _bytecode; }
  bool is_32bit() const                          { return _is_32bit; }

  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpConvert* as_OpConvert() { return this; }
  virtual void print_instr() const PRODUCT_RETURN;

  static void print_bytecode(Bytecodes::Code code) PRODUCT_RETURN;
};


// LIR_OpAllocObj
class LIR_OpAllocObj : public LIR_Op1 {
 private:
  LIR_Opr _tmp1;
  LIR_Opr _tmp2;
  LIR_Opr _tmp3;
  int     _hdr_size;
  int     _obj_size;
  CodeStub* _stub;

 public:
  LIR_OpAllocObj(LIR_Opr klass, LIR_Opr result, LIR_Opr t1, LIR_Opr t2, LIR_Opr t3, int hdr_size, int obj_size, CodeStub* stub)
    : LIR_Op1(lir_alloc_object, klass, result)
    , _tmp1(t1)
    , _tmp2(t2)
    , _tmp3(t3)
    , _hdr_size(hdr_size)
    , _obj_size(obj_size)
    , _stub(stub)                                { }

  LIR_Opr klass()        const                   { return in_opr();     }
  LIR_Opr obj()          const                   { return result_opr(); }
  LIR_Opr tmp1()         const                   { return _tmp1;        }
  LIR_Opr tmp2()         const                   { return _tmp2;        }
  LIR_Opr tmp3()         const                   { return _tmp3;        }
  int     header_size()  const                   { return _hdr_size;    }
  int     object_size()  const                   { return _obj_size;    }
  CodeStub* stub()       const                   { return _stub;        }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpAllocObj * as_OpAllocObj () { return this; }
  virtual void print_instr() const PRODUCT_RETURN;
};


// LIR_OpTypeCheck
class LIR_OpTypeCheck: public LIR_Op {
 friend class LIR_Optimizer;

 private:
  LIR_Opr       _object;
  LIR_Opr       _array;
  ciKlass*      _klass;
  LIR_Opr       _tmp1;
  LIR_Opr       _tmp2;
  LIR_Opr       _tmp3;
  bool          _fast_check;
  CodeEmitInfo* _info_for_patch;
  CodeEmitInfo* _info_for_exception;
  CodeStub*     _stub;

public:
  LIR_OpTypeCheck(LIR_Code code, LIR_Opr result, LIR_Opr object, ciKlass* klass,
                  LIR_Opr tmp1, LIR_Opr tmp2, bool fast_check,
                  CodeEmitInfo* info_for_exception, CodeEmitInfo* info_for_patch, CodeStub* stub);
  LIR_OpTypeCheck(LIR_Code code, LIR_Opr object, LIR_Opr array,
                  LIR_Opr tmp1, LIR_Opr tmp2, LIR_Opr tmp3, CodeEmitInfo* info_for_exception);

  LIR_Opr object() const                         { return _object;         }
  LIR_Opr array() const                          { assert(code() == lir_store_check, "not valid"); return _array;         }
  LIR_Opr tmp1() const                           { return _tmp1;           }
  LIR_Opr tmp2() const                           { return _tmp2;           }
  LIR_Opr tmp3() const                           { return _tmp3;           }
  ciKlass* klass() const                         { assert(code() == lir_instanceof || code() == lir_checkcast, "not valid"); return _klass;          }
  bool fast_check() const                        { assert(code() == lir_instanceof || code() == lir_checkcast, "not valid"); return _fast_check;     }
  CodeEmitInfo* info_for_patch() const           { return _info_for_patch;  }
  CodeEmitInfo* info_for_exception() const       { return _info_for_exception; }
  CodeStub* stub() const                         { return _stub;           }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpTypeCheck* as_OpTypeCheck() { return this; }
  void print_instr() const PRODUCT_RETURN;
};

// LIR_Op2
class LIR_Op2: public LIR_Op {
 friend class LIR_Optimizer;

 protected:
  LIR_Opr   _opr1;
  LIR_Opr   _opr2;
  BasicType _type;
  LIR_Opr   _tmp;
  LIR_OpBranch::LIR_Condition _condition;

 public:
  LIR_Op2(LIR_Code code, LIR_OpBranch::LIR_Condition condition, LIR_Opr opr1, LIR_Opr opr2, CodeEmitInfo* info = NULL, BasicType type = T_ILLEGAL)
    : LIR_Op(code, LIR_OprFact::illegalOpr, info)
    , _opr1(opr1)
    , _opr2(opr2)
    , _type(type)
    , _condition(condition)
    , _tmp(LIR_OprFact::illegalOpr) {
    assert(code == lir_cmp, "code check");
  }

  LIR_Op2(LIR_Code code, LIR_Opr opr1, LIR_Opr opr2, LIR_Opr result = LIR_OprFact::illegalOpr,
          CodeEmitInfo* info = NULL, BasicType type = T_ILLEGAL)
    : LIR_Op(code, result, info)
    , _opr1(opr1)
    , _opr2(opr2)
    , _type(type)
    , _condition(LIR_OpBranch::unknown)
    , _tmp(LIR_OprFact::illegalOpr)              {
    assert(code != lir_cmp && is_in_range(code, begin_op2, end_op2), "code check");
  }

  LIR_Op2(LIR_Code code, LIR_Opr opr1, LIR_Opr opr2, LIR_Opr result, LIR_Opr tmp)
    : LIR_Op(code, result, NULL)
    , _opr1(opr1)
    , _opr2(opr2)
    , _type(T_ILLEGAL)
    , _condition(LIR_OpBranch::unknown)
    , _tmp(tmp) {
    assert(code != lir_cmp && is_in_range(code, begin_op2, end_op2), "code check");
  }

  LIR_Opr in_opr1() const                        { return _opr1; }
  LIR_Opr in_opr2() const                        { return _opr2; }
  BasicType type()  const		         { return _type; }
  LIR_Opr tmp_opr() const                        { return _tmp; }
  LIR_OpBranch::LIR_Condition condition() const  { assert(code() == lir_cmp, "only valid for cmp"); return _condition; }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_Op2* as_Op2() { return this; }
  virtual void print_instr() const PRODUCT_RETURN;
};

class LIR_OpAllocArray : public LIR_Op2 {
 private:
  LIR_Opr   _tmp1;
  LIR_Opr   _tmp2;
  LIR_Opr   _tmp3;
  LIR_Opr   _tmp4;
  BasicType _type;
  CodeStub* _stub;

 public:
  LIR_OpAllocArray(LIR_Opr klass, LIR_Opr len, LIR_Opr result, LIR_Opr t1, LIR_Opr t2, LIR_Opr t3, LIR_Opr t4, BasicType type, CodeStub* stub)
    : LIR_Op2(lir_alloc_array, klass, len, result)
    , _tmp1(t1)
    , _tmp2(t2)
    , _tmp3(t3)
    , _tmp4(t4)
    , _type(type)
    , _stub(stub) {}

  LIR_Opr   klass()   const                      { return in_opr1();    }
  LIR_Opr   len()     const                      { return in_opr2();    }
  LIR_Opr   obj()     const                      { return result_opr(); }
  LIR_Opr   tmp1()    const                      { return _tmp1;        }
  LIR_Opr   tmp2()    const                      { return _tmp2;        }
  LIR_Opr   tmp3()    const                      { return _tmp3;        }
  LIR_Opr   tmp4()    const                      { return _tmp4;        }
  BasicType type()    const                      { return _type;        }
  CodeStub* stub()    const                      { return _stub;        }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpAllocArray * as_OpAllocArray () { return this; }
  virtual void print_instr() const PRODUCT_RETURN;
};


class LIR_Op3: public LIR_Op {
 friend class LIR_Optimizer;

 private:
  LIR_Opr _opr1;
  LIR_Opr _opr2;
  LIR_Opr _opr3;
 public:
  LIR_Op3(LIR_Code code, LIR_Opr opr1, LIR_Opr opr2, LIR_Opr opr3, LIR_Opr result, CodeEmitInfo* info = NULL)
    : LIR_Op(code, result, info)
    , _opr1(opr1)
    , _opr2(opr2)
    , _opr3(opr3)                                { assert(is_in_range(code, begin_op3, end_op3), "code check"); }
  LIR_Opr in_opr1() const                        { return _opr1; }
  LIR_Opr in_opr2() const                        { return _opr2; }
  LIR_Opr in_opr3() const                        { return _opr3; }
  
  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_Op3* as_Op3() { return this; }
  virtual void print_instr() const PRODUCT_RETURN;
};


//--------------------------------
class LabelObj: public CompilationResourceObj {
 private:
  Label _label;
 public:
  LabelObj()                                     {}
  Label* label()                                 { return &_label; }
};


class LIR_OpLock: public LIR_Op {
 private:
  LIR_Opr _hdr;
  LIR_Opr _obj;
  LIR_Opr _lock;
  LIR_Opr _scratch;
  CodeStub* _stub;
 public:
  LIR_OpLock(LIR_Code code, LIR_Opr hdr, LIR_Opr obj, LIR_Opr lock, LIR_Opr scratch, CodeStub* stub, CodeEmitInfo* info) 
    : LIR_Op(code, LIR_OprFact::illegalOpr, info)
    , _hdr(hdr)
    , _obj(obj)
    , _lock(lock)
    , _scratch(scratch)
    , _stub(stub)                      {}

  LIR_Opr hdr_opr() const                        { return _hdr; }
  LIR_Opr obj_opr() const                        { return _obj; }
  LIR_Opr lock_opr() const                       { return _lock; }
  LIR_Opr scratch_opr() const                    { return _scratch; }
  CodeStub* stub() const                         { return _stub; }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpLock* as_OpLock() { return this; }
  void print_instr() const PRODUCT_RETURN;
};


class LIR_OpDelay: public LIR_Op {
 private:
  LIR_Op* _op;

 public:
  LIR_OpDelay(LIR_Op* op, CodeEmitInfo* info): LIR_Op(lir_delay_slot, LIR_OprFact::illegalOpr, info), _op(op) { assert(op->code() == lir_nop || LIRFillDelaySlots, "should be filling with nops"); }
  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpDelay* as_OpDelay() { return this; }
  void print_instr() const PRODUCT_RETURN;
  LIR_Op* delay_op() const { return _op; }
  CodeEmitInfo* call_info() const { return info(); }
};


// LIR_OpCompareAndSwap
class LIR_OpCompareAndSwap : public LIR_Op {
 private:
  LIR_Opr _addr;
  LIR_Opr _cmp_value;
  LIR_Opr _new_value;
  LIR_Opr _tmp1;
  LIR_Opr _tmp2;

 public:
  LIR_OpCompareAndSwap(LIR_Code code, LIR_Opr addr, LIR_Opr cmp_value, LIR_Opr new_value, LIR_Opr t1, LIR_Opr t2)
    : LIR_Op(code, LIR_OprFact::illegalOpr, NULL)  // no result, no info
    , _addr(addr)
    , _cmp_value(cmp_value)
    , _new_value(new_value)
    , _tmp1(t1)
    , _tmp2(t2)                                  { }

  LIR_Opr addr()        const                    { return _addr;  }
  LIR_Opr cmp_value()   const                    { return _cmp_value; }
  LIR_Opr new_value()   const                    { return _new_value; }
  LIR_Opr tmp1()        const                    { return _tmp1;      }
  LIR_Opr tmp2()        const                    { return _tmp2;      }

  virtual void visit(LIR_OpVisitState* visitor);
  virtual void emit_code(LIR_AbstractAssembler* masm);
  virtual LIR_OpCompareAndSwap * as_OpCompareAndSwap () { return this; }
  virtual void print_instr() const PRODUCT_RETURN;
};


//--------------------------------LIR_List---------------------------------------------------
// Maintains a list of LIR instructions (one instance of LIR_List per basic block)
// The LIR instructions are appended by the LIR_List class itself; 
// The conversion of RInfo, stack indeces, spill indeces, monitor indeces
// heap addresses to LIR operands is done here too.
//
// Notes:
// - all offsets are(should be) in bytes
// - local positions are specified with an offset, with offset 0 being local 0

class LIR_List: public CompilationResourceObj {
 private:
  CodeStubList* _stubs;
  LIR_OpList*   _insts; // instructions
  Compilation*  _compilation;
#ifdef SPARC
  bool          _delayed;
#endif // SPARC

  void append(LIR_Op* i) {
#ifdef SPARC
    if (_delayed) {
      _delayed = false;
      i = new LIR_OpDelay(i, NULL);
    }
#endif // SPARC
#ifndef PRODUCT
    if (PrintIRWithLIR) {
      _compilation->maybe_print_current_instruction();
      i->print();
    }
#endif // PRODUCT
     _insts->append(i); 
  }

 public:
  LIR_List(Compilation* compilation);

  //---------- accessors ---------------
  CodeStubList* stubs()                          { return _stubs; }
  LIR_OpList* instructions_list() const          { return _insts; }


  //---------- printing -------------
  void print_instructions() PRODUCT_RETURN;


  //---------- instructions ------------- 
  void call_opt_virtual(RInfo receiver, LIR_Opr result, address dest, CodeEmitInfo* info, StaticCallStub* stub) { append(new LIR_OpJavaCall(lir_optvirtual_call, receiver, result, dest, info, stub)); }
  void call_static(LIR_Opr result, address dest, CodeEmitInfo* info, StaticCallStub* stub)  { append(new LIR_OpJavaCall(lir_static_call, norinfo, result, dest, info, stub)); }
  void call_icvirtual(RInfo receiver, LIR_Opr result, address dest, CodeEmitInfo* info) { append(new LIR_OpJavaCall(lir_icvirtual_call, receiver, result, dest, info));  }
  void call_virtual(RInfo receiver, LIR_Opr result, intptr_t vtable_offset, CodeEmitInfo* info) { append(new LIR_OpJavaCall(lir_virtual_call, receiver, result, vtable_offset, info));  }

  void get_thread(RInfo result)                  { append(new LIR_Op0(lir_get_thread, result)); }
  void word_align()                              { append(new LIR_Op0(lir_word_align)); }
  void membar()                                  { append(new LIR_Op0(lir_membar)); }
  void membar_acquire()                          { append(new LIR_Op0(lir_membar_acquire)); }
  void membar_release()                          { append(new LIR_Op0(lir_membar_release)); }

  void nop(CodeEmitInfo* info)                   { append(new LIR_Op0(lir_nop, info)); }
  void backward_branch_target()                  { append(new LIR_Op0(lir_backwardbranch_target)); }
  void align_entry()                             { append(new LIR_Op0(lir_align_entry)); }
  void verified_entry_point()                    { append(new LIR_Op0(lir_verified_entry)); }
  void build_frame()                             { append(new LIR_Op0(lir_build_frame)); }

  void branch_destination(Label* lbl)            { append(new LIR_OpLabel(lbl)); }

  void negate(RInfo from, RInfo to)              { append(new LIR_Op1(lir_neg, LIR_OprFact::rinfo(from), LIR_OprFact::rinfo(to))); }
  void leal(LIR_Opr from, LIR_Opr result_reg)    { append(new LIR_Op1(lir_leal, from, result_reg)); }

  void push_fpu(RInfo reg)                       { append(new LIR_Op1(lir_fpu_push, LIR_OprFact::rinfo(reg))); }
  void pop_fpu (RInfo reg)                       { append(new LIR_Op1(lir_fpu_pop , LIR_OprFact::rinfo(reg))); }
  void dup_fpu(RInfo from, RInfo to)             { append(new LIR_Op1(lir_fpu_dup, LIR_OprFact::rinfo(from), LIR_OprFact::rinfo(to))); }
 
  void round32bit(RInfo reg, int locIx)          { append(new LIR_Op1(lir_round32, LIR_OprFact::rinfo(reg, T_FLOAT), LIR_OprFact::single_stack(locIx, T_FLOAT))); }
  void move(LIR_Opr src, LIR_Opr dst, CodeEmitInfo* info = NULL) { append(new LIR_Op1(lir_move, src, dst, dst->type(), LIR_Op1::patch_none, info)); }
  void move(LIR_Address* src, LIR_Opr dst, CodeEmitInfo* info = NULL) { append(new LIR_Op1(lir_move, LIR_OprFact::address(src), dst, dst->type(), LIR_Op1::patch_none, info)); }
  void move(LIR_Opr src, LIR_Address* dst, CodeEmitInfo* info = NULL) { append(new LIR_Op1(lir_move, src, LIR_OprFact::address(dst), src->type(), LIR_Op1::patch_none, info)); }
  void reg2reg(RInfo fromReg, RInfo toReg, BasicType type)       { append(new LIR_Op1(lir_move, LIR_OprFact::rinfo(fromReg, type), LIR_OprFact::rinfo(toReg, type))); }
  void reg2stack(RInfo reg, LIR_Opr dst)         { append(new LIR_Op1(lir_move, LIR_OprFact::rinfo(reg, dst->type()), dst)); }
  void reg2single_stack(RInfo reg, int locIx, BasicType type)    { append(new LIR_Op1(lir_move, LIR_OprFact::rinfo(reg, type), LIR_OprFact::single_stack(locIx, type), type)); }
  void reg2double_stack(RInfo reg, int locIx, BasicType type)    { append(new LIR_Op1(lir_move, LIR_OprFact::rinfo(reg, type), LIR_OprFact::double_stack(locIx, type), type)); }
  void stack2reg(LIR_Opr src, RInfo toReg)       { append(new LIR_Op1(lir_move, src, LIR_OprFact::rinfo(toReg, src->type()))); }
  void single_stack2reg(int locIx, RInfo reg, BasicType type); 
  void double_stack2reg(int locIx, RInfo reg, BasicType type); 
  void int2stack(jint i, int locIx)              { append(new LIR_Op1(lir_move, LIR_OprFact::intConst(i),    LIR_OprFact::single_stack(locIx, T_INT))); }
  void long2stack(jlong l, int locIx)            { append(new LIR_Op1(lir_move, LIR_OprFact::longConst(l),   LIR_OprFact::double_stack(locIx, T_LONG))); }
  void oop2stack(jobject o, int locIx)           { append(new LIR_Op1(lir_move, LIR_OprFact::oopConst(o),    LIR_OprFact::single_stack(locIx, T_OBJECT))); }

  void int2reg  (jint i, RInfo reg)              { append(new LIR_Op1(lir_move, LIR_OprFact::intConst(i),    LIR_OprFact::rinfo(reg)));   }
  void oop2reg  (jobject o, RInfo reg)           { append(new LIR_Op1(lir_move, LIR_OprFact::oopConst(o),    LIR_OprFact::rinfo(reg, T_OBJECT)));   }
  void float2reg(jfloat f, RInfo reg)            { append(new LIR_Op1(lir_move, LIR_OprFact::floatConst(f),  LIR_OprFact::rinfo(reg)));   }
  void double2reg(jdouble d, RInfo reg)          { append(new LIR_Op1(lir_move, LIR_OprFact::doubleConst(d), LIR_OprFact::rinfo(reg)));   }
  void oop2reg_patch(jobject o, RInfo reg, CodeEmitInfo* info);

  void return_op(LIR_Opr result)                 { append(new LIR_Op1(lir_return, result)); }
 
  void safepoint(RInfo tmp, CodeEmitInfo* info)  { append(new LIR_Op1(lir_safepoint, LIR_OprFact::rinfo(tmp), info)); }
 
  void convert(Bytecodes::Code code, LIR_Opr left, LIR_Opr dst, bool is_32bit = false) { append(new LIR_OpConvert(code, left, dst, is_32bit)); }

  void logical_and(RInfo left, RInfo right, RInfo dst)     { append(new LIR_Op2(lir_logic_and, LIR_OprFact::rinfo(left), LIR_OprFact::rinfo(right), LIR_OprFact::rinfo(dst))); }
  void logical_or(RInfo left, RInfo right, RInfo dst)      { append(new LIR_Op2(lir_logic_or,  LIR_OprFact::rinfo(left), LIR_OprFact::rinfo(right), LIR_OprFact::rinfo(dst))); }
  void logical_orcc(RInfo left, RInfo right, RInfo dst)    { append(new LIR_Op2(lir_logic_orcc,  LIR_OprFact::rinfo(left), LIR_OprFact::rinfo(right), LIR_OprFact::rinfo(dst))); }
  void logical_xor(RInfo left, RInfo right, RInfo dst)     { append(new LIR_Op2(lir_logic_xor, LIR_OprFact::rinfo(left), LIR_OprFact::rinfo(right), LIR_OprFact::rinfo(dst))); }
  void logical_and(RInfo left, LIR_Opr right, RInfo dst)   { append(new LIR_Op2(lir_logic_and, LIR_OprFact::rinfo(left), right, LIR_OprFact::rinfo(dst))); }
  void logical_or(RInfo left,  LIR_Opr right, RInfo dst)   { append(new LIR_Op2(lir_logic_or,  LIR_OprFact::rinfo(left), right, LIR_OprFact::rinfo(dst))); }
  void logical_orcc(RInfo left,  LIR_Opr right, RInfo dst) { append(new LIR_Op2(lir_logic_orcc,  LIR_OprFact::rinfo(left), right, LIR_OprFact::rinfo(dst))); }
  void logical_xor(RInfo left, LIR_Opr right, RInfo dst)   { append(new LIR_Op2(lir_logic_xor, LIR_OprFact::rinfo(left), right, LIR_OprFact::rinfo(dst))); }

  void null_check(RInfo r, CodeEmitInfo* info)             { append(new LIR_Op1(lir_null_check,  LIR_OprFact::rinfo(r, T_OBJECT), info)); }
  void throw_exception(RInfo exceptionPC, RInfo exceptionOop, CodeEmitInfo* info) { append(new LIR_Op2(lir_throw, LIR_OprFact::rinfo(exceptionPC), LIR_OprFact::rinfo(exceptionOop), LIR_OprFact::illegalOpr, info)); }
  void unwind_exception(RInfo exceptionPC, RInfo exceptionOop, CodeEmitInfo* info) { append(new LIR_Op2(lir_unwind, LIR_OprFact::rinfo(exceptionPC), LIR_OprFact::rinfo(exceptionOop), LIR_OprFact::illegalOpr, info)); }

  void push(LIR_Opr opr)                                   { append(new LIR_Op1(lir_push, opr)); }

  void push_reg(RInfo reg)                                 { append(new LIR_Op1(lir_push, LIR_OprFact::rinfo(reg)));        }
  void push_single_local(int local_ix, BasicType type)     { append(new LIR_Op1(lir_push, LIR_OprFact::single_stack(local_ix, type)));   }
  void push_double_local(int local_ix, BasicType type)     { append(new LIR_Op1(lir_push, LIR_OprFact::double_stack(local_ix, type)));   }
  void push_jint(jint i)                                   { append(new LIR_Op1(lir_push, LIR_OprFact::intConst(i)));       }
  void push_oop(jobject o)                                 { append(new LIR_Op1(lir_push, LIR_OprFact::oopConst(o)));       }
  void pop_reg(RInfo reg)                                  { append(new LIR_Op1(lir_pop,  LIR_OprFact::rinfo(reg)));        }

  void unverified_entry_point(RInfo receiver, RInfo ic_klass) { append(new LIR_Op2(lir_unverified_entry, LIR_OprFact::rinfo(receiver), LIR_OprFact::rinfo(ic_klass))); }
  void cmp(LIR_OpBranch::LIR_Condition condition, LIR_Opr left, LIR_Opr right, CodeEmitInfo* info = NULL) {
    append(new LIR_Op2(lir_cmp, condition, left, right, info));
  }
  void cmp(LIR_OpBranch::LIR_Condition condition, LIR_Opr left, int right, CodeEmitInfo* info = NULL) {
    cmp(condition, left, LIR_OprFact::intConst(right), info);
  }

  void cmp_mem_int(LIR_OpBranch::LIR_Condition condition, RInfo base, int disp, int c, CodeEmitInfo* info);
  void cmp_reg_mem(LIR_OpBranch::LIR_Condition condition, RInfo reg, LIR_Address* addr, BasicType type, CodeEmitInfo* info);

  void cas_long(LIR_Opr addr, LIR_Opr cmp_value, LIR_Opr new_value, LIR_Opr t1, LIR_Opr t2);
  void cas_obj(LIR_Opr addr, LIR_Opr cmp_value, LIR_Opr new_value, LIR_Opr t1, LIR_Opr t2);
  void cas_int(LIR_Opr addr, LIR_Opr cmp_value, LIR_Opr new_value, LIR_Opr t1, LIR_Opr t2);

  void sqrt(RInfo from, RInfo to, RInfo tmp)               { append(new LIR_Op2(lir_sqrt, LIR_OprFact::rinfo(from), LIR_OprFact::rinfo(tmp), LIR_OprFact::rinfo(to))); }
  void sin (RInfo from, RInfo to, RInfo tmp)               { append(new LIR_Op2(lir_sin , LIR_OprFact::rinfo(from), LIR_OprFact::rinfo(tmp), LIR_OprFact::rinfo(to))); }
  void cos (RInfo from, RInfo to, RInfo tmp)               { append(new LIR_Op2(lir_cos , LIR_OprFact::rinfo(from), LIR_OprFact::rinfo(tmp), LIR_OprFact::rinfo(to))); }
 
  void add (RInfo left, int right, RInfo res)              { append(new LIR_Op2(lir_add, LIR_OprFact::rinfo(left), LIR_OprFact::intConst(right), LIR_OprFact::rinfo(res)));       }
  void add (LIR_Opr left, LIR_Opr right, LIR_Opr res)      { append(new LIR_Op2(lir_add, left, right, res)); }
  void sub (LIR_Opr left, LIR_Opr right, LIR_Opr res, CodeEmitInfo* info = NULL) { append(new LIR_Op2(lir_sub, left, right, res, info)); }
  void mul (LIR_Opr left, LIR_Opr right, LIR_Opr res) { append(new LIR_Op2(lir_mul, left, right, res)); }
  void mul_strictfp (LIR_Opr left, LIR_Opr right, LIR_Opr res) { append(new LIR_Op2(lir_mul_strictfp, left, right, res)); }
  void mul3(LIR_Opr left, LIR_Opr right, LIR_Opr tmp, LIR_Opr res) { append(new LIR_Op3(lir_mul, left, right, tmp, res)); }
  void div (LIR_Opr left, LIR_Opr right, LIR_Opr res, CodeEmitInfo* info = NULL)      { append(new LIR_Op2(lir_div, left, right, res, info)); }
  void div_strictfp (LIR_Opr left, LIR_Opr right, LIR_Opr res) { append(new LIR_Op2(lir_div_strictfp, left, right, res)); }
  void rem (LIR_Opr left, LIR_Opr right, LIR_Opr res, CodeEmitInfo* info = NULL)      { append(new LIR_Op2(lir_rem, left, right, res, info)); }

  void volatile_load_mem_reg(RInfo base, int offset_in_bytes, RInfo dst,  BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);
  void volatile_load_unsafe_reg(RInfo base, RInfo offset, RInfo dst, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code);

  void load_mem_reg(RInfo base, int offset_in_bytes, RInfo dst,  BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);
  void load_mem_reg(LIR_Address* addr, RInfo dst,  BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);
  void load(LIR_Address* addr, LIR_Opr src, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);

  void store_mem_int(jint v,    RInfo base, int offset_in_bytes, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);
  void store_mem_oop(jobject o, RInfo base, int offset_in_bytes, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);
  void store_mem_reg(RInfo src, RInfo base, int offset_in_bytes, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);
  void store_mem_reg(RInfo src, LIR_Address* addr, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);
  void store(LIR_Opr src, LIR_Address* addr, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);
  void volatile_store_mem_reg(RInfo src, RInfo base, int offset_in_bytes, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code = LIR_Op1::patch_none);
  void volatile_store_unsafe_reg(RInfo src, RInfo base, RInfo offset, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code);

  void load_array(LIR_Address* addr, BasicType t, RInfo dst, CodeEmitInfo* info);
  void store_array(RInfo   src, LIR_Address* addr, BasicType t, CodeEmitInfo* info);
  void store_array(jint    src, LIR_Address* addr, BasicType t, CodeEmitInfo* info);
  void store_array(jobject src, LIR_Address* addr, BasicType t, CodeEmitInfo* info);

  void idiv(RInfo left, RInfo right, RInfo res, RInfo tmp, CodeEmitInfo* info);
  void idiv(RInfo left, int   right, RInfo res, RInfo tmp, CodeEmitInfo* info);
  void irem(RInfo left, RInfo right, RInfo res, RInfo tmp, CodeEmitInfo* info);
  void irem(RInfo left, int   right, RInfo res, RInfo tmp, CodeEmitInfo* info);

  void allocate_object(RInfo dst, RInfo t1, RInfo t2, RInfo t3, int header_size, int object_size, RInfo klass, CodeStub* stub);
  void allocate_array(RInfo dst, RInfo len, RInfo t1,RInfo t2, RInfo t3,RInfo t4, BasicType type, RInfo klass, CodeStub* stub);

  // jump is an unconditional branch which may be a safepoint
  void jump(BlockBegin* block, CodeEmitInfo* info);
  // branch is for conditional branches which are not allowed to be
  // safepoints since it's not guaranteed that conditions codes are saved
  // (they aren't on sparc)
  void branch(LIR_OpBranch::LIR_Condition cond, Label* lbl);
  void branch(LIR_OpBranch::LIR_Condition cond, BlockBegin* block);
  void branch(LIR_OpBranch::LIR_Condition cond, CodeStub* stub);
  void branch_float(LIR_OpBranch::LIR_Condition cond, Label* lbl, Label* unordered);

  void shift_left(LIR_Opr value, LIR_Opr count, LIR_Opr dst, LIR_Opr tmp);
  void shift_left_long(RInfo value, RInfo count, RInfo dst, RInfo tmp);
  void shift_right(LIR_Opr value, LIR_Opr count, LIR_Opr dst, LIR_Opr tmp);
  void unsigned_shift_right(LIR_Opr value, LIR_Opr count, LIR_Opr dst, LIR_Opr tmp);

  void shift_left(RInfo value, RInfo count, RInfo dst, RInfo tmp) { shift_left(LIR_OprFact::rinfo(value), LIR_OprFact::rinfo(count), LIR_OprFact::rinfo(dst), LIR_OprFact::rinfo(tmp)); }
  void shift_left_long(RInfo value, int count, RInfo dst)       { append(new LIR_Op2(lir_shlx,  LIR_OprFact::rinfo(value), LIR_OprFact::intConst(count), LIR_OprFact::rinfo(dst))); }
  void shift_right(RInfo value, RInfo count, RInfo dst, RInfo tmp) { shift_right(LIR_OprFact::rinfo(value), LIR_OprFact::rinfo(count), LIR_OprFact::rinfo(dst), LIR_OprFact::rinfo(tmp)); }
  void unsigned_shift_right(RInfo value, RInfo count, RInfo dst, RInfo tmp) { unsigned_shift_right(LIR_OprFact::rinfo(value), LIR_OprFact::rinfo(count), LIR_OprFact::rinfo(dst), LIR_OprFact::rinfo(tmp)); }

  void shift_left(LIR_Opr value, int count, LIR_Opr dst)       { shift_left(value, LIR_OprFact::intConst(count), dst, LIR_OprFact::illegalOpr); }
  void shift_right(LIR_Opr value, int count, LIR_Opr dst)      { shift_right(value, LIR_OprFact::intConst(count), dst, LIR_OprFact::illegalOpr); }
  void unsigned_shift_right(LIR_Opr value, int count, LIR_Opr dst) { unsigned_shift_right(value, LIR_OprFact::intConst(count), dst, LIR_OprFact::illegalOpr); }

  void shift_left(RInfo value, int count, RInfo dst)       { shift_left(LIR_OprFact::rinfo(value), LIR_OprFact::intConst(count), LIR_OprFact::rinfo(dst), LIR_OprFact::illegalOpr); }
  void shift_right(RInfo value, int count, RInfo dst)      { shift_right(LIR_OprFact::rinfo(value), LIR_OprFact::intConst(count), LIR_OprFact::rinfo(dst), LIR_OprFact::illegalOpr); }
  void unsigned_shift_right(RInfo value, int count, RInfo dst) { unsigned_shift_right(LIR_OprFact::rinfo(value), LIR_OprFact::intConst(count), LIR_OprFact::rinfo(dst), LIR_OprFact::illegalOpr); }

  void lcmp2int(RInfo left, RInfo right, RInfo dst)        { append(new LIR_Op2(lir_cmp_l2i,  LIR_OprFact::rinfo(left), LIR_OprFact::rinfo(right), LIR_OprFact::rinfo(dst))); }
  void fcmp2int(RInfo left, RInfo right, RInfo dst, bool is_unordered_less);

  void call_runtime_leaf(address routine, RInfo tmp, int size, int args) { append(new LIR_OpRTCall(routine, LIR_OprFact::rinfo(tmp), size, args)); }

  void load_stack_address_monitor(int monitor_ix, RInfo dst)  { append(new LIR_Op1(lir_monaddr, LIR_OprFact::intConst(monitor_ix), LIR_OprFact::rinfo(dst))); }
  void unlock_object(RInfo hdr, RInfo obj, RInfo lock, CodeStub* stub);
  void lock_object(RInfo hdr, RInfo obj, RInfo lock, RInfo scratch, CodeStub* stub, CodeEmitInfo* info);

  void set_24bit_fpu()                                               { append(new LIR_Op0(lir_24bit_FPU )); }
  void restore_fpu()                                                 { append(new LIR_Op0(lir_reset_FPU )); }
  void breakpoint()                                                  { append(new LIR_Op0(lir_breakpoint)); }

  void allocate_multi_array(RInfo dst, int rank, CodeEmitInfo* info) { append(new LIR_Op1(lir_new_multi, LIR_OprFact::intConst(rank), LIR_OprFact::rinfo(dst), T_ILLEGAL, LIR_Op1::patch_none, info)); }

#ifdef SPARC
  LIR_List* delayed() { assert(_delayed == false, "improper use"); _delayed = true; return this; }
#endif // SPARC

  void arraycopy(LIR_Opr src, LIR_Opr src_pos, LIR_Opr dst, LIR_Opr dst_pos, LIR_Opr length, LIR_Opr tmp, ciArrayKlass* expected_type, int flags, CodeEmitInfo* info) { append(new LIR_OpArrayCopy(src, src_pos, dst, dst_pos, length, tmp, expected_type, flags, info)); }

  void fpop_raw()                                { append(new LIR_Op0(lir_fpop_raw)); }
  void set_fpu_stack_empty()                     { append(new LIR_Op0(lir_empty_fpu)); }

  void jvmpi_method_enter(CodeEmitInfo* info)    { append (new LIR_Op0(lir_jvmpi_method_enter, info)); }
  void jvmpi_method_exit()                       { append (new LIR_Op0(lir_jvmpi_method_exit));        }


  void checkcast (LIR_Opr result, LIR_Opr object, ciKlass* klass,
                  LIR_Opr tmp1, LIR_Opr tmp2, bool fast_check,
                  CodeEmitInfo* info_for_exception, CodeEmitInfo* info_for_patch, CodeStub* stub);
  void instanceof(LIR_Opr result, LIR_Opr object, ciKlass* klass, LIR_Opr tmp1, LIR_Opr tmp2, bool fast_check, CodeEmitInfo* info_for_patch);
  void store_check(LIR_Opr object, LIR_Opr array, LIR_Opr tmp1, LIR_Opr tmp2, LIR_Opr tmp3, CodeEmitInfo* info_for_exception);
};

void print_LIR(BlockList* blocks);
void check_LIR();


//
// LIR_OpVisitState is used for manipulating LIR_Ops in an abstract way.
// Calling a LIR_Op's visit function with a LIR_OpVisitState causes
// information about the input, output and temporaries used by the
// op to be recorded.  It also records whether the op has call semantics
// and also records all the CodeEmitInfos used by this op.
//


class LIR_OpVisitState: public StackObj {
 public:
  typedef enum { inputMode, firstMode = inputMode, tempMode, outputMode, numModes, invalidMode = -1 } OprMode;

 private:
  LIR_Op*          _op;
  LIR_OprRefList   _oprs[numModes];
  CodeEmitInfoList _info;
  LIR_Opr          _rinfo[10];
  int              _rinfo_count;
  bool             _has_call;

  const LIR_OprRefList& oprs(OprMode mode) const {
    assert(mode >= 0 && mode < numModes, "bad mode");
    return _oprs[mode];
  }

  LIR_OprRefList& oprs(OprMode mode) {
    assert(mode >= 0 && mode < numModes, "bad mode");
    return _oprs[mode];
  }


 public:
  LIR_OpVisitState() {
    reset();
  }

  void set_op(LIR_Op* op) { reset(); _op = op; }
  LIR_Op* op() const      { return _op; }

  void set_has_call()     { _has_call = true; }
  bool has_call() const   { return _has_call; }

  void reset() {
    _op = NULL;
    _has_call = false;
    oprs(inputMode).clear();
    oprs(outputMode).clear();
    oprs(tempMode).clear();
    _info.clear();
    _rinfo_count = 0;
  }

  void append(LIR_Opr& opr, OprMode mode) {
    assert(mode >= 0 && mode < numModes, "bad mode");
    _oprs[mode].append(&opr);
  }

  void append(RInfo rinfo) {
    // RInfos are immutable
    assert(_rinfo_count < sizeof(_rinfo) / sizeof(RInfo), "full");
    _rinfo[_rinfo_count] = LIR_OprFact::rinfo(rinfo);
    append(_rinfo[_rinfo_count], tempMode);
    _rinfo_count++;
  }

  void append(CodeEmitInfo* info) {
    _info.append(info);
  }

  int opr_count(OprMode mode) const {
    return oprs(mode).length();
  }

  LIR_Opr opr_at(OprMode mode, int index) {
    return *oprs(mode).at(index);
  }
                                           
  void set_opr_at(OprMode mode, int index, LIR_Opr opr) {
    *(oprs(mode).at(index)) = opr;
  }

  int info_count() const { return _info.length(); }
  CodeEmitInfo* info_at(int index) {
    return _info[index];
  }

  void visit(LIR_Op* op) {
    // copy information from the LIR_Op
    reset();
    set_op(op);
    op->visit(this);
  }

  // LIR_Op visitor functions use these to fill in the state
  void do_input(LIR_Opr& opr)  { append(opr, LIR_OpVisitState::inputMode);  }
  void do_output(LIR_Opr& opr) { append(opr, LIR_OpVisitState::outputMode); }
  void do_temp(LIR_Opr& opr)   { append(opr, LIR_OpVisitState::tempMode);   }
  void do_rinfo(RInfo rinfo)   { if (rinfo.is_valid()) append(rinfo); }
  void do_info(CodeEmitInfo*& info)       { append(info); }
  void do_stub_info(CodeEmitInfo*& info)  { append(info); }
  void do_call()                          { set_has_call(); }
};
