#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c2_globals_amd64.hpp	1.14 04/06/04 20:02:49 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Sets the default values for platform dependent flags used by the
// server compiler.  (see c2_globals.hpp).

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
define_pd_global(intx, FLOATPRESSURE,                15);
define_pd_global(intx, FreqInlineSize,		     325);
define_pd_global(intx, INTPRESSURE,                  13);
define_pd_global(intx, InteriorEntryAlignment,       16);
define_pd_global(intx, NewRatio,                     2);
define_pd_global(intx, NewSizeThreadIncrease, ScaleForWordSize(4*K));
define_pd_global(intx, OptoLoopAlignment,            16);
define_pd_global(intx, RegisterCostAreaRatio,        16000);

define_pd_global(intx, AllocatePrefetchStyle,        2);
define_pd_global(intx, AllocatePrefetchDistance,     256); // 4 dcache lines

define_pd_global(intx, UseSSE,                       2);

// Peephole and CISC spilling both break the graph, and so makes the
// scheduler sick.
define_pd_global(bool, OptoPeephole,                 true);
define_pd_global(bool, UseCISCSpill,                 true);
define_pd_global(bool, OptoScheduling,               false);
define_pd_global(bool, OptoBundling,                 false);

// We need to make sure that all generated code is within 2G of the
// libjvm.so runtime routines so we can use the faster CALL rel32off
// insn rather than the expensive sequence of isns to load a 64 bit
// pointer.  Note that CALL rel32off insns must be patched for
// safepoints
define_pd_global(intx, InitialCodeCacheSize,         1024*K);
define_pd_global(intx, ReservedCodeCacheSize,        1024*M);
define_pd_global(intx, CodeCacheExpansionSize,       64*K);
define_pd_global(uintx,CodeCacheMinBlockLength,      4);
define_pd_global(intx, LoopUnrollLimit,		     60);
define_pd_global(uintx, PermSize, ScaleForWordSize(16*M));
define_pd_global(uintx, MaxPermSize, ScaleForWordSize(64*M));
