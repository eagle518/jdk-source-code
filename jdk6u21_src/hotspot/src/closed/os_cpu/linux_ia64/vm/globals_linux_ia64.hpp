/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
//
define_pd_global(bool, DontYieldALot,            false);
define_pd_global(intx, ThreadStackSize,          0); // 0 => use system default
define_pd_global(intx, VMThreadStackSize,        1024);
define_pd_global(intx, CompilerThreadStackSize,  0);
define_pd_global(intx, SurvivorRatio,            8);

define_pd_global(uintx, JVMInvokeMethodSlack,    8192);

define_pd_global(intx, StackYellowPages, 1);
define_pd_global(intx, StackRedPages, 1);
define_pd_global(intx, StackShadowPages, 2);

// Only used on 64 bit Windows platforms
define_pd_global(bool, UseVectoredExceptions,    false);
