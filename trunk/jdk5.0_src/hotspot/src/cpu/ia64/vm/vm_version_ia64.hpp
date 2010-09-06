#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vm_version_ia64.hpp	1.6 04/02/03 11:19:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class VM_Version: public Abstract_VM_Version {
public:
  // Initialization
  static void initialize();

  static const char* cpu_features() { return ""; }
};
