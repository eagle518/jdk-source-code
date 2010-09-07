/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

// This file holds platform-independent bodies of inline functions for the C++ based interpreter

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
# include "incls/_bytecodeInterpreter_pd.inline.hpp.incl"
#endif // CC_INTERP
