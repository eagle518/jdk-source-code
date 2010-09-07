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

define_pd_global(bool, DontYieldALot,            false);
#ifdef AMD64
define_pd_global(intx, ThreadStackSize,          1024); // 0 => use system default
define_pd_global(intx, VMThreadStackSize,        1024);
#else
// ThreadStackSize 320 allows TaggedStackInterpreter and a couple of test cases
// to run while keeping the number of threads that can be created high.
// System default ThreadStackSize appears to be 512 which is too big.
define_pd_global(intx, ThreadStackSize,          320);
define_pd_global(intx, VMThreadStackSize,        512);
#endif // AMD64

define_pd_global(intx, CompilerThreadStackSize,  0);

define_pd_global(uintx,JVMInvokeMethodSlack,     8192);

// Only used on 64 bit platforms
define_pd_global(uintx,HeapBaseMinAddress,       2*G);
// Only used on 64 bit Windows platforms
define_pd_global(bool, UseVectoredExceptions,    false);
