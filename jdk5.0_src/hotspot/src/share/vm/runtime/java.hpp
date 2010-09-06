#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)java.hpp	1.22 03/12/23 16:43:51 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Register function to be called by before_exit
extern "C" { void register_on_exit_function(void (*func)(void)) ;}

// Execute code before all handles are released and thread is killed; prologue to vm_exit
extern void before_exit(JavaThread * thread);

// Forced VM exit (i.e, internal error or JVM_Exit)
extern void vm_exit(int code);

// Shutdown the VM but do not exit the process
extern void vm_shutdown();

// VM exit if error occurs during initialization of VM
extern void vm_exit_during_initialization(Handle exception);
extern void vm_exit_during_initialization(symbolHandle exception_name, const char* message);
extern void vm_exit_during_initialization(const char* error, const char* message = NULL);
extern void vm_shutdown_during_initialization(const char* error, const char* message = NULL);


// VM exit if C heap allocation fails
extern void vm_exit_out_of_memory(size_t size, const char* name = NULL);

