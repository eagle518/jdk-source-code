#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)opcodes.hpp	1.25 03/12/23 16:42:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Build a big enum of class names to give them dense integer indices
#define macro(x) Op_##x,
enum Opcodes {
  Op_Node = 0,
  macro(Set)                    // Instruction selection match rule
  macro(RegI)                   // Machine integer register
  macro(RegP)                   // Machine pointer register
  macro(RegF)                   // Machine float   register
  macro(RegD)                   // Machine double  register
  macro(RegL)                   // Machine long    register
  macro(RegFlags)               // Machine flags   register
  _last_machine_leaf,           // Split between regular opcodes and machine
#include "classes.hpp"
  _last_opcode
};
#undef macro

// Table of names, indexed by Opcode
extern const char *NodeClassNames[];
