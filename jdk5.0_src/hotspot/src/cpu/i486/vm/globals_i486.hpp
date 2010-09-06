#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_i486.hpp	1.32 04/05/13 09:57:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
//

define_pd_global(bool,  UseCallBackInInterpreter, true);
define_pd_global(bool,  ConvertSleepToYield,      true);
define_pd_global(bool,  ShareVtableStubs,         true);
define_pd_global(bool,  CountInterpCalls,         true);

define_pd_global(bool, ImplicitNullChecks,          true);  // Generate code for implicit null checks
define_pd_global(bool, UncommonNullCast,            true);  // Uncommon-trap NULLs past to check cast

// See 4827828 for this change. There is no globals_core_i486.hpp. I can't
// assign a different value for C2 without touching a number of files. Use 
// #ifdef to minimize the change as it's late in Mantis. -- FIXME.
#ifdef COMPILER2
define_pd_global(intx,  CodeEntryAlignment,       32); 
#else
define_pd_global(intx,  CodeEntryAlignment,       16); 
#endif

define_pd_global(uintx, TLABSize,                 0); 
define_pd_global(uintx, NewSize,                  640 * K);
define_pd_global(intx,  InlineFrequencyCount,     100);
define_pd_global(intx,  PreInflateSpin,		  10);

define_pd_global(intx, PrefetchCopyIntervalInBytes, -1);
define_pd_global(intx, PrefetchScanIntervalInBytes, -1);
define_pd_global(intx, PrefetchFieldsAhead,         -1);

define_pd_global(intx, StackYellowPages, 2);
define_pd_global(intx, StackRedPages, 1);
define_pd_global(intx, StackShadowPages, 3 DEBUG_ONLY(+1));

define_pd_global(bool, SafepointPolling,     true);
define_pd_global(bool, RewriteBytecodes,     true);
define_pd_global(bool, RewriteFrequentPairs, true);
