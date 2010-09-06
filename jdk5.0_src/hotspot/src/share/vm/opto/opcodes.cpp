#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)opcodes.cpp	1.9 03/12/23 16:42:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ----------------------------------------------------------------------------
// Build a table of class names as strings.  Used both for debugging printouts
// and in the ADL machine descriptions.
#define macro(x) #x,
const char *NodeClassNames[] = {
  "Node",
  "Set",
  "RegI",
  "RegP",
  "RegF",
  "RegD",
  "RegL",
  "RegFlags",
  "_last_machine_leaf",
#include "classes.hpp"
  "_last_class_name",
};
#undef macro

