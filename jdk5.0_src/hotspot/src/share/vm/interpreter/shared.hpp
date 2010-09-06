#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)shared.hpp	1.26 03/12/23 16:40:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This value is provided by the compilers in compiler-specific ways.
// In the CORE build it's used to trim out various compiler/interpreter
// shared structures.
#ifdef CORE
#define REGNAME_SIZE ConcreteRegisterImpl::number_of_registers
#endif
#ifdef COMPILER1
#define REGNAME_SIZE ConcreteRegisterImpl::number_of_registers
#endif
#ifdef COMPILER2
#define REGNAME_SIZE _last_Mach_Reg
#endif


//------------------------------OptoReg----------------------------------------
// We eventually need Registers for the Real World.  Registers are essentially
// non-SSA names.  A Register is represented as a number.  Non-regular values
// (e.g., Control, Memory, I/O) use the Special register.  The actual machine
// registers (as described in the ADL file for a machine) start at zero.
// Stack-slots (spill locations) start at the nest Chunk past the last machine
// register.
//
// Note that stack spill-slots are treated as a very large register set.
// They have all the correct properties for a Register: not aliased (unique
// named).  There is some simple mapping from a stack-slot register number
// to the actual location on the stack; this mapping depends on the calling
// conventions and is described in the ADL.
//
// This silly enum is my attempt to be both efficient and type-safe.  This
// really should be a C++ class, but passing and returning structures is
// usually much worse than passing/returning an int - even when the structure
// is merely a wrapper for an int.  Class OptoReg is merely used as a way
// to control the C++ namespace.
class OptoReg VALUE_OBJ_CLASS_SPEC { 
 public:
  enum Name {
    // Chunk 0
#ifdef COMPILER2
    Physical = AdlcVMDeps::Physical, // Start of physical regs 
#endif
    // A few oddballs at the edge of the world
    Special = -2,		// All special (not allocated) values
    Bad = -1			// Not a register
  };

  // Increment a register number.  As in:
  //    "for ( OptoReg::Name i; i=Control; i = add(i,1) ) ..."
  static Name add( Name x, int y ) { return Name(x+y); }

  // (We would like to have an operator+ for RegName, but it is not
  // a class, so this would be illegal in C++.)

  static void dump( int );
};

//------------------------------VMReg------------------------------------------
// The VM uses 'unwarped' stack slots; the compiler uses 'warped' stack slots.
// Register numbers below SharedInfo::stack0 are the same for both.  Register
// numbers above stack0 are either warped (in the compiler) or unwarped
// (in the VM).  Unwarped numbers represent stack indices, offsets from
// the current stack pointer.  Warped numbers are required during compilation
// when we do not yet know how big the frame will be.
class VMReg VALUE_OBJ_CLASS_SPEC { 
public:
  enum Name { };
};


//------------------------------SharedInfo------------------------------------
// Interesting bits shared between the compiled High Performance system
// and the core interpreted-only system.  For the interpreted-only system
// these bits take on default values (usually zero).  For the compiled
// system, the compiler initialization overwrites them.

class SharedInfo : AllStatic {
 public:
  // Names for registers
  static const char *regName[REGNAME_SIZE];

  // Stack pointer register
  static OptoReg::Name c_frame_pointer;

  static OptoReg::Name stack0;

  // Convert register numbers to stack slots and vice versa
  static OptoReg::Name stack2reg( int idx ) { 
    return OptoReg::Name(stack0+idx); }

  static unsigned reg2stack( OptoReg::Name r ) {
    assert( r >=stack0, "Not a stack-based register" );
    return r - stack0; 
  }

  static void set_stack0(int n);
  static void set_regName();
};
