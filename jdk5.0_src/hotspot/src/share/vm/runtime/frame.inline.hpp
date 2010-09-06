#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)frame.inline.hpp	1.15 03/12/23 16:43:41 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file holds platform-independant bodies of inline functions for frames.

// Note: The bcx usually contains the bcp; however during GC it contains the bci
//       (changed by gc_prologue() and gc_epilogue()) to be methodOop position
//       independent. These accessors make sure the correct value is returned
//       by testing the range of the bcx value. bcp's are guaranteed to be above
//       max_method_code_size, since methods are always allocated in OldSpace and
//       Eden is allocated before OldSpace.
//
//       The bcp is accessed sometimes during GC for ArgumentDescriptors; than
//       the correct translation has to be performed (was bug).

bool frame::is_bci(intptr_t bcx) {
#ifdef _LP64
  return ((uintptr_t) bcx) <= ((uintptr_t) max_method_code_size) ; 
#else	
  return 0 <= bcx && bcx <= max_method_code_size;
#endif
}

// here are the platform-dependent bodies:

# include "incls/_frame_pd.inline.hpp.incl"
