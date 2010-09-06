#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c2_globals_i486.hpp	1.41 04/03/03 13:59:08 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the server compiler.
// (see c2_globals.hpp).  Alpha-sorted.

define_pd_global(bool, BackgroundCompilation,        true);
define_pd_global(bool, UseTLAB,                      true);
define_pd_global(bool, ResizeTLAB,		     true);
define_pd_global(bool, CICompileOSR,                 true);
define_pd_global(bool, InlineIntrinsics,             true);
define_pd_global(bool, PreferInterpreterNativeStubs, true);
define_pd_global(bool, ProfileInterpreter,           true);
define_pd_global(bool, ProfileTraps,                 true);
define_pd_global(bool, UseOnStackReplacement,        true);
define_pd_global(bool, TieredCompilation,            false);
define_pd_global(intx, CompileThreshold,             10000);
define_pd_global(intx, CompileThresholdMax,          100000);
define_pd_global(intx, CompileThresholdMin,          2000);
define_pd_global(intx, Tier2CompileThreshold,        15000);
define_pd_global(intx, OnStackReplacePercentage,     140);
define_pd_global(intx, ConditionalMoveLimit,         3);
define_pd_global(intx, FLOATPRESSURE,                6);
define_pd_global(intx, FreqInlineSize,		     325);
define_pd_global(intx, INTPRESSURE,                  6);
define_pd_global(intx, InteriorEntryAlignment,       4);
define_pd_global(intx, NewRatio,                     8); // Design center runs on 1.3.1
define_pd_global(intx, NewSizeThreadIncrease,        4*K);
define_pd_global(intx, OptoLoopAlignment,            16);
define_pd_global(intx, RegisterCostAreaRatio,        16000);
define_pd_global(intx, AllocatePrefetchStyle,        0); // Turned off on Intel
define_pd_global(intx, AllocatePrefetchDistance,     64);
define_pd_global(intx, UseSSE,                       2);

// Peephole and CISC spilling both break the graph, and so makes the
// scheduler sick.
define_pd_global(bool, OptoPeephole,                 true);
define_pd_global(bool, UseCISCSpill,                 true);
define_pd_global(bool, OptoScheduling,               false);
define_pd_global(bool, OptoBundling,                 false);

define_pd_global(intx, InitialCodeCacheSize,         160*K);
define_pd_global(intx, ReservedCodeCacheSize,        32*M);
define_pd_global(intx, CodeCacheExpansionSize,       32*K);
define_pd_global(uintx,CodeCacheMinBlockLength,      4);
define_pd_global(intx, LoopUnrollLimit,		     50); // Design center runs on 1.3.1
define_pd_global(uintx, PermSize,		     16*M );
define_pd_global(uintx, MaxPermSize,		     64*M );

define_pd_global(bool, NeverActAsServerClassMachine, false);
