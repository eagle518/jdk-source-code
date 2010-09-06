#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)hpi_solaris.cpp	1.16 03/12/23 16:37:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_hpi_solaris.cpp.incl"

# include <sys/param.h>

typedef jint (JNICALL *init_t)(GetInterfaceFunc *, void *);

// HotSpot integration note:
// code below matches code in JDK1.2fcs K build.
// "$JDK/src/solaris/javavm/runtime/javai_md.c":23 (ver. 1.7 98/09/15)

void hpi::initialize_get_interface(vm_calls_t *callbacks)
{
#define args callbacks

    // "$JDK/src/solaris/javavm/runtime/javai_md.c":32 (ver. 1.7 98/09/15)
    char buf[MAXPATHLEN];
    Dl_info dlinfo;
    void *hpi_handle;
    GetInterfaceFunc& getintf = _get_interface;
    jint (JNICALL * DLL_Initialize)(GetInterfaceFunc *, void *);

    char *thread_type = strdup("native_threads");

    os::jvm_path(buf, sizeof buf);

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
    /* we use RTLD_NOW because of bug 4032715 */
    if (TraceHPI)  tty->print("Loading HPI %s ", buf);
    hpi_handle = dlopen(buf, RTLD_NOW);
    if (hpi_handle == NULL) {
	if (TraceHPI) tty->print_cr("HPI dlopen failed: %s", dlerror());
	goto bail;
    }
    DLL_Initialize = CAST_TO_FN_PTR(jint (JNICALL *)(GetInterfaceFunc *, void *),  
                                    dlsym(hpi_handle, "DLL_Initialize"));
    if (TraceHPI && DLL_Initialize == NULL) tty->print_cr("HPI dlsym of DLL_Initialize failed: %s", dlerror());
    if (DLL_Initialize == NULL ||
        (*DLL_Initialize)(&getintf, args) < 0) {
	if (TraceHPI) tty->print_cr("HPI DLL_Initialize failed");
        goto bail;
    }
    if (TraceHPI)  tty->print_cr("HPI loaded successfully");
bail:
    free(thread_type);

#undef args
}

//Reconciliation History
// 1.3 98/10/01 11:19:43 hpi_win32.cpp
// 1.4 98/10/06 13:57:10 hpi_win32.cpp
// 1.6 99/06/22 16:38:59 hpi_win32.cpp
//End
