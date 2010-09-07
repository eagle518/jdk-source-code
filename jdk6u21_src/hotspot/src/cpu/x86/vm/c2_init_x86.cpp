/*
 * Copyright (c) 2000, 2005, Oracle and/or its affiliates. All rights reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_c2_init_x86.cpp.incl"

// processor dependent initialization for i486

void Compile::pd_compiler2_init() {
  guarantee(CodeEntryAlignment >= InteriorEntryAlignment, "" );
  // QQQ presumably all 64bit cpu's support this. Seems like the ifdef could
  // simply be left out.
#ifndef AMD64
  if (!VM_Version::supports_cmov()) {
    ConditionalMoveLimit = 0;
  }
#endif // AMD64
}
