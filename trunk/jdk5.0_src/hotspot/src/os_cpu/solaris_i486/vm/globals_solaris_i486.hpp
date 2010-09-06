#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_solaris_i486.hpp	1.18 03/12/23 16:38:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
//
define_pd_global(bool, DontYieldALot,            true); // Determined in the design center
define_pd_global(bool, UseStackBanging,          true); 
define_pd_global(intx, ThreadStackSize,		 256);
define_pd_global(intx, VMThreadStackSize,	 256);
define_pd_global(intx, SurvivorRatio,		 32);   // Design center runs on 1.3.1
define_pd_global(intx, CompilerThreadStackSize,	 0);
define_pd_global(uintx, JVMInvokeMethodSlack,    10*K);
define_pd_global(bool, UseDefaultStackSize,      false);
