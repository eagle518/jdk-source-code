#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_linux_ia64.hpp	1.9 03/12/23 16:38:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
//
define_pd_global(bool, DontYieldALot,            false);
define_pd_global(intx, ThreadStackSize,		 0); // 0 => use system default
define_pd_global(intx, VMThreadStackSize,	 1024);
define_pd_global(intx, CompilerThreadStackSize,  0);
define_pd_global(intx, SurvivorRatio,            8);

define_pd_global(uintx, JVMInvokeMethodSlack,    8192);

define_pd_global(intx, StackYellowPages, 1);
define_pd_global(intx, StackRedPages, 1);
define_pd_global(intx, StackShadowPages, 2);
define_pd_global(bool, UseDefaultStackSize,      false);
