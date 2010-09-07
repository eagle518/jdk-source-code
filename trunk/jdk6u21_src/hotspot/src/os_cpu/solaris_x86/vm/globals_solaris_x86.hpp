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

// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)

define_pd_global(bool, DontYieldALot,            true); // Determined in the design center
#ifdef AMD64
define_pd_global(intx, ThreadStackSize,          1024); // 0 => use system default
define_pd_global(intx, VMThreadStackSize,        1024);
define_pd_global(uintx,JVMInvokeMethodSlack,     8*K);
#else
// ThreadStackSize 320 allows TaggedStackInterpreter and a couple of test cases
// to run while keeping the number of threads that can be created high.
define_pd_global(intx, ThreadStackSize,          320);
define_pd_global(intx, VMThreadStackSize,        512);
define_pd_global(uintx,JVMInvokeMethodSlack,     10*K);
#endif // AMD64

define_pd_global(intx, CompilerThreadStackSize,  0);

// Only used on 64 bit platforms
define_pd_global(uintx,HeapBaseMinAddress,       256*M);
// Only used on 64 bit Windows platforms
define_pd_global(bool, UseVectoredExceptions,    false);
