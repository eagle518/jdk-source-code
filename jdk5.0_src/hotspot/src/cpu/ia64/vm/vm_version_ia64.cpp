#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vm_version_ia64.cpp	1.5 04/02/03 11:19:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vm_version_ia64.cpp.incl"


void VM_Version_init()
{
  VM_Version::initialize();
}

void VM_Version::initialize()
{
  _supports_cx8 = true;
}

