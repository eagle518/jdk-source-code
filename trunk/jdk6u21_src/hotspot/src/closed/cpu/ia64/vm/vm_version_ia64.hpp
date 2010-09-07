/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class VM_Version: public Abstract_VM_Version {
public:
  // Initialization
  static void initialize();

  static const char* cpu_features() { return ""; }
};
