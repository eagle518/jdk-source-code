#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)assembler_i486.hpp	1.129 04/02/25 22:15:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Contains all the definitions needed for x86 assembly code generation.

// Calling convention
class Argument VALUE_OBJ_CLASS_SPEC {
public:
  enum {
    n_register_parameters = 0 	// 0 registers used to pass arguments
  };
};


// Address is an abstraction used to represent a memory location
// using any of the x86 addressing modes with one object.
//
// Note: A register location is represented via a Register, not
//       via an address for efficiency & simplicity reasons.

class Address VALUE_OBJ_CLASS_SPEC {
 public:
  enum ScaleFactor {
    no_scale = -1,
    times_1  =  0,
    times_2  =  1,
    times_4  =  2,
    times_8  =  3
  };

 private:
  Register         _base;
  Register         _index;
  ScaleFactor      _scale;
  int              _disp;
  RelocationHolder _rspec;

 public:
  // creation
  Address()
  : _base (noreg)
  , _index(noreg)
  , _scale(no_scale)
  , _disp (0)
  {}

  Address(int disp, relocInfo::relocType rtype);

  Address(int disp, RelocationHolder const& rspec)
  : _base (noreg)
  , _index(noreg)
  , _scale(no_scale)
  , _disp (disp)
  , _rspec(rspec)
  {}

  Address(Register base, int disp = 0)
  : _base (base)
  , _index(noreg)
  , _scale(no_scale)
  , _disp (disp)
  {}

  Address(Register base, Register index, ScaleFactor scale, int disp = 0)
  : _base (base)
  , _index(index)
  , _scale(scale)
  , _disp (disp)
  {
    assert(!index->is_valid() == (scale == Address::no_scale), "inconsistent address");
  }

  // The following two overloads are used in connection with the ByteSize type (see sizes.hpp).
  // They simplify the use of ByteSize'd arguments in assembly code. Note that their equivalent
  // for the optimized build are the member functions with int disp argument since ByteSize is
  // mapped to an int type in that case.
  //
  // Note: DO NOT introduce similar overloaded functions for WordSize arguments as in the
  //       optimized mode, both ByteSize and WordSize are mapped to the same type and thus
  //       the compiler cannot make a distinction anymore (=> compiler errors).

#ifdef ASSERT
  Address(Register base, ByteSize disp)
  : _base (base)
  , _index(noreg)
  , _scale(no_scale)
  , _disp (in_bytes(disp))
  {}

  Address(Register base, Register index, ScaleFactor scale, ByteSize disp)
  : _base (base)
  , _index(index)
  , _scale(scale)
  , _disp (in_bytes(disp))
  {
    assert(!index->is_valid() == (scale == Address::no_scale), "inconsistent address");
  }
#endif // ASSERT

  // accessors
  bool uses(Register reg) const                  { return _base == reg || _index == reg; }

  friend class Assembler;
};


// Intel-version of Displacement:
//
// A Displacement describes the 32bit immediate field of an instruction which
// may be used together with a Label in order to refer to a yet unknown code
// position. Displacements stored in the instruction stream are used to describe
// the instruction and to chain a list of instructions using the same Label.
//
// A Displacement contains 3 different fields encoded into one _data field:
//
// |31....10|9......8|7......0|
// [  next  |  type  |  info  ]
//
// next field: position of next displacement in the chain (0 = end of list)
// type field: instruction type
// info field: instruction specific information
//
// A next value of null (0) indicates the end of a chain (note that there can
// be no displacement at position zero, because there is always at least one
// instruction byte before the displacement).

class Displacement : public AbstractDisplacement {
  friend class AbstractAssembler;
  friend class Assembler;
  friend class MacroAssembler;
  friend class AbstractDisplacement;

 private:
  int _data;

  enum Layout {
    info_size	=  8,
    type_size	=  2,
    next_size	= 32 - (type_size + info_size),
    
    info_pos	= 0,
    type_pos	= info_pos + info_size,
    next_pos	= type_pos + type_size,
    
    info_mask	= (1 << info_size) - 1,
    type_mask	= (1 << type_size) - 1,
    next_mask	= (1 << next_size) - 1
  };

  enum Type {			// info field usage
    call,			// unused
    absolute_jump,		// unused
    conditional_jump		// condition code
  };

  void init(Label& L, Type type, int info) {
    assert(!L.is_bound(), "label is bound");
    int next = 0;
    if (L.is_unbound()) {
      next = L.pos();
      assert(next > 0, "Displacements must be at positions > 0");
    }
    assert((next & ~next_mask) == 0, "next field too small");
    assert((type & ~type_mask) == 0, "type field too small");
    assert((info & ~info_mask) == 0, "info field too small");
    _data = (next << next_pos) | (type << type_pos) | (info << info_pos);
  }

  int  data() const		{ return _data; }
  int  info() const		{ return     ((_data >> info_pos) & info_mask); }
  Type type() const		{ return Type((_data >> type_pos) & type_mask); }
  void next(Label & L) const	{ int n    = ((_data >> next_pos) & next_mask); n > 0 ? L.link_to(n) : L.unuse(); }
  void link_to(Label& L, int )	{ init(L, type(), info()); }
    
  Displacement(int data)	{ _data = data; }

  Displacement(Label& L, Type type, int info) {
    init(L, type, info);
  }

  void bind(Label &L, int pos, AbstractAssembler* masm);

#ifndef PRODUCT
  void print() {
    const char* s;
    switch (type()) {
      case call            : s = "call"; break;
      case absolute_jump   : s = "jmp "; break;
      case conditional_jump: s = "jcc "; break;
      default              : s = "????"; break;
    }
    tty->print("%s (info = 0x%x)", s, info());
  }
#endif // PRODUCT
};


const int FPUStateSizeInWords = 27;

// The Intel x86 Assembler: Pure assembler doing NO optimizations on the instruction
// level (e.g. mov eax, 0 is not translated into xor eax, eax!); i.e., what you write
// is what you get. The Assembler is generating code into a CodeBuffer.

class Assembler : public AbstractAssembler  {
  friend class AbstractAssembler; // for the non-virtual hack

 protected:
  #ifdef ASSERT
  void check_relocation(RelocationHolder const& rspec, int format);
  #endif

  void emit_data(jint data, relocInfo::relocType    rtype, int format = 0);
  void emit_data(jint data, RelocationHolder const& rspec, int format = 0);

  // Helper functions for groups of instructions
  void emit_arith_b(int op1, int op2, Register dst, int imm8);

  void emit_arith(int op1, int op2, Register dst, int imm32);
  void emit_arith(int op1, int op2, Register dst, jobject obj);
  void emit_arith(int op1, int op2, Register dst, Register src);

  void emit_operand(Register reg, Register base, Register index, Address::ScaleFactor scale, int disp, RelocationHolder const& rspec);
  void emit_operand(Register reg, Address adr);

  // Immediate-to-memory forms
  void emit_arith_operand(int op1, Register rm, Address adr, int imm32);

  void emit_farith(int b1, int b2, int i);

  // Displacement routines

  Displacement disp_at(Label& L) const                     { return Displacement(long_at(L.pos())); }
  void         disp_at_put(Label& L, Displacement& disp)   { long_at_put(L.pos(), disp.data()); }

  inline void  emit_disp(Label& L,  int type, int info) {
    Displacement disp(L, Displacement::Type(type), info);
    L.link_to(offset());
    emit_long(int(disp.data()));
  }
 
 public:
  enum Condition {                     // The x86 condition codes used for conditional jumps/moves.
    zero          = 0x4,
    notZero       = 0x5,
    equal         = 0x4,
    notEqual      = 0x5,
    less          = 0xc,
    lessEqual     = 0xe,
    greater       = 0xf,
    greaterEqual  = 0xd,
    below         = 0x2,
    belowEqual	  = 0x6,
    above         = 0x7,
    aboveEqual	  = 0x3,
    overflow      = 0x0,
    noOverflow	  = 0x1,
    carrySet      = 0x2,
    carryClear	  = 0x3,
    negative      = 0x8,
    positive      = 0x9,
    parity        = 0xa,
    noParity      = 0xb
  };

  enum Prefix {
    // segment overrides
    CS_segment = 0x2e,
    SS_segment = 0x36,
    DS_segment = 0x3e,
    ES_segment = 0x26,
    FS_segment = 0x64,
    GS_segment = 0x65
  };

  enum WhichOperand {
    // input to locate_operand, and format code for relocations
    imm32_operand  = 0,          // embedded 32-bit immediate operand
    disp32_operand = 1,          // embedded 32-bit displacement or address
    call32_operand = 2,          // embedded 32-bit self-relative displacement
    _WhichOperand_limit = 3
  };

  // Creation
  Assembler(CodeBuffer* code) : AbstractAssembler(code) {}

  // Decoding
  static address locate_operand(address inst, WhichOperand which);
  static address locate_next_instruction(address inst);

  // Stack
  void pushad();
  void popad();

  void pushfd();
  void popfd();

  void pushl(int imm32);
  void pushl(int imm32, relocInfo::relocType rtype);
  void pushl(int imm32, RelocationHolder const& rspec);
  void pushl(jobject obj);
  void pushl(Register src);
  void pushl(Address src);
  void pushl(Label& L, relocInfo::relocType rtype);

  void popl(Register dst);
  void popl(Address dst);

  // Instruction prefixes
  void prefix(Prefix p);

  // Moves
  void movb(Register dst, Address src);
  void movb(Address dst, int imm8);
  void movb(Address dst, Register src);

  void movw(Address dst, int imm16);
  void movw(Register dst, Address src);
  void movw(Address dst, Register src);

  void movl(Register dst, int imm32);
  void movl(Register dst, jobject obj);
  void movl(Register dst, Register src);
  void movl(Register dst, Address src);

  void movl(Address dst, int imm32);
  void movl(Address dst, jobject obj);
  void movl(Address dst, Register src);

  void movsxb(Register dst, Address src);
  void movsxb(Register dst, Register src);

  void movsxw(Register dst, Address src);
  void movsxw(Register dst, Register src);

  void movzxb(Register dst, Address src);
  void movzxb(Register dst, Register src);

  void movzxw(Register dst, Address src);
  void movzxw(Register dst, Register src);

  // Conditional moves (P6 only)
  void cmovl(Condition cc, Register dst, Register src);
  void cmovl(Condition cc, Register dst, Address src);

  // Arithmetics
  void adcl(Register dst, int imm32);
  void adcl(Register dst, Address src);
  void adcl(Register dst, Register src);

  void addl(Address dst, int imm32);
  void addl(Address dst, Register src);
  void addl(Register dst, int imm32);
  void addl(Register dst, Address src);
  void addl(Register dst, Register src);

  void andl(Register dst, int imm32);
  void andl(Register dst, Address src);
  void andl(Register dst, Register src);

  void cmpb(Address dst, int imm8);
  void cmpl(Address dst, int imm32);
  void cmpl(Address dst, jobject obj);
  void cmpl(Register dst, int imm32);
  void cmpl(Register dst, jobject obj);
  void cmpl(Register dst, Register src);
  void cmpl(Register dst, Address src);

  void decb(Register dst);
  void decl(Register dst);
  void decl(Address dst);

  void idivl(Register src);
  void cdql();

  void imull(Register dst, Register src);
  void imull(Register dst, Register src, int value);

  void incl(Register dst);
  void incl(Address dst);

  void leal(Register dst, Address src);

  void mull(Address src);
  void mull(Register src);

  void negl(Register dst);

  void notl(Register dst);

  void orl(Address dst, int imm32);
  void orl(Register dst, int imm32);
  void orl(Register dst, Address src);
  void orl(Register dst, Register src);
  
  void rcll(Register dst, int imm8);

  void sarl(Register dst, int imm8);
  void sarl(Register dst);

  void sbbl(Address dst, int imm32);
  void sbbl(Register dst, int imm32);
  void sbbl(Register dst, Address src);
  void sbbl(Register dst, Register src);

  void shldl(Register dst, Register src);

  void shll(Register dst, int imm8);
  void shll(Register dst);

  void shrdl(Register dst, Register src);

  void shrl(Register dst, int imm8);
  void shrl(Register dst);

  void subl(Address dst, int imm32);
  void subl(Address dst, Register src);
  void subl(Register dst, int imm32);
  void subl(Register dst, Address src);
  void subl(Register dst, Register src);

  void testb(Register dst, int imm8);
  void testl(Register dst, int imm32);
  void testl(Register dst, Address src);
  void testl(Register dst, Register src);

  void xaddl(Address dst, Register src);

  void xorl(Register dst, int imm32);
  void xorl(Register dst, Address src);
  void xorl(Register dst, Register src);

  // Miscellaneous
  void bswap(Register reg);
  void lock();
  void xchg (Register reg, Address adr);
  void xchgl(Register dst, Register src);
  void cmpxchg (Register reg, Address adr);
  void cmpxchg8 (Address adr);
  void hlt();
  void int3();
  void nop();
  void ret(int imm16);
  void set_byte_if_not_zero(Register dst); // sets reg to 1 if not zero, otherwise 0
  void rep_move();
  void rep_set();
  void repne_scan();
  void setb(Condition cc, Register dst);
  void membar();		// Serializing memory-fence
  void cpuid();

  // Calls
  void call(Label& L, relocInfo::relocType rtype);
  void call(address entry, relocInfo::relocType rtype);
  void call(address entry, RelocationHolder const& rspec);
  void call(Register reg,  relocInfo::relocType rtype);
  void call(Address adr);
  
  // Jumps
  void jmp(address entry, relocInfo::relocType rtype);
  void jmp(Register reg,  relocInfo::relocType rtype = relocInfo::none);
  void jmp(Address adr);

  // Label operations & relative jumps (PPUM Appendix D)
  void jmp(Label& L, relocInfo::relocType rtype = relocInfo::none);   // unconditional jump to L

  // jcc is the generic conditional branch generator to run-
  // time routines, jcc is used for branches to labels. jcc
  // takes a branch opcode (cc) and a label (L) and generates
  // either a backward branch or a forward branch and links it
  // to the label fixup chain. Usage:
  //
  // Label L;      // unbound label
  // jcc(cc, L);   // forward branch to unbound label
  // bind(L);      // bind label to the current pc
  // jcc(cc, L);   // backward branch to bound label
  // bind(L);      // illegal: a label may be bound only once
  //
  // Note: The same Label can be used for forward and backward branches
  // but it may be bound only once.

  void jcc(Condition cc, address dst, relocInfo::relocType rtype = relocInfo::runtime_call_type);
  void jcc(Condition cc, Label& L, relocInfo::relocType rtype = relocInfo::none);

  // Floating-point operations
  void fld1();
  void fldz();

  void fld_s(Address adr);
  void fld_s(int index);
  void fld_d(Address adr);
  void fld_x(Address adr);  // extended-precision (80-bit) format

  void fst_s(Address adr);
  void fst_d(Address adr);

  void fstp_s(Address adr);
  void fstp_d(Address adr);
  void fstp_d(int index);
  void fstp_x(Address adr); // extended-precision (80-bit) format

  void fild_s(Address adr);
  void fild_d(Address adr);

  void fist_s (Address adr);
  void fistp_s(Address adr);
  void fistp_d(Address adr);

  void fabs();
  void fchs();

  void fcos();
  void fsin();
  void fsqrt();

  void fadd_s(Address src);
  void fadd_d(Address src);
  void fadd(int i);
  
  void fsub_s(Address src);
  void fsub_d(Address src);
  void fsubr_s(Address src);
  void fsubr_d(Address src);

  void fmul_s(Address src);
  void fmul_d(Address src);
  void fmul(int i);
  
  void fdiv_s(Address src);
  void fdiv_d(Address src);
  void fdivr_s(Address src);
  void fdivr_d(Address src);
  
  void fsub(int i);
  void fdiv(int i);

  void faddp(int i = 1);
  void fsubp(int i = 1);
  void fsubrp(int i = 1);
  void fmulp(int i = 1);
  void fdivp(int i = 1);
  void fdivrp(int i = 1);
  void fprem();
  void fprem1();

  void fxch(int i = 1);
  void fincstp();
  void fdecstp();
  void ffree(int i = 0);

  void fcomp_s(Address src);
  void fcomp_d(Address src);
  void fcompp();

  void fucomi(int i = 1);
  void fucomip(int i = 1);
  
  void ftst();
  void fnstsw_ax();
  void fwait();
  void finit();
  void fldcw(Address src);
  void fnstcw(Address src);

  void fnsave(Address dst);
  void frstor(Address src);
  void fldenv(Address src);

  void sahf();

  // xmm operations
  void emit_operand(XMMRegister reg, Address adr);
  void movss( XMMRegister dst, Address src );
  void movsd( XMMRegister dst, Address src );
  void movss( Address dst, XMMRegister src );
  void movsd( Address dst, XMMRegister src );
  void ldmxcsr( Address src );
  void stmxcsr( Address dst );
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
  
  virtual void call_VM_leaf_base(
    address entry_point,               // the entry point
    int     number_of_arguments        // the number of arguments to pop after the call
  );

  // This is the base routine called by the different versions of call_VM. The interpreter
  // may customize this version by overriding it for its purposes (e.g., to save/restore
  // additional registers when doing a VM call).
  //
  // If no java_thread register is specified (noreg) than edi will be used instead. call_VM_base
  // returns the register which contains the thread upon return. If a thread register has been
  // specified, the return value will correspond to that register. If no last_java_sp is specified
  // (noreg) than esp will be used instead.
  virtual void call_VM_base(           // returns the register containing the thread upon return
    Register oop_result,               // where an oop-result ends up if any; use noreg otherwise
    Register java_thread,              // the thread if computed before     ; use noreg otherwise
    Register last_java_sp,             // to set up last_Java_frame in stubs; use noreg otherwise
    address  entry_point,              // the entry point
    int      number_of_arguments,      // the number of arguments (w/o thread) to pop after the call
    bool     check_exceptions          // whether to check for pending exceptions after return
  ); 

  // This routine should emit JVMDI PopFrame handling code. The
  // implementation is only non-empty for the InterpreterMacroAssembler,
  // as only the interpreter handles PopFrame requests.
  virtual void check_and_handle_popframe(Register java_thread);

  void call_VM_helper(Register oop_result, address entry_point, int number_of_arguments, bool check_exceptions = true);
  
  // helpers for FPU flag access
  // tmp is a temporary register, if none is available use noreg
  void save_eax   (Register tmp);
  void restore_eax(Register tmp);

 public:
  MacroAssembler(CodeBuffer* code) : Assembler(code) {}

  // Support for NULL-checks
  //
  // Generates code that causes a NULL OS exception if the content of reg is NULL.
  // If the accessed location is M[reg + offset] and the offset is known, provide the
  // offset. No explicit code generation is needed if the offset is within a certain
  // range (0 <= offset <= page_size).

  void null_check(Register reg, int offset = -1);
  static bool needs_explicit_null_check(int offset);

  // The following 4 methods return the offset of the appropriate move instruction

  // Support for fast byte/word loading with zero extension (depending on particular CPU)
  int load_unsigned_byte(Register dst, Address src);
  int load_unsigned_word(Register dst, Address src);

  // Support for fast byte/word loading with sign extension (depending on particular CPU)
  int load_signed_byte(Register dst, Address src);
  int load_signed_word(Register dst, Address src);

  // Support for sign-extension (hi:lo = extend_sign(lo))
  void extend_sign(Register hi, Register lo);

  // Support for inc/dec with optimal instruction selection depending on value
  void increment(Register reg, int value);
  void decrement(Register reg, int value);

  // Alignment
  void align(int modulus);

  // Misc
  void fat_nop(); // 5 byte nop 

  // Stack frame creation/removal
  void enter();
  void leave();

  // Support for getting the JavaThread pointer (i.e.; a reference to thread-local information)
  // The pointer will be loaded into the thread register.
  void get_thread(Register thread);

  // Support for VM calls
  //
  // It is imperative that all calls into the VM are handled via the call_VM macros.
  // They make sure that the stack linkage is setup correctly. call_VM's correspond
  // to ENTRY/ENTRY_X entry points while call_VM_leaf's correspond to LEAF entry points.

  void call_VM(Register oop_result, address entry_point, bool check_exceptions = true);
  void call_VM(Register oop_result, address entry_point, Register arg_1, bool check_exceptions = true);
  void call_VM(Register oop_result, address entry_point, Register arg_1, Register arg_2, bool check_exceptions = true);
  void call_VM(Register oop_result, address entry_point, Register arg_1, Register arg_2, Register arg_3, bool check_exceptions = true);

  void call_VM(Register oop_result, Register last_java_sp, address entry_point, int number_of_arguments = 0, bool check_exceptions = true);
  void call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, bool check_exceptions = true);
  void call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2, bool check_exceptions = true);
  void call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2, Register arg_3, bool check_exceptions = true);

  void call_VM_leaf(address entry_point, int number_of_arguments = 0);
  void call_VM_leaf(address entry_point, Register arg_1);
  void call_VM_leaf(address entry_point, Register arg_1, Register arg_2);
  void call_VM_leaf(address entry_point, Register arg_1, Register arg_2, Register arg_3);

  // last Java Frame (fills frame anchor)
  void set_last_Java_frame(Register thread, Register last_java_sp, Register last_java_fp, address last_java_pc);
  void reset_last_Java_frame(Register thread, bool clear_pc = false);

  // Stores
  void store_check(Register obj);                // store check for obj - register is destroyed afterwards
  void store_check(Register obj, Address dst);   // same as above, dst is exact store location (reg. is destroyed)

  // split store_check(Register obj) to enhance instruction interleaving
  void store_check_part_1(Register obj);
  void store_check_part_2(Register obj);

  // C 'boolean' to Java boolean: x == 0 ? 0 : 1
  void c2bool(Register x);

  // Int division/reminder for Java
  // (as idivl, but checks for special case as described in JVM spec.)
  // returns idivl instruction offset for implicit exception handling
  int corrected_idivl(Register reg);

  // Long negation for Java
  void lneg(Register hi, Register lo);

  // Long multiplication for Java
  // (destroys contents of eax, ebx, ecx and edx)
  void lmul(int x_esp_offset, int y_esp_offset); // edx:eax = x * y

  // Long shifts for Java
  // (semantics as described in JVM spec.)
  void lshl(Register hi, Register lo);                               // hi:lo << (ecx & 0x3f)
  void lshr(Register hi, Register lo, bool sign_extension = false);  // hi:lo >> (ecx & 0x3f)

  // Long compare for Java
  // (semantics as described in JVM spec.)
  void lcmp2int(Register x_hi, Register x_lo, Register y_hi, Register y_lo); // x_hi = lcmp(x, y)

  // Compares the top-most stack entries on the FPU stack and sets the eflags as follows:
  //
  // CF (corresponds to C0) if x < y
  // PF (corresponds to C2) if unordered
  // ZF (corresponds to C3) if x = y
  //
  // The arguments are in reversed order on the stack (i.e., top of stack is first argument).
  // tmp is a temporary register, if none is available use noreg (only matters for non-P6 code)
  void fcmp(Register tmp);
  
  // Floating-point comparison for Java
  // Compares the top-most stack entries on the FPU stack and stores the result in dst.
  // The arguments are in reversed order on the stack (i.e., top of stack is first argument).
  // (semantics as described in JVM spec.)
  void fcmp2int(Register dst, bool unordered_is_less);

  // Floating-point remainder for Java (ST0 = ST0 fremr ST1, ST1 is empty afterwards)
  // tmp is a temporary register, if none is available use noreg
  void fremr(Register tmp);

  // Inlined sin/cos generator for Java; must not use CPU instruction
  // directly on Intel as it does not have high enough precision
  // outside of the range [-pi/4, pi/4]. Arguments indicate whether
  // inlined code should save/restore modified CPU registers and the
  // number of FPU stack slots in use; all but the topmost will
  // require saving if a slow case is necessary. Assumes argument is
  // on FP TOS; result is on FP TOS.
  void sincos(bool is_sin, bool preserve_cpu_regs, int num_fpu_regs_in_use = 1);

  // branch to L if FPU flag C2 is set/not set
  // tmp is a temporary register, if none is available use noreg
  void jC2 (Register tmp, Label& L);
  void jnC2(Register tmp, Label& L);

  // Pop ST (ffree & fincstp combined)
  void fpop();

  // pushes double TOS element of FPU stack on CPU stack; pops from FPU stack
  void push_fTOS();

  // pops double TOS element from CPU stack and pushes on FPU stack 
  void pop_fTOS();

  void empty_FPU_stack();

  void push_IU_state();
  void pop_IU_state();

  void push_FPU_state();
  void pop_FPU_state();

  void push_CPU_state();
  void pop_CPU_state();

  // Sign extension
  void sign_extend_short(Register reg);
  void sign_extend_byte(Register reg);

  // Division by power of 2, rounding towards 0
  void division_with_shift(Register reg, int shift_value);

  // Round up to a power of two
  void round_to(Register reg, int modulus);

  // Callee saved registers handling
  void push_callee_saved_registers();
  void pop_callee_saved_registers();

  // allocation
  void eden_allocate(
    Register obj,                      // result: pointer to object after successful allocation
    Register var_size_in_bytes,        // object size in bytes if unknown at compile time; invalid otherwise
    int      con_size_in_bytes,        // object size in bytes if   known at compile time
    Register t1,                       // temp register
    Label&   slow_case                 // continuation point if fast allocation fails
  );
  void tlab_allocate(
    Register obj,                      // result: pointer to object after successful allocation
    Register var_size_in_bytes,        // object size in bytes if unknown at compile time; invalid otherwise
    int      con_size_in_bytes,        // object size in bytes if   known at compile time
    Register t1,                       // temp register
    Register t2,                       // temp register
    Label&   slow_case                 // continuation point if fast allocation fails
  );
  void tlab_refill(Label& retry_tlab, Label& try_eden, Label& slow_case);

  //----
  void set_word_if_not_zero(Register reg); // sets reg to 1 if not zero, otherwise 0

  // Debugging
  void verify_oop(Register reg, const char* s = "broken oop");             // only if +VerifyOops
  void verify_FPU(int stack_depth, const char* s = "illegal FPU state");   // only if +VerifyFPU
  void stop(const char* msg);                    // prints msg, dumps registers and stops execution
  static void debug(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax, int eip, char* msg);
  void os_breakpoint();
  void untested()                                { stop("untested"); }
  void unimplemented(const char* what = "")      { char* b = new char[1024];  sprintf(b, "unimplemented: %s", what);  stop(b); }
  void should_not_reach_here()                   { stop("should not reach here"); }
  void print_CPU_state();

  // Stack overflow checking
  void bang_stack_with_offset(int offset) {
    // stack grows down, caller passes positive offset
    assert(offset > 0, "must bang with negative offset");
    movl(Address(esp, (-offset)), eax);
  }

  void verify_tlab();
};
