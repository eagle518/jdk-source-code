#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c2_init_sparc.cpp	1.11 03/12/23 16:37:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c2_init_sparc.cpp.incl"

// processor dependent initialization for sparc

void Compile::pd_compiler2_init() {
  guarantee(CodeEntryAlignment >= InteriorEntryAlignment, "" );
  guarantee( VM_Version::v9_instructions_work(), "Server compiler does not run on V8 systems" );
}
