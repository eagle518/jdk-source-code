/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vm_version_ia64.cpp.incl"


void VM_Version::initialize()
{
  _supports_cx8 = true;
  UseSSE = 0; // Only on x86 and x64
  intx cache_line_size = 64; // Itanium2 L1 data
  if( cache_line_size > AllocatePrefetchStepSize )
    AllocatePrefetchStepSize = cache_line_size;
  if( FLAG_IS_DEFAULT(AllocatePrefetchLines) )
    AllocatePrefetchLines = 1; // Conservative value
  assert(AllocatePrefetchLines > 0, "invalid value");
  if( AllocatePrefetchLines < 1 ) // set valid value in product VM
    AllocatePrefetchLines = 1; // Conservative value

  if (AllocatePrefetchDistance < 0) { // default is not defined ?
    AllocatePrefetchDistance = 512;
  }
  assert(AllocatePrefetchStyle >= 0, "AllocatePrefetchStyle should be positive");
  // Set 0 if AllocatePrefetchDistance was not defined.
  AllocatePrefetchStyle = AllocatePrefetchDistance > 0 ? AllocatePrefetchStyle : 0;
}
