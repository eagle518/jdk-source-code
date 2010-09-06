#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)frame_ia64.hpp	1.12 03/12/23 16:36:40 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
//******************************************************************************
//*
//* Copyright (c) 1998-2002 Hewlett-Packard Company. All Rights Reserved.
//*
//******************************************************************************

// A frame represents a physical stack frame (an activation).  Frames can be
// C or Java frames, and the Java frames can be interpreted or compiled.
// In contrast, vframes represent source-level activations, so that one physical frame 
// can correspond to multiple source level frames because of inlining.
// A frame is comprised of {pc, fp, sp}

// On IA64 frames are split into two portions: one part is located in the
// memory stack (which grows from higher toward lower addresses); the other
// portion is located in the Register Stack Engine (RSE). On entry to the
// VM runtime the RSE is flushed to the backing store (which grows toward
// higher addresses) so the portion of the frame that existed in registers
// is now also present in memory. On return to java RSE loads
// are performed so that the frame (potentially modifies) now exists in 
// registers again. Since the interpreter does not keep oops in registers
// in a CORE only system there would not be any need for RSE loads. In a
// mixed mode system we have no choice.

// ------------------------------ C++ interpreter ----------------------------------------
// Layout of C++ interpreter frame at entry to frame manager
//
//
//
// o0    [ java arg(0)]  (locals[0])
//       [ java arg...]
//       [ java arg(n)]
//       [ scratch 2  ]  i7/o6 (parm slot 7)
// sp -> [ scratch 1  ]  i6/o6 (parm slot 6) (location is sp-orig)
//
// On entry o0 == locals[0]
//          o1 == methodOop
//          o2 == thread
//          o3 == (NULL) prev. state

// After initial frame manager frame is setup (non-sync method, i.e. no initial monitor)
// o0    [ java arg(0)   ]  (locals[0])
//       [ java arg...   ]
//       [ java arg(n)   ]
//       [ addntl locals ]
//       [ addntl locals ]  variable size holds locals that weren't args
//       [ addntl locals ]
//       [ scratch 2     ]  i7/o6 (parm slot 7)
//       [ scratch 1     ]  i6/o6 (parm slot 6)
//       [               ]
//       [ cInterpreter  ]  activations cInterpreter object (state)
//       [               ]
//       [               ]
//       [               ]
//       [ slop word2    ]  used to be sure we have sufficent expr stack space
//       [ slop word1    ]  "    "
//       [ java stack    ]  <=  state->_monitor_base, state->_stack_base,
//       [ java stack    ]  
//       [ java stack    ]  
//       [ java stack    ]  
//       [ java stack    ] 
//       [ scratch 2     ]  <= state->_stack_limit
// sp -> [ scratch 1     ] 
//
// Note this is only the memory stack. There is also a register window allocated. The
// Java "FP" is the value of ar.bsp for each interpreter frame we create. None of
// these registers ever contain oops and don't need to be walked. However the registers
// have a consistent layout which allow us to find the return address (i.e. our caller)
// and most importantly the register containing the pointer to the cInterpreter object.
// This object contains oops and also contains values that allow us to walk frames.
// Note that when we "recurse" in the interpreter we are simply tail calling in the
// frame manager (while trimming the java expression stack to its live range) and
// since we don't really return we simply create a dummy (InterpretMethodDummy)
// return address as the caller for the recursive entry. On other platforms this
// fools the debugger into thinking a real call happened and since the dummy routine
// has the same signature as cInterpreter::InterpretMethod things look perfectly 
// normal in the debugger. That trick is not possible on IA64.
// 
//
// Interpreter Frame Layout (RSE)
// ------------------------------
//    |                                                  |
//    +--------------------------------------------------+
//    | GR63 - 8 registers used to pass arguments to     |
//    | ....   compiled/VM runtime/JNI code              |
//    | GR56                                             |
//    +--------------------------------------------------+
//    | GR55 - scratch                                   | L15
//    | GR54 - scratch                                   | L14
//    | GR53 - scratch                                   | L13
//    | GR52 - scratch                                   | L12
//    | GR51 - scratch                                   | L11
//    | GR50 - scratch                                   | L10
//    | GR49 - scratch                                   | L9
//    | GR48 - scratch                                   | L8
//    | GR47 - Lstate (current cInterpreter*)            | L7
//    | GR46 - Preserved value of PR                     | L6
//    | GR45 - Preserved value of UNAT                   | L5
//    | GR44 - Preserved value of GP                     | L4
//    | GR43 - Preserved value of LC                     | L3
//    | GR42 - Preserved value of RP                     | L2
//    | GR41 - Preserved value of PFS                    | L1
//    | GR40 - Sender's SP                               | L0
//    +--------------------------------------------------+
//    | GR39 - Unused                                    |
//    | GR38 - Unused                                    |
//    | GR37 - Unused                                    |
//    | GR36 - Unused                                    |
//    | GR35 - Unused                                    |
//    | GR34 - Unused                                    |
//    | GR33 - Previous cInterpreter obj (null from stub)|
//    | GR32 - Locals[0] pointer                         | <-- ar.bsp
//    +--------------------------------------------------+     (Java fp)
//
// Note: intention is to have interpreter use compiler calling conventions
// This will change the use of GR32-GR39 to contain the 1st eight parameters
// to the method. The interpreter will via a signature handler copy the
// parameters to the stack in such a mannner that we wind up with a
// Locals array.



// All frames: 

 public:

  enum {
    // normal return address is 1 bundle past PC
    pc_return_offset                         =  0,

    // size of each block, in order of increasing address:
    register_save_words                      = 2,
    // c2 uses 32 bits slots for its calculations so double save_words
    register_save_slots                      = register_save_words * 2,

    // JUNK but ad file needs it
    varargs_offset                           = register_save_slots,

#if 0 /* QQQ */
    // Offsets in various frames for locations of saved registers holding important
    // data we need during stack walking
    // Offsets of each RSE backing store slot relative from java fp

    interpreter_frame_sender_sp_offset      =  GR_Lsave_SP  - GR_I0,
    interpreter_frame_pfs_offset            =  GR_Lsave_PFS - GR_I0,
    interpreter_frame_sender_pc_offset      =  GR_Lsave_RP  - GR_I0,
    interpreter_frame_state_ptr_offset      =  GR_Lstate    - GR_I0,
   
    // Entry extra adjust if call stub changes
    entry_frame_wrapper_offset              =  GR_I0        - GR_I0,
#endif

  };


 private:
  address*   _pc_address;                        // address where pc is stored (in callee frame)
  uintptr_t  _pfs;                               // hardware ar.pfs
  intptr_t*  _bsp;                               // BSP at entry to frame
  intptr_t*  _raw_sp; 

  // On IA64 the saved PC can be anywhere a compiler chooses. Since there is no
  // way at present for adding unwind information for dynamically generated code
  // we have to have a convention for this with our frames. The conventions are
  // as follows:
  //
  // Interpreter- We only need to be able to find the cInterpreter state. We
  // know this will be the first input register so this is simply a known
  // offset from the bsp. 
  // return addresses by using the unwind convention to see where the language
  // system has stored the value for the frame in question. Way more complicated
  // than sparc or ia32.

 public:
  // Accessors

  intptr_t*   sp() const       { return _raw_sp; }
  intptr_t*   fp() const       { return _bsp; }
  address*    pc_addr() const  { return _pc_address; }

  static int size_of_frame(uint64_t pfs)    { return  pfs & 0x7f; }
  static int size_of_locals(uint64_t pfs)   { return  ( pfs >> 7) & 0x7f; }
  static int size_of_rotating(uint64_t pfs) { return  ( pfs >> 14) & 0xf; }
  static int rename_gbase(uint64_t pfs)     { return  ( pfs >> 18) & 0x7f; }
  static int rename_fbase(uint64_t pfs)     { return  ( pfs >> 25) & 0x7f; }
  static int rename_pbase(uint64_t pfs)     { return  ( pfs >> 32) & 0x3f; }

  // int interpreter_sp_adjustment() const { return _interpreter_sp_adjustment; }
  // void set_interpreter_sp_adjustment(int number_of_words) { _interpreter_sp_adjustment = number_of_words; }

  // Constructors

  frame(intptr_t* sp, intptr_t* fp): _raw_sp(sp) {
    assert(((uint64_t)fp & 0x7) == 0, "FP must be 8-byte aligned");
    _bsp = fp;
    _pc_address = NULL; // unpatchable
    _pc         = NULL; // unpatchable
    _pfs = 0;  // Do we really need this??
  }
       
  frame(intptr_t* sp, intptr_t* fp, address* pc_address, address pc): _raw_sp(sp) {
    assert(((uint64_t)fp & 0x7) == 0, "FP must be 8-byte aligned");
    if (pc == NULL) {
      os::breakpoint();
    }
    _bsp = fp;
    _pc_address = pc_address;
    _pc = pc;
    _pfs = 0;  // Do we really need this??
    assert(pc_address == NULL || (address) *pc_address == pc, "Frame doesn't match up");
  } 

  // Helper functions

  // this method has been added to enable passing in of a flag which indicates
  // whether the pc needs to be adjusted (in case the frame is a safepoint patched
  // frame). Adding the method here avoids changes to the shared sender method
  frame sender_with_pc_adjustment(RegisterMap* map, CodeBlob *cb, bool pc_adjustment) const;

 public:

  // patching operations
  void   patch_pc(Thread* thread, address pc);

  // All frames

  //
  //
  // Returns true iff the address range [from..to] (inclusive) contains 
  // the address of a backing store NAT collection.
  //
  // Assumes that from and to are valid backing store addresses.
  //
  // The determination of a NAT collection is taken from 
  // Itanium ISA Guide 1.0 (Feb. 00):
  //
  // 	The RSE writes its NaT collection register to the backing store
  // 	whenever BSPSTORE{8:3} = 0x3F (1 NaT collection occurring after
  // 	every 63 registers).
  //
  //  To round down to the nat collection we first round down to first
  //  of the block of 63 registers and then subtract one register word.
  //
  static bool contains_nat_collection(intptr_t* from, intptr_t* to) {
    assert ((uint64_t)to >= (uint64_t)from, "sanity check");
    intptr_t* nat = (intptr_t *)((uint64_t)(to + 1) & (~0x1ff)) - 1;
    return (nat >= from);
  }

  static bool is_nat_collection(intptr_t* addr) {
    return ((((unsigned int)addr & 0x1f8) >> 3) == 0x3f);
  }


  // Return the base address of the backing store slot at index.
  // Note: NAT collection is visible thru this interface

  static intptr_t*  fp_addr_at(int index, intptr_t* bsp)  { return &bsp[index];    }
  intptr_t*  fp_addr_at(int index) const                  { return fp_addr_at(index, fp()); }

  //
  //
  // NAT collection is invisible thru this interface
  //
  static inline intptr_t* frame::addr_at2(int index, intptr_t* bsp) {
    // Get the address of the entire backing store slot at index.
    intptr_t *addr =  &bsp[index];

    // Check to see the if the range [fp...addr] contains a NAT collection.
    if (frame::contains_nat_collection(bsp, addr)) {
	// return the address of the next backing store slot
	addr++;
    }
    return addr;
  }
  inline intptr_t* frame::addr_at2(int index) const { return addr_at2(index, fp()); }

  intptr_t* register_addr(Register reg) const                  { return addr_at2(reg - GR_I0, fp()); }
  static intptr_t* register_addr(Register reg, intptr_t* bsp)  { return addr_at2(reg - GR_I0, bsp); }
#if 0
  void set_sp( intptr_t*   newsp )         { _sp = newsp; }
#endif /* 0 */

  intptr_t*  sp_addr_at(int index) const   { return &sp()[index];    }
  intptr_t   fp_at(     int index) const   { return *fp_addr_at(index); }
  intptr_t   sp_at(     int index) const   { return *sp_addr_at(index); }

 private:

#ifndef CORE
  intptr_t* compiled_sender_sp(CodeBlob* cb) const;
  address*  compiled_sender_pc_addr(CodeBlob* cb) const;
  intptr_t* compiled_sender_fp(CodeBlob* cb) const;
#endif

  address* sender_pc_addr(void) const;

 public:

  // Interpreter frames

  inline interpreterState get_interpreterState() const {
    return (interpreterState) (*register_addr(GR_Lstate));
  }

 public:

  // Offsets from SP of saved scratch registers
  enum {
    saved_br0_offset  = 16+ 0*8,
    saved_gr2_offset  = 16+ 2*8,
    saved_gr3_offset  = 16+ 3*8,
    saved_br6_offset  = 16+ 6*8,
    saved_br7_offset  = 16+ 7*8,
    saved_gr8_offset  = 16+ 8*8,
    saved_gr9_offset  = 16+ 9*8,
    saved_gr10_offset = 16+10*8,
    saved_gr11_offset = 16+11*8,
    saved_gr14_offset = 16+14*8,
    saved_gr15_offset = 16+15*8,
    saved_gr16_offset = 16+16*8,
    saved_gr17_offset = 16+17*8,
    saved_gr18_offset = 16+18*8,
    saved_gr19_offset = 16+19*8,
    saved_gr20_offset = 16+20*8,
    saved_gr21_offset = 16+21*8,
    saved_gr22_offset = 16+22*8,
    saved_gr23_offset = 16+23*8,
    saved_gr24_offset = 16+24*8,
    saved_gr25_offset = 16+25*8,
    saved_gr26_offset = 16+26*8,
    saved_gr27_offset = 16+27*8,
    saved_gr28_offset = 16+28*8,
    saved_gr29_offset = 16+29*8,
    saved_gr30_offset = 16+30*8,
    saved_gr31_offset = 16+31*8,

    scratch_regs_save_area_size = 32*8
  };

 private:

  constantPoolCacheOop* frame::interpreter_frame_cpoolcache_addr() const;


 // Compiled frames

 public:
#if 0
  // Tells if this register can hold 64 bits on V9 (really, V8+).
  static bool holds_a_doubleword(Register reg) {
    return true;
  }
#endif /* 0 */
