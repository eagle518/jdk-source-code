#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime_linux_i486.cpp	1.2 04/05/04 13:44:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_linux_i486.cpp.incl"

extern "C" {
  void _mmx_Copy_arrayof_conjoint_jshorts(HeapWord* from, HeapWord* to, size_t count);
}

void OptoRuntime::generate_arraycopy_stubs() {
  _jbyte_arraycopy  = CAST_FROM_FN_PTR(address, _Copy_conjoint_bytes);
  _jshort_arraycopy = CAST_FROM_FN_PTR(address, _Copy_conjoint_jshorts_atomic);
  _jint_arraycopy   = CAST_FROM_FN_PTR(address, _Copy_conjoint_jints_atomic);
  _jlong_arraycopy  = CAST_FROM_FN_PTR(address, _Copy_conjoint_jlongs_atomic);

  _arrayof_jbyte_arraycopy    = CAST_FROM_FN_PTR(address, _Copy_arrayof_conjoint_bytes);
  if (VM_Version::supports_mmx()) {
    _arrayof_jshort_arraycopy = CAST_FROM_FN_PTR(address, _mmx_Copy_arrayof_conjoint_jshorts);
  } else {
    _arrayof_jshort_arraycopy = CAST_FROM_FN_PTR(address, _Copy_arrayof_conjoint_jshorts);
  }
  _arrayof_jint_arraycopy     = CAST_FROM_FN_PTR(address, _Copy_arrayof_conjoint_jints);
  // No specialized _Copy_arrayof_conjoint_jlongs, so use the generic one
  _arrayof_jlong_arraycopy    = CAST_FROM_FN_PTR(address, _Copy_conjoint_jlongs_atomic);
}
