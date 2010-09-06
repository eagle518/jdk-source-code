#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)runtime_linux_amd64.cpp	1.1 04/04/30 16:18:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_runtime_linux_amd64.cpp.incl"

void OptoRuntime::generate_arraycopy_stubs() {
  // Use default for _jbyte_arraycopy
  _jshort_arraycopy = CAST_FROM_FN_PTR(address, _Copy_conjoint_jshorts_atomic);
  _jint_arraycopy   = CAST_FROM_FN_PTR(address, _Copy_conjoint_jints_atomic);
  _jlong_arraycopy  = CAST_FROM_FN_PTR(address, _Copy_conjoint_jlongs_atomic);

  _arrayof_jbyte_arraycopy  = CAST_FROM_FN_PTR(address, _Copy_arrayof_conjoint_bytes);
  _arrayof_jshort_arraycopy = CAST_FROM_FN_PTR(address, _Copy_arrayof_conjoint_jshorts);
  _arrayof_jint_arraycopy   = CAST_FROM_FN_PTR(address, _Copy_arrayof_conjoint_jints);
  _arrayof_jlong_arraycopy  = CAST_FROM_FN_PTR(address, _Copy_arrayof_conjoint_jlongs);
}
