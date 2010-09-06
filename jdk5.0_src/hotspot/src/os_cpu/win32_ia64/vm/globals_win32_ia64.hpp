#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_win32_ia64.hpp	1.9 03/12/23 16:38:35 JVM"
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
define_pd_global(intx, VMThreadStackSize,	 0); // 0 => use system default
define_pd_global(intx, SurvivorRatio,            8); 

#ifdef ASSERT
define_pd_global(intx, CompilerThreadStackSize,  1024);
#else
define_pd_global(intx, CompilerThreadStackSize,  0);
#endif

define_pd_global(uintx, JVMInvokeMethodSlack,    8192);

define_pd_global(intx, StackShadowPages, 6);
define_pd_global(intx, StackYellowPages, 2);
define_pd_global(intx, StackRedPages, 2);
define_pd_global(bool, UseDefaultStackSize,      false);
