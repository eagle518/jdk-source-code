#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_globals.hpp	1.88 04/05/05 16:15:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Defines all global flags used by the client compiler.
//

#define C1_FLAGS(develop, develop_pd, product, product_pd)                  \
                                                                            \
  /* Printing */                                                            \
  develop(bool, PrintC1Statistics, false,                                   \
          "Print Compiler1 statistics" )                                    \
                                                                            \
  develop(bool, PrintOopMap, false,                                         \
          "Print oop maps")                                                 \
                                                                            \
  develop(bool, PrintCFG, false,                                            \
          "Print control flow graph after each change")                     \
                                                                            \
  develop(bool, PrintCFG0, false,                                           \
          "Print control flow graph after construction")                    \
                                                                            \
  develop(bool, PrintCFG1, false,                                           \
          "Print control flow graph after optimizations")                   \
                                                                            \
  develop(bool, PrintCFG2, false,                                           \
          "Print control flow graph before code generation")                \
                                                                            \
  develop(bool, PrintIRDuringConstruction, false,                            \
          "Print IR as it's being constructed (helpful for debugging frontend)")\
                                                                            \
  develop(bool, PrintIR, false,                                             \
          "Print full intermediate representation after each change")       \
                                                                            \
  develop(bool, PrintIR0, false,                                            \
          "Print full intermediate representation after construction")      \
                                                                            \
  develop(bool, PrintIR1, false,                                            \
          "Print full intermediate representation after optimizations")     \
                                                                            \
  develop(bool, PrintIR2, false,                                            \
          "Print full intermediate representation before code generation")  \
                                                                            \
  develop(bool, PrintSimpleStubs, false,                                    \
          "Print SimpleStubs")                                              \
                                                                            \
  develop(bool, PrintC1RegAlloc, false,                                     \
          "Trace register allocation in C1")                                \
                                                                            \
  /* C1 optimizations */                                                    \
                                                                            \
  develop(bool, UseC1Optimizations, true,                                   \
          "Turn on C1 optimizations")                                       \
                                                                            \
  product(bool, LIRCacheLocals, true,                                       \
          "Cache locals in registers throughout the method using LIR")      \
                                                                            \
  develop(bool, TraceCachedLocals, false,                                   \
          "Trace cached local / register mapping")                          \
                                                                            \
  develop(bool, PrintCachedLocals, false,                                   \
          "Print cached local / register mapping")                          \
                                                                            \
  develop(bool, DoCEE, true,                                                \
          "Do Conditional Expression Elimination to simplify CFG")          \
                                                                            \
  develop(bool, PrintCEE, false,                                            \
          "Print Conditional Expression Elimination")                       \
                                                                            \
  develop_pd(bool, ComputeLoops,                                            \
          "Compute loop structures")                                        \
                                                                            \
  develop(bool, WantsLargerLoops, false,                                    \
          "Prefer outer loops instead of inner loops when possible")        \
                                                                            \
  develop(bool, PrintLoops, false,                                          \
          "Print loop structures")                                          \
                                                                            \
  develop(bool, UseValueNumbering, true,                                    \
          "Use Value Numbering")                                            \
                                                                            \
  develop(bool, PrintValueNumbering, false,                                 \
          "Print Value Numbering")                                          \
                                                                            \
  develop(bool, EliminateLoads, true,                                       \
          "Eliminate unneccessary loads")                                   \
                                                                            \
  develop_pd(bool, EliminateLoadsAcrossCalls,                               \
          "Eliminate unnecessary calls across calls")                       \
                                                                            \
  develop(bool, PrintLoadElimination, false,                                \
          "Print load elimination")                                         \
                                                                            \
  develop(bool, EliminateStores, true,                                      \
          "Eliminate unneccessary stores")                                  \
                                                                            \
  develop(bool, PrintStoreElimination, false,                               \
          "Print store elimination")                                        \
                                                                            \
  develop_pd(bool, EliminateFieldAccess,                                    \
          "Eliminate unneccessary field accesses")                          \
                                                                            \
  develop_pd(bool, EliminateArrayAccess,                                    \
          "Eliminate unneccessary array accesses")                          \
                                                                            \
  develop(bool, PrintAccessElimination, false,                              \
          "Print field and array access elimination")                       \
                                                                            \
  develop(bool, EliminateBlocks, true,                                      \
          "Eliminate unneccessary basic blocks")                            \
                                                                            \
  develop(bool, PrintBlockElimination, false,                               \
          "Print basic block elimination")                                  \
                                                                            \
  develop(bool, EliminateNullChecks, true,                                  \
          "Eliminate unneccessary null checks")                             \
                                                                            \
  develop(bool, PrintNullCheckElimination, false,                           \
          "Print null check elimination")                                   \
                                                                            \
  develop(bool, InlineMethodsWithExceptionHandlers, false,                  \
          "Inline methods containing exception handlers "                   \
          "(NOTE: does not work with current backend)")                     \
                                                                            \
  develop(bool, InlineNIOCheckIndex, true,                                  \
          "Intrinsify java.nio.Buffer.checkIndex")                          \
                                                                            \
  develop(bool, CanonicalizeNodes, true,                                    \
          "Canonicalize graph nodes")                                       \
                                                                            \
  develop(bool, CanonicalizeExperimental, false,                            \
          "Canonicalize graph nodes, experimental code")                    \
                                                                            \
  develop(bool, PrintCanonicalization, false,                               \
          "Print graph node canonicalization")                              \
                                                                            \
  develop_pd(bool, OptimizeLibraryCalls,                                    \
          "Special versions of rt.jar library routines")                    \
                                                                            \
  develop_pd(bool, UseTableRanges,                                          \
          "Faster versions of lookup table using ranges")                   \
                                                                            \
  develop(bool, UseFastExceptionHandling, true,                             \
          "Faster handling of exceptions")                                  \
                                                                            \
  develop_pd(bool, RoundFloatsWithStore,                                    \
          "Use (memory) store to round floating point results")             \
                                                                            \
  develop(intx, NestedInliningSizeRatio, 90,                                \
          "Percentage of prev. allowed inline size in recursive inlining")  \
                                                                            \
  develop(bool, PrintIRWithLIR, false,                                      \
          "Print IR instructions with generated LIR")                       \
                                                                            \
  develop(bool, PrintLIRWithAssembly, false,                                \
          "Show LIR instruction with generated assembly")                   \
                                                                            \
  develop(bool, LIRTracePeephole, false,                                    \
          "Trace peephole optimizer")                                       \
                                                                            \
  develop(bool, LIRTraceExecution, false,                                   \
          "add LIR code which logs the execution of blocks")                \
                                                                            \
  product_pd(bool, LIRFillDelaySlots,                                       \
             "fill delays on on SPARC with LIR")                            \
                                                                            \
  develop(intx, LIROptoStart, 0,                                            \
          "the id of the first peephole opt to permit")                     \
                                                                            \
  develop(intx, LIROptoStop, -1,                                            \
          "the id of the last peephole opt to permit")                      \
                                                                            \
  develop(intx, LIRCacheLoopStart, 0,                                       \
          "the id of the first loop to cache locals in")                    \
                                                                            \
  develop(intx, LIRCacheLoopStop, -1,                                       \
          "the id of the last loop to cache locals in")                     \
                                                                            \
  develop(intx, LIRLocalCachingMask, 0,                                     \
          "mask to be applied to locals which are cached")                  \
                                                                            \
  develop(bool, UseNewCodeGen, false,                                       \
          "Use the new code generator")                                     \
                                                                            \
  develop_pd(bool, CSEArrayLength,                                          \
          "Create separate nodes for length in array accesses")             \
                                                                            \
                                                                            \
  /* C1 variable */                                                         \
                                                                            \
  develop(bool, C1TraceMethod, false,                                       \
          "trace method entry and exit in compiled code")                   \
                                                                            \
  develop(bool, C1Breakpoint, false,                                        \
          "Sets a breakpoint at entry of each compiled method")             \
                                                                            \
  develop(bool, ImplicitDiv0Checks, true,                                   \
          "Use implicit division by zero checks")                           \
                                                                            \
  develop(bool, PinAllInstructions, false,                                  \
          "All instructions are pinned")                                    \
                                                                            \
  develop(bool, ValueStackPinStackAll, true,                                \
          "Pinning in ValueStack pin everything")                           \
                                                                            \
  develop(bool, UseFastNewInstance, true,                                   \
          "Use fast inlined instance allocation")                           \
                                                                            \
  develop(bool, UseFastNewTypeArray, true,                                  \
          "Use fast inlined type array allocation")                         \
                                                                            \
  develop(bool, UseFastNewObjectArray, true,                                \
          "Use fast inlined object array allocation")                       \
                                                                            \
  develop(bool, UseFastLocking, true,                                       \
          "Use fast inlined locking code")                                  \
                                                                            \
  product(bool, FastTLABRefill, true,                                       \
          "Use fast TLAB refill code")                                      \
                                                                            \
  develop(bool, UseSlowPath, false,                                         \
          "For debugging: test slow cases by always using them")            \
                                                                            \
  develop(bool, GenerateArrayStoreCheck, true,                              \
          "Generates code for array store checks")                          \
                                                                            \
  develop(bool, UseFPConstTables, true,                                     \
          "Use constant tables for fp constants")                           \
                                                                            \
  develop(bool, DeoptC1, true,                                              \
          "Use deoptimization in C1")                                       \
                                                                            \
  develop(bool, DeoptOnAsyncException, true,                                \
          "Deoptimize upon Thread.stop(); improves precision of IR")        \
                                                                            \
  develop(bool, LateBailout, true,                                          \
          "Bailout as late as possible (i.e., during code generation)")     \
                                                                            \
  develop(bool, PrintBailouts, false,                                       \
          "Print bailout and its reason")                                   \
                                                                            \
  develop(bool, TracePatching, false,                                       \
         "Trace patching of field access on uninitialized classes")         \
                                                                            \
  develop(bool, UsePatching,  true,                                         \
          "Use code instead of patching when "                              \
          "class is not loaded/initialized")                                \
                                                                            \
  develop(bool, PatchALot, false,                                           \
          "Marks all fields as having unloaded classes")                    \
                                                                            \
  develop_pd(bool, PrintNotLoaded,                                          \
          "Prints where classes are not loaded during code generation")     \
                                                                            \
  develop(bool, PrintSpilling, false,                                       \
          "Print all spills while compiling")                               \
                                                                            \
  develop(bool, C1InvalidateCachedOopLocation, false,                       \
          "Store a non-oop value after caching it into a register")         \
                                                                            \
  develop(bool, VerifyC1Code, false,                                        \
          "Adds verification code to the generated code")                   \
                                                                            \
  develop(bool, PrintLIR, false,                                            \
          "print low-level IR")                                             \
                                                                            \
  develop(bool, BailoutAfterHIR, false,                                     \
          "bailout of compilation after building of HIR")                   \
                                                                            \
  develop(bool, AlwaysEmitDebugInfo, false,                                 \
          "always emit debug info")                                         \
                                                                            \
  develop(bool, CloneBlocks, false,                                         \
          "clone blocks to make basic blocks larger")                       \
                                                                            \
  develop(bool, InstallMethods, true,                                       \
          "Install methods at the end of successful compilations")          \
                                                                            \
  product(intx, CompilationRepeat, 0,                                       \
          "Number of times to recompile method before returning result")    \
                                                                            \
  product(intx, CompilationPolicyChoice, 0,                                 \
          "which compilation policy")                                       \
                                                                            \
  develop_pd(bool, CacheCallFreeLoopsOnly,                                  \
          "whether to exclude loops with calls from caching of locals")     \
                                                                            \
  develop(intx, ValueMapMaxSize, 37,                                        \
          "Maximum number of buckets in ValueMap hashtable")                \
                                                                            \
  develop(intx, ValueMapBucketInitialSize, 3,                               \
          "Intial size of ValueMap Buckets")                                \
                                                                            \
  develop(intx, ValueMapBucketMaxSize, 21,                                  \
          "Maximum size of ValueMap Buckets")                               \
                                                                            \
  develop_pd(bool, CacheFloats,                                             \
            "Cache floats in FP registers")                                 \
                                                                            \
  develop_pd(bool, CacheDoubleWord,                                         \
            "Cache double word item in registers")                          \
                                                                            \
  develop(bool, LIROptimize, true,                                          \
          "Perform LIR optimizations (peephole, et al.)")                   \
                                                                            \
  develop_pd(bool, LIROptimizeDeleteOps,                                    \
          "Delete move operations which appears to be unused")              \
                                                                            \
  develop_pd(bool, LIROptimizeFloats,                                       \
          "Perform LIR optimizations for float registers")                  \
                                                                            \
  develop_pd(bool, OptimizeSinglePrecision,                                 \
          "Eliminate single precision rounding")                            \
                                                                            \
  develop(bool, TraceFPUStack, false,                                       \
          "Trace emulation of the FPU stack (intel only)")                  \
                                                                            \
  develop(bool, ForceStackAlignment, true,                                  \
          "Force the stacks of compiled frames to be aligned")              \
                                                                            \
  develop(bool, OptimizeUnsafes, true,                                      \
          "Optimize raw unsafe ops")                                        \
                                                                            \
  develop(bool, PrintUnsafeOptimization, false,                             \
          "Print optimization of raw unsafe ops")                           \
                                                                            \
  develop(bool, EagerInitialization, false,                                 \
          "Eagerly initialize classes if possible")                         \
                                                                            \
  develop(bool, LIROopMaps, false,                                          \
          "Generate oop maps for locals from LIR instead of using CI")      \
                                                                            \
  develop(bool, VerifyLIROopMaps, false,                                    \
          "Verify oop maps for locals generated from LIR")                  \
                                                                            \
  develop(bool, TraceLIROopMaps, false,                                     \
          "Trace generation of LIR oop maps")                               \
                                                                            \
  develop(intx, InstructionCountCutoff, 37000,                              \
          "If GraphBuilder adds this many instructions, bails out")         \
                                                                            \
  product(intx, CICompilerCount, 1,                                         \
          "Number of compiler threads to run")         			    \
                                                                            \
  product_pd(intx, SafepointPollOffset,                                     \
          "Offset added to polling address (Intel only)")                   \
                                                                            \
  product(bool, UseNewFeature1, false,                                      \
          "Enable new feature for testing.  This is a dummy flag.")         \
                                                                            \
  product(bool, UseNewFeature2, false,                                      \
          "Enable new feature for testing.  This is a dummy flag.")         \
                                                                            \
  product(bool, UseNewFeature3, false,                                      \
          "Enable new feature for testing.  This is a dummy flag.")         \
                                                                            \
  product(bool, UseNewFeature4, false,                                      \
          "Enable new feature for testing.  This is a dummy flag.")         \
                                                                            \

// Read default values for c1 globals
// #include "incls/_c1_globals_pd.hpp.incl"

C1_FLAGS(DECLARE_DEVELOPER_FLAG, DECLARE_PD_DEVELOPER_FLAG, DECLARE_PRODUCT_FLAG, DECLARE_PD_PRODUCT_FLAG)
