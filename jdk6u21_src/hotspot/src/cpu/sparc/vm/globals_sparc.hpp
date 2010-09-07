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

// For sparc we do not do call backs when a thread is in the interpreter, because the
// interpreter dispatch needs at least two instructions - first to load the dispatch address
// in a register, and second to jmp. The swapping of the dispatch table may occur _after_
// the load of the dispatch address and hence the jmp would still go to the location
// according to the prior table. So, we let the thread continue and let it block by itself.
define_pd_global(bool, DontYieldALot,               true);  // yield no more than 100 times per second
define_pd_global(bool, ConvertSleepToYield,         false); // do not convert sleep(0) to yield. Helps GUI
define_pd_global(bool, ShareVtableStubs,            false); // improves performance markedly for mtrt and compress
define_pd_global(bool, CountInterpCalls,            false); // not implemented in the interpreter
define_pd_global(bool, NeedsDeoptSuspend,           true); // register window machines need this

define_pd_global(bool, ImplicitNullChecks,          true);  // Generate code for implicit null checks
define_pd_global(bool, UncommonNullCast,            true);  // Uncommon-trap NULLs past to check cast

define_pd_global(intx, CodeEntryAlignment,    32);
define_pd_global(intx, InlineFrequencyCount,  50);  // we can use more inlining on the SPARC
define_pd_global(intx, InlineSmallCode,       1500);
#ifdef _LP64
// Stack slots are 2X larger in LP64 than in the 32 bit VM.
define_pd_global(intx, ThreadStackSize,       1024);
define_pd_global(intx, VMThreadStackSize,     1024);
#else
define_pd_global(intx, ThreadStackSize,       512);
define_pd_global(intx, VMThreadStackSize,     512);
#endif

define_pd_global(intx, StackYellowPages, 2);
define_pd_global(intx, StackRedPages, 1);
define_pd_global(intx, StackShadowPages, 3 DEBUG_ONLY(+1));

define_pd_global(intx, PreInflateSpin,       40);  // Determined by running design center

define_pd_global(bool, RewriteBytecodes,     true);
define_pd_global(bool, RewriteFrequentPairs, true);
