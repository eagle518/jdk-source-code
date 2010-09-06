#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_FrameMap_i486.hpp	1.48 03/12/23 16:36:05 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

//  On i486 the frame looks as follows:
//
//  +----------------+---------+----------------------------+----------------+-----------
//  | size_arguments | 2 words | size_locals-size_arguments | _size_monitors | spilling .
//  +----------------+---------+----------------------------+----------------+-----------
//
//  The FPU registers are mapped with their offset from TOS; therefore the
//  status of FPU stack must be updated during code emission.


 public:

//  Simulates the FPU stack and maintains mapping [fpu-register -> stack offset]
//  FPU registers are described as numbers from 0..nof_fpu_regs-1 


  // the new style

  class FpuStackRes VALUE_OBJ_CLASS_SPEC {
   private:
    int _offset;
    bool _reversed;
   public:
    FpuStackRes(int offset, bool reversed) : _offset(offset), _reversed(reversed) {}
    int offset() const                           { return _offset; }
    bool is_reversed() const                     { return _reversed; }
  };

  // floating point registers are specified by a number 0..nof_fpu_regs-1
  class FpuStackSim VALUE_OBJ_CLASS_SPEC {
   private:
    GrowableArray<intStack*> _regs;   
    int _stack_size; 

    int tos_index() const                        { return _stack_size - 1; }
    intStack* regs_at_tos() const                { return _regs.at(tos_index()); }

    void dec_stack_size()                        { _stack_size--; }
    void inc_stack_size()                        { _stack_size++; }

    void exchange_with_tos(int offset);

   public:
    // the stack can contain several entries in a slot (useing dup)
    // the pop eliminates one entry, and returns true if that was the last entry adn the stack is popped
    FpuStackSim();
    bool pop(int rnr); // rnr must be on tos; return true if we can pop the fpu stack
    void push(int rnr);
    void dup(int from_rnr, int to_rnr);
    void swap();

    int offset_from_tos(int rnr) const;          // return the offset of rnr from TOS
    int move_on_tos(int rnr);                    // moves rnr on TOS and returns the offset of the original offset_from_tos(rnr)
    bool has_stack_offset(int rnr, int offset) const { return offset_from_tos(rnr) == offset; }
    bool is_empty();
    int stack_size() const                       { return _stack_size; }
    void clear();
    void print() PRODUCT_RETURN;

  };

 private:
  FpuStackSim        _fpu_stack;
  static c1_RegMask  _byte_reg_mask;
  
  WordSize fp_offset_for_slot          (int slot) const;
  int      local_to_slot               (int local_name, bool is_two_word) const;
  WordSize fp_offset_for_name          (int name, bool is_two_word, bool for_hi_word) const;
  WordSize fp_offset_for_monitor_lock  (int monitor_index) const;
  WordSize fp_offset_for_monitor_object(int monitor_index) const;
  Address  make_new_address            (WordSize fp_offset) const;
  bool     location_for_fp_offset      (WordSize word_offset_from_fp,
                                        Location::Type loc_type,
                                        Location* loc) const;
  WordSize fp2sp_offset                (WordSize fp_offset) const;

 public:
  static RInfo _esiRInfo;
  static RInfo _ediRInfo;
  static RInfo _ebxRInfo;
  static RInfo _eaxRInfo;
  static RInfo _edxRInfo;
  static RInfo _ecxRInfo;
  static RInfo _espRInfo;
  static RInfo _ebpRInfo;
  static RInfo _eax_edxRInfo;
  static RInfo _f0RInfo;
  static RInfo _d0RInfo;


  // no callee save registers on i486
  static c1_RegMask callee_save_regs()  { ShouldNotReachHere(); return c1_RegMask(); }

  FpuStackSim* fpu_stack () { return &_fpu_stack; }

  // OptoReg name for spilled physical FPU stack slot n
  static OptoReg::Name fpu_stack_regname (int n);

  // OptoReg name for spilled virtual FPU register n
  OptoReg::Name fpu_regname (int n);

  static Register first_register();
  static Register first_byte_register();
  static c1_RegMask byte_reg_mask () { return _byte_reg_mask; }
