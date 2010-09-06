#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)nativeLookup.hpp	1.19 03/12/23 16:43:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// NativeLookup provides an interface for finding DLL entry points for 
// Java native functions. 

class NativeLookup : AllStatic {
 private:
  // JNI name computation
  static char* pure_jni_name(methodHandle method);
  static char* long_jni_name(methodHandle method);

  // Style specific lookup 
  static address lookup_style(methodHandle method, char* pure_name, const char* long_name, int args_size, bool os_style, bool& in_base_library, TRAPS);
  static address lookup_base (methodHandle method, bool& in_base_library, TRAPS);
 public:
  // Lookup native function. May throw UnsatisfiedLinkError.
  static address lookup(methodHandle method, bool& in_base_library, TRAPS);

  // Lookup native functions in base library.
  static address base_library_lookup(const char* class_name, const char* method_name, const char* signature);
};
