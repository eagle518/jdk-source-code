/*
 * Copyright (c) 2000, 2009, Oracle and/or its affiliates. All rights reserved.
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

//
// Sets the default values for platform dependent flags used by the
// runtime system.  (see globals.hpp)
//

define_pd_global(uintx, JVMInvokeMethodSlack,    12288);
define_pd_global(intx, CompilerThreadStackSize,  0);

// Only used on 64 bit platforms
define_pd_global(uintx, HeapBaseMinAddress,      4*G);
// Only used on 64 bit Windows platforms
define_pd_global(bool, UseVectoredExceptions, false);
