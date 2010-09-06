#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)adlcVMDeps.hpp	1.11 03/12/23 16:38:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Stub for the real thing.

class AdlcVMDeps : public AllStatic {  
 public:
  // Mirror of TypeFunc types
  enum { Control, I_O, Memory, FramePtr, ReturnAdr, Parms };

  enum Cisc_Status { Not_cisc_spillable = -1 };

  // Mirror of OptoReg::Name names
  enum Name {   
    Physical = 0		// Start of physical regs    
  };

  // relocInfo
  static const char* oop_reloc_type()  { return "relocInfo::oop_type"; }
  static const char* none_reloc_type() { return "relocInfo::none"; }  
};
