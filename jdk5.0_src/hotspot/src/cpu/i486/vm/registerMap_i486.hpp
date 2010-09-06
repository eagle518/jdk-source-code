#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)registerMap_i486.hpp	1.9 03/12/23 16:36:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// machine-dependent implemention for register maps
  friend class frame;

 private:
#ifndef CORE
  // This is the hook for finding a register in an "well-known" location,
  // such as a register block of a predetermined format.
  // Since there is none, we just return NULL.
  // See registerMap_sparc.hpp for an example of grabbing registers
  // from register save areas of a standard layout.
   address pd_location(VMReg::Name reg) const {return NULL;}
#endif

  // no PD state to clear or copy:
  void pd_clear() {}
  void pd_initialize() {}
  void pd_initialize_from(const RegisterMap* map) {}
