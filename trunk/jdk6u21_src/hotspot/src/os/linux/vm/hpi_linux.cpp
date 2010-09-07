/*
 * Copyright (c) 1999, 2006, Oracle and/or its affiliates. All rights reserved.
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
# include "incls/_hpi_linux.cpp.incl"

# include <sys/param.h>
# include <dlfcn.h>

typedef jint (JNICALL *init_t)(GetInterfaceFunc *, void *);

void hpi::initialize_get_interface(vm_calls_t *callbacks) {
    char buf[JVM_MAXPATHLEN];
    void *hpi_handle;
    GetInterfaceFunc& getintf = _get_interface;
    jint (JNICALL * DLL_Initialize)(GetInterfaceFunc *, void *);

    if (HPILibPath && HPILibPath[0]) {
      strncpy(buf, HPILibPath, JVM_MAXPATHLEN - 1);
      buf[JVM_MAXPATHLEN - 1] = '\0';
    } else {
      const char *thread_type = "native_threads";

      os::jvm_path(buf, JVM_MAXPATHLEN);

#ifdef PRODUCT
      const char * hpi_lib = "/libhpi.so";
#else
      char * ptr = strrchr(buf, '/');
      assert(strstr(ptr, "/libjvm") == ptr, "invalid library name");
      const char * hpi_lib = strstr(ptr, "_g") ? "/libhpi_g.so" : "/libhpi.so";
#endif

      *(strrchr(buf, '/')) = '\0';  /* get rid of /libjvm.so */
      char* p = strrchr(buf, '/');
      if (p != NULL) p[1] = '\0';   /* get rid of hotspot    */
      strcat(buf, thread_type);
      strcat(buf, hpi_lib);
    }

    if (TraceHPI) tty->print_cr("Loading HPI %s ", buf);
#ifdef SPARC
    // On 64-bit Ubuntu Sparc RTLD_NOW leads to unresolved deps in libpthread.so
#   define OPEN_MODE RTLD_LAZY
#else
    // We use RTLD_NOW because of bug 4032715
#   define OPEN_MODE RTLD_NOW
#endif
    hpi_handle = dlopen(buf, OPEN_MODE);
#undef OPEN_MODE

    if (hpi_handle == NULL) {
        if (TraceHPI) tty->print_cr("HPI dlopen failed: %s", dlerror());
        return;
    }
    DLL_Initialize = CAST_TO_FN_PTR(jint (JNICALL *)(GetInterfaceFunc *, void *),
                                    dlsym(hpi_handle, "DLL_Initialize"));
    if (TraceHPI && DLL_Initialize == NULL) tty->print_cr("HPI dlsym of DLL_Initialize failed: %s", dlerror());
    if (DLL_Initialize == NULL ||
        (*DLL_Initialize)(&getintf, callbacks) < 0) {
        if (TraceHPI) tty->print_cr("HPI DLL_Initialize failed");
        return;
    }
    if (TraceHPI)  tty->print_cr("HPI loaded successfully");
}
