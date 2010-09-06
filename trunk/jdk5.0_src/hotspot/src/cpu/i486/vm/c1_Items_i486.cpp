#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Items_i486.cpp	1.4 03/12/23 16:36:06 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_Items_i486.cpp.incl"


// we always pop registers from FPU stack after finishing an operation;
// therefore mark floats and doubles as destroying register
void Item::handle_float_kind() {
  if (type()->is_float_kind()) {
    set_destroys_register();
  }
}


