/*
 * Copyright (c) 1997, 2005, Oracle and/or its affiliates. All rights reserved.
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
  static address lookup_entry(methodHandle method, bool& in_base_library, TRAPS);
  static address lookup_entry_prefixed(methodHandle method, bool& in_base_library, TRAPS);
 public:
  // Lookup native function. May throw UnsatisfiedLinkError.
  static address lookup(methodHandle method, bool& in_base_library, TRAPS);

  // Lookup native functions in base library.
  static address base_library_lookup(const char* class_name, const char* method_name, const char* signature);
};
