#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_sparc.hpp	1.34 04/05/13 09:57:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
//

// For sparc we do not do call backs when a thread is in the interpreter, because the
// interpreter dispatch needs at least two instructions - first to load the dispatch address
// in a register, and second to jmp. The swapping of the dispatch table may occur _after_
// the load of the dispatch address and hence the jmp would still go to the location
// according to the prior table. So, we let the thread continue and let it block by itself.
define_pd_global(bool, UseCallBackInInterpreter,    false);
define_pd_global(bool, DontYieldALot,               true);  // yield no more than 100 times per second
define_pd_global(bool, ConvertSleepToYield,         false); // do not convert sleep(0) to yield. Helps GUI 
define_pd_global(bool, ShareVtableStubs,            false); // improves performance markedly for mtrt and compress
define_pd_global(bool, CountInterpCalls,            false); // not implemented in the interpreter

define_pd_global(bool, ImplicitNullChecks,          true);  // Generate code for implicit null checks
define_pd_global(bool, UncommonNullCast,            true);  // Uncommon-trap NULLs past to check cast

define_pd_global(intx,  CodeEntryAlignment,    32); 
define_pd_global(uintx, TLABSize,              0);
define_pd_global(uintx, NewSize, ScaleForWordSize((2048 * K) + (2 * (64 * K))));
define_pd_global(intx,  SurvivorRatio,         32);  // Design center runs on 1.3.1
define_pd_global(intx,  InlineFrequencyCount,  50);  // we can use more inlining on the SPARC
#ifdef _LP64
// Stack slots are 2X larger in LP64 than in the 32 bit VM.
define_pd_global(intx,  ThreadStackSize,       1024);
define_pd_global(intx,  VMThreadStackSize,     1024);
#else
define_pd_global(intx,  ThreadStackSize,       512); 
define_pd_global(intx,  VMThreadStackSize,     512); 
#endif

define_pd_global(intx, StackYellowPages, 2);
define_pd_global(intx, StackRedPages, 1);
define_pd_global(intx, StackShadowPages, 3 DEBUG_ONLY(+1));

define_pd_global(intx,  PreInflateSpin,        40);  // Determined by running design center

define_pd_global(intx, PrefetchCopyIntervalInBytes, -1);
define_pd_global(intx, PrefetchScanIntervalInBytes, -1);
define_pd_global(intx, PrefetchFieldsAhead,         -1);

define_pd_global(bool, SafepointPolling,     true);
define_pd_global(bool, RewriteBytecodes,     true);
define_pd_global(bool, RewriteFrequentPairs, true);
