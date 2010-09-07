/*
 * Copyright (c) 2000, 2007, Oracle and/or its affiliates. All rights reserved.
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

// Sets the default values for platform dependent flags used by the client compiler.
// (see c1_globals.hpp)

#ifndef TIERED
define_pd_global(bool, BackgroundCompilation,        true );
define_pd_global(bool, CICompileOSR,                 true );
define_pd_global(bool, InlineIntrinsics,             true );
define_pd_global(bool, PreferInterpreterNativeStubs, false);
define_pd_global(bool, ProfileTraps,                 false);
define_pd_global(bool, UseOnStackReplacement,        true );
define_pd_global(bool, TieredCompilation,            false);
define_pd_global(intx, CompileThreshold,             1000 ); // Design center runs on 1.3.1
define_pd_global(intx, Tier2CompileThreshold,        1500 );
define_pd_global(intx, Tier3CompileThreshold,        2000 );
define_pd_global(intx, Tier4CompileThreshold,        2500 );

define_pd_global(intx, BackEdgeThreshold,            100000);
define_pd_global(intx, Tier2BackEdgeThreshold,       100000);
define_pd_global(intx, Tier3BackEdgeThreshold,       100000);
define_pd_global(intx, Tier4BackEdgeThreshold,       100000);

define_pd_global(intx, OnStackReplacePercentage,     1400 );
define_pd_global(bool, UseTLAB,                      true );
define_pd_global(bool, ProfileInterpreter,           false);
define_pd_global(intx, FreqInlineSize,               325  );
define_pd_global(bool, ResizeTLAB,                   true );
define_pd_global(intx, ReservedCodeCacheSize,        32*M );
define_pd_global(intx, CodeCacheExpansionSize,       32*K );
define_pd_global(uintx,CodeCacheMinBlockLength,      1);
define_pd_global(uintx,PermSize,                     12*M );
define_pd_global(uintx,MaxPermSize,                  64*M );
define_pd_global(bool, NeverActAsServerClassMachine, true );
define_pd_global(intx, NewSizeThreadIncrease,        16*K );
define_pd_global(uint64_t,MaxRAM,                    1ULL*G);
define_pd_global(intx, InitialCodeCacheSize,         160*K);
#endif // !TIERED

define_pd_global(bool, UseTypeProfile,               false);
define_pd_global(bool, RoundFPResults,               false);

define_pd_global(bool, LIRFillDelaySlots,            true );
define_pd_global(bool, OptimizeSinglePrecision,      false);
define_pd_global(bool, CSEArrayLength,               true );
define_pd_global(bool, TwoOperandLIRForm,            false);

define_pd_global(intx, SafepointPollOffset,          0    );
