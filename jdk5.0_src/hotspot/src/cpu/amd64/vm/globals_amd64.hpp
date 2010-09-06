#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_amd64.hpp	1.10 04/05/13 09:57:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the
// runtime system.  (see globals.hpp)
//

define_pd_global(bool,  UseCallBackInInterpreter, true);
define_pd_global(bool,  ConvertSleepToYield,      true);
define_pd_global(bool,  ShareVtableStubs,         true);
define_pd_global(bool,  CountInterpCalls,         true);

// Generate code for implicit null checks
define_pd_global(bool,  ImplicitNullChecks,       true);

// Uncommon-trap NULLs past to check cast
define_pd_global(bool,  UncommonNullCast,         true);

define_pd_global(intx,  CodeEntryAlignment,       32); 
define_pd_global(uintx, TLABSize,                 0); 
define_pd_global(uintx, NewSize, ScaleForWordSize(2048 * K));
define_pd_global(intx,  InlineFrequencyCount,     100);
define_pd_global(intx,  PreInflateSpin,		  10);

// XXX benchmark this with various values, supporting code is ready
define_pd_global(intx, PrefetchCopyIntervalInBytes, -1);
define_pd_global(intx, PrefetchScanIntervalInBytes, -1);
define_pd_global(intx, PrefetchFieldsAhead,         -1);

define_pd_global(intx, StackYellowPages, 2);
define_pd_global(intx, StackRedPages, 1);
define_pd_global(intx, StackShadowPages, 6 DEBUG_ONLY(+2));

define_pd_global(bool, SafepointPolling,     true);
define_pd_global(bool, RewriteBytecodes,     true);
define_pd_global(bool, RewriteFrequentPairs, true);

define_pd_global(bool, NeverActAsServerClassMachine, false);

