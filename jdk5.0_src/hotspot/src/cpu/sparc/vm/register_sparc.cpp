#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)register_sparc.cpp	1.7 03/12/23 16:37:20 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_register_sparc.cpp.incl"


const char* RegisterImpl::name() const {
  const char* names[number_of_registers] = {
    "G0", "G1", "G2", "G3", "G4", "G5", "G6", "G7",
    "O0", "O1", "O2", "O3", "O4", "O5", "SP", "O7",
    "L0", "L1", "L2", "L3", "L4", "L5", "L6", "L7",
    "I0", "I1", "I2", "I3", "I4", "I5", "FP", "I7"
  };
  return is_valid() ? names[encoding()] : "noreg";
}


const char* FloatRegisterImpl::name() const {
  const char* names[number_of_registers] = {
    "F0",  "F1",   "F2",  "F3",   "F4",  "F5",   "F6",  "F7",   "F8",  "F9",
    "F10", "F11",  "F12", "F13",  "F14", "F15",  "F16", "F17",  "F18", "F19",
    "F20", "F21",  "F22", "F23",  "F24", "F25",  "F26", "F27",  "F28", "F29",
    "F30", "F31",  "F32", "F33?", "F34", "F35?", "F36", "F37?", "F38", "F39?",
    "F40", "F41?", "F42", "F43?", "F44", "F45?", "F46", "F47?", "F48", "F49?",
    "F50", "F51?", "F52", "F53?", "F54", "F55?", "F56", "F57?", "F58", "F59?",
    "F60", "F61?", "F62"
  };
  return is_valid() ? names[encoding()] : "fnoreg";
}
