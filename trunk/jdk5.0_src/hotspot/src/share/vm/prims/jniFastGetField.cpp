#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jniFastGetField.cpp	1.1 04/03/23 17:25:14 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jniFastGetField.cpp.incl"

address JNI_FastGetField::speculative_load_pclist [LIST_CAPACITY];
address JNI_FastGetField::slowcase_entry_pclist   [LIST_CAPACITY];
int     JNI_FastGetField::count = 0;

address JNI_FastGetField::find_slowcase_pc(address pc) {
  for (int i=0; i<count; i++) {
    if (speculative_load_pclist[i] == pc) {
      return slowcase_entry_pclist[i];
    }
  }
  return (address)-1;
}

