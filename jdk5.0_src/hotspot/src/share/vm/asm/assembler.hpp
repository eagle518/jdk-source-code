#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)assembler.hpp	1.35 03/12/23 16:38:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file contains platform-independant assembler declarations.

class CodeBuffer;
class MacroAssembler;

// Labels represent target destinations for jumps and calls.
//
// After declaration they can be freely used to denote known or (yet) unknown
// target destinations. Assembler::bind is used to bind a label to the current
// code position. A label can be bound only once.

class Label VALUE_OBJ_CLASS_SPEC {
 private:
  // _pos encodes both the binding state (via its sign)
  // and the binding position (via its value) of a label.
  //
  // _pos <  0	bound label, pos() returns the target (jump) position
  // _pos == 0	unused label
  // _pos >  0	unbound label, pos() returns the last displacement (see .cpp file) in the chain
  int _pos;

  void report_bad_label() const;

  int pos() const {
    if (_pos < 0) return -_pos - 1;
    if (_pos > 0) return  _pos - 1;
    report_bad_label();
    return 0;
  }

  void bind_to(int pos)		{ assert(pos >= 0, "illegal position"); _pos = -pos - 1; }
  void link_to(int pos)		{ assert(pos >= 0, "illegal position"); _pos =  pos + 1; }
  void unuse()			{ _pos = 0; }

 public:
  void bind(int pos)            { assert( pos >= 0, "illegal position");
                                  assert(_pos == 0, "already bound");
                                  _pos = -pos - 1; }

  int offset() const {
    assert(_pos < 0, "unbound label");
    return -_pos - 1;
  }

  bool is_bound() const		{ return _pos <  0; }
  bool is_unbound() const	{ return _pos >  0; }
  bool is_unused() const	{ return _pos == 0; }

  bool is_backward_branch(uint curr)
                                { return _pos < 0 && curr <= (uint)(-_pos - 1); }
 
  Label() : _pos(0)		{}
  ~Label()			{ assert(!is_unbound(), "unbound label"); }

  friend class AbstractAssembler;
  friend class Assembler;
  friend class MacroAssembler;
  friend class Displacement;
  friend class CodeBuffer;
};


// Clients of the assembler need to emit instructions (such as calls and branches)
// that refer to addresses in the same code stream BEFORE the addresses are known.
// When asked to emit such an instruction, the assembler emits a Displacement
// instead. The Displacement contains enough information to allow the assembler
// to backpatch, and replace the displacement with the right instruction afterwards.
//
// Here we define only an abstract, platform-independent framework.

class AbstractDisplacement : public ResourceObj {

  // The following must be implemented by the platform-spefific Displacement
  // class. Cannot use virtual foo() = 0; because I don't want virtuals,
  // for efficiency's sake. -- dmu 4/97

  // AbstractDisplacments are created in an implementation-specific
  // manner, when emitting an instruction with a forward reference in it.

  // Note, all members below are really abstract, just use inlining
  // to avoid virtuals.
   
  // bind is called repeatedly at each link in the chain.
  // It backpatches the instruction as appropriate, then calls
  // next to step the label to point to the next displacement in the chain.
  inline void bind(Label &L, int pos, AbstractAssembler* masm);
  
  // An iterator, next binds the label to the next displacement
  // or unused, if this is the end of the displacement chain.
  inline void next(Label& L) const;
  
  inline void print();
};



// The Abstract Assembler: Pure assembler doing NO optimizations on the instruction
// level; i.e., what you write
// is what you get. The Assembler is generating code into a CodeBuffer.

class AbstractAssembler : public ResourceObj  {
  friend class Displacement;

 protected:
  CodeBuffer*  _code;
  address      _code_begin;            // first byte of code buffer
  address      _code_limit;            // first byte after code buffer
  address      _code_pos;              // current code generation position
  address      _inst_mark;             // marked start of current instruction
  OopRecorder* _oop_recorder;          // support for relocInfo::oop_type

  // Code emission & accessing
  address addr_at(int pos) const       { return _code_begin + pos; }

  int  byte_at(int pos) const          { return *addr_at(pos); }
  void byte_at_put(int pos, int x)     { *addr_at(pos) = (unsigned char)x; }

  jint long_at(int pos)	const          { return *(jint*)addr_at(pos); }
  void long_at_put(int pos, jint x)    { *(jint*)addr_at(pos) = x; }

  bool is8bit(int x) const             { return -0x80 <= x && x < 0x80; }
  bool isByte(int x) const             { return 0 <= x && x < 0x100; }
  bool isShiftCount(int x) const       { return 0 <= x && x < 32; }

  inline void emit_byte(int x);
  inline void emit_word(int x);
  inline void emit_long(jint x);

  address  inst_mark() const           { return _inst_mark; }
  void set_inst_mark(address x)        { _inst_mark = x;    }

  // Instruction boundaries (required when emitting relocatable values).
  class InstructionMark {
   private:
    AbstractAssembler* _assm;

   public:
    InstructionMark(AbstractAssembler* assm) : _assm(assm) {
      assert(assm->inst_mark() == NULL, "overlapping instructions");
      _assm->set_inst_mark(assm->pc());
    }
    ~InstructionMark() {
      _assm->set_inst_mark(NULL);
    }
  };
  friend class InstructionMark;

  // Label functions

  void print  (Label& L);
  void bind_to(Label& L, int pos);

  // Displacement functions

  // get Displacement object in code stream at position in L
  inline Displacement disp_at(     Label& L) const;

  // setup reference from code stream at L's pos to disp
  inline void         disp_at_put( Label& L,  Displacement& disp);

 public:
  // Creation
  AbstractAssembler(CodeBuffer* code);

  // Accessors
  void		flush();               // make sure code buffer contains all code (call this before using/copying the code)
  CodeBuffer*	code() const           { return _code; }
  address       pc() const             { return _code_pos; }
  int		offset() const         { return _code_pos - _code_begin; }
  OopRecorder*  oop_recorder() const   { return _oop_recorder; }
  void      set_oop_recorder(OopRecorder* r) { _oop_recorder = r; }

  // Constants in code
  void a_byte(int x);
  void a_long(jint x);
  inline void relocate(RelocationHolder const& rspec, int format = 0);
  inline void relocate(   relocInfo::relocType rtype, int format = 0) {
    if (rtype != relocInfo::none)
      relocate(Relocation::spec_simple(rtype), format);
  }
  inline void relocate_raw(relocInfo::relocType rtype, const short* data = NULL, int datalen = 0, int format = 0);

  static int code_fill_byte();         // used to pad out odd-sized code buffers

  // Labels
  void bind(Label& L);                 // binds an unbound label L to the current code position

  // Bang stack to trigger StackOverflowError at a safe location
  // implementation delegates to machine-specific bang_stack_with_offset
  void generate_stack_overflow_check( int frame_size_in_bytes );
  virtual void bang_stack_with_offset(int offset) = 0;

};


#include "incls/_assembler_pd.hpp.incl"
