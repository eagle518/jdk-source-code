#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_core.hpp	1.23 04/03/03 17:04:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for the core platform otherwise set in {c1,c2}_globals{,_<arch>}.hpp
// (see globals.hpp)
//

define_pd_global(bool, BackgroundCompilation,        false);
define_pd_global(bool, UseTLAB,                      false);
define_pd_global(bool, CICompileOSR,                 false);
define_pd_global(bool, UseTypeProfile,               false); 
define_pd_global(bool, UseOnStackReplacement,        false);
define_pd_global(bool, InlineIntrinsics,             false);
define_pd_global(bool, PreferInterpreterNativeStubs, true);
define_pd_global(bool, ProfileInterpreter,           false);
define_pd_global(bool, ProfileTraps,                 false);
define_pd_global(bool, TieredCompilation,            false);
define_pd_global(intx, CompileThreshold,	     0);
define_pd_global(intx, CompileThresholdMin,	     0);
define_pd_global(intx, CompileThresholdMax,	     0);
define_pd_global(intx, Tier2CompileThreshold,        15000);
define_pd_global(intx, OnStackReplacePercentage,     0);
define_pd_global(bool, ResizeTLAB,		     false);
define_pd_global(intx, FreqInlineSize,		     0);
define_pd_global(intx, NewSizeThreadIncrease,	     4*K);
define_pd_global(intx, NewRatio,	     	     4);
define_pd_global(intx, InlineClassNatives,           true);
define_pd_global(intx, InlineUnsafeOps,              true);
define_pd_global(intx, InitialCodeCacheSize,         160*K);
define_pd_global(intx, ReservedCodeCacheSize,        32*M);
define_pd_global(intx, CodeCacheExpansionSize,       32*K);
define_pd_global(intx, CodeCacheMinBlockLength,      1);
define_pd_global(uintx,PermSize,    ScaleForWordSize(4*M));
define_pd_global(uintx,MaxPermSize, ScaleForWordSize(64*M));
define_pd_global(bool, NeverActAsServerClassMachine, true);
