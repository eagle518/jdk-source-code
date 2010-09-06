#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cInterpreter.inline.hpp	1.3 03/12/23 16:40:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file holds platform-independant bodies of inline functions for the C++ based interpreter

#ifdef CC_INTERP

#ifdef ASSERT
extern "C" { typedef void (*verify_oop_fn_t)(oop, const char *);};
#define VERIFY_OOP(o) \
	/*{ verify_oop_fn_t verify_oop_entry = \
            *StubRoutines::verify_oop_subroutine_entry_address(); \
          if (verify_oop_entry) { \
	     (*verify_oop_entry)((o), "Not an oop!"); \
	  } \
	}*/
#else
#define VERIFY_OOP(o)
#endif

// Platform dependent data manipulation
# include "incls/_cInterpreter_pd.inline.hpp.incl"
#endif // CC_INTERP
