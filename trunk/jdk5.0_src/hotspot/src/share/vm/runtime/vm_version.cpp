#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vm_version.cpp	1.39 04/03/03 09:35:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vm_version.cpp.incl"

const char* Abstract_VM_Version::_s_vm_release = Abstract_VM_Version::vm_release();
const char* Abstract_VM_Version::_s_internal_vm_info_string = Abstract_VM_Version::internal_vm_info_string();
bool Abstract_VM_Version::_supports_cx8 = false;
unsigned int Abstract_VM_Version::_logical_processors_per_package = 1U;

#ifndef HOTSPOT_BUILD_VERSION
  #define HOTSPOT_BUILD_VERSION "1.5-internal"
#endif

#if defined(_LP64)
  #define VMNAME CORE_ONLY("Java HotSpot(TM) 64-Bit Core VM")          \
                 COMPILER1_ONLY("Java HotSpot(TM) 64-Bit Client VM")   \
                 COMPILER2_ONLY("Java HotSpot(TM) 64-Bit Server VM")
#else
  #define VMNAME CORE_ONLY("Java HotSpot(TM) Core VM")                 \
                 COMPILER1_ONLY("Java HotSpot(TM) Client VM")          \
                 COMPILER2_ONLY("Java HotSpot(TM) Server VM")
#endif

const char* Abstract_VM_Version::vm_name() {
  return VMNAME;
}


const char* Abstract_VM_Version::vm_vendor() {
#ifdef VENDOR
  return XSTR(VENDOR);
#else
  return "Sun Microsystems Inc.";
#endif
}


const char* Abstract_VM_Version::vm_info_string() {
  switch (Arguments::mode()) {
    case Arguments::_int:
      return UseSharedSpaces ? "interpreted mode, sharing" : "interpreted mode";
    case Arguments::_mixed:
      return UseSharedSpaces ? "mixed mode, sharing"       :  "mixed mode";
    case Arguments::_comp:
      return UseSharedSpaces ? "compiled mode, sharing"    : "compiled mode";
  };
  ShouldNotReachHere();
  return "";
}

// NOTE: do *not* use stringStream. this function is called by 
//       fatal error handler. if the crash is in native thread,
//       stringStream cannot get resource allocated and will SEGV.
const char* Abstract_VM_Version::vm_release() {
  #ifdef PRODUCT
    return HOTSPOT_BUILD_VERSION;
  #elif defined(ASSERT)
    return HOTSPOT_BUILD_VERSION "-debug";
  #else
    return HOTSPOT_BUILD_VERSION "-release";
  #endif
}

const char* Abstract_VM_Version::internal_vm_info_string() {
  #ifndef HOTSPOT_BUILD_USER
    #define HOTSPOT_BUILD_USER unknown
  #endif

  #ifndef HOTSPOT_BUILD_COMPILER
    #ifdef _MSC_VER
      #if   _MSC_VER == 1100
        #define HOTSPOT_BUILD_COMPILER "MS VC++ 5.0"
      #elif _MSC_VER == 1200
        #define HOTSPOT_BUILD_COMPILER "MS VC++ 6.0"
      #else
        #define HOTSPOT_BUILD_COMPILER "unknown MS VC++:" XSTR(_MSC_VER)
      #endif
    #elif defined(__SUNPRO_CC)
      #if   __SUNPRO_CC == 0x520
        #define HOTSPOT_BUILD_COMPILER "Workshop 5.2 compat=" \
          XSTR(__SUNPRO_CC_COMPAT)
      #elif __SUNPRO_CC == 0x500
        #define HOTSPOT_BUILD_COMPILER "Workshop 5.0 compat=" \
          XSTR(__SUNPRO_CC_COMPAT)
      #elif __SUNPRO_CC == 0x420
        #define HOTSPOT_BUILD_COMPILER "Workshop 4.2"
      #else
        #define HOTSPOT_BUILD_COMPILER "unknown Workshop:" XSTR(__SUNPRO_CC)
      #endif
    #elif defined(__GNUC__)
        #define HOTSPOT_BUILD_COMPILER "gcc " __VERSION__
    #else
      #define HOTSPOT_BUILD_COMPILER "unknown compiler"
    #endif
  #endif

  #define OS     LINUX_ONLY("linux")             \
                 WINDOWS_ONLY("windows")         \
                 SOLARIS_ONLY("solaris")

  #define CPU    IA32_ONLY("x86")                \
                 IA64_ONLY("ia64")               \
                 AMD64_ONLY("amd64")             \
                 SPARC_ONLY("sparc")

  return VMNAME " (" HOTSPOT_BUILD_VERSION ") for " OS "-" CPU 
         ", built on " __DATE__ " " __TIME__ 
         " by " XSTR(HOTSPOT_BUILD_USER) " with " HOTSPOT_BUILD_COMPILER;
}

