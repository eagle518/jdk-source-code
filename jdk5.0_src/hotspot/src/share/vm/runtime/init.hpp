#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)init.hpp	1.13 04/06/15 12:17:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// init_globals replaces C++ global objects so we can use the standard linker
// to link Delta (which is at least twice as fast as using the GNU C++ linker). 
// Also, init.c gives explicit control over the sequence of initialization.

// Programming convention: instead of using a global object (e,g, "Foo foo;"), 
// use "Foo* foo;", create a function init_foo() in foo.c, and add a call
// to init_foo in init.cpp.

jint init_globals();     // call constructors at startup (main Java thread)
void vm_init_globals();  // call constructors at startup (VM thread)
void exit_globals();     // call destructors before exit

bool is_init_completed();     // returns true when bootstrapping has completed
void set_init_completed();    // set basic init to completed
