/*
 * Copyright (c) 1998, 2008, Oracle and/or its affiliates. All rights reserved.
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

// ----------------------------------------------------------------------------
// Build a table of class names as strings.  Used both for debugging printouts
// and in the ADL machine descriptions.
#define macro(x) #x,
const char *NodeClassNames[] = {
  "Node",
  "Set",
  "RegN",
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
