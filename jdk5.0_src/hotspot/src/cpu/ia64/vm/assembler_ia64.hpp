#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)assembler_ia64.hpp	1.78 04/03/08 11:15:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Contains all the definitions needed for ia64 assembly code generation.

// Address is an abstraction used to represent a memory location
// using any of the ia64 addressing modes with one object.
//
// Note: A register location is represented via a Register, not
//       via an address for efficiency & simplicity reasons.

//=============================================================================
// Register Usage
//=============================================================================

// ABI Definitions
const Register GP      = GR1;
const Register GR_RET  = GR8;
const Register GR_RET0 = GR8;
const Register GR_RET1 = GR9;
const Register GR_RET2 = GR10;
const Register GR_RET3 = GR11;
const Register SP      = GR12;
const Register TP      = GR13;

const Register GR2_SCRATCH = GR2;
const Register GR3_SCRATCH = GR3;

const Register GR4_PRESERVED = GR4;
const Register GR5_PRESERVED = GR5;
const Register GR6_PRESERVED = GR6;
const Register GR7_PRESERVED = GR7;

const Register GR14_SCRATCH = GR14;
const Register GR15_SCRATCH = GR15;
const Register GR16_SCRATCH = GR16;
const Register GR17_SCRATCH = GR17;
const Register GR18_SCRATCH = GR18;
const Register GR19_SCRATCH = GR19;
const Register GR20_SCRATCH = GR20;
const Register GR21_SCRATCH = GR21;
const Register GR22_SCRATCH = GR22;
const Register GR23_SCRATCH = GR23;
const Register GR24_SCRATCH = GR24;
const Register GR25_SCRATCH = GR25;
const Register GR26_SCRATCH = GR26;
const Register GR27_SCRATCH = GR27;
const Register GR28_SCRATCH = GR28;
const Register GR29_SCRATCH = GR29;
const Register GR30_SCRATCH = GR30;
const Register GR31_SCRATCH = GR31;

const Register GR_I0 = GR32;
const Register GR_I1 = GR33;
const Register GR_I2 = GR34;
const Register GR_I3 = GR35;
const Register GR_I4 = GR36;
const Register GR_I5 = GR37;
const Register GR_I6 = GR38;
const Register GR_I7 = GR39;


const FloatRegister FR2_PRESERVED = FR2;
const FloatRegister FR3_PRESERVED = FR3;
const FloatRegister FR4_PRESERVED = FR4;
const FloatRegister FR5_PRESERVED = FR5;

const FloatRegister FR6_SCRATCH  = FR6;
const FloatRegister FR7_SCRATCH  = FR7;
const FloatRegister FR8_SCRATCH  = FR8;
const FloatRegister FR9_SCRATCH  = FR9;
const FloatRegister FR10_SCRATCH = FR10;
const FloatRegister FR11_SCRATCH = FR11;
const FloatRegister FR12_SCRATCH = FR12;
const FloatRegister FR13_SCRATCH = FR13;
const FloatRegister FR14_SCRATCH = FR14;
const FloatRegister FR15_SCRATCH = FR15;

const FloatRegister FR16_PRESERVED = FR16;
const FloatRegister FR17_PRESERVED = FR17;
const FloatRegister FR18_PRESERVED = FR18;
const FloatRegister FR19_PRESERVED = FR19;
const FloatRegister FR20_PRESERVED = FR20;
const FloatRegister FR21_PRESERVED = FR21;
const FloatRegister FR22_PRESERVED = FR22;
const FloatRegister FR23_PRESERVED = FR23;
const FloatRegister FR24_PRESERVED = FR24;
const FloatRegister FR25_PRESERVED = FR25;
const FloatRegister FR26_PRESERVED = FR26;
const FloatRegister FR27_PRESERVED = FR27;
const FloatRegister FR28_PRESERVED = FR28;
const FloatRegister FR29_PRESERVED = FR29;
const FloatRegister FR30_PRESERVED = FR30;
const FloatRegister FR31_PRESERVED = FR31;

const FloatRegister FR_RET = FR8;

const FloatRegister FR_I0 = FR8;
const FloatRegister FR_I1 = FR9;
const FloatRegister FR_I2 = FR10;
const FloatRegister FR_I3 = FR11;
const FloatRegister FR_I4 = FR12;
const FloatRegister FR_I5 = FR13;
const FloatRegister FR_I6 = FR14;
const FloatRegister FR_I7 = FR15;

const FloatRegister FR_O0 = FR8;
const FloatRegister FR_O1 = FR9;
const FloatRegister FR_O2 = FR10;
const FloatRegister FR_O3 = FR11;
const FloatRegister FR_O4 = FR12;
const FloatRegister FR_O5 = FR13;
const FloatRegister FR_O6 = FR14;
const FloatRegister FR_O7 = FR15;


const PredicateRegister PR1_PRESERVED = PR1;
const PredicateRegister PR2_PRESERVED = PR2;
const PredicateRegister PR3_PRESERVED = PR3;
const PredicateRegister PR4_PRESERVED = PR4;
const PredicateRegister PR5_PRESERVED = PR5;

const PredicateRegister PR6_SCRATCH  = PR6;
const PredicateRegister PR7_SCRATCH  = PR7;
const PredicateRegister PR8_SCRATCH  = PR8;
const PredicateRegister PR9_SCRATCH  = PR9;
const PredicateRegister PR10_SCRATCH = PR10;
const PredicateRegister PR11_SCRATCH = PR11;
const PredicateRegister PR12_SCRATCH = PR12;
const PredicateRegister PR13_SCRATCH = PR13;
const PredicateRegister PR14_SCRATCH = PR14;
const PredicateRegister PR15_SCRATCH = PR15;


const BranchRegister RP = BR0;

const BranchRegister BR1_PRESERVED = BR1;
const BranchRegister BR2_PRESERVED = BR2;
const BranchRegister BR3_PRESERVED = BR3;
const BranchRegister BR4_PRESERVED = BR4;
const BranchRegister BR5_PRESERVED = BR5;

const BranchRegister BR6_SCRATCH = BR6;
const BranchRegister BR7_SCRATCH = BR7;


// Definitions common to both interpreter and compilers
const Register GR4_thread          = GR4;
const Register GR27_method         = GR27;
const Register GR5_poll_page_addr  = GR5;
const Register GR6_caller_BSP      = GR6;
const Register GR7_reg_stack_limit = GR7;

// Inline Cache register usage
const Register GR30_compiler_method_oop_reg = GR30;
const Register GR27_inline_cache_reg        = GR27;

// Interpreter Definitions
const Register GR_L0  = GR40;
const Register GR_L1  = GR41;
const Register GR_L2  = GR42;
const Register GR_L3  = GR43;
const Register GR_L4  = GR44;
const Register GR_L5  = GR45;
const Register GR_L6  = GR46;
const Register GR_L7  = GR47;
const Register GR_L8  = GR48;
const Register GR_L9  = GR49;
const Register GR_L10 = GR50;
const Register GR_L11 = GR51;
const Register GR_L12 = GR52;
const Register GR_L13 = GR53;
const Register GR_L14 = GR54;
const Register GR_L15 = GR55;
const Register GR_O0  = GR56;
const Register GR_O1  = GR57;
const Register GR_O2  = GR58;
const Register GR_O3  = GR59;
const Register GR_O4  = GR60;
const Register GR_O5  = GR61;
const Register GR_O6  = GR62;
const Register GR_O7  = GR63;

// Standard Local register assignements. Any generated code (compiler or by hand) 
// that must stack walked must use this layout. This numbering also assumes that
// these routines always allocate 8 input registers.
// NEED CLEANUP (Renumber to make contiguous)
// 
const Register GR_Lsave_SP    = GR_L0;
const Register GR_Lsave_PFS   = GR_L1;
const Register GR_Lsave_RP    = GR_L2;

// These are standard location but are not required for stack walking and so can
// be overloaded if the routine does not need them.

const Register GR_Lsave_caller_BSP = GR_L3;

const Register GR_Lsave_UNAT  = GR_L4;
const Register GR_Lsave_GP    = GR_L5;
const Register GR_Lsave_LC    = GR_L6;
const Register GR_Lsave_PR    = GR_L7;

// Entry frame local register definitions 

const Register GR_entry_frame_GR4 = GR_Lsave_caller_BSP;
const Register GR_entry_frame_TOS = GR_Lsave_GP;
const Register GR_entry_frame_GR5 = GR_L8;
const Register GR_entry_frame_GR6 = GR_Lsave_UNAT;
const Register GR_entry_frame_GR7 = GR_Lsave_PR;

// Compiler jni frame local register deinitions

const Register GR_jni_save_RSC = GR_Lsave_UNAT;
const Register GR_jni_mod_RSC  = GR_Lsave_LC;

// Interpreter frame register definitions

const Register GR_Itos        = GR_I0;
const Register GR_Iprev_state = GR_I1;
const Register GR_Ilocals     = GR_I2; // Not really inbound interpreter computes it

const Register GR_Otos        = GR_O0;
const Register GR_Oprev_state = GR_O1;
// const Register GR_Olocals     = GR_O2; // Not

const Register GR_Lstate      = GR_L8;

const Register GR_Lscratch0   = GR_L9;
const Register GR_Lmethod_addr= GR_L10;

// exception handling register definitions

const Register GR8_exception   = GR_RET;
const Register GR9_issuing_pc  = GR_RET1;


//=============================================================================
// IA64 Instruction
//=============================================================================

typedef uint64_t uint41_t;

class Inst VALUE_OBJ_CLASS_SPEC {
public:
  // Different kinds of instructions.
  // Note that the ordering is important!  Sorting the instructions by this
  // ordering allows for simple matching of units vs. legal bundling.
  enum Kind {
    M_inst,
    F_inst,
    I_inst,
    B_inst,
    L_inst,
    X_inst,
    A_inst,
    Kind_count,
    Kind_none = Kind_count
  };

  // Different kinds of execution units.  Note that
  // the values match the corresponding Kind.
  enum Unit {
    M_unit    = M_inst,
    F_unit    = F_inst,
    I_unit    = I_inst,
    B_unit    = B_inst,
    L_unit    = L_inst,
    X_unit    = X_inst,
    Unit_count,
    Unit_none = Unit_count
  };

  // Branch displacement classes, according to
  // how the displacement is encoded.
  enum ImmedClass {
    I20, // I20, M20, M21
    M22, // M22, M23, B1, B2, B3, B6
    F14, // F14
    X2,  // X2
    X3   // X3, X4
  };

protected:
  // Registers read and written by the instruction
  RegisterState _read;
  RegisterState _write;

  // Little-endian
  struct {
    uint64_t _bits          : 41, // Bits for the instruction
                            : 10,
             _br_fast_write :  1, // Branch register can be written in same cycle as fast read
             _br_fast_read  :  1, // Branch register can be read in same cycle as fast write
             _pr_fast_write :  1, // Predicate register can be written in same cycle as fast read
             _pr_fast_read  :  1, // Predicate register can be read in same cycle as fast write
             _is_ordered    :  1, // Indicates this instruction is
                                  // ordered within a bundle
             _kind          :  8; // Instruction kind
  };

public:
  Inst(uint41_t bits, Kind kind, bool is_ordered = false)
    : _bits(bits),
      _br_fast_write(false), _br_fast_read(false), _pr_fast_write(false), _pr_fast_read(false),
      _kind(kind), _is_ordered(is_ordered)
  {}

  uint41_t      bits() const { return _bits; }
  Kind          kind() const { return (Kind)_kind; }
  bool    is_ordered() const { return (bool)_is_ordered; }
  bool br_fast_write() const { return (bool)_br_fast_write; }
  bool  br_fast_read() const { return (bool)_br_fast_read; }
  bool pr_fast_write() const { return (bool)_pr_fast_write; }
  bool  pr_fast_read() const { return (bool)_pr_fast_read; }

  // Indicate if this instruction must appear in an ordered manner
  // in the bundle.
  void set_is_ordered()      { _is_ordered = true; }

  // Indicate if this permits branch registers to be written and
  // read in the same cycle
  void set_br_fast_write()   { _br_fast_write = true; }
  void set_br_fast_read()    { _br_fast_read  = true; }

  // Indicate if this permits predicate registers to be written and
  // read in the same cycle
  void set_pr_fast_write()   { _pr_fast_write = true; }
  void set_pr_fast_read()    { _pr_fast_read  = true; }

  // Indicate register is read
  void read_gr(uint i) { _read.set_gr(i); }
  void read_fr(uint i) { _read.set_fr(i); }
  void read_pr(uint i) { _read.set_pr(i); }
  void read_br(uint i) { _read.set_br(i); }
  void read_ar(uint i) { _read.set_ar(i); }

  // Indicate register is modified
  void modify_gr(uint i) { _read.set_gr(i); _write.set_gr(i); }
  void modify_fr(uint i) { _read.set_fr(i); _write.set_fr(i); }
  void modify_pr(uint i) { _read.set_pr(i); _write.set_pr(i); }
  void modify_br(uint i) { _read.set_br(i); _write.set_br(i); }
  void modify_ar(uint i) { _read.set_ar(i); _write.set_ar(i); }

  // Indicate register is written
  void write_gr(uint i) { _write.set_gr(i); }
  void write_fr(uint i) { _write.set_fr(i); }
  void write_pr(uint i) { _write.set_pr(i); }
  void write_br(uint i) { _write.set_br(i); }
  void write_ar(uint i) { _write.set_ar(i); }

  // Note reads of registers
  void read(Register            gr) { read_gr(gr->encoding()); }
  void read(FloatRegister       fr) { read_fr(fr->encoding()); }
  void read(PredicateRegister   pr) { read_pr(pr->encoding()); }
  void read(BranchRegister      br) { read_br(br->encoding()); }
  void read(ApplicationRegister ar) { read_ar(ar->encoding()); }

  // Note reads of register sets
  void read(PredicateRegisterImplMask mask) { _read.or_mask(mask); }

  // Note writes of registers
  void write(Register            gr) { write_gr(gr->encoding()); }
  void write(FloatRegister       fr) { write_fr(fr->encoding()); }
  void write(PredicateRegister   pr) { write_pr(pr->encoding()); }
  void write(BranchRegister      br) { write_br(br->encoding()); }
  void write(ApplicationRegister ar) { write_ar(ar->encoding()); }

  // Note writes of register sets
  void write(PredicateRegisterImplMask mask) { _write.or_mask(mask); }

  // Note read/writes of registers
  void modify(Register            gr) { modify_gr(gr->encoding()); }
  void modify(FloatRegister       fr) { modify_fr(fr->encoding()); }
  void modify(PredicateRegister   pr) { modify_pr(pr->encoding()); }
  void modify(BranchRegister      br) { modify_br(br->encoding()); }
  void modify(ApplicationRegister ar) { modify_ar(ar->encoding()); }

  // Determine if read
  bool is_read(Register            gr) { return _read.is_set(gr); }
  bool is_read(FloatRegister       fr) { return _read.is_set(fr); }
  bool is_read(PredicateRegister   pr) { return _read.is_set(pr); }
  bool is_read(BranchRegister      br) { return _read.is_set(br); }
  bool is_read(ApplicationRegister ar) { return _read.is_set(ar); }

  // Determine if written
  bool is_written(Register            gr) { return _write.is_set(gr); }
  bool is_written(FloatRegister       fr) { return _write.is_set(fr); }
  bool is_written(PredicateRegister   pr) { return _write.is_set(pr); }
  bool is_written(BranchRegister      br) { return _write.is_set(br); }
  bool is_written(ApplicationRegister ar) { return _write.is_set(ar); }

  // Get the read and write states
  RegisterState&  readState() { return _read; }
  RegisterState& writeState() { return _write; }


#ifdef ASSERT
  static uint41_t u_field(uint41_t x, size_t pos, size_t count) {
    assert((x & ~right_n_bits(count)) == 0, "value out of range");
    return x << pos;
  }
#else
  // make sure this is inlined as it will reduce code size significantly
  #define u_field(x, pos, count) ((uint41_t)(x) << (pos))
#endif

  static uint64_t inv_u_field(uint64_t x, size_t pos, size_t count) {
    return (x >> pos) & right_n_bits(count);
  }

  static uint64_t inv_u_field_at(uint64_t x, size_t pos, size_t count, size_t dst_pos) {
    return ((x >> pos) & right_n_bits(count)) << dst_pos;
  }

  static int64_t inv_s_field(uint64_t x, size_t pos, size_t count) {
    return ((int64_t)x << (64 - count - pos)) >> (64 - count);
  }

  static int64_t inv_s_field_at(uint64_t x, size_t pos, size_t count, size_t dst_pos) {
    return (((int64_t)x << (64 - count - pos)) >> (64 - count - dst_pos)) & ~right_n_bits(dst_pos);
  }

  static uint64_t fmask(size_t pos, size_t count) {
    return right_n_bits(count) << pos;
  }


  static uint41_t u_op( uint op)  { return u_field(op,  37, 4); }
  static uint41_t u_x3( uint x3)  { return u_field(x3,  33, 3); }
  static uint41_t u_x6b(uint x6b) { return u_field(x6b, 30, 6); }
  static uint41_t u_x6a(uint x6a) { return u_field(x6a, 27, 6); }

  static uint41_t u_a3(     ApplicationRegister a3) { return u_field(a3->encoding(), 20, 7); }

  static uint41_t u_b2(     BranchRegister      b2) { return u_field(b2->encoding(), 13, 3); }
  static uint41_t u_b1(     BranchRegister      b1) { return u_field(b1->encoding(),  6, 3); }

  static uint41_t u_f4(     FloatRegister       f4) { return u_field(f4->encoding(), 27, 7); }
  static uint41_t u_f3(     FloatRegister       f3) { return u_field(f3->encoding(), 20, 7); }
  static uint41_t u_f2(     FloatRegister       f2) { return u_field(f2->encoding(), 13, 7); }
  static uint41_t u_f1(     FloatRegister       f1) { return u_field(f1->encoding(),  6, 7); }

  static uint41_t u_p2(     PredicateRegister   p2) { return u_field(p2->encoding(), 27, 6); }
  static uint41_t u_p1(     PredicateRegister   p1) { return u_field(p1->encoding(),  6, 6); }

  static uint41_t u_qp(     PredicateRegister   qp) { return u_field(qp->encoding(),  0, 6); }

  static uint41_t u_r3(     Register            r3) { return u_field(r3->encoding(), 20, 7); }
  static uint41_t u_r3_0to3(Register            r3) { return u_field(r3->encoding(), 20, 2); }
  static uint41_t u_r2(     Register            r2) { return u_field(r2->encoding(), 13, 7); }
  static uint41_t u_r1(     Register            r1) { return u_field(r1->encoding(),  6, 7); }

  static uint41_t u_d(    uint d)     { return u_field(d,     35, 1); }
  static uint41_t u_wh(   uint wh)    { return u_field(wh,    33, 2); }
  static uint41_t u_p(    uint p)     { return u_field(p,     12, 1); }
  static uint41_t u_btype(uint btype) { return u_field(btype,  6, 3); }

  static uint41_t u_s8( uint imm8)  { return u_field(imm8  & 0x80,     36- 7,  1+ 7); }
  static uint41_t u_s21(uint imm21) { return u_field(imm21 & 0x100000, 36-20,  1+20); }
  static uint41_t u_i21(uint imm21) { return u_field(imm21 & 0x100000, 36-20,  1+20); }
  static uint41_t u_7a( uint imm)   { return u_field(imm   & 0x7F,         6,  7); }
  static uint41_t u_7b( uint imm)   { return u_field(imm   & 0x7F,        13,  7); }
  static uint41_t u_13c(uint imm21) { return u_field(imm21 & 0xFFF80,  20- 6, 13+ 6); }
  static uint41_t u_20a(uint imm21) { return u_field(imm21 & 0xFFFFF,      6, 20); }
  static uint41_t u_20b(uint imm21) { return u_field(imm21 & 0xFFFFF,     13, 20); }

  static uint41_t u_imm8a( uint imm8)  { return u_s8(imm8)                  | u_7a(imm8); }
  static uint41_t u_imm8b( uint imm8)  { return u_s8(imm8)                  | u_7b(imm8); }
  static uint41_t u_imm21a(uint imm21) { return u_s21(imm21) | u_13c(imm21) | u_7a(imm21); }
  static uint41_t u_imm21b(uint imm21) { return u_s21(imm21) | u_20b(imm21); }
  static uint41_t u_imm21u(uint imm21) { return u_i21(imm21) | u_20a(imm21); }

  static uint41_t u_target25a(uint target25) { return u_imm21a(target25 >> 4); }
  static uint41_t u_target25b(uint target25) { return u_imm21b(target25 >> 4); }
  static uint41_t u_target25u(uint target25) { return u_imm21u(target25 >> 4); }


  static uint inv_u_op( uint64_t x) { return inv_u_field(x, 37, 4); }
  static uint inv_u_x3( uint64_t x) { return inv_u_field(x, 33, 3); }
  static uint inv_u_x6b(uint64_t x) { return inv_u_field(x, 30, 6); }
  static uint inv_u_x6a(uint64_t x) { return inv_u_field(x, 27, 6); }

  static  int64_t inv_s_at(  uint64_t x, size_t pos) { return inv_s_field_at(x, 36,  1, pos); }
  static uint64_t inv_7a_at( uint64_t x, size_t pos) { return inv_u_field_at(x,  6,  7, pos); }
  static uint64_t inv_7b_at( uint64_t x, size_t pos) { return inv_u_field_at(x, 13,  7, pos); }
  static uint64_t inv_13c_at(uint64_t x, size_t pos) { return inv_u_field_at(x, 20, 13, pos); }
  static uint64_t inv_20a_at(uint64_t x, size_t pos) { return inv_u_field_at(x,  6, 20, pos); }
  static uint64_t inv_20b_at(uint64_t x, size_t pos) { return inv_u_field_at(x, 13, 20, pos); }

  static int64_t inv_s_imm8a( uint64_t x) { return inv_s_at(x,  7) |                    inv_7a_at(x, 0); }
  static int64_t inv_s_imm8b( uint64_t x) { return inv_s_at(x,  7) |                    inv_7b_at(x, 0); }
  static int64_t inv_s_imm21a(uint64_t x) { return inv_s_at(x, 20) | inv_13c_at(x, 7) | inv_7a_at(x, 0); }
  static int64_t inv_s_imm21b(uint64_t x) { return inv_s_at(x, 20) | inv_20b_at(x, 0); }
  static int64_t inv_s_imm21u(uint64_t x) { return inv_s_at(x, 20) | inv_20a_at(x, 0); }

  static int64_t inv_target25a(uint64_t x) { return inv_s_imm21a(x) << 4; }
  static int64_t inv_target25b(uint64_t x) { return inv_s_imm21b(x) << 4; }
  static int64_t inv_target25u(uint64_t x) { return inv_s_imm21u(x) << 4; }

  static uint64_t m_op()  { return fmask(37,  4); }
  static uint64_t m_vc()  { return fmask(20,  1); }
  static uint64_t m_s()   { return fmask(36,  1); }
  static uint64_t m_i()   { return fmask(36,  1); }
  static uint64_t m_7a()  { return fmask( 6,  7); }
  static uint64_t m_7b()  { return fmask(13,  7); }
  static uint64_t m_13c() { return fmask(20, 13); }
  static uint64_t m_20a() { return fmask( 6, 20); }
  static uint64_t m_20b() { return fmask(13, 20); }

  static uint64_t m_imm8a()  { return m_s() |           m_7a(); }
  static uint64_t m_imm8b()  { return m_s() |           m_7b(); }
  static uint64_t m_imm21a() { return m_s() | m_13c() | m_7a(); }
  static uint64_t m_imm21b() { return m_s() | m_20b(); }
  static uint64_t m_imm21u() { return m_i() | m_20a(); }
};


// Integer ALU I/M Unit

class A_Inst : public Inst {
public:
  A_Inst(uint41_t inst)
    : Inst(inst, A_inst)
  {};

  // A1, A2
  A_Inst(uint41_t inst, Register dst, Register src1, Register src2, PredicateRegister qp)
    : Inst(inst, A_inst)
  { write(dst); read(src1); read(src2); read(qp); }

  // A3, A4, A5, A9, A10
  A_Inst(uint41_t inst, Register dst, Register src, PredicateRegister qp)
    : Inst(inst, A_inst)
  { write(dst); read(src); read(qp); }

  // A6
  A_Inst(uint41_t inst, PredicateRegister p1, PredicateRegister p2, Register src1, Register src2, PredicateRegister qp)
    : Inst(inst, A_inst)
  { write(p1); write(p2); read(src1); read(src2); read(qp); set_pr_fast_write(); }

  // A7, A8
  A_Inst(uint41_t inst, PredicateRegister p1, PredicateRegister p2, Register src, PredicateRegister qp)
    : Inst(inst, A_inst)
  { write(p1); write(p2); read(src); read(qp); set_pr_fast_write(); }

  static uint41_t u_tb(  uint tb)   { return u_field(tb,   36, 1); }
  static uint41_t u_za(  uint za)   { return u_field(za,   36, 1); }
  static uint41_t u_x2(  uint x2)   { return u_field(x2,   34, 2); }
  static uint41_t u_x2a( uint x2a)  { return u_field(x2a,  34, 2); }
  static uint41_t u_ta(  uint ta)   { return u_field(ta,   33, 1); }
  static uint41_t u_ve(  uint ve)   { return u_field(ve,   33, 1); }
  static uint41_t u_zb(  uint zb)   { return u_field(zb,   33, 1); }
  static uint41_t u_x4(  uint x4)   { return u_field(x4,   29, 4); }
  static uint41_t u_x2b( uint x2b)  { return u_field(x2b,  27, 2); }
  static uint41_t u_ct2d(uint ct2d) { return u_field(ct2d, 27, 2); }
  static uint41_t u_c(   uint c)    { return u_field(c,    12, 1); }

  static uint41_t u_s14(uint imm14) { return u_field(imm14 & 0x2000,   36-13, 1+13); }
  static uint41_t u_s22(uint imm22) { return u_field(imm22 & 0x200000, 36-21, 1+21); }
  static uint41_t u_6d( uint imm14) { return u_field(imm14 & 0x1F80,   27- 7, 6+ 7); }
  static uint41_t u_9d( uint imm22) { return u_field(imm22 & 0xFF80,   27- 7, 9+ 7); }
  static uint41_t u_5c( uint imm22) { return u_field(imm22 & 0x1F0000, 22-16, 9+16); }

  static uint41_t u_imm14(uint imm14) { return u_s14(imm14) | u_6d(imm14) | u_7b(imm14); }
  static uint41_t u_imm22(uint imm22) { return u_s22(imm22) | u_9d(imm22) | u_5c(imm22) | u_7b(imm22); }
};

// C.2.1.1  Integer ALU - Register-Register
class A1 : public A_Inst {
public:
  A1(uint x4, uint x2b,
     Register dst, Register src1, Register src2, PredicateRegister qp)
    : A_Inst(u_op(0x8)  | u_x2a(0)   | u_ve(0)   | u_x4(x4) | u_x2b(x2b) |
             u_r3(src2) | u_r2(src1) | u_r1(dst) | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// C.2.1.2  Shift Left and Add
class A2 : public A_Inst {
public:
  A2(uint x4,
     Register dst, Register src1, uint count2, Register src2, PredicateRegister qp)
    : A_Inst(u_op(0x8)  | u_x2a(0)   | u_ve(0)   | u_x4(x4) | u_ct2d(count2 - 1) |
             u_r3(src2) | u_r2(src1) | u_r1(dst) | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// C.2.1.3  Integer ALU - Immediate(8)-Register
class A3 : public A_Inst {
public:
  A3(uint x4, uint x2b,
     Register dst, uint imm8, Register src, PredicateRegister qp)
    : A_Inst(u_op(0x8) | u_x2a(0)      | u_ve(0)   | u_x4(x4) | u_x2b(x2b) |
             u_r3(src) | u_imm8b(imm8) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.2.1.4  Add Immediate(14)
class A4 : public A_Inst {
public:
  A4(uint x2a,
     Register dst, uint imm14, Register src, PredicateRegister qp)
    : A_Inst(u_op(0x8) | u_x2a(x2a)     | u_ve(0)  |
             u_r3(src) | u_imm14(imm14) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.2.1.5  Add Immediate(22)
class A5 : public A_Inst {
public:
  A5(Register dst, uint imm22, Register src, PredicateRegister qp)
    : A_Inst(u_op(0x9)      |
             u_r3_0to3(src) | u_imm22(imm22) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.2.2.1  Integer Compare - Register-Register
class A6 : public A_Inst {
public:
  A6(uint op, uint x2, uint tb, uint ta, uint c,
     PredicateRegister p1, PredicateRegister p2, Register src1, Register src2, PredicateRegister qp)
    : A_Inst(u_op(op) | u_tb(tb)   | u_x2(x2)   | u_ta(ta) | u_c(c) |
             u_p2(p2) | u_r3(src2) | u_r2(src1) | u_p1(p1) | u_qp(qp),
             p1, p2, src1, src2, qp)
  {}
};

// C.2.2.2  Integer Compare to Zero - Register
class A7 : public A_Inst {
public:
  A7(uint op, uint x2, uint tb, uint ta, uint c,
     PredicateRegister p1, PredicateRegister p2, Register src, PredicateRegister qp)
    : A_Inst(u_op(op) | u_tb(tb)  | u_x2(x2) | u_ta(ta) | u_c(c) |
             u_p2(p2) | u_r3(src) | u_p1(p1) | u_qp(qp),
             p1, p2, src, qp)
  {}
};

// C.2.2.3  Integer Compare - Immediate-Register
class A8 : public A_Inst {
public:
  A8(uint op, uint x2, uint ta, uint c,
     PredicateRegister p1, PredicateRegister p2, uint imm8, Register src, PredicateRegister qp)
    : A_Inst(u_op(op) | u_x2(x2)  | u_ta(ta)      | u_c(c) |
             u_p2(p2) | u_r3(src) | u_imm8b(imm8) | u_p1(p1) | u_qp(qp),
             p1, p2, src, qp)
  {}
};

// Multimedia ALU
class A9: public A_Inst {
public:
  A9(uint za, uint zb, uint x4, uint x2b,
     Register dst, Register src1, Register src2, PredicateRegister qp)
    : A_Inst(u_op(0x8)  | u_za(za)   | u_x2a(1)  | u_zb(zb) | u_x4(x4) | u_x2b(x2b) |
             u_r3(src2) | u_r2(src1) | u_r1(dst) | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// Multimedia Shift and Add
class A10: public A_Inst {
public:
  A10(uint x4,
      Register dst, Register src1, uint count2, Register src2, PredicateRegister qp)
    : A_Inst(u_op(0x8)  | u_za(0)    | u_x2a(1)  | u_zb(1) | u_x4(x4) | u_ct2d(count2 - 1) |
             u_r3(src2) | u_r2(src1) | u_r1(dst) | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// Non-ALU Integer I Unit
class I_Inst : public Inst {
public:
  I_Inst(uint41_t inst)
    : Inst(inst, I_inst)
  {}

  // I5, I7, I10, I15
  I_Inst(uint41_t inst, Register dst, Register src1, Register src2, PredicateRegister qp)
    : Inst(inst, I_inst)
  { write(dst); read(src1); read(src2); read(qp); }

  // I9, I11, I12, I14, I29
  I_Inst(uint41_t inst, Register dst, Register src, PredicateRegister qp)
    : Inst(inst, I_inst)
  { write(dst); read(src); read(qp); }

  // I13, I25
  I_Inst(uint41_t inst, Register dst, PredicateRegister qp)
    : Inst(inst, I_inst)
  { write(dst); read(qp); }

  // I16, I17
  I_Inst(uint41_t inst, PredicateRegister p1, PredicateRegister p2, Register src, PredicateRegister qp)
    : Inst(inst, I_inst)
  { write(p1); write(p2); read(src); read(qp); }

  // I19, I24
  I_Inst(uint41_t inst, PredicateRegister qp)
    : Inst(inst, I_inst)
  { read(qp); }

  // I20, I23
  I_Inst(uint41_t inst, PredicateRegister qp, Register src)
    : Inst(inst, I_inst)
  { read(src); read(qp); }

  // I21
  I_Inst(uint41_t inst, BranchRegister dst, Register src, PredicateRegister qp)
    : Inst(inst, I_inst)
  { write(dst); read(src); read(qp); }

  // I22
  I_Inst(uint41_t inst, Register dst, BranchRegister src, PredicateRegister qp)
    : Inst(inst, I_inst)
  { write(dst); read(src); read(qp); }

  // I26
  I_Inst(uint41_t inst, ApplicationRegister dst, Register src, PredicateRegister qp)
    : Inst(inst, I_inst)
  { write(dst); read(src); read(qp); }

  // I27
  I_Inst(uint41_t inst, ApplicationRegister dst, PredicateRegister qp)
    : Inst(inst, I_inst)
  { write(dst); read(qp); }

  // I28
  I_Inst(uint41_t inst, Register dst, ApplicationRegister src, PredicateRegister qp)
    : Inst(inst, I_inst)
  { write(dst); read(src); read(qp); }

  static uint41_t u_tb(  uint tb)   { return u_field(tb,   36, 1); }
  static uint41_t u_za(  uint za)   { return u_field(za,   36, 1); }
  static uint41_t u_s(   uint s)    { return u_field(s,    36, 1); }
  static uint41_t u_x2(  uint x2)   { return u_field(x2,   34, 2); }
  static uint41_t u_x2a( uint x2a)  { return u_field(x2a,  34, 2); }
  static uint41_t u_ta(  uint ta)   { return u_field(ta,   33, 1); }
  static uint41_t u_x(   uint x)    { return u_field(x,    33, 1); }
  static uint41_t u_zb(  uint zb)   { return u_field(zb,   33, 1); }
  static uint41_t u_ve(  uint ve)   { return u_field(ve,   32, 1); }
  static uint41_t u_ct2d(uint ct2d) { return u_field(ct2d, 30, 2); }
  static uint41_t u_x2c( uint x2c)  { return u_field(x2c,  30, 2); }
  static uint41_t u_x2b( uint x2b)  { return u_field(x2b,  28, 2); }
  static uint41_t u_yc(  uint yc)   { return u_field(yc,   26, 1); }
  static uint41_t u_ih(  uint ih)   { return u_field(ih,   23, 1); }
  static uint41_t u_xc(  uint xc)   { return u_field(xc,   22, 1); }
  static uint41_t u_wh(  uint wh)   { return u_field(wh,   20, 2); }
  static uint41_t u_yb(  uint yb)   { return u_field(yb,   13, 1); }
  static uint41_t u_c(   uint c)    { return u_field(c,    12, 1); }

  static uint41_t u_cpos6d( uint cpos6d)  { return u_field(cpos6d,  31, 6); }
  static uint41_t u_count6d(uint count6d) { return u_field(count6d, 27, 6); }
  static uint41_t u_len4d(  uint len4d)   { return u_field(len4d,   27, 4); }
  static uint41_t u_len6d(  uint len6d)   { return u_field(len6d,   27, 6); }
  static uint41_t u_timm9c( uint timm9c)  { return u_field(timm9c,  24, 9); }
  static uint41_t u_cpos6c( uint cpos6c)  { return u_field(cpos6c,  20, 6); }
  static uint41_t u_cpos6b( uint cpos6b)  { return u_field(cpos6b,  14, 6); }
  static uint41_t u_pos6b(  uint pos6b)   { return u_field(pos6b,   14, 6); }

  static uint41_t u_s16(   uint imm16) { return u_field(imm16 & 0x8000,    36-15,  1+15); }
  static uint41_t u_mask8c(uint imm16) { return u_field(imm16 & 0x7F80,    24- 7,  8+ 7); }
  static uint41_t u_s28(   uint imm28) { return u_field(imm28 & 0x8000000, 36-27,  1+27); }
  static uint41_t u_27a(   uint imm28) { return u_field(imm28 & 0x7FFFFFF,     6, 27); }

  static uint41_t u_mask16(uint mask16) { return u_s16(mask16) | u_mask8c(mask16) | u_7a(mask16); }
  static uint41_t u_imm28( uint imm28)  { return u_s28(imm28)  | u_27a(imm28); }
};

// C.3.1.5  Shift Right - Variable
class I5 : public I_Inst {
public:
  I5(uint za, uint zb, uint x2b,
     Register dst, Register src1, Register src2, PredicateRegister qp)
    : I_Inst(u_op(0x7)  | u_za(za)   | u_x2a(0)  | u_zb(zb) | u_ve(0) | u_x2c(0) | u_x2b(x2b) |
             u_r3(src1) | u_r2(src2) | u_r1(dst) | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// C.3.1.7  Shift Left - Variable
class I7 : public I_Inst {
public:
  I7(uint za, uint zb,
     Register dst, Register src1, Register src2, PredicateRegister qp)
    : I_Inst(u_op(0x7)  | u_za(za)   | u_x2a(0)  | u_zb(zb) | u_ve(0) | u_x2c(1) | u_x2b(0) |
             u_r3(src2) | u_r2(src1) | u_r1(dst) | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// C.3.1.9  Population Count
class I9 : public I_Inst {
public:
  I9(Register dst, Register src, PredicateRegister qp)
    : I_Inst(u_op(0x7) | u_za(0)   | u_x2a(1) | u_zb(1) | u_ve(0) | u_x2c(2) | u_x2b(1) |
             u_r3(src) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.3.2.2  Shift Right Pair
class I10 : public I_Inst {
public:
  I10(Register dst, Register src1, Register src2, uint count6, PredicateRegister qp)
    : I_Inst(u_op(0x5)  | u_x2(3)    | u_x(0)    | u_count6d(count6) |
             u_r3(src2) | u_r2(src1) | u_r1(dst) | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// C.3.2.2  Extract
class I11 : public I_Inst {
public:
  I11(uint y,
      Register dst, Register src, uint pos6, uint len6, PredicateRegister qp)
    : I_Inst(u_op(0x5) | u_x2(1)   | u_x(0) | u_len6d(len6 - 1) | u_pos6b(pos6) | u_yb(y) |
             u_r3(src) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.3.2.3  Zero and Deposit
class I12 : public I_Inst {
public:
  I12(Register dst, Register src, uint pos6, uint len6, PredicateRegister qp)
    : I_Inst(u_op(0x5) | u_x2(1)   | u_x(1) | u_len6d(len6 - 1) | u_yc(0) | u_cpos6c(63 - pos6) |
             u_r2(src) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.3.2.4  Zero and Deposit Immediate(8)
class I13 : public I_Inst {
public:
  I13(Register dst, uint imm8, uint pos6, uint len6, PredicateRegister qp)
    : I_Inst(u_op(0x5) | u_x2(1) | u_x(1) | u_len6d(len6 - 1) | u_yc(1) | u_cpos6c(63 - pos6) | u_imm8b(imm8) |
             u_r1(dst) | u_qp(qp),
             dst, qp)
  {}
};

// C.3.2.5  Deposit Immediate(1)
class I14 : public I_Inst {
public:
  I14(Register dst, uint imm1, Register src, uint pos6, uint len6, PredicateRegister qp)
    : I_Inst(u_op(0x5) | u_s(imm1) | u_x2(3) | u_x(1) | u_len6d(len6 - 1) | u_cpos6b(63 - pos6) |
             u_r3(src) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.3.2.6  Deposit
class I15 : public I_Inst {
public:
  I15(Register dst, Register src1, Register src2, uint pos6, uint len4, PredicateRegister qp)
    : I_Inst(u_op(0x4)  | u_cpos6d(63 - pos6) | u_len4d(len4 - 1) |
             u_r3(src2) | u_r2(src1)          | u_r1(dst)         | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// C.3.3.1  Test Bit
class I16 : public I_Inst {
public:
  I16(uint ta, uint tb, uint y, uint c,
      PredicateRegister p1, PredicateRegister p2, Register src, uint pos6, PredicateRegister qp)
    : I_Inst(u_op(0x5) | u_tb(tb)  | u_x2(0)  | u_ta(ta) | u_pos6b(pos6) | u_yb(y) | u_c(c) |
             u_p2(p2)  | u_r3(src) | u_p1(p1) | u_qp(qp),
             p1, p2, src, qp)
  {}
};

#if 0 // I17 is I6 with pos6 == 0
// C.3.3.2  Test NaT
class I17 : public I_Inst {
public:
  I17(uint ta, uint tb, uint y, uint c,
      PredicateRegister p1, PredicateRegister p2, Register src, PredicateRegister qp)
    : I_Inst(u_op(0x5) | u_tb(tb)  | u_x2(0)  | u_ta(ta) | u_pos6b(0) | u_yb(y) | u_c(c) |
             u_p2(p2)  | u_r3(src) | u_p1(p1) | u_qp(qp),
             p1, p2, src, qp)
  {}
};
#endif

// C.3.4.1  Break/Nop (I-Unit)
class I19 : public I_Inst {
public:
  I19(uint x6,
      uint imm21, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(0) | u_x6a(x6) | u_imm21u(imm21) |
             u_qp(qp),
             qp)
  {}
};

// C.3.4.2  Integer Speculation Check (I-Unit)
class I20 : public I_Inst {
public:
  I20(Register src, uint target25, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(1)               |
             u_r2(src) | u_target25a(target25) | u_qp(qp),
             qp, src)
  { set_is_ordered(); }

  static int64_t inv_target(uint41_t I) {
    return inv_target25a(I);
  }

  static void set_target(uint target25, uint41_t I, uint41_t& new_I) {
    new_I = (I & ~m_imm21a()) | u_target25a(target25);
  }
};

// C.3.5.1  Move to BR
class I21 : public I_Inst {
public:
  I21(uint x, uint ih, uint wh,
      BranchRegister dst, Register src, uint tag13, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(7)   | u_timm9c(tag13 >> 4) | u_ih(ih) | u_xc(x) | u_wh(wh) |
             u_r2(src) | u_b1(dst) | u_qp(qp),
             dst, src, qp)
  { set_br_fast_write(); }
};

// C.3.5.2  Move from BR
class I22 : public I_Inst {
public:
  I22(Register dst, BranchRegister src, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(0)   | u_x6a(0x31) |
             u_b2(src) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.3.6.1  Move to Predicates - Register
class I23 : public I_Inst {
public:
  I23(Register src, uint mask17, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(3) |
             u_r2(src) | u_mask16(mask17 >> 1) | u_qp(qp),
             qp, src)
  { write(PredicateRegisterImplMask(((int64_t)mask17 << (BitsPerLong-18)) >> (BitsPerLong - 18))); }
};

// C.3.6.2  Move to Predicates - Immediate 44
class I24 : public I_Inst {
public:
  I24(uint imm44, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(2)  |
             u_imm28(imm44 >> 16) | u_qp(qp),
             qp)
  { write(PredicateRegisterImplMask(left_n_bits(48))); }
};

// C.3.6.2  Move from Predicates/IP
class I25 : public I_Inst {
public:
  I25(uint x6,
      Register dst, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(0) | u_x6a(x6) |
             u_r1(dst) | u_qp(qp),
             dst, qp)
  { if (x6 == 0x33) read(PredicateRegisterImplMask((int64_t)~0)); }
};

// C.3.7.1  Move to AR - Register (I-Unit)
class I26 : public I_Inst {
public:
  I26(ApplicationRegister dst, Register src, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(0)   | u_x6a(0x2A) |
             u_a3(dst) | u_r2(src) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.3.7.1  Move to AR - Immediate (I-Unit)
class I27 : public I_Inst {
public:
  I27(ApplicationRegister dst, uint imm8, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(0)       | u_x6a(0xA) |
             u_a3(dst) | u_imm8b(imm8) | u_qp(qp),
             dst, qp)
  {}
};

// C.3.7.3  Move from AR (I-Unit)
class I28 : public I_Inst {
public:
  I28(Register dst, ApplicationRegister src, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(0)   | u_x6a(0x32) |
             u_a3(src) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.3.8    Size/Zero Extend/Compute Zero Index
class I29 : public I_Inst {
public:
  I29(uint x6,
      Register dst, Register src, PredicateRegister qp)
    : I_Inst(u_op(0x0) | u_x3(0)   | u_x6a(0x10 | x6) |
             u_r3(src) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};


// Memory M Unit
class M_Inst : public Inst {
public:
  M_Inst(uint41_t inst)
    : Inst(inst, M_inst)
  {}

  // M1, M3, M17
  M_Inst(uint41_t inst, Register dst, Register src, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst); read(src); read(qp); }

  // M2, M16
  M_Inst(uint41_t inst, Register dst, Register src1, Register src2, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst); read(src1); read(src2); read(qp); }

  // M4, M5, M14
  M_Inst(uint41_t inst, PredicateRegister qp, Register src1, Register src2)
    : Inst(inst, M_inst)
  { read(src1); read(src2); read(qp); }

  // M6. M18
  M_Inst(uint41_t inst, FloatRegister dst, Register src, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst); read(src); read(qp); }

  // M7, M8
  M_Inst(uint41_t inst, FloatRegister dst, Register src1, Register src2, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst); read(src1); read(src2); read(qp); }

  // M9, M10
  M_Inst(uint41_t inst, PredicateRegister qp, Register src1, FloatRegister src2)
    : Inst(inst, M_inst)
  { read(src1); read(src2); read(qp); }

  // M11, M12
  M_Inst(uint41_t inst, FloatRegister dst1, FloatRegister dst2, Register src, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst1); write(dst2); read(src); read(qp); }

  // M13, M15, M20, M28, M35
  M_Inst(uint41_t inst, PredicateRegister qp, Register src)
    : Inst(inst, M_inst)
  { read(src); read(qp); }

  // M19
  M_Inst(uint41_t inst, Register dst, FloatRegister src, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst); read(src); read(qp); }

  // M21
  M_Inst(uint41_t inst, PredicateRegister qp, FloatRegister src)
    : Inst(inst, M_inst)
  { read(src); read(qp); }

  // M22, M23, M24, M25, M26, M27. M37
  M_Inst(uint41_t inst, PredicateRegister qp)
    : Inst(inst, M_inst)
  { read(qp); }

  // M29
  M_Inst(uint41_t inst, ApplicationRegister dst, Register src, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst); read(src); read(qp); }

  // M30
  M_Inst(uint41_t inst, ApplicationRegister dst, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst); read(qp); }

  // M31
  M_Inst(uint41_t inst, Register dst, ApplicationRegister src, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst); read(src); read(qp); }

  // M34
  M_Inst(uint41_t inst, Register dst)
    : Inst(inst, M_inst)
  { write(dst); }

  // M36
  M_Inst(uint41_t inst, Register dst, PredicateRegister qp)
    : Inst(inst, M_inst)
  { write(dst); read(qp); }

  static uint41_t u_m(   uint m)    { return u_field(m,    36, 1); }
  static uint41_t u_s(   uint s)    { return u_field(s,    36, 1); }
  static uint41_t u_x2(  uint x2)   { return u_field(x2,   31, 2); }
  static uint41_t u_hint(uint hint) { return u_field(hint, 28, 2); }
  static uint41_t u_x(   uint x)    { return u_field(x,    27, 1); }
  static uint41_t u_x4(  uint x4)   { return u_field(x4,   27, 4); }

  static uint41_t u_sor(uint sor) { return u_field(sor, 27, 4); }
  static uint41_t u_sol(uint sol) { return u_field(sol, 20, 7); }
  static uint41_t u_sof(uint sof) { return u_field(sof, 13, 7); }

  static uint41_t u_i9( uint imm9) { return u_field(imm9 & 0x80,  27-7, 1+7); }
  static uint41_t u_s9( uint imm9) { return u_field(imm9 & 0x100, 36-8, 1+8); }
  static uint41_t u_s6( uint imm6) { return u_field(imm6 & 0x20,  15-5, 1+5); }

  // imm6  => i2b   ( ((~imm6 >> 2) + ((imm6 >> 4) & 1)) & 3 ) << 13
  // ------------
  // 00001 =>  11
  // 00100 =>  10
  // 01000 =>  01
  // 10000 =>  00
  static uint41_t u_i2b(uint imm6) { return u_field((~imm6 + ((imm6 >> 2) & 0x4)) & 0xC,
                                                                  13-2, 2+2); }

  static uint41_t u_imm9a(uint imm9) { return u_s9(imm9) | u_i9(imm9) | u_7a(imm9); }
  static uint41_t u_imm9b(uint imm9) { return u_s9(imm9) | u_i9(imm9) | u_7b(imm9); }
  static uint41_t u_inc3( uint imm6) { return u_s6(imm6) | u_i2b((imm6 & 0x20) ? (uint)(-(int)imm6) : imm6); }
};

// C.4.1.1  Integer Load
class M1 : public M_Inst {
public:
  M1(uint x6, uint hint,
     Register dst, Register adr, PredicateRegister qp)
    : M_Inst(u_op(0x4) | u_m(0)    | u_x6b(x6) | u_hint(hint) | u_x(0) |
             u_r3(adr) | u_r1(dst) | u_qp(qp),
             dst, adr, qp)
  { if ((x6 & 0xF) == 0xB) write(AR_UNAT); }
};

// C.4.1.2  Integer Load - Increment by Register
class M2 : public M_Inst {
public:
  M2(uint x6, uint hint,
     Register dst, Register adr, Register adj, PredicateRegister qp)
    : M_Inst(u_op(0x4) | u_m(1)    | u_x6b(x6) | u_hint(hint) | u_x(0) |
             u_r3(adr) | u_r2(adj) | u_r1(dst) | u_qp(qp),
             dst, adr, adj, qp)
  { if ((x6 & 0xF) == 0xB) write(AR_UNAT); write(adr); }
};

// C.4.1.3  Integer Load - Increment by Immediate
class M3 : public M_Inst {
public:
  M3(uint x6, uint hint,
     Register dst, Register adr, uint imm9, PredicateRegister qp)
    : M_Inst(u_op(0x5) | u_x6b(x6)     | u_hint(hint) |
             u_r3(adr) | u_imm9b(imm9) | u_r1(dst)    | u_qp(qp),
             dst, adr, qp)
  { if ((x6 & 0xF) == 0xB) write(AR_UNAT); write(adr); }
};

// C.4.1.4  Integer Store
class M4 : public M_Inst {
public:
  M4(uint x6, uint hint,
     Register adr, Register src, PredicateRegister qp)
    : M_Inst(u_op(0x4) | u_m(0)    | u_x6b(0x30 | x6) | u_hint(hint) | u_x(0) |
             u_r3(adr) | u_r2(src) | u_qp(qp),
             qp, adr, src)
  { if ((x6 & 0xF) == 0xB) read(AR_UNAT); }
};

// C.4.1.5  Integer Store - Increment by Immediate
class M5 : public M_Inst {
public:
  M5(uint x6, uint hint,
     Register adr, Register src, uint imm9, PredicateRegister qp)
    : M_Inst(u_op(0x5) | u_x6b(0x30 | x6) | u_hint(hint)  |
             u_r3(adr) | u_r2(src)        | u_imm9a(imm9) | u_qp(qp),
             qp, adr, src)
  { if ((x6 & 0xF) == 0xB) read(AR_UNAT); write(adr); }
};

// C.4.1.6  Floating-point Load
class M6 : public M_Inst {
public:
  M6(uint x6, uint hint,
     FloatRegister dst, Register adr, PredicateRegister qp)
    : M_Inst(u_op(0x6) | u_m(0)    | u_x6b(x6) | u_hint(hint) | u_x(0) |
             u_r3(adr) | u_f1(dst) | u_qp(qp),
             dst, adr, qp)
  {}
};

// C.4.1.7  Floating-point Load - Increment by Register
class M7 : public M_Inst {
public:
  M7(uint x6, uint hint,
     FloatRegister dst, Register adr, Register adj, PredicateRegister qp)
    : M_Inst(u_op(0x6) | u_m(1)    | u_x6b(x6) | u_hint(hint) | u_x(0) |
             u_r3(adr) | u_r2(adj) | u_f1(dst) | u_qp(qp),
             dst, adr, adj, qp)
  { write(adr); }
};

// C.4.1.8  Floating-point Load - Increment by Immediate
class M8 : public M_Inst {
public:
  M8(uint x6, uint hint,
     FloatRegister dst, Register adr, uint imm9, PredicateRegister qp)
    : M_Inst(u_op(0x7) | u_x6b(x6)     | u_hint(hint) |
             u_r3(adr) | u_imm9b(imm9) | u_f1(dst)    | u_qp(qp),
             dst, adr, qp)
  { write(adr); }
};

// C.4.1.9  Floating-point Store
class M9 : public M_Inst {
public:
  M9(uint x6, uint hint,
     Register adr, FloatRegister src, PredicateRegister qp)
    : M_Inst(u_op(0x6) | u_m(0)    | u_x6b(0x30 | x6) | u_hint(hint) | u_x(0) |
             u_r3(adr) | u_f2(src) | u_qp(qp),
             qp, adr, src)
  {}
};

// C.4.1.10 Floating-point Store - Increment by Immediate
class M10 : public M_Inst {
public:
  M10(uint x6, uint hint,
      Register adr, FloatRegister src, uint imm9, PredicateRegister qp)
    : M_Inst(u_op(0x7) | u_x6b(0x30 | x6) | u_hint(hint)  |
             u_r3(adr) | u_f2(src)        | u_imm9a(imm9) | u_qp(qp),
             qp, adr, src)
  { write(adr); }
};

// Floating-point Load Pair
class M11 : public M_Inst {
public:
  M11(uint x6, uint hint,
      FloatRegister dst1, FloatRegister dst2, Register adr, PredicateRegister qp)
    : M_Inst(u_op(0x6) | u_m(0)     | u_x6b(x6)  | u_hint(hint) | u_x(1) |
             u_r3(adr) | u_f2(dst2) | u_f1(dst1) | u_qp(qp),
             dst1, dst2, adr, qp)
  {}
};

// Floating-point Load Pair - Increment by Immediate
class M12 : public M_Inst {
public:
  M12(uint x6, uint hint,
      FloatRegister dst1, FloatRegister dst2, Register adr, PredicateRegister qp)
    : M_Inst(u_op(0x6) | u_m(1)     | u_x6b(x6)  | u_hint(hint) | u_x(1) |
             u_r3(adr) | u_f2(dst2) | u_f1(dst1) | u_qp(qp),
             dst1, dst2, adr, qp)
  { write(adr); }
};

// Line Prefetch
class M13 : public M_Inst {
public:
  M13(uint x6, uint hint,
      Register adr, PredicateRegister qp)
    : M_Inst(u_op(0x6) | u_m(0) | u_x6b(0x2C | x6) | u_hint(hint) | u_x(0) |
             u_r3(adr) | u_qp(qp),
             qp, adr)
  {}
};

// Line Prefetch - Increment by Register
class M14 : public M_Inst {
public:
  M14(uint x6, uint hint,
      Register adr, Register adj, PredicateRegister qp)
    : M_Inst(u_op(0x6) | u_m(1)    | u_x6b(0x2C | x6) | u_hint(hint) | u_x(0) |
             u_r3(adr) | u_r2(adj) | u_qp(qp),
             qp, adr, adj)
  { write(adr); }
};

// Line Prefetch - Increment by Immediate
class M15 : public M_Inst {
public:
  M15(uint x6, uint hint,
      Register adr, uint imm9, PredicateRegister qp)
    : M_Inst(u_op(0x7) | u_x6b(0x2C | x6) | u_hint(hint) |
             u_r3(adr) | u_imm9b(imm9)    | u_qp(qp),
             qp, adr)
  { write(adr); }
};

// C.4.3.1  Exchange/Compare and Exchange
class M16 : public M_Inst {
public:
  M16(uint x6, uint hint,
      Register dst, Register adr, Register src, PredicateRegister qp)
    : M_Inst(u_op(0x4) | u_m(0)    | u_x6b(x6) | u_hint(hint) | u_x(1) |
             u_r3(adr) | u_r2(src) | u_r1(dst) | u_qp(qp),
             dst, adr, src, qp)
  { if (x6 < 8) /* cmpxchg */ read(AR_CCV); }
};

// C.4.3.2  Fetch and Add - Immediate
class M17 : public M_Inst {
public:
  M17(uint x6, uint hint,
      Register dst, Register adr, uint imm6, PredicateRegister qp)
    : M_Inst(u_op(0x4) | u_m(0)       | u_x6b(0x10 | x6) | u_x(1) |
             u_r3(adr) | u_inc3(imm6) | u_r1(dst)        | u_qp(qp),
             dst, adr, qp)
  {}
};

// C.4.4.1  Set FR
class M18 : public M_Inst {
public:
  M18(uint x6, FloatRegister dst, Register src, PredicateRegister qp)
    : M_Inst(u_op(0x6) | u_m(0)    | u_x6b(0x10 | x6) | u_x(1) |
             u_r2(src) | u_f1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.4.4.2  Get FR
class M19 : public M_Inst {
public:
  M19(uint x6, Register dst, FloatRegister src, PredicateRegister qp)
    : M_Inst(u_op(0x4) | u_m(0)    | u_x6b(0x10 | x6) | u_x(1) |
             u_f2(src) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.4.5.1  Integer Speculation Check (M-Unit)
class M20 : public M_Inst {
public:
  M20(Register src, uint target25, PredicateRegister qp)
    : M_Inst(u_op(0x1) | u_x3(1)               |
             u_r2(src) | u_target25a(target25) | u_qp(qp),
             qp, src)
  { set_is_ordered(); }
};

// C.4.5.2  Floating-point Speculation Check
class M21 : public M_Inst {
public:
  M21(FloatRegister src, uint target25, PredicateRegister qp)
    : M_Inst(u_op(0x1) | u_x3(3)               |
             u_f2(src) | u_target25a(target25) | u_qp(qp),
             qp, src)
  { set_is_ordered(); }
};

// C.4.5.3  Integer Advanced Load Check
class M22 : public M_Inst {
public:
  M22(uint x3,
      Register src, uint target25, PredicateRegister qp)
    : M_Inst(u_op(0x0) | u_x3(4 | x3)          |
             u_r1(src) | u_target25b(target25) | u_qp(qp),
             qp)
  { set_is_ordered(); }

  static int64_t inv_target(uint41_t M) {
    return inv_target25b(M);
  }

  static void set_target(uint target25, uint41_t M, uint41_t& new_M) {
    new_M = (M & ~m_imm21b()) | u_target25b(target25);
  }
};

// C.4.5.4  Floating-point Advanced Load Check
class M23 : public M_Inst {
public:
  M23(uint x3,
      FloatRegister src, uint target25, PredicateRegister qp)
    : M_Inst(u_op(0x0) | u_x3(6 | x3)          |
             u_f1(src) | u_target25b(target25) | u_qp(qp),
             qp)
  { set_is_ordered(); }
};

// C.4.6.1  Sync/Fence/Serialize/ALAT Control
class M24 : public M_Inst {
public:
  M24(uint x4, uint x2,
      PredicateRegister qp)
    : M_Inst(u_op(0x0) | u_x3(0) | u_x2(x2) | u_x4(x4) |
             u_qp(qp),
             qp)
  {}
};

// C.4.6.2  RSE Control
class M25 : public M_Inst {
public:
  M25(uint x4,
      PredicateRegister qp)
    : M_Inst(u_op(0x0) | u_x3(0) | u_x2(0) | u_x4(x4) |
             u_qp(qp),
             qp)
  {}
};

// C.4.6.3  Integer ALAT Entry Invalidate
class M26 : public M_Inst {
public:
  M26(Register src, PredicateRegister qp)
    : M_Inst(u_op(0x0) | u_x3(0) | u_x2(1) | u_x4(2) |
             u_r1(src) | u_qp(qp),
             qp)
  {}
};

// C.4.6.4  Floating-point ALAT Entry Invalidate
class M27 : public M_Inst {
public:
  M27(FloatRegister src, PredicateRegister qp)
    : M_Inst(u_op(0x0) | u_x3(0) | u_x2(1) | u_x4(3) |
             u_f1(src) | u_qp(qp),
             qp)
  {}
};

// C.4.6.5  Flush Cache
class M28 : public M_Inst {
public:
  M28(uint x6,
      Register adr, PredicateRegister qp)
    : M_Inst(u_op(0x1) | u_x3(0) | u_x6a(x6) |
             u_r3(adr) | u_qp(qp),
             qp, adr)
  {}
};

// C.4.7.1  Move to AR - Register (M-Unit)
class M29 : public M_Inst {
public:
  M29(ApplicationRegister dst, Register src, PredicateRegister qp)
    : M_Inst(u_op(0x1) | u_x3(0)   | u_x6a(0x2A) |
             u_a3(dst) | u_r2(src) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.4.7.1  Move to AR - Immediate (M-Unit)
class M30 : public M_Inst {
public:
  M30(ApplicationRegister dst, uint imm8, PredicateRegister qp)
    : M_Inst(u_op(0x0) | u_x3(0)       | u_x2(2) | u_x4(8) |
             u_a3(dst) | u_imm8b(imm8) | u_qp(qp),
             dst, qp)
  {}
};

// C.4.7.3  Move from AR (M-Unit)
class M31 : public M_Inst {
public:
  M31(Register dst, ApplicationRegister src, PredicateRegister qp)
    : M_Inst(u_op(0x1) | u_x3(0)   | u_x6a(0x22) |
             u_a3(src) | u_r1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.4.8.1  Allocate Register Stack Frame
class M34 : public M_Inst {
public:
  M34(Register dst, uint i, uint l, uint o, uint r)
    : M_Inst(u_op(0x1)     | u_x3(6) |
             u_sor(r >> 3) | u_sol(i + l) | u_sof(i + l + o) |
             u_r1(dst),
             dst)
  { read(AR_PFS); }
};

// Move to PSR
class M35 : public M_Inst {
public:
  M35(uint x6,
      Register src, PredicateRegister qp)
    : M_Inst(u_op(0x1) | u_x3(0) | u_x6a(x6) |
             u_r2(src) | u_qp(qp),
             qp, src)
  {}
};

// Move from PSR
class M36 : public M_Inst {
public:
  M36(uint x6,
      Register dst, PredicateRegister qp)
    : M_Inst(u_op(0x1) | u_x3(0) | u_x6a(x6) |
             u_r1(dst) | u_qp(qp),
             dst, qp)
  {}
};

// C.4.8.4  Break/Nop (M-Unit)
class M37 : public M_Inst {
public:
  M37(uint x4,
      uint imm21, PredicateRegister qp)
    : M_Inst(u_op(0x0) | u_x3(0) | u_x2(0) | u_x4(x4) | u_imm21u(imm21) |
             u_qp(qp),
             qp)
  {}
};


// Branch B-Unit
class B_Inst : public Inst {
public:
  // B2
  B_Inst(uint41_t inst)
    : Inst(inst, B_inst)
  {}

  // B1, B8, B9
  B_Inst(uint41_t inst, PredicateRegister qp)
    : Inst(inst, B_inst)
  { read(qp); }

  // B3
  B_Inst(uint41_t inst, BranchRegister dst, PredicateRegister qp)
    : Inst(inst, B_inst)
  { write(dst); read(qp); }

  // B4
  B_Inst(uint41_t inst, PredicateRegister qp, BranchRegister src)
    : Inst(inst, B_inst)
  { read(src); read(qp); }

  // B5
  B_Inst(uint41_t inst, BranchRegister dst, BranchRegister src, PredicateRegister qp)
    : Inst(inst, B_inst)
  { write(dst); read(src); read(qp); }

  static uint41_t u_ih( uint ih)  { return u_field(ih,  35, 1); }
  static uint41_t u_whd(uint whd) { return u_field(whd, 32, 3); }
  static uint41_t u_wha(uint wha) { return u_field(wha,  3, 2); }

  static uint41_t u_t2e(uint imm9) { return u_field(imm9 & 0x18, 33-7, 2+7); }

  static uint41_t u_timm9(uint imm9) { return u_t2e(imm9) | u_7a(imm9); }
};

// C.5.1.1  IP-Relative Branch
class B1 : public B_Inst {
public:
  B1(uint btype, uint wh, uint p, uint d,
     uint target25, PredicateRegister qp)
    : B_Inst(u_op(0x4) | u_d(d)    | u_wh(wh) | u_p(p) | u_btype(btype) |
             u_target25b(target25) | u_qp(qp),
             qp) {
    set_is_ordered();
    set_pr_fast_read();
    if ((btype & 2) != 0) {
      // wtop, wexit
      modify(AR_EC);
    }
  }
};

// C.5.1.1  IP-Relative Branch
class B2 : public B_Inst {
public:
  B2(uint btype, uint wh, uint p, uint d,
     uint target25)
    : B_Inst(u_op(0x4) | u_d(d) | u_wh(wh) | u_p(p) | u_btype(btype) |
             u_target25b(target25)) {
    set_is_ordered();
    set_pr_fast_read();
    modify(AR_LC);
    if ((btype & 4) != 0) {
      // ctop, cexit
      modify(AR_EC);
    }
  }
};

// C.5.1.3  IP-Relative Call
class B3 : public B_Inst {
public:
  B3(uint wh, uint p, uint d,
     BranchRegister dst, uint target25, PredicateRegister qp)
    : B_Inst(u_op(0x5) | u_d(d)    | u_wh(wh)  | u_p(p) |
             u_target25b(target25) | u_b1(dst) | u_qp(qp),
             dst, qp)
  {
    set_is_ordered();
    set_pr_fast_read();
  }
};

// C.5.1.4  Indirect Branch
class B4 : public B_Inst {
public:
  B4(uint x6, uint btype, uint wh, uint p, uint d,
     BranchRegister src, PredicateRegister qp)
    : B_Inst(u_op(0x0) | u_d(d) | u_wh(wh) | u_x6a(x6) | u_p(p) | u_btype(btype) |
             u_b2(src) | u_qp(qp),
             qp, src)
  {
    set_is_ordered();
    set_pr_fast_read();
    set_br_fast_read();
  }
};

// C.5.1.5  Indirect Branch
class B5 : public B_Inst {
  static uint41_t call_indirect_mask() { return m_op(); }
  static uint41_t call_indirect_bits() { return u_op(1); }

public:
  B5(uint wh, uint p, uint d,
     BranchRegister dst, BranchRegister src, PredicateRegister qp)
    : B_Inst(u_op(0x1) | u_d(d)    | u_whd(wh) | u_p(p) |
             u_b2(src) | u_b1(dst) | u_qp(qp),
             dst, src, qp)
  {
    set_is_ordered();
    set_pr_fast_read();
    set_br_fast_read();
  }

  static bool is_call_indirect(uint templt, uint41_t X, uint legal_templts) {
    // Note use of bitset inclusion.  Ought to be generalized.
    return ((1 << templt) & legal_templts) != 0 &&
           (X & call_indirect_mask()) == call_indirect_bits();
  }
};

// Miscellaneous (B-Unit)
class B8 : public B_Inst {
public:
  B8(uint x6)
    : B_Inst(u_op(0x0) | u_x6a(x6))
  {}
};

// C.5.3.2  Break/Nop (B-Unit)
class B9 : public B_Inst {
public:
  B9(uint op,
     uint imm21, PredicateRegister qp)
    : B_Inst(u_op(op)        | u_x6a(0) |
             u_imm21u(imm21) | u_qp(qp),
             qp)
  {}
};


// Floating-point F Unit
class F_Inst : public Inst {
public:
  F_Inst(uint41_t inst)
    : Inst(inst, F_inst)
  {}

  // F1, F2, F3
  F_Inst(uint41_t inst, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3, PredicateRegister qp)
    : Inst(inst, F_inst)
  { write(dst); read(src1); read(src2); read(src3); read(qp); }

  // F4
  F_Inst(uint41_t inst, PredicateRegister p1, PredicateRegister p2, FloatRegister src1, FloatRegister src2, PredicateRegister qp)
    : Inst(inst, F_inst)
  { write(p1); write(p2); read(src1); read(src2); read(qp); }

  // F5
  F_Inst(uint41_t inst, PredicateRegister p1, PredicateRegister p2, FloatRegister src, PredicateRegister qp)
    : Inst(inst, F_inst)
  { write(p1); write(p2); read(src); read(qp); }

  // F6
  F_Inst(uint41_t inst, FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2, PredicateRegister qp)
    : Inst(inst, F_inst)
  { write(dst); write(p); read(src1); read(src2); read(qp); }

  // F7
  F_Inst(uint41_t inst, FloatRegister dst, PredicateRegister p, FloatRegister src, PredicateRegister qp)
    : Inst(inst, F_inst)
  { write(dst); write(p); read(src); read(qp); }

  // F8, F9
  F_Inst(uint41_t inst, FloatRegister dst, FloatRegister src1, FloatRegister src2, PredicateRegister qp)
    : Inst(inst, F_inst)
  { write(dst); read(src1); read(src2); read(qp); }

  // F10, F11
  F_Inst(uint41_t inst, FloatRegister dst, FloatRegister src, PredicateRegister qp)
    : Inst(inst, F_inst)
  { write(dst); read(src); read(qp); }

  // F12, F13, F14, F15
  F_Inst(uint41_t inst, PredicateRegister qp)
    : Inst(inst, F_inst)
  { read(qp); }


  static uint41_t u_rb(uint rb) { return u_field(rb, 36, 1); }
  static uint41_t u_q( uint q)  { return u_field(q,  36, 1); }
  static uint41_t u_x( uint x)  { return u_field(x,  36, 1); }
  static uint41_t u_sf(uint sf) { return u_field(sf, 34, 2); }
  static uint41_t u_x2(uint x2) { return u_field(x2, 34, 2); }
  static uint41_t u_ra(uint ra) { return u_field(ra, 33, 1); }
  static uint41_t u_xd(uint xd) { return u_field(xd, 33, 1); }
  static uint41_t u_ta(uint ta) { return u_field(ta, 12, 1); }

  static uint41_t u_omask(uint omask) { return u_field(omask, 20, 7); }
  static uint41_t u_amask(uint amask) { return u_field(amask, 13, 7); }

  static uint41_t u_fc2(     uint fclass) { return u_field(fclass & 0x3,    33,   2); }
  static uint41_t u_fclass7c(uint fclass) { return u_field(fclass & 0x1FC,  20-2, 7+2); }

  static uint41_t u_fclass(uint fclass) { return u_fc2(fclass) | u_fclass7c(fclass); }


  static uint inv_u_x( uint64_t x) { return inv_u_field(x, 36, 1); }
};

// Floating-point Multiply Add
class F1 : public F_Inst {
public:
  F1(uint op, uint x, uint sf,
     FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3, PredicateRegister qp)
    : F_Inst(u_op(op)   | u_x(x)     | u_sf(sf) |
             u_f4(src2) | u_f3(src1) | u_f2(src3) | u_f1(dst) | u_qp(qp),
             dst, src1, src2, src3, qp)
  {}
};

// Fixed-point Multiply Add
class F2 : public F_Inst {
public:
  F2(uint x2,
     FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3, PredicateRegister qp)
    : F_Inst(u_op(0xE)  | u_x(1)     | u_x2(x2)   |
             u_f4(src2) | u_f3(src1) | u_f2(src3) | u_f1(dst) | u_qp(qp),
             dst, src1, src2, src3, qp)
  {}
};

// Parellel Floating-point Select
class F3 : public F_Inst {
public:
  F3(FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3, PredicateRegister qp)
    : F_Inst(u_op(0xE)  | u_x(0)     |
             u_f4(src2) | u_f3(src1) | u_f2(src3) | u_f1(dst) | u_qp(qp),
             dst, src1, src2, src3, qp)
  {}
};

// C.6.3.1  Floating-point Compare
class F4 : public F_Inst {
public:
  F4(uint ra, uint rb, uint ta, uint sf,
     PredicateRegister p1, PredicateRegister p2, FloatRegister src1, FloatRegister src2, PredicateRegister qp)
    : F_Inst(u_op(0x4) | u_rb(rb)   | u_sf(sf)   | u_ra(ra) | u_ta(ta) |
             u_p2(p2)  | u_f3(src2) | u_f2(src1) | u_p1(p1) | u_qp(qp),
             p1, p2, src1, src2, qp)
  {}
};

// C.6.3.2  Floating-point Class
class F5 : public F_Inst {
public:
  F5(uint ta,
     PredicateRegister p1, PredicateRegister p2, FloatRegister src, uint fclass, PredicateRegister qp)
    : F_Inst(u_op(0x5) | u_ta(ta)  | u_fclass(fclass) |
             u_p2(p2)  | u_f2(src) | u_p1(p1) | u_qp(qp),
             p1, p2, src, qp)
  {}
};

// C.6.4.1  Floating-point Reciprocal Approximation
class F6 : public F_Inst {
public:
  F6(uint op, uint sf,
     FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2, PredicateRegister qp)
    : F_Inst(u_op(op) | u_q(0)     | u_xd(1)    | u_sf(sf)  |
             u_p2(p)  | u_f3(src2) | u_f2(src1) | u_f1(dst) | u_qp(qp),
             dst, p, src1, src2, qp)
  {}
};

// C.6.4.2  Floating-point Reciprocal Square Root Approximation
class F7 : public F_Inst {
public:
  F7(uint op, uint sf,
     FloatRegister dst, PredicateRegister p, FloatRegister src, PredicateRegister qp)
    : F_Inst(u_op(op) | u_q(1)    | u_xd(1)   | u_sf(sf)  |
             u_p2(p)  | u_f3(src) | u_f1(dst) | u_qp(qp),
             dst, p, src, qp)
  {}
};

// C.6.5    Floating-point Minimum/Maximum and Parallel Compare
class F8 : public F_Inst {
public:
  F8(uint op, uint x6, uint sf,
     FloatRegister dst, FloatRegister src1, FloatRegister src2, PredicateRegister qp)
    : F_Inst(u_op(op)   | u_sf(sf)   | u_xd(0)   | u_x6a(x6) |
             u_f3(src2) | u_f2(src1) | u_f1(dst) | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// C.6.6    Merge and Logical
class F9 : public F_Inst {
public:
  F9(uint op, uint x6,
     FloatRegister dst, FloatRegister src1, FloatRegister src2, PredicateRegister qp)
    : F_Inst(u_op(op)   | u_xd(0)    | u_x6a(x6) |
             u_f3(src2) | u_f2(src1) | u_f1(dst) | u_qp(qp),
             dst, src1, src2, qp)
  {}
};

// C.6.7.1  Convert Floating-point to Fixed-point
class F10 : public F_Inst {
public:
  F10(uint op, uint x6, uint truncate, uint sf,
      FloatRegister dst, FloatRegister src, PredicateRegister qp)
    : F_Inst(u_op(op)  | u_sf(sf)  | u_xd(0) | u_x6a(0x18 | x6 | (truncate << 1)) |
             u_f2(src) | u_f1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// C.6.7.2  Convert Fixed-point to Floating-point
class F11 : public F_Inst {
public:
  F11(FloatRegister dst, FloatRegister src, PredicateRegister qp)
    : F_Inst(u_op(0x0) | u_xd(0)   | u_x6a(0x1C) |
             u_f2(src) | u_f1(dst) | u_qp(qp),
             dst, src, qp)
  {}
};

// Floating-point Set Controls
class F12 : public F_Inst {
public:
  F12(uint sf,
      uint amask, uint omask,
      PredicateRegister qp)
    : F_Inst(u_op(0x0)      | u_sf(sf)       | u_xd(0) | u_x6a(4) |
             u_omask(omask) | u_amask(amask) | u_qp(qp),
             qp)
  {}
};

// Floating-point Clear Flags
class F13 : public F_Inst {
public:
  F13(uint sf,
      PredicateRegister qp)
    : F_Inst(u_op(0x0) | u_sf(sf) | u_xd(0) | u_x6a(5) |
             u_qp(qp),
             qp)
  {}
};

// Floating-point Check Flags
class F14 : public F_Inst {
public:
  F14(uint sf, uint target25,
      PredicateRegister qp)
    : F_Inst(u_op(0x0) | u_sf(sf)  | u_xd(0) | u_x6a(8) |
             u_target25u(target25) | u_qp(qp),
             qp)
  { set_is_ordered(); }

  static int64_t inv_target(uint41_t F) {
    return inv_target25u(F);
  }

  static void set_target(uint target25, uint41_t F, uint41_t& new_F) {
    new_F = (F & ~m_imm21u()) | u_target25u(target25);
  }
};

// C.6.9.1  Break/Nop (F-Unit)
class F15 : public F_Inst {
public:
  F15(uint x6,
      uint imm21, PredicateRegister qp)
    : F_Inst(u_op(0x0)       | u_xd(0) | u_x6a(x6) |
             u_imm21u(imm21) | u_qp(qp),
             qp)
  {}
};


// X-Unit
class X_Inst : public Inst {
private:
  uint41_t _imm41;

public:
  X_Inst(uint41_t inst, uint41_t imm41)
    : Inst(inst, X_inst),
      _imm41(imm41)
  {}

  // X1, X3
  X_Inst(uint41_t inst, uint41_t imm41,
         PredicateRegister qp)
    : Inst(inst, X_inst),
      _imm41(imm41)
  { read(qp); }

  // X2
  X_Inst(uint41_t inst, uint41_t imm41,
         Register dst, PredicateRegister qp)
    : Inst(inst, X_inst),
      _imm41(imm41)
  { write(dst); read(qp); }

  // X4
  X_Inst(uint41_t inst, uint41_t imm41,
         BranchRegister dst, PredicateRegister qp)
    : Inst(inst, X_inst),
      _imm41(imm41)
  { write(dst); read(qp); }

  static uint41_t u_vc(uint vc) { return u_field(vc, 20, 1); }

  static uint41_t u_i( uint64_t imm64) { return u_field(imm64 >> 63,      36,    1); }
  static uint41_t u_9d(uint64_t imm64) { return u_field(imm64 & 0xFF80,   27- 7, 9+ 7); }
  static uint41_t u_5c(uint64_t imm64) { return u_field(imm64 & 0x1F0000, 22-16, 5+16); }
  static uint41_t u_ic(uint64_t imm64) { return u_field(imm64 & 0x200000, 21-21, 1+21); }

  static uint41_t u_imm21b(uint64_t imm64) { return u_i(imm64) | u_20b(imm64 >> 4); }
  static uint41_t u_imm23( uint64_t imm64) { return u_i(imm64) | u_9d(imm64) | u_5c(imm64) | u_ic(imm64) | u_7b(imm64); }

  // Construct values for _imm41
  static uint41_t u_imm39( uint64_t imm64) { return (imm64 >> 22) & 0x1FFFFFFFFFC; }
  static uint41_t u_imm41s(uint64_t imm64) { return (imm64 >> 22) & 0x1FFFFFFFFFF; }
  static uint41_t u_imm41u(uint64_t imm62) { return (imm62 >> 21) & 0x1FFFFFFFFFF; }


  static uint64_t inv_i_at( uint64_t x, size_t pos) { return inv_u_field_at(x, 36, 1, pos); }
  static uint64_t inv_9d_at(uint64_t x, size_t pos) { return inv_u_field_at(x, 27, 9, pos); }
  static uint64_t inv_5c_at(uint64_t x, size_t pos) { return inv_u_field_at(x, 22, 5, pos); }
  static uint64_t inv_ic_at(uint64_t x, size_t pos) { return inv_u_field_at(x, 21, 1, pos); }

  static uint64_t inv_u_imm21b(uint64_t x) { return inv_i_at(x, 59) | inv_20b_at(x, 0); }
  static uint64_t inv_u_imm23( uint64_t x) {
    return inv_i_at(x, 63) | inv_ic_at(x, 21) | inv_5c_at(x, 16) | inv_9d_at(x, 7) | inv_7b_at(x, 0);
  }

  static uint64_t inv_u_imm39_at( uint64_t x, size_t pos) { return inv_u_field_at(x, 2, 39, pos); }
  static uint64_t inv_u_imm41s_at(uint64_t x, size_t pos) { return inv_u_field_at(x, 0, 41, pos); }
  static uint64_t inv_u_imm41u_at(uint64_t x, size_t pos) { return inv_u_field_at(x, 0, 41, pos); }


  static uint64_t m_9d()  { return fmask(27, 9); }
  static uint64_t m_5c()  { return fmask(22, 5); }
  static uint64_t m_ic()  { return fmask(21, 1); }

  static uint64_t m_imm23() { return m_i() | m_9d() | m_5c() | m_ic() | m_7b(); }


  uint41_t L() { return _imm41; }
};

// C.7.1    Break/Nop (X Unit)
class X1 : public X_Inst {
public:
  X1(uint x6,
     uint64_t imm62, PredicateRegister qp)
    : X_Inst(u_op(0x0)       | u_x3(0) | u_x6a(x6) |
             u_imm21u(imm62) | u_qp(qp),
             u_imm41u(imm62),
             qp)
  {}
};

// C.7.2    Move Long Immediate(64)
class X2 : public X_Inst {
private:
  static uint41_t movl_mask() { return m_op()  | m_vc(); }
  static uint41_t movl_bits() { return u_op(6) | u_vc(0); }

public:
  X2(Register dst, uint64_t imm64, PredicateRegister qp)
    : X_Inst(movl_bits() |
             u_r1(dst)   | u_imm23(imm64) | u_qp(qp),
             u_imm41s(imm64),
             dst, qp)
  {}

  static uint64_t inv_imm(uint41_t X, uint41_t L) { return inv_u_imm23(X) | inv_u_imm41s_at(L, 22); }

  static void set_imm(uint64_t imm64, uint41_t X, uint41_t& new_X, uint41_t& new_L) {
    new_X = (X & ~m_imm23()) | u_imm23(imm64);
    new_L = u_imm41s(imm64);
  }

  static bool is_movl(uint templt, uint41_t X, uint legal_templts) {
    // Note use of bitset inclusion.  Ought to be generalized.
    return ((1 << templt) & legal_templts) != 0 &&
           (X & movl_mask()) == movl_bits();
  }
};

// C.7.3.1  Long Branch
class X3 : public X_Inst {
public:
  X3(uint btype, uint wh, uint p, uint d,
     uint64_t target64, PredicateRegister qp)
    : X_Inst(u_op(0xC) | u_d(d) | u_wh(wh) | u_p(p) | u_btype(btype) |
             u_imm21b(target64) | u_qp(qp),
             u_imm39(target64),
             qp)
  { set_is_ordered(); }

  static int64_t inv_target(uint41_t X, uint41_t L) { return (inv_u_imm21b(X) | inv_u_imm39_at(L, 20)) << 4; }

  static void set_target(uint64_t target64, uint41_t X, uint41_t& new_X, uint41_t& new_L) {
    new_X = (X & ~m_imm21b()) | u_imm21b(target64);
    new_L = u_imm39(target64);
  }
};

// C.7.3.2  Long Call
class X4 : public X_Inst {
public:
  X4(uint wh, uint p, uint d,
     BranchRegister dst, uint64_t target64, PredicateRegister qp)
    : X_Inst(u_op(0xC) | u_d(d) | u_wh(wh)  | u_p(p) |
             u_imm21b(target64) | u_b1(dst) | u_qp(qp),
             u_imm39(target64),
             dst, qp)
  { set_is_ordered(); }
};


//=============================================================================
// IPF Bundle
//=============================================================================

class IPF_Bundle VALUE_OBJ_CLASS_SPEC {
public:
  enum Template {
    MII     = 0,
    MIIs    = 1,
    MIsI    = 2,
    MIsIs   = 3,
    MLX     = 4,
    MLXs    = 5,

    MMI     = 8,
    MMIs    = 9,
    MsMI    = 10,
    MsMIs   = 11,
    MFI     = 12,
    MFIs    = 13,
    MMF     = 14,
    MMFs    = 15,
    MIB     = 16,
    MIBs    = 17,
    MBB     = 18,
    MBBs    = 19,

    BBB     = 22,
    BBBs    = 23,
    MMB     = 24,
    MMBs    = 25,

    MFB     = 28,
    MFBs    = 29,

    illegal = 31,
    none    = 32
  };

  enum TemplateMask {
    m_MII     = 1 << MII,
    m_MIIs    = 1 << MIIs,
    m_MIsI    = 1 << MIsI,
    m_MIsIs   = 1 << MIsIs,
    m_MLX     = 1 << MLX,
    m_MLXs    = 1 << MLXs,

    m_MMI     = 1 << MMI,
    m_MMIs    = 1 << MMIs,
    m_MsMI    = 1 << MsMI,
    m_MsMIs   = 1 << MsMIs,
    m_MFI     = 1 << MFI,
    m_MFIs    = 1 << MFIs,
    m_MMF     = 1 << MMF,
    m_MMFs    = 1 << MMFs,
    m_MIB     = 1 << MIB,
    m_MIBs    = 1 << MIBs,
    m_MBB     = 1 << MBB,
    m_MBBs    = 1 << MBBs,

    m_BBB     = 1 << BBB,
    m_BBBs    = 1 << BBBs,
    m_MMB     = 1 << MMB,
    m_MMBs    = 1 << MMBs,

    m_MFB     = 1 << MFB,
    m_MFBs    = 1 << MFBs,

    m_illegal = 1 << illegal
  };

private:
  uint64_t _bits[2];

  Template get_template(uint64_t bits0) const { return Template(bits0 & right_n_bits(5)); }

  uint41_t get_slot0(uint64_t bits0)                 const { return bits0 << 18 >> (18 + 5); }
  uint41_t get_slot1(uint64_t bits0, uint64_t bits1) const { return (bits1 << 41 >> (41 - 18)) | (bits0 >> 46); }
  uint41_t get_slot2(uint64_t bits1)                 const { return bits1 >> 23; }

  // A constant array to map a bundle template and slot to an execution unit
  static const uint8_t /* InstUnit */ bundle_layout[];
  static const uint8_t                first_group_length[];

public:
  static Inst::Unit get_unit(Template templt, uint slot) {
    return Inst::Unit(bundle_layout[templt << 2 | slot]);
  }

  static uint firstGroupLength(Template templt) {
    return first_group_length[templt];
  }

  Template get_template() const { return get_template(_bits[0]); }

  uint41_t get_slot0() const { return get_slot0(_bits[0]); }
  uint41_t get_slot1() const { return get_slot1(_bits[0], _bits[1]); }
  uint41_t get_slot2() const { return get_slot2(_bits[1]); }

  uint41_t get_slot(uint slot) {
    if (slot == 0) {
      return get_slot0();
    } else if (slot == 1) {
      return get_slot1();
    }
    assert(slot == 2, "bad slot index");
    return get_slot2();
  }

  void get(Template& templt, uint41_t& inst0, uint41_t& inst1, uint41_t& inst2) const {
    uint64_t bits0 = _bits[0];
    uint64_t bits1 = _bits[1];
    templt = get_template(bits0);
    inst0 = get_slot0(bits0);
    inst1 = get_slot1(bits0, bits1);
    inst2 = get_slot2(bits1);
  }

  void set_template(Template templt) {
    _bits[0] = (_bits[0] & ~Inst::fmask(0, 5)) | templt;
  }

  void set_slot0(uint41_t inst) {
    _bits[0] = (_bits[0] & ~Inst::fmask(5, 41)) | (inst << 5);
  }

  void set_slot1(uint41_t inst) {
    _bits[0] = (_bits[0] & ~Inst::fmask(46, 18)) | (inst << 46);
    _bits[1] = (_bits[1] & ~Inst::fmask( 0, 23)) | (inst >> 18);
  }

  void set_slot2(uint41_t inst) {
    _bits[1] = (_bits[1] & ~Inst::fmask(23, 41)) | (inst << 23);
  }

  void set_slot(uint41_t inst, uint slot) {
    if (slot == 0) {
      set_slot0(inst);
    } else if (slot == 1) {
      set_slot1(inst);
    } else {
      assert(slot == 2, "bad slot index");
      set_slot2(inst);
    }
  }

  void set(Template templt, uint41_t inst0, uint41_t inst1, uint41_t inst2) {
    _bits[0] = inst1 << 46 | inst0 << 5 | templt;
    _bits[1] = inst2 << 23 | inst1 >> 18;
  }

  // Support halves, which are 64 bit quantities
  uint64_t get_half0() const { return _bits[0]; }
  uint64_t get_half1() const { return _bits[1]; }

  void set_half0(uint64_t bits) { _bits[0] = bits; }
  void set_half1(uint64_t bits) { _bits[1] = bits; }

  uint64_t *addr_half0() { return &_bits[0]; }
  uint64_t *addr_half1() { return &_bits[1]; }
};


//=============================================================================
// EmitSpecifier
//=============================================================================

class EmitSpecifier {

private:
  uint8_t              _length;
  IPF_Bundle::Template _kind[2];

public:
  EmitSpecifier()
    : _length(0)
  {
    _kind[0] = IPF_Bundle::none;
    _kind[1] = IPF_Bundle::none;
  }

  EmitSpecifier(IPF_Bundle::Template t0)
    : _length(1)
  {
    _kind[0] = t0;
    _kind[1] = IPF_Bundle::none;
  }

  EmitSpecifier(IPF_Bundle::Template t0, IPF_Bundle::Template t1)
    : _length(2)
  {
    _kind[0] = t0;
    _kind[1] = t1;
  }

  uint length() const { return _length; }

  IPF_Bundle::Template kind(uint i) const { return _kind[i]; }
};


//=============================================================================
// Address
//=============================================================================

// We don't use Address's for anything at the moment

class Address VALUE_OBJ_CLASS_SPEC {
private:
  Register         _base;
  RelocationHolder _rspec;

public:
  // creation
  Address()
    : _base(GR0)
  {}

  Address(RelocationHolder const& rspec)
    : _base(GR0),
      _rspec(rspec)
  {}

  friend class Assembler;
};


//=============================================================================
// Argument
//=============================================================================

// Argument is an abstraction used to represent an outgoing
// actual argument or an incoming formal parameter, whether
// it resides in memory or in a register, in a manner consistent
// with the IA64 ABI.  This is often referred to as the native
// or C calling convention.

class Argument VALUE_OBJ_CLASS_SPEC {
private:
  int _number;

public:
  enum {
    n_register_parameters = 8, // 8 integer/pointer/fp parameters may be in registers
  };

  enum Sig {
    // bit 0 == 0 => integral
    // bit 0 == 1 => float
    // bit 1 == 0 => 4-byte
    // bit 1 == 1 => 8-byte
    // bit 2 == 1 => object
    int_sig    = 0,
    long_sig   = 2,
    float_sig  = 1,
    double_sig = 3,
    obj_sig    = 6,
    no_sig     = 7  // contradictory
  };

  static bool is_integral(Sig sig) { return (sig & 1) == 0; }
  static bool is_float(   Sig sig) { return (sig & 1) != 0; }
  static bool is_4byte(   Sig sig) { return (sig & 2) == 0; }
  static bool is_8byte(   Sig sig) { return (sig & 2) != 0; }
  static bool is_obj(     Sig sig) { return (sig & 4) != 0; }

  Argument(int number) : _number(number) {}

  int number() const { return _number; }

  Argument successor() const { return Argument(number() + 1); }

  // locating integer register-based arguments:
  bool is_register() const { return _number < n_register_parameters; }

  Register as_register() const {
    assert(is_register(), "must be a register argument");
    return as_Register(number() + 32);
  }

  int offset_in_frame() {
    return (number() - 6) * BytesPerWord;
  }

  int jni_offset_in_frame() {
    return (number() + 2) * BytesPerWord;
  }
};


//=============================================================================
// Function Descriptor
//=============================================================================

struct FuncDesc VALUE_OBJ_CLASS_SPEC {
private:
  address _entry;
  address _gp;

public:
  address entry() const { return _entry; }
  address gp()    const { return _gp; }

  void set_entry(address  entry) { _entry = entry; }
  void set_gp(   address  gp)    { _gp    = gp; }
  
  static ByteSize entry_offset() { return byte_offset_of(FuncDesc, _entry); }
  static ByteSize gp_offset()    { return byte_offset_of(FuncDesc, _gp); }
};


//=============================================================================
// Displacement
//=============================================================================
//
// A Displacement describes a field within a instruction which
// may be used together with a Label in order to refer to a yet unknown code
// position. Compressed Displacements stored in the instruction stream in place
// of a branch instruction are used to describe that instruction and to chain a
// list of instructions using the same Label.  An unbound Label refers to
// the most recent (highest offset) Displacement.  That Displacement in turn
// refers to the next most recent one and so on to the first reference to
// the unbound Label, which refers to itself.
//
// A compressed Displacement is simply a branch instruction whose displacement
// field contains the offset of the previous compressed Displacement in its
// upper 19 bits and the slot number of the previous compressed Displacement
// in its low 2 bits.
//
// An uncompressed Displacement consists of the bits of a branch instruction
// plus the the slot it occupies in its bundle plus a tag that identifies the
// binary format of its displacement field, the better to patch it with.

class Displacement : public AbstractDisplacement {
  friend class AbstractAssembler;
  friend class Assembler;
  friend class MacroAssembler;
  friend class AbstractDisplacement;

private:
  struct {
    uint64_t _inst   : 41, // Branch instruction bits
             _slot   :  2, // Slot within bundle
             _dclass :  3, // Inst::ImmedClass
                     : 18; // -UNUSED-
  };

  inline Displacement(int pos, const AbstractAssembler* masm);

  uint41_t         inst()   const { return _inst; }
  uint             slot()   const { return _slot; }
  Inst::ImmedClass dclass() const { return Inst::ImmedClass(_dclass); }

  inline void bind(Label& L, int pos, AbstractAssembler* masm);
  inline void next(Label& L) const;

#ifndef PRODUCT
  void print() {
    const char* s = "";
    tty->print("%s", s);
  }
#endif // PRODUCT
};


//=============================================================================
// Assembler
//=============================================================================

class Assembler : public AbstractAssembler  {
  friend class AbstractAssembler; // for the non-virtual hack

private:
  address _gp; // The value of GP within the shared library containing the Assembler.
               // It's initialized using a horrible hack in the Assembler constructor,
               // and then stored in all the function descriptors generated for stub code.
  static void gp_dummy();
  address gp() const { return _gp; }

  inline void set_pc_from_code();

  // IPF-specific methods for emitting code by pushing them thru the bundler
  inline void emit(Inst& inst);
  inline void emit(Inst& inst, Label* target);
  inline void emit(Inst& inst, address target, relocInfo::relocType rtype);
  inline void emit(X_Inst& inst);
  inline void emit(X_Inst& inst, Label* target);
  inline void emit(X_Inst& inst, address target, relocInfo::relocType rtype);

protected:
  #ifdef ASSERT
  void check_relocation(RelocationHolder const& rspec, int format);
  #endif

  // Displacement routines
  Displacement disp_at(Label& L) const                     { return Displacement(L.pos(), this); }
  void         disp_at_put(Label& L, Displacement& disp)   { ShouldNotReachHere(); }

public:
  inline void flush_bundle();
  inline void eog();

  // Override AbstractAssembler::flush() to make sure the current bundle is flushed
  inline void flush() { flush_bundle(); AbstractAssembler::flush(); }

  inline void explicit_bundling();
  inline void check_bundling();

  void bind(Label& L) {
    flush_bundle();
    AbstractAssembler::bind(L);
  }

  static bool is_simm(int x, int nbits) { return -( 1 << (nbits-1) )  <= x   &&   x  <  ( 1 << (nbits-1) ); }

  static bool is_simm22 (int x) { return is_simm(x,  22); }
  static bool is_simm14 (int x) { return is_simm(x,  14); }
  static bool is_simm9  (int x) { return is_simm(x,   9); }
  static bool is_simm8  (int x) { return is_simm(x,   8); }
  static bool is_simm8m1(int x) { return is_simm(x-1, 8); }

  static bool is_shift_constant(int x) { return (x > 0 && x <= 64); }

  // Branch Hints - taken, importance, prefetch
  //   Note: please leave the values for sptk, spnt, dptk, dpnt unchanged!
  enum Branch_Hint { 
    sptk    = 0, // static taken
    spnt    = 1, // static not-taken
    no_hint = 1,
    dptk    = 2, // dynamic taken
    dpnt    = 3  // dynamic not-taken
  };

  enum Importance_Hint { useless = 0, important = 1 };

  // Note: don't change these, they are hardware-based
  enum Prefetch_Hint {
    few = 0,  // few or none
    many 
  };

  // Note: don't change these, they are hardware-based
  enum Cache_Hint { // bcache, alat, ...
    keep = 0, // nc
    free = 1  // clr
  };

  enum Cmp_Result {
    None = 0, // cmp_ methods rely on this value
    Unc  = 1, // cmp_ methods rely on this value
    And_,
    Or_,
    OrAndCM,
    OrCM,
    AndCM,
    AndOrCM
  };

  enum Cmp_Relation {
    equal    = 0, // cmp_ methods rely on this value
    notEqual = 1, // cmp_ methods rely on this value
    less,
    lessEqual,
    greater,
    greaterEqual,
    lower,
    lowerEqual,
    higher,
    greaterUnsigned = higher,
    higherEqual,
    greaterEqualUnsigned = higherEqual,
    ordered,
    unordered,
    member,
    notmember
  };

  enum Float_Class {
    NatVal       = 0x100,
    QuietNan     = 0x080,
    SignalNan    = 0x040,
    Positive     = 0x001,
    Negative     = 0x002,
    Zero         = 0x004,
    Unnormalized = 0x008,
    Normalized   = 0x010,
    Infinity     = 0x020
  };
   


  // For use in building an array to invert the result
#define Cmp_Relation_Invert \
  notEqual, equal, greaterEqual, greater, lessEqual, less, higherEqual, higher, lowerEqual, lower, ordered, unordered

  // For use in building an array to reverse the order of the operands
#define Cmp_Relation_Reverse \
  equal, notEqual, greater, greaterEqual, less, lessEqual, higher, higherEqual, lower, lowerEqual, ordered, unordered

  // A constant array to invert relation types
  static const Cmp_Relation invert_relation[];

  // A constant array to reverse relation types
  static const Cmp_Relation reverse_relation[];

  // store uses release == 0x4
  enum Semaphore_Completer { acquire = 0, release = 0x4 };

  // Note: don't change these, they are hardware-based
  enum LoadStore_Hint { none = 0, nt1 = 1, nt2 = 2, nta = 3 };

  // Note: don't change these, they are hardware-based
  enum LoadStore_Type {
    normal          = 0,    // none
    ordered_release = 0x4,  // .rel - stores
    speculative     = 0x4,  // .s
    advanced        = 0x8,  // .a
    spec_adv        = 0xc,  // .sa
    bias            = 0x10, // .bias
    acquired        = 0x14, // .acq
    check_clr       = 0x20, // .c.clr
    check_noclr     = 0x24, // .c.nc
    ordered_clr     = 0x28  // .c.clr.acq
  };

  // Note: don't change these, they are hardware-based
  enum FloatStatusField {
    sf0 = 0,
    sf1 = 1,
    sf2 = 2,
    sf3 = 3
  };

  // Note: don't change these, they are hardware-based
  enum FloatClass_Set {
    positive = 1 << 0,
    negative = 1 << 1,
    zero     = 1 << 2,
    unnorm   = 1 << 3,
    norm     = 1 << 4,
    infinity = 1 << 5,
    snan     = 1 << 6,
    qnan     = 1 << 7,
    natval   = 1 << 8
  };

  // Note: don't change these, they are hardware-based
  enum Saturation {
    modulo = 0,
    sss    = 1,
    uuu    = 2,
    uus    = 3
  };

  // Note: don't change these, they are hardware-based
  enum pavg_Round {
    rup = 2,
    raz = 3
  };

  // Note: don't change these, they are hardware-based
  enum LinePrefetch_Type {
    ignorefault               = 0,
    nonexclusive              = 0,
    exclusive                 = 1,
    fault                     = 2,
    ignorefault_nonexclusive  = ignorefault | nonexclusive,
    ignorefault_exclusive     = ignorefault |    exclusive,
    fault_nonexclusive        =       fault | nonexclusive,
    fault_exclusive           =       fault |    exclusive
  };

  // Creation
  Assembler(CodeBuffer* code);

  // Misc.
  inline void align(int modulus);
  inline address emit_addr(address addr = NULL);

  // Function Descriptors
  inline address emit_fd(address entry = NULL);

  // Branches
  inline address target(Label& L);

  static Inst::ImmedClass get_immed_class(IPF_Bundle::Template templt, uint slot, uint41_t inst);

  inline static void patched_branch(int64_t dest_offset, IPF_Bundle::Template templt, uint slot,
                                    uint41_t inst, int64_t inst_offset, uint41_t& new_inst, uint41_t& new_L);
  inline static void patched_branch(int64_t dest_offset, IPF_Bundle::Template templt, uint slot,
                                    uint41_t inst, int64_t inst_offset, uint41_t& new_inst);

         static void patched_branch(int64_t dest_offset, Inst::ImmedClass dclass,
                                    uint41_t inst, int64_t inst_offset, uint41_t& new_inst, uint41_t& new_L);
  inline static void patched_branch(int64_t dest_offset, Inst::ImmedClass dclass,
                                    uint41_t inst, int64_t inst_offset, uint41_t& new_inst);

  inline static int64_t branch_destination(IPF_Bundle::Template templt, uint slot,
                                           uint41_t inst, int64_t offset, uint41_t L = 0);
         static int64_t branch_destination(Inst::ImmedClass dclass,
                                           uint41_t inst, int64_t offset, uint41_t L = 0);


  // Verification
  static void verify_signed_range(intptr_t imm, size_t size) {
    assert(imm >= -nth_bit(size-1) && imm < nth_bit(size-1), "signed immediate out of range");
  }

  static void verify_unsigned_range(uintptr_t imm, size_t size) {
    assert((imm & right_n_bits(size)) == imm, "unsigned immediate out of range");
  }

  static void verify_0to3(Register gr) {
    assert(gr->encoding() <= 3, "register out of range");
  }

  static void verify_shladd_count(uint count) {
    assert(count >= 1 && count <= 4, "shift count out of range");
  }

  static void verify_prel(Cmp_Relation prel) {
    assert(prel == equal || prel == greater, "bad parallel compare relation");
  }

  static void verify_pshift_count(uint count) {
    assert(count >= 1 && count <= 3, "shift count out of range");
  }

  static void verify_field_len(uint len, uint bound) {
    assert(len > 0 && len <= bound, "field length out of range");
  }

  static void verify_store(LoadStore_Type sttype, LoadStore_Hint sthint) {
    assert(sttype == normal || sttype == ordered_release, "bad store type");
    assert(sthint == none || sthint == nta, "bad store hint");
  }

  static void verify_load(LoadStore_Hint ldhint) {
    assert(ldhint != nt2, "bad load hint");
  }

  static void verify_fload(LoadStore_Type ldtype) {
    assert(ldtype != bias && ldtype != acquired && ldtype != ordered_clr, "bad float load type");
  }

  static void verify_fetchadd(int inc) {
    int abs_inc = inc >= 0 ? inc : -inc;
    assert(abs_inc == 16 || abs_inc == 8 || abs_inc == 4 || abs_inc == 1, "bad fetchadd increment");
  }

  static void verify_alloc(uint inputs, uint locals, uint outputs, uint rotates) {
    assert(inputs <= 96, "bad number of input registers");
    assert(locals <= 96, "bad number of local registers");
    assert(outputs <= 96, "bad number of output registers");
    assert((rotates & 7) == 0 && (rotates <= 96), "bad number of rotating registers");
    assert(inputs + locals + outputs + rotates <= 96, "bad frame size");
  }

  // Inquiry
  static bool in_signed_range(intptr_t imm, size_t size) {
    return imm >= -right_n_bits(size-1)-1 && imm <= right_n_bits(size-1);
  }

  static inline bool is_movl         (IPF_Bundle::Template templt, uint41_t X);
  static inline bool is_call_indirect(IPF_Bundle::Template templt, uint41_t X);

  // Swap registers
  static void swap(Register& one,          Register& two)          { Register          t = one; one = two; two = t; }
  static void swap(PredicateRegister& one, PredicateRegister& two) { PredicateRegister t = one; one = two; two = t; }

  // ======================================
  // I-unit
  // ======================================

  // ALU
  inline void adda(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void adda(                      Register dst, Register src1, Register src2);
  inline void add1(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void add1(                      Register dst, Register src1, Register src2);
  inline void adds(PredicateRegister qp, Register dst, int imm14,     Register src);
  inline void adds(                      Register dst, int imm14,     Register src);
  inline void addl(PredicateRegister qp, Register dst, int imm22,     Register src);
  inline void addl(                      Register dst, int imm22,     Register src);

  inline void Assembler::addp4(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void Assembler::addp4(                      Register dst, Register src1, Register src2);
  inline void Assembler::addp4(PredicateRegister qp, Register dst, int imm,       Register src);
  inline void Assembler::addp4(                      Register dst, int imm,       Register src);
  
  inline void shladd(  PredicateRegister qp, Register dst, Register src, uint count, Register addend);
  inline void shladd(                        Register dst, Register src, uint count, Register addend);
  inline void shladdp4(PredicateRegister qp, Register dst, Register src, uint count, Register addend);
  inline void shladdp4(                      Register dst, Register src, uint count, Register addend);
  
  inline void sub( PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void sub(                       Register dst, Register src1, Register src2);
  inline void sub1(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void sub1(                      Register dst, Register src1, Register src2);
  inline void sub( PredicateRegister qp, Register dst, int imm,       Register src);
  inline void sub(                       Register dst, int imm,       Register src);

  inline void and3(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void and3(                      Register dst, Register src1, Register src2);
  inline void and3(PredicateRegister qp, Register dst, int imm,       Register src);
  inline void and3(                      Register dst, int imm,       Register src);

  inline void andcm(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void andcm(                      Register dst, Register src1, Register src2);
  inline void andcm(PredicateRegister qp, Register dst, int imm,       Register src);
  inline void andcm(                      Register dst, int imm,       Register src);

  inline void or3(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void or3(                      Register dst, Register src1, Register src2);
  inline void or3(PredicateRegister qp, Register dst, int imm,       Register src);
  inline void or3(                      Register dst, int imm,       Register src);

  inline void xor3(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void xor3(                      Register dst, Register src1, Register src2);
  inline void xor3(PredicateRegister qp, Register dst, int imm,       Register src);
  inline void xor3(                      Register dst, int imm,       Register src);

  void cmp_( PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
             Cmp_Relation crel, Cmp_Result ctype, uint x2);
  void cmp0_(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src,
             Cmp_Relation crel, Cmp_Result ctype, uint x2);
  void cmp_( PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, int imm8, Register src,
             Cmp_Relation crel, Cmp_Result ctype, uint x2);

  inline void cmp( PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
                   Cmp_Relation crel, Cmp_Result ctype = None);
  inline void cmp(                       PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
                   Cmp_Relation crel, Cmp_Result ctype = None);
  inline void cmp0(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src,
                   Cmp_Relation crel, Cmp_Result ctype);
  inline void cmp0(                      PredicateRegister p1, PredicateRegister p2, Register src,
                   Cmp_Relation crel, Cmp_Result ctype);
  inline void cmp( PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, int imm, Register src,
                   Cmp_Relation crel, Cmp_Result ctype = None);
  inline void cmp(                       PredicateRegister p1, PredicateRegister p2, int imm, Register src,
                   Cmp_Relation crel, Cmp_Result ctype = None);
  
  inline void cmp4( PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
                    Cmp_Relation crel, Cmp_Result ctype = None);
  inline void cmp4(                       PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
                    Cmp_Relation crel, Cmp_Result ctype = None);
  inline void cmp40(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src,
                    Cmp_Relation crel, Cmp_Result ctype);
  inline void cmp40(                      PredicateRegister p1, PredicateRegister p2, Register src,
                    Cmp_Relation crel, Cmp_Result ctype);
  inline void cmp4( PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, int imm, Register src,
                    Cmp_Relation crel, Cmp_Result ctype = None);
  inline void cmp4(                       PredicateRegister p1, PredicateRegister p2, int imm, Register src,
                    Cmp_Relation crel, Cmp_Result ctype = None);

  // Multimedia ALU
  inline void padd1(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s);
  inline void padd1(                      Register dst, Register src1, Register src2, Saturation s);
  inline void padd2(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s);
  inline void padd2(                      Register dst, Register src1, Register src2, Saturation s);
  inline void padd4(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s);
  inline void padd4(                      Register dst, Register src1, Register src2, Saturation s);

  inline void psub1(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s);
  inline void psub1(                      Register dst, Register src1, Register src2, Saturation s);
  inline void psub2(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s);
  inline void psub2(                      Register dst, Register src1, Register src2, Saturation s);
  inline void psub4(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s);
  inline void psub4(                      Register dst, Register src1, Register src2, Saturation s);

  inline void pavg1(PredicateRegister qp, Register dst, Register src1, Register src2, pavg_Round r);
  inline void pavg1(                       Register dst, Register src1, Register src2, pavg_Round r);
  inline void pavg2(PredicateRegister qp, Register dst, Register src1, Register src2, pavg_Round r);
  inline void pavg2(                       Register dst, Register src1, Register src2, pavg_Round r);

  inline void pavgsub1(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void pavgsub1(                      Register dst, Register src1, Register src2);
  inline void pavgsub2(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void pavgsub2(                      Register dst, Register src1, Register src2);

  inline void pcmp1(PredicateRegister qp, Register dst, Register src1, Register src2, Cmp_Relation prel);
  inline void pcmp1(                      Register dst, Register src1, Register src2, Cmp_Relation prel);
  inline void pcmp2(PredicateRegister qp, Register dst, Register src1, Register src2, Cmp_Relation prel);
  inline void pcmp2(                      Register dst, Register src1, Register src2, Cmp_Relation prel);
  inline void pcmp4(PredicateRegister qp, Register dst, Register src1, Register src2, Cmp_Relation prel);
  inline void pcmp4(                      Register dst, Register src1, Register src2, Cmp_Relation prel);

  inline void pshladd2(PredicateRegister qp, Register dst, Register src1, uint count, Register src2);
  inline void pshladd2(                      Register dst, Register src1, uint count, Register src2);
  inline void pshradd2(PredicateRegister qp, Register dst, Register src1, uint count, Register src2);
  inline void pshradd2(                      Register dst, Register src1, uint count, Register src2);

  // Shift
  inline void shr( PredicateRegister qp, Register dst, Register src, Register count);
  inline void shr(                       Register dst, Register src, Register count);
  inline void shru(PredicateRegister qp, Register dst, Register src, Register count);
  inline void shru(                      Register dst, Register src, Register count);
  inline void shl( PredicateRegister qp, Register dst, Register src, Register count);
  inline void shl(                       Register dst, Register src, Register count);

  inline void popc(PredicateRegister qp, Register dst, Register src);
  inline void popc(                      Register dst, Register src);

  inline void shrp(PredicateRegister qp, Register dst, Register src1, Register src2, uint count);
  inline void shrp(                      Register dst, Register src1, Register src2, uint count);
  
  inline void extru(PredicateRegister qp, Register dst, Register src, uint pos, uint len);
  inline void extru(                      Register dst, Register src, uint pos, uint len);
  inline void extr( PredicateRegister qp, Register dst, Register src, uint pos, uint len);
  inline void extr(                       Register dst, Register src, uint pos, uint len);
  
  inline void depz(PredicateRegister qp, Register dst,           Register src, uint pos, uint len);
  inline void depz(                      Register dst,           Register src, uint pos, uint len);
  inline void depz(PredicateRegister qp, Register dst, int imm8,               uint pos, uint len);
  inline void depz(                      Register dst, int imm8,               uint pos, uint len);
  inline void dep( PredicateRegister qp, Register dst, int imm1, Register src, uint pos, uint len);
  inline void dep(                       Register dst, int imm1, Register src, uint pos, uint len);

  // Misc. I-Unit
  void tbit_(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src, uint pos,
             Cmp_Relation trel, Cmp_Result ctype, uint y);

  inline void tbit(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src, uint pos,
                   Cmp_Relation trel, Cmp_Result ctype = None);
  inline void tbit(                      PredicateRegister p1, PredicateRegister p2, Register src, uint pos,
                   Cmp_Relation trel, Cmp_Result ctype = None);
  inline void tnat(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src,
                   Cmp_Relation trel, Cmp_Result ctype = None);
  inline void tnat(                      PredicateRegister p1, PredicateRegister p2, Register src,
                   Cmp_Relation trel, Cmp_Result ctype = None);

  inline void breaki(PredicateRegister qp, uint imm21);
  inline void breaki(                      uint imm21);
  inline void nopi(  PredicateRegister qp, uint imm21 = 0);
  inline void nopi(                        uint imm21 = 0);

  inline void chksi(PredicateRegister qp, Register src, Label& target);
  inline void chksi(                      Register src, Label& target);

  inline void movi(  PredicateRegister qp, BranchRegister dst, Register src, uint tag13 = 0,
                     Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void movi(                        BranchRegister dst, Register src, uint tag13 = 0,
                     Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void movret(PredicateRegister qp, BranchRegister dst, Register src, uint tag13 = 0,
                     Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void movret(                      BranchRegister dst, Register src, uint tag13 = 0,
                     Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void movi(  PredicateRegister qp, BranchRegister dst, Register src, Label& tag,
                     Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void movi(                        BranchRegister dst, Register src, Label& tag,
                     Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void movret(PredicateRegister qp, BranchRegister dst, Register src, Label& tag,
                     Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void movret(                      BranchRegister dst, Register src, Label& tag,
                     Branch_Hint mwh = no_hint, Importance_Hint ih = useless);

  inline void movi(PredicateRegister qp, Register dst, BranchRegister src);
  inline void movi(                      Register dst, BranchRegister src);
  
  inline void mov_to_pr(   PredicateRegister qp, Register src, int mask17 = -1);
  inline void mov_to_pr(                         Register src, int mask17 = -1);
  inline void mov_to_prrot(PredicateRegister qp, int64_t imm44);
  inline void mov_to_prrot(                      int64_t imm44);

  inline void mov_from_ip(PredicateRegister qp, Register dst);
  inline void mov_from_ip(                      Register dst);
  inline void mov_from_pr(PredicateRegister qp, Register dst);
  inline void mov_from_pr(                      Register dst);

  inline void movi(PredicateRegister qp, ApplicationRegister dst, Register src);
  inline void movi(                      ApplicationRegister dst, Register src);
  inline void movi(PredicateRegister qp, ApplicationRegister dst, int imm8);
  inline void movi(                      ApplicationRegister dst, int imm8);
  inline void movi(PredicateRegister qp, Register dst, ApplicationRegister src);
  inline void movi(                      Register dst, ApplicationRegister src);
  
  inline void zxt1(PredicateRegister qp, Register dst, Register src);
  inline void zxt1(                      Register dst, Register src);
  inline void zxt2(PredicateRegister qp, Register dst, Register src);
  inline void zxt2(                      Register dst, Register src);
  inline void zxt4(PredicateRegister qp, Register dst, Register src);
  inline void zxt4(                      Register dst, Register src);

  inline void sxt1(PredicateRegister qp, Register dst, Register src);
  inline void sxt1(                      Register dst, Register src);
  inline void sxt2(PredicateRegister qp, Register dst, Register src);
  inline void sxt2(                      Register dst, Register src);
  inline void sxt4(PredicateRegister qp, Register dst, Register src);
  inline void sxt4(                      Register dst, Register src);

  inline void czx1l(PredicateRegister qp, Register dst, Register src);
  inline void czx1l(                      Register dst, Register src);
  inline void czx1r(PredicateRegister qp, Register dst, Register src);
  inline void czx1r(                      Register dst, Register src);
  inline void czx2l(PredicateRegister qp, Register dst, Register src);
  inline void czx2l(                      Register dst, Register src);
  inline void czx2r(PredicateRegister qp, Register dst, Register src);
  inline void czx2r(                      Register dst, Register src);
  
  // ======================================
  // M-unit
  // ======================================

  // (qp) ldsz.ldtype.ldhint r1 = [r3]
  inline void ld1(PredicateRegister qp, Register dst, Register addr,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld1(                      Register dst, Register addr,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld2(PredicateRegister qp, Register dst, Register addr,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld2(                      Register dst, Register addr,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld4(PredicateRegister qp, Register dst, Register addr,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld4(                      Register dst, Register addr,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld8(PredicateRegister qp, Register dst, Register addr,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld8(                      Register dst, Register addr,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  
  // (qp) ld8.fill.ldhint r1 = [r3]
  inline void ld8fill(PredicateRegister qp, Register dst, Register addr,
                      LoadStore_Hint ldhint = none);
  inline void ld8fill(                      Register dst, Register addr,
                      LoadStore_Hint ldhint = none);

  // (qp) ldsz.ldtype.ldhint r1 = [r3],r2
  inline void ld1(PredicateRegister qp, Register dst, Register addr, Register adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld1(                      Register dst, Register addr, Register adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld2(PredicateRegister qp, Register dst, Register addr, Register adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld2(                      Register dst, Register addr, Register adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld4(PredicateRegister qp, Register dst, Register addr, Register adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld4(                      Register dst, Register addr, Register adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld8(PredicateRegister qp, Register dst, Register addr, Register adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld8(                      Register dst, Register addr, Register adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);

  // (qp) ld8.fill.ldhint r1 = [r3],r2
  inline void ld8fill(PredicateRegister qp, Register dst, Register addr, Register adjust,
                      LoadStore_Hint ldhint = none);
  inline void ld8fill(                      Register dst, Register addr, Register adjust,
                      LoadStore_Hint ldhint = none);
  
  // (qp) ldsz.ldtype.ldhint r1 = [r3],imm9
  inline void ld1(PredicateRegister qp, Register dst, Register addr, int adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld1(                      Register dst, Register addr, int adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld2(PredicateRegister qp, Register dst, Register addr, int adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld2(                      Register dst, Register addr, int adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld4(PredicateRegister qp, Register dst, Register addr, int adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld4(                      Register dst, Register addr, int adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld8(PredicateRegister qp, Register dst, Register addr, int adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld8(                      Register dst, Register addr, int adjust,
                  LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  
  // (qp) ld8.fill.ldhint r1 = [r3],imm9
  inline void ld8fill(PredicateRegister qp, Register dst, Register addr, int adjust,
                      LoadStore_Hint ldhint = none);
  inline void ld8fill(                      Register dst, Register addr, int adjust,
                      LoadStore_Hint ldhint = none);
  
  // (qp) stsz.sttype.sthint [r3]=r2
  inline void st1(PredicateRegister qp, Register addr, Register src,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st1(                      Register addr, Register src,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st2(PredicateRegister qp, Register addr, Register src,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st2(                      Register addr, Register src,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st4(PredicateRegister qp, Register addr, Register src,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st4(                      Register addr, Register src,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st8(PredicateRegister qp, Register addr, Register src,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st8(                      Register addr, Register src,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);

  inline void st8spill(PredicateRegister qp, Register addr, Register src,
                       LoadStore_Hint sthint = none);
  inline void st8spill(                      Register addr, Register src,
                       LoadStore_Hint sthint = none);

  // (qp) stsz.sttype.sthint [r3]=r2,imm9
  inline void st1(PredicateRegister qp, Register addr, Register src, int adjust,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st1(                      Register addr, Register src, int adjust,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st2(PredicateRegister qp, Register addr, Register src, int adjust,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st2(                      Register addr, Register src, int adjust,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st4(PredicateRegister qp, Register addr, Register src, int adjust,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st4(                      Register addr, Register src, int adjust,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st8(PredicateRegister qp, Register addr, Register src, int adjust,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  inline void st8(                      Register addr, Register src, int adjust,
                  LoadStore_Type sttype = normal, LoadStore_Hint sthint = none);
  
  inline void st8spill(PredicateRegister qp, Register addr, Register src, int adjust,
                       LoadStore_Hint sthint = none);
  inline void st8spill(                      Register addr, Register src, int adjust,
                       LoadStore_Hint sthint = none);

  // (qp) ldffsz.ldtype.ldhint f1=[r3]
  inline void ldfs(PredicateRegister qp, FloatRegister dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfs(                      FloatRegister dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfd(PredicateRegister qp, FloatRegister dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfd(                      FloatRegister dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldf8(PredicateRegister qp, FloatRegister dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldf8(                      FloatRegister dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfe(PredicateRegister qp, FloatRegister dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfe(                      FloatRegister dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  
  inline void ldffill(PredicateRegister qp, FloatRegister dst, Register addr,
                      LoadStore_Hint ldhint = none);
  inline void ldffill(                      FloatRegister dst, Register addr,
                      LoadStore_Hint ldhint = none);
  
  // (qp) ldffsz.ldtype.ldhint f1=[r3],r2
  inline void ldfs(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfs(                      FloatRegister dst, Register addr, Register adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfd(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfd(                      FloatRegister dst, Register addr, Register adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldf8(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldf8(                      FloatRegister dst, Register addr, Register adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfe(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfe(                      FloatRegister dst, Register addr, Register adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  
  inline void ldffill(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                      LoadStore_Hint ldhint = none);
  inline void ldffill(                      FloatRegister dst, Register addr, Register adjust,
                      LoadStore_Hint ldhint = none);

  // (qp) ldffsz.ldtype.ldhint f1=[r3],imm9
  inline void ldfs(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfs(                      FloatRegister dst, Register addr, int adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfd(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfd(                      FloatRegister dst, Register addr, int adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldf8(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldf8(                      FloatRegister dst, Register addr, int adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfe(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfe(                      FloatRegister dst, Register addr, int adjust,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  
  inline void ldffill(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                      LoadStore_Hint ldhint = none);
  inline void ldffill(                      FloatRegister dst, Register addr, int adjust,
                      LoadStore_Hint ldhint = none);

  // (qp) stffsz.sthint [r3]=r2
  inline void stfs(PredicateRegister qp, Register addr, FloatRegister src,
                   LoadStore_Hint sthint = none);
  inline void stfs(                      Register addr, FloatRegister src,
                   LoadStore_Hint sthint = none);
  inline void stfd(PredicateRegister qp, Register addr, FloatRegister src,
                   LoadStore_Hint sthint = none);
  inline void stfd(                      Register addr, FloatRegister src,
                   LoadStore_Hint sthint = none);
  inline void stf8(PredicateRegister qp, Register addr, FloatRegister src,
                   LoadStore_Hint sthint = none);
  inline void stf8(                      Register addr, FloatRegister src,
                   LoadStore_Hint sthint = none);
  inline void stfe(PredicateRegister qp, Register addr, FloatRegister src,
                   LoadStore_Hint sthint = none);
  inline void stfe(                      Register addr, FloatRegister src,
                   LoadStore_Hint sthint = none);
  
  inline void stfspill(PredicateRegister qp, Register addr, FloatRegister src,
                       LoadStore_Hint sthint = none);
  inline void stfspill(                      Register addr, FloatRegister src,
                       LoadStore_Hint sthint = none);

  // (qp) stffsz.sthint [r3]=r2,imm9
  inline void stfs(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                   LoadStore_Hint sthint = none);
  inline void stfs(                      Register addr, FloatRegister src, int adjust,
                   LoadStore_Hint sthint = none);
  inline void stfd(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                   LoadStore_Hint sthint = none);
  inline void stfd(                      Register addr, FloatRegister src, int adjust,
                   LoadStore_Hint sthint = none);
  inline void stf8(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                   LoadStore_Hint sthint = none);
  inline void stf8(                      Register addr, FloatRegister src, int adjust,
                   LoadStore_Hint sthint = none);
  inline void stfe(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                   LoadStore_Hint sthint = none);
  inline void stfe(                      Register addr, FloatRegister src, int adjust,
                   LoadStore_Hint sthint = none);

  inline void stfspill(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                       LoadStore_Hint sthint = none);
  inline void stfspill(                      Register addr, FloatRegister src, int adjust,
                       LoadStore_Hint sthint = none);
  
  // (qp) ldfpfsz.ldhint f1,f2=[r3]
  inline void ldfps(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfps(                      FloatRegister dstlo, FloatRegister dsthi, Register addr,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfpd(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfpd(                      FloatRegister dstlo, FloatRegister dsthi, Register addr,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfp8(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfp8(                      FloatRegister dstlo, FloatRegister dsthi, Register addr,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  
  // (qp) ldfpfsz.ldhint f1,f2=[r3],8/16
  inline void ldfps(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfps(                      FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfpd(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfpd(                      FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfp8(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ldfp8(                      FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                    LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  
  inline void lfetch(PredicateRegister qp, Register addr,
                     LinePrefetch_Type lftype = ignorefault_nonexclusive, LoadStore_Hint ldhint = none);
  inline void lfetch(                      Register addr,
                     LinePrefetch_Type lftype = ignorefault_nonexclusive, LoadStore_Hint ldhint = none);
  inline void lfetch(PredicateRegister qp, Register addr, Register adjust,
                     LinePrefetch_Type lftype = ignorefault_nonexclusive, LoadStore_Hint ldhint = none);
  inline void lfetch(                      Register addr, Register adjust,
                     LinePrefetch_Type lftype = ignorefault_nonexclusive, LoadStore_Hint ldhint = none);
  inline void lfetch(PredicateRegister qp, Register addr, int adjust,
                     LinePrefetch_Type lftype = ignorefault_nonexclusive, LoadStore_Hint ldhint = none);
  inline void lfetch(                      Register addr, int adjust,
                     LinePrefetch_Type lftype = ignorefault_nonexclusive, LoadStore_Hint ldhint = none);

  // (qp) cmpxchgsz.sem.ldhint r1=[r3],r2,ar.ccv
  inline void cmpxchg1(PredicateRegister qp, Register dst, Register addr, Register src,
                       Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void cmpxchg1(                      Register dst, Register addr, Register src,
                       Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void cmpxchg2(PredicateRegister qp, Register dst, Register addr, Register src,
                       Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void cmpxchg2(                      Register dst, Register addr, Register src,
                       Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void cmpxchg4(PredicateRegister qp, Register dst, Register addr, Register src,
                       Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void cmpxchg4(                      Register dst, Register addr, Register src,
                       Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void cmpxchg8(PredicateRegister qp, Register dst, Register addr, Register src,
                       Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void cmpxchg8(                      Register dst, Register addr, Register src,
                       Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  
  // (qp) xchgsz.ldhint r1=[r3],r2
  inline void xchg1(PredicateRegister qp, Register dst, Register addr, Register src,
                    LoadStore_Hint ldhint = none);
  inline void xchg1(                      Register dst, Register addr, Register src,
                    LoadStore_Hint ldhint = none);
  inline void xchg2(PredicateRegister qp, Register dst, Register addr, Register src,
                    LoadStore_Hint ldhint = none);
  inline void xchg2(                      Register dst, Register addr, Register src,
                    LoadStore_Hint ldhint = none);
  inline void xchg4(PredicateRegister qp, Register dst, Register addr, Register src,
                    LoadStore_Hint ldhint = none);
  inline void xchg4(                      Register dst, Register addr, Register src,
                    LoadStore_Hint ldhint = none);
  inline void xchg8(PredicateRegister qp, Register dst, Register addr, Register src,
                    LoadStore_Hint ldhint = none);
  inline void xchg8(                      Register dst, Register addr, Register src,
                    LoadStore_Hint ldhint = none);
  
  inline void fetchadd4(PredicateRegister qp, Register dst, Register addr, int inc,
                        Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void fetchadd4(                      Register dst, Register addr, int inc,
                        Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void fetchadd8(PredicateRegister qp, Register dst, Register addr, int inc,
                        Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  inline void fetchadd8(                      Register dst, Register addr, int inc,
                        Semaphore_Completer sem, LoadStore_Hint ldhint = none);
  
  inline void setfsig(PredicateRegister qp, FloatRegister dst, Register src);
  inline void setfsig(                      FloatRegister dst, Register src);
  inline void setfexp(PredicateRegister qp, FloatRegister dst, Register src);
  inline void setfexp(                      FloatRegister dst, Register src);
  inline void setfs(  PredicateRegister qp, FloatRegister dst, Register src);
  inline void setfs(                        FloatRegister dst, Register src);
  inline void setfd(  PredicateRegister qp, FloatRegister dst, Register src);
  inline void setfd(                        FloatRegister dst, Register src);

  inline void getfsig(PredicateRegister qp, Register dst, FloatRegister src);
  inline void getfsig(                      Register dst, FloatRegister src);
  inline void getfexp(PredicateRegister qp, Register dst, FloatRegister src);
  inline void getfexp(                      Register dst, FloatRegister src);
  inline void getfs(  PredicateRegister qp, Register dst, FloatRegister src);
  inline void getfs(                        Register dst, FloatRegister src);
  inline void getfd(  PredicateRegister qp, Register dst, FloatRegister src);
  inline void getfd(                        Register dst, FloatRegister src);

  inline void chksm(PredicateRegister qp, Register src,      Label& target);
  inline void chksm(                      Register src,      Label& target);
  inline void chks( PredicateRegister qp, FloatRegister src, Label& target);
  inline void chks(                       FloatRegister src, Label& target);

  inline void chka(PredicateRegister qp, Register src,      Cache_Hint hint, Label& target);
  inline void chka(                      Register src,      Cache_Hint hint, Label& target);
  inline void chka(PredicateRegister qp, FloatRegister src, Cache_Hint hint, Label& target);
  inline void chka(                      FloatRegister src, Cache_Hint hint, Label& target);

  inline void invala(PredicateRegister qp = PR0);
  inline void fwb(   PredicateRegister qp = PR0);
  inline void mf(    PredicateRegister qp = PR0);
  inline void mfa(   PredicateRegister qp = PR0);
  inline void srlzd( PredicateRegister qp = PR0);
  inline void srlzi( PredicateRegister qp = PR0);
  inline void synci( PredicateRegister qp = PR0);
  
  inline void flushrs(PredicateRegister qp = PR0);
  inline void loadrs( PredicateRegister qp = PR0);
  
  inline void invalae(PredicateRegister qp, Register src);
  inline void invalae(                      Register src);
  inline void invalae(PredicateRegister qp, FloatRegister src);
  inline void invalae(                      FloatRegister src);

  inline void fc(  PredicateRegister qp, Register adr);
  inline void fc(                        Register adr);
  inline void ptce(PredicateRegister qp, Register adr);
  inline void ptce(                      Register adr);

  inline void movm(PredicateRegister qp, ApplicationRegister dst, Register src);
  inline void movm(                      ApplicationRegister dst, Register src);
  inline void movm(PredicateRegister qp, ApplicationRegister dst, int imm8);
  inline void movm(                      ApplicationRegister dst, int imm8);
  inline void movm(PredicateRegister qp, Register dst, ApplicationRegister src);
  inline void movm(                      Register dst, ApplicationRegister src);
  
  inline void alloc(Register saved_pfs,
                    uint inputs, uint locals, uint outputs, uint rotates);
  
  inline void mov_to_psrum(  PredicateRegister qp, Register src);
  inline void mov_to_psrum(                        Register src);
  inline void mov_from_psrum(PredicateRegister qp, Register src);
  inline void mov_from_psrum(                      Register src);

  inline void breakm(PredicateRegister qp, uint imm21);
  inline void breakm(                      uint imm21);
  inline void nopm(  PredicateRegister qp, uint imm21 = 0);
  inline void nopm(                        uint imm21 = 0);

  // ======================================
  // B-unit
  // ======================================

  inline void br(   PredicateRegister qp, address target, relocInfo::relocType rtype = relocInfo::none,
                    Branch_Hint bwh = sptk, Prefetch_Hint ph = few, Cache_Hint dh = keep);
  inline void br(                         address target, relocInfo::relocType rtype = relocInfo::none,
                    Branch_Hint bwh = sptk, Prefetch_Hint ph = few, Cache_Hint dh = keep);
  inline void br(   PredicateRegister qp, Label& target,
                    Branch_Hint bwh = sptk, Prefetch_Hint ph = few, Cache_Hint dh = keep);
  inline void br(                         Label& target,
                    Branch_Hint bwh = sptk, Prefetch_Hint ph = few, Cache_Hint dh = keep);

  inline void wexit(PredicateRegister qp, Label& target, Branch_Hint bwh = dptk, 
                    Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void wexit(                      Label& target, Branch_Hint bwh = dptk, 
                    Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void wtop( PredicateRegister qp, Label& target, Branch_Hint bwh = dptk, 
                    Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void wtop(                       Label& target, Branch_Hint bwh = dptk, 
                    Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void cloop(                      Label& target, Branch_Hint bwh = dptk, 
                    Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void cexit(                      Label& target, Branch_Hint bwh = dptk, 
                    Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void ctop(                       Label& target, Branch_Hint bwh = dptk, 
                    Prefetch_Hint ph = many, Cache_Hint dh = keep);

  inline void call(PredicateRegister qp, address target, relocInfo::relocType rtype = relocInfo::runtime_call_type,
                   BranchRegister link = RP, Branch_Hint = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void call(                      address target, relocInfo::relocType rtype = relocInfo::runtime_call_type,
                   BranchRegister link = RP, Branch_Hint = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void call(PredicateRegister qp, Label& target,
                   BranchRegister link = RP, Branch_Hint = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void call(                      Label& target,
                   BranchRegister link = RP, Branch_Hint = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);

  inline void br( PredicateRegister qp, BranchRegister target,
                  Branch_Hint bwh = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void br(                       BranchRegister target,
                  Branch_Hint bwh = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void ret(PredicateRegister qp, BranchRegister target = RP,
                  Branch_Hint bwh = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void ret(                      BranchRegister target = RP,
                  Branch_Hint bwh = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);

  inline void call(PredicateRegister qp, BranchRegister target, BranchRegister link = RP,
                   Branch_Hint = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);
  inline void call(                      BranchRegister target, BranchRegister link = RP,
                   Branch_Hint = sptk, Prefetch_Hint ph = many, Cache_Hint dh = keep);
  
  inline void cover();
  inline void clrrrb();
  inline void clrrrbpr();
  inline void epc();

  inline void breakb(PredicateRegister qp, uint imm21);
  inline void breakb(                      uint imm21);
  inline void nopb(  PredicateRegister qp, uint imm21 = 0);
  inline void nopb(                        uint imm21 = 0);

  // ======================================
  // F-unit
  // ======================================

  inline void fma(  PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fma(                        FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fmas( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fmas(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fmad( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fmad(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fpma( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fpma(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fms(  PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fms(                        FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fmss( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fmss(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fmsd( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fmsd(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fpms( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fpms(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fnma( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fnma(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fnmas(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fnmas(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fnmad(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fnmad(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fpnma(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);
  inline void fpnma(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                    FloatStatusField sf = sf0);

  inline void xmal( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);
  inline void xmal(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);
  inline void xmah( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);
  inline void xmah(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);
  inline void xmahu(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);
  inline void xmahu(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);

  inline void fselect(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);
  inline void fselect(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);

  // (qp) fcmp.fcrel.fctype.sf p1,p2=f2,f3
  void fcmp(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, FloatRegister src1, FloatRegister src2,
            Cmp_Relation fcrel, Cmp_Result fctype = None, FloatStatusField sf = sf0);
  void fcmp(                      PredicateRegister p1, PredicateRegister p2, FloatRegister src1, FloatRegister src2,
            Cmp_Relation fcrel, Cmp_Result fctype = None, FloatStatusField sf = sf0);
  
  // (qp) fclass.fcrel,fctype p1,p2=f2,fclass
  void fclass(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, FloatRegister src, Float_Class fclass,
              Cmp_Relation fcrel, Cmp_Result fctype = None);
  void fclass(                      PredicateRegister p1, PredicateRegister p2, FloatRegister src, uint fclass,
              Cmp_Relation fcrel, Cmp_Result fctype = None);

  inline void frcpa( PredicateRegister qp, FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);
  inline void frcpa(                       FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);
  inline void fprcpa(PredicateRegister qp, FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);
  inline void fprcpa(                      FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);
  
  inline void frsqrta( PredicateRegister qp, FloatRegister dst, PredicateRegister p, FloatRegister src,
                       FloatStatusField sf = sf0);
  inline void frsqrta(                       FloatRegister dst, PredicateRegister p, FloatRegister src,
                       FloatStatusField sf = sf0);
  inline void fprsqrta(PredicateRegister qp, FloatRegister dst, PredicateRegister p, FloatRegister src,
                       FloatStatusField sf = sf0);
  inline void fprsqrta(                      FloatRegister dst, PredicateRegister p, FloatRegister src,
                       FloatStatusField sf = sf0);

  inline void fmin( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fmin(                       FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fmax( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fmax(                       FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void famin(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void famin(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void famax(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void famax(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);

  inline void fmerges( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmerges(                       FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmergens(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmergens(                      FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmergese(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmergese(                      FloatRegister dst, FloatRegister src1, FloatRegister src2);

  inline void fmixlr(   PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmixlr(                         FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmixr(    PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmixr(                          FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmixl(    PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fmixl(                          FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fsxtr(    PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fsxtr(                          FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fsxtl(    PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fsxtl(                          FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fpack(    PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fpack(                          FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fswap(    PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fswap(                          FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fswapnl(  PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fswapnl(                        FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fswapnr(  PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fswapnr(                        FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fand(     PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fand(                           FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fandcm(   PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fandcm(                         FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void for3(     PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void for3(                           FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fxor(     PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fxor(                           FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fpmerges( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fpmerges(                       FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fpmergens(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fpmergens(                      FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fpmergese(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void fpmergese(                      FloatRegister dst, FloatRegister src1, FloatRegister src2);

  inline void fcvtfx(  PredicateRegister qp, FloatRegister dst, FloatRegister src, bool truncate = true,
                       FloatStatusField sf = sf0); 
  inline void fcvtfx(                        FloatRegister dst, FloatRegister src, bool truncate = true,
                       FloatStatusField sf = sf0); 
  inline void fcvtfxu( PredicateRegister qp, FloatRegister dst, FloatRegister src, bool truncate = true,
                       FloatStatusField sf = sf0); 
  inline void fcvtfxu(                       FloatRegister dst, FloatRegister src, bool truncate = true,
                       FloatStatusField sf = sf0); 
  inline void fpcvtfx( PredicateRegister qp, FloatRegister dst, FloatRegister src, bool truncate = true,
                       FloatStatusField sf = sf0); 
  inline void fpcvtfx(                       FloatRegister dst, FloatRegister src, bool truncate = true,
                       FloatStatusField sf = sf0); 
  inline void fpcvtfxu(PredicateRegister qp, FloatRegister dst, FloatRegister src, bool truncate = true,
                       FloatStatusField sf = sf0); 
  inline void fpcvtfxu(                      FloatRegister dst, FloatRegister src, bool truncate = true,
                       FloatStatusField sf = sf0); 

  inline void fcvtxf(PredicateRegister qp, FloatRegister dst, FloatRegister src);
  inline void fcvtxf(                      FloatRegister dst, FloatRegister src);
  
  inline void fsetc(PredicateRegister qp, uint amask, uint omask, FloatStatusField sf = sf0);
  inline void fsetc(                      uint amask, uint omask, FloatStatusField sf = sf0);
  inline void fclrf(PredicateRegister qp, FloatStatusField sf = sf0);
  inline void fclrf(                      FloatStatusField sf = sf0);
  inline void fchkf(PredicateRegister qp, Label& target, FloatStatusField sf = sf0);
  inline void fchkf(                      Label& target, FloatStatusField sf = sf0);

  inline void breakf(PredicateRegister qp, uint imm21);
  inline void breakf(                      uint imm21);
  inline void nopf(  PredicateRegister qp, uint imm21 = 0);
  inline void nopf(                        uint imm21 = 0);

  // ======================================
  // X-unit
  // ======================================

  inline void breakx(PredicateRegister qp, uint64_t imm62);
  inline void breakx(                      uint64_t imm62);
  inline void nopx(  PredicateRegister qp, uint64_t imm62 = 0);
  inline void nopx(                        uint64_t imm62 = 0);

  inline void movl(PredicateRegister qp, Register dst, int64_t imm64);
  inline void movl(                      Register dst, int64_t imm64);
  inline void mova(PredicateRegister qp, Register dst, address addr, relocInfo::relocType rtype = relocInfo::none);
  inline void mova(                      Register dst, address addr, relocInfo::relocType rtype = relocInfo::none);

  inline void brl(PredicateRegister qp, Label& target,
                  Branch_Hint bwh = sptk, Prefetch_Hint ph = few, Cache_Hint dh = keep);
  inline void brl(                      Label& target,
                  Branch_Hint bwh = sptk, Prefetch_Hint ph = few, Cache_Hint dh = keep);

  inline void calll(PredicateRegister qp, Label& target, BranchRegister link = RP,
                    Branch_Hint = sptk, Prefetch_Hint ph = few, Cache_Hint dh = keep);
  inline void calll(                      Label& target, BranchRegister link = RP,
                    Branch_Hint = sptk, Prefetch_Hint ph = few, Cache_Hint dh = keep);
};


// MacroAssembler extends Assembler by frequently used macros.
//
// Instructions for which a 'better' code sequence exists depending
// on arguments should also go in here.

class MacroAssembler: public Assembler {
 protected:
  // Support for VM calls
  //
  // This is the base routine called by the different versions of call_VM_leaf. The interpreter
  // may customize this version by overriding it for its purposes (e.g., to save/restore
  // additional registers when doing a VM call).
  
  virtual void call_VM_leaf_base(address entry_point);

  // It is imperative that all calls into the VM are handled via the call_VM macros.
  // They make sure that the stack linkage is setup correctly. call_VM's correspond
  // to ENTRY/ENTRY_X entry points while call_VM_leaf's correspond to LEAF entry points.
  //
  // This is the base routine called by the different versions of call_VM. The interpreter
  // may customize this version by overriding it for its purposes (e.g., to save/restore
  // additional registers when doing a VM call).
  //
  // A non-volatile java_thread_cache register should be specified so that the Gthread
  // value can be preserved across the call.  (If java_thread_cache is noreg, then a slow
  // get_thread call will re-initialize the Gthread.) call_VM_base returns the register
  // that contains the thread.
  //
  // If no last_java_sp is specified (noreg) than SP will be used instead.

  virtual int call_VM_base(
    Register        oop_result,             // where an oop-result ends up if any; use noreg otherwise
    Register        java_thread_cache,      // the thread if computed before     ; use noreg otherwise
    Register        last_java_sp,           // to set up last_Java_frame in stubs; use noreg otherwise
    address         entry_point,            // the entry point
    bool            check_exception=true    // flag which indicates if exception should be checked
  );

 public:
  MacroAssembler(CodeBuffer* code) : Assembler(code) {}

  // Support for NULL-checks
  //
  // Generates code that causes a NULL OS exception if the content of reg is NULL.
  // If the accessed location is M[reg + offset] and the offset is known, provide the
  // offset. No explicit code generation is needed if the offset is within a certain
  // range (0 <= offset <= page_size).

  void null_check(Register reg, int offset = -1);
  static bool needs_explicit_null_check(intptr_t offset);

  // Alignment - not needed with a 128 bit/16 byte bundle
  void align(int modulus) {}

  // Fetch the result of a runtime call
  void get_vm_result(  Register oop_result);
  void get_vm_result_2(Register oop_result);

  // Support for last Java frame (but use call_VM instead where possible)
  // caller only needs to set last_Java_sp
  int set_last_Java_frame(Register last_java_sp);
  // Let caller specify the entire top frame
  void set_last_Java_frame(Register last_java_sp, Register last_java_fp, Register last_java_pc);
  void reset_last_Java_frame();

  // Call into the VM.
  // Passes the thread pointer (in GR_ARG0) as a prepended argument.
  // Makes sure oop return values are visible to the GC.
  void call_VM(Register oop_result, address entry_point, bool check_exceptions = true);
  void call_VM(Register oop_result, address entry_point, Register arg_1, bool check_exceptions = true);
  void call_VM(Register oop_result, address entry_point, Register arg_1, Register arg_2, bool check_exceptions = true);
  void call_VM(Register oop_result, address entry_point, Register arg_1, Register arg_2, Register arg_3, bool check_exceptions = true);

  void call_VM(Register oop_result, Register last_java_sp, address entry_point, bool check_exceptions = true);
  void call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, bool check_exceptions = true);
  void call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2, bool check_exceptions = true);
  void call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2, Register arg_3, bool check_exceptions = true);

  void call_VM_leaf(address entry_point);
  void call_VM_leaf(address entry_point, Register arg_1);
  void call_VM_leaf(address entry_point, Register arg_1, Register arg_2);
  void call_VM_leaf(address entry_point, Register arg_1, Register arg_2, Register arg_3);

  // Call into the VM, returning the offset of a pc with which to associate an OopMap entry.
  int call_VM_for_oopmap(Register oop_result, address entry_point, bool check_exceptions = true);

  // Return to the next bundle.
  void return_from_dummy_frame();

  // Push and pop an thin == minimal == compiler frame on/off the register stack
  void push_thin_frame();
  void pop_thin_frame();
  void pop_dummy_thin_frame();

  // Push and pop a full == interpreter frame on/off the register stack
  void push_full_frame();
  void save_full_frame(const uint frame_size_in_bytes);
  void pop_full_to_thin_frame();
  void pop_full_frame();

  void push_dummy_thin_frame(const Register fake_RP);
  void push_dummy_full_frame(const Register fake_RP);
  void pop_dummy_full_frame();

  // Save/restore scratch registers

  void push_scratch_regs();
  void pop_scratch_regs();

  // Stack layout of saved registers in push_scratch_regs

  enum push_regs_layout {

    abi_scratch1_off, abi_scratch2_off,

    gr2_off,  gr3_off,  gr8_off,  gr9_off,  
    gr10_off, gr11_off, gr14_off, gr15_off, 
    gr16_off, gr17_off, gr18_off, gr19_off, 
    gr20_off, gr21_off, gr22_off, gr23_off, 
    gr24_off, gr25_off, gr26_off, gr27_off, 
    gr28_off, gr29_off, gr30_off, gr31_off,

    // Float spills are 16 bytes
    fr6spill_off,  fr6Hspill_off,  fr7spill_off,  fr7Hspill_off,
    fr8spill_off,  fr8Hspill_off,  fr9spill_off,  fr9Hspill_off,
    fr10spill_off, fr10Hspill_off, fr11spill_off, fr11Hspill_off,
    fr12spill_off, fr12Hspill_off, fr13spill_off, fr13Hspill_off,
    fr14spill_off, fr14Hspill_off, fr15spill_off, fr15Hspill_off,

    // Branch and Predicate Registers 
    br6_off, br7_off, pr_off, pad_off,

    // Floats stored as doubles for deopt
    // contents are undefined if float register contains NatVal
    fr6_off,  fr7_off,  fr8_off,  fr9_off,  
    fr10_off, fr11_off, fr12_off, fr13_off, 
    fr14_off, fr15_off, 

    push_regs_size
  };


  // Add
  inline void add(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void add(                      Register dst, Register src1, Register src2);
  inline void add(PredicateRegister qp, Register dst, int imm,       Register src);
  inline void add(                      Register dst, int imm,       Register src);
  inline void add(PredicateRegister qp, Register dst, Register src,  int imm);
  inline void add(                      Register dst, Register src,  int imm);

  // Subtract
  inline void sub(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void sub(                      Register dst, Register src1, Register src2);
  inline void sub(PredicateRegister qp, Register dst, Register src,  int imm);
  inline void sub(                      Register dst, Register src,  int imm);
  inline void sub(PredicateRegister qp, Register dst, int imm,       Register src);
  inline void sub(                      Register dst, int imm,       Register src);

  // Logicals
  inline void and3( PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void and3(                       Register dst, Register src1, Register src2);
  inline void and3( PredicateRegister qp, Register dst, int imm,       Register src);
  inline void and3(                       Register dst, int imm,       Register src);
  inline void and3( PredicateRegister qp, Register dst, Register src,  int imm);
  inline void and3(                       Register dst, Register src,  int imm);

  inline void andcm(PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void andcm(                      Register dst, Register src1, Register src2);
  inline void andcm(PredicateRegister qp, Register dst, int imm,       Register src);
  inline void andcm(                      Register dst, int imm,       Register src);
  inline void andcm(PredicateRegister qp, Register dst, Register src,  int imm);
  inline void andcm(                      Register dst, Register src,  int imm);

  inline void or3(  PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void or3(                        Register dst, Register src1, Register src2);
  inline void or3(  PredicateRegister qp, Register dst, int imm,       Register src);
  inline void or3(                        Register dst, int imm,       Register src);
  inline void or3(  PredicateRegister qp, Register dst, Register src,  int imm);
  inline void or3(                        Register dst, Register src,  int imm);

  inline void xor3( PredicateRegister qp, Register dst, Register src1, Register src2);
  inline void xor3(                       Register dst, Register src1, Register src2);
  inline void xor3( PredicateRegister qp, Register dst, int imm,       Register src);
  inline void xor3(                       Register dst, int imm,       Register src);
  inline void xor3( PredicateRegister qp, Register dst, Register src,  int imm);
  inline void xor3(                       Register dst, Register src,  int imm);

  // Move
  inline void mov(PredicateRegister qp, Register dst, Register src);
  inline void mov(                      Register dst, Register src);
  inline void mov(PredicateRegister qp, Register dst, int imm);
  inline void mov(                      Register dst, int imm);

  inline void mov(PredicateRegister qp, ApplicationRegister dst, Register src);
  inline void mov(                      ApplicationRegister dst, Register src);
  inline void mov(PredicateRegister qp, ApplicationRegister dst, int imm8);
  inline void mov(                      ApplicationRegister dst, int imm8);
  inline void mov(PredicateRegister qp, Register dst, ApplicationRegister src);
  inline void mov(                      Register dst, ApplicationRegister src);

  inline void mov(PredicateRegister qp, BranchRegister dst, Register src, uint tag13 = 0,
                  Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void mov(                      BranchRegister dst, Register src, uint tag13 = 0,
                  Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void mov(PredicateRegister qp, BranchRegister dst, Register src, Label& tag,
                  Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void mov(                      BranchRegister dst, Register src, Label& tag,
                  Branch_Hint mwh = no_hint, Importance_Hint ih = useless);
  inline void mov(PredicateRegister qp, Register dst, BranchRegister src);
  inline void mov(                      Register dst, BranchRegister src);

  // Clear
  inline void clr(PredicateRegister qp, Register dst);
  inline void clr(                      Register dst);

  // Shift
  inline void shru(PredicateRegister qp, Register dst, Register src, uint count);
  inline void shru(                      Register dst, Register src, uint count);
  inline void shru(PredicateRegister qp, Register dst, Register src, Register count);
  inline void shru(                      Register dst, Register src, Register count);
  inline void shr( PredicateRegister qp, Register dst, Register src, uint count);
  inline void shr(                       Register dst, Register src, uint count);
  inline void shr( PredicateRegister qp, Register dst, Register src, Register count);
  inline void shr(                       Register dst, Register src, Register count);
  inline void shl( PredicateRegister qp, Register dst, Register src, uint count);
  inline void shl(                       Register dst, Register src, uint count);
  inline void shl( PredicateRegister qp, Register dst, Register src, Register count);
  inline void shl(                       Register dst, Register src, Register count);

  // 32-bit Shift
  inline void shr4u(PredicateRegister qp, Register dst, Register src, Register count);
  inline void shr4u(                      Register dst, Register src, Register count);
  inline void shr4u(PredicateRegister qp, Register dst, Register src, uint count);
  inline void shr4u(                      Register dst, Register src, uint count);
  inline void shr4( PredicateRegister qp, Register dst, Register src, Register count);
  inline void shr4(                       Register dst, Register src, Register count);
  inline void shr4( PredicateRegister qp, Register dst, Register src, uint count);
  inline void shr4(                       Register dst, Register src, uint count);

  // Signed Loads
  inline void ld1s(PredicateRegister qp, Register dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld1s(                      Register dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld2s(PredicateRegister qp, Register dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld2s(                      Register dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld4s(PredicateRegister qp, Register dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld4s(                      Register dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld8s(PredicateRegister qp, Register dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);
  inline void ld8s(                      Register dst, Register addr,
                   LoadStore_Type ldtype = normal, LoadStore_Hint ldhint = none);

  // Floating-point
  inline void fabs( PredicateRegister qp, FloatRegister dst, FloatRegister src);
  inline void fabs(                       FloatRegister dst, FloatRegister src);

  inline void fadd( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fadd(                       FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fadds(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fadds(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void faddd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void faddd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);

  inline void fsub( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fsub(                       FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fsubs(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fsubs(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fsubd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fsubd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);

  inline void fcvtxufs(PredicateRegister qp, FloatRegister dst, FloatRegister src,
                       FloatStatusField sf = sf0);
  inline void fcvtxufs(                      FloatRegister dst, FloatRegister src,
                       FloatStatusField sf = sf0);
  inline void fcvtxufd(PredicateRegister qp, FloatRegister dst, FloatRegister src,
                       FloatStatusField sf = sf0);
  inline void fcvtxufd(                      FloatRegister dst, FloatRegister src,
                       FloatStatusField sf = sf0);

  void fdivs(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  void fdivs(                      FloatRegister dst, FloatRegister src1, FloatRegister src2);
  void fdivd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
             FloatRegister tmp3, FloatRegister tmp4, PredicateRegister failed);
  void fdivd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
             FloatRegister tmp3, FloatRegister tmp4, PredicateRegister failed);

  inline void fmpy( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fmpy(                       FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fmpys(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fmpys(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fmpyd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);
  inline void fmpyd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                    FloatStatusField sf = sf0);

  inline void fnmpy( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);
  inline void fnmpy(                       FloatRegister dst, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);
  inline void fnmpys(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);
  inline void fnmpys(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);
  inline void fnmpyd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);
  inline void fnmpyd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                     FloatStatusField sf = sf0);

  inline void fneg(   PredicateRegister qp, FloatRegister dst, FloatRegister src);
  inline void fneg(                         FloatRegister dst, FloatRegister src);
  inline void fnegabs(PredicateRegister qp, FloatRegister dst, FloatRegister src);
  inline void fnegabs(                      FloatRegister dst, FloatRegister src);

  inline void fnorm( PredicateRegister qp, FloatRegister dst, FloatRegister src, FloatStatusField sf = sf0);
  inline void fnorm(                       FloatRegister dst, FloatRegister src, FloatStatusField sf = sf0);
  inline void fnorms(PredicateRegister qp, FloatRegister dst, FloatRegister src, FloatStatusField sf = sf0);
  inline void fnorms(                      FloatRegister dst, FloatRegister src, FloatStatusField sf = sf0);
  inline void fnormd(PredicateRegister qp, FloatRegister dst, FloatRegister src, FloatStatusField sf = sf0);
  inline void fnormd(                      FloatRegister dst, FloatRegister src, FloatStatusField sf = sf0);

  // Integer multiply
  void impy(PredicateRegister qp, Register dst, Register src1, Register src2, FloatRegister tmp1, FloatRegister tmp2);
  void impy(                      Register dst, Register src1, Register src2, FloatRegister tmp1, FloatRegister tmp2);

  inline void mov(PredicateRegister qp, FloatRegister dst, FloatRegister src);
  inline void mov(                      FloatRegister dst, FloatRegister src);

  inline void xmalu( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);
  inline void xmalu(                       FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3);
  inline void xmpy(  PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void xmpy(                        FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void xmpyl( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void xmpyl(                       FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void xmpylu(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void xmpylu(                      FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void xmpyh( PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void xmpyh(                       FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void xmpyhu(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2);
  inline void xmpyhu(                      FloatRegister dst, FloatRegister src1, FloatRegister src2);
 
  // Debugging
  void verify_oop(Register reg, const char* s = "broken oop");             // only if +VerifyOops
  void verify_FPU(int stack_depth, const char* s = "illegal FPU state");   // only if +VerifyFPU
  void stop(const char* msg);                    // prints msg, dumps registers and stops execution
  void os_breakpoint();
  void untested()                                { stop("untested"); }
  void unimplemented(const char* what = "")      { char* b = new char[1024]; sprintf(b, "unimplemented: %s", what); stop(b); }
  void should_not_reach_here()                   { stop("should not reach here"); }

  // Kills GR2
  void bang_stack_with_offset(int offset) {
    assert(offset >= 0, "must bang at positive offset");
    if (in_signed_range(offset, 14)) {
      sub(GR2, SP, offset);
    } else {
      mov(GR2, offset);
      sub(GR2, SP, GR2);
    }
    st8(GR2, GR0);         // bang stack
  }
};
