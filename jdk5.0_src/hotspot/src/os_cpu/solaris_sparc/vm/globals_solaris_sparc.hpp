#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_solaris_sparc.hpp	1.14 03/12/23 16:38:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
//

define_pd_global(uintx, JVMInvokeMethodSlack,    12288);
define_pd_global(intx, CompilerThreadStackSize,  0);
define_pd_global(bool, UseDefaultStackSize,      false);
