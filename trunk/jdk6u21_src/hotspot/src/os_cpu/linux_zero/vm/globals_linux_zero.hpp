/*
 * Copyright (c) 2000, 2005, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008 Red Hat, Inc.
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
// Set the default values for platform dependent flags used by the
// runtime system.  See globals.hpp for details of what they do.
//

define_pd_global(bool,  DontYieldALot,           false);
#ifdef _LP64
define_pd_global(intx,  ThreadStackSize,         1536);
define_pd_global(intx,  VMThreadStackSize,       1024);
#else
define_pd_global(intx,  ThreadStackSize,         1024);
define_pd_global(intx,  VMThreadStackSize,       512);
#endif // _LP64
define_pd_global(intx,  SurvivorRatio,           8);
define_pd_global(intx,  CompilerThreadStackSize, 0);
define_pd_global(uintx, JVMInvokeMethodSlack,    8192);

define_pd_global(bool,  UseVectoredExceptions,   false);
// Only used on 64 bit platforms
define_pd_global(uintx, HeapBaseMinAddress,      2*G);
