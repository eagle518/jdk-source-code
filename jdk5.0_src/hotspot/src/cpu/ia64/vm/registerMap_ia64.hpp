#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)registerMap_ia64.hpp	1.5 03/12/23 16:36:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// machine-dependent implemention for register maps
  friend class frame;

 private:

  intptr_t* _bsp;
  // Add a bunch of stuff so we don't have to recompiled the worlds all the time
  intptr_t* _extra1;
  intptr_t* _extra2;
  intptr_t* _extra3;

#ifndef CORE
  // This is the hook for finding a register in an "well-known" location,
  // such as a register block of a predetermined format.
  // Like sparc this is used for the windowed registers. This is because
  // the hardware is storing the values of these register in a know location
  // relative to bsp. The non-windowed registers (of any type) are stored
  // by the compilation system and are always relative to the memory stack
  // pointer. The non-windowed registers are handled much like global
  // registers on sparc.
   address pd_location(VMReg::Name reg) const;
#endif

  void pd_clear() ;
  void pd_initialize() ;
  void pd_initialize_from(const RegisterMap* map); 

  void shift_window(frame& caller, frame& callee);
  void shift_individual_registers(frame& caller, frame& callee);
  void make_integer_regs_unsaved(void);
