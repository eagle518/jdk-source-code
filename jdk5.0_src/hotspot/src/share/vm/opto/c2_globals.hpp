#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c2_globals.hpp	1.57 04/06/15 12:57:18 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Defines all globals flags used by the server compiler.
//

#define C2_FLAGS(develop, develop_pd, product, product_pd)                  \
                                                                            \
  develop(intx, CompileZapFirst, 0,                                         \
          "If +ZapDeadCompiledLocals, "                                     \
          "skip this many before compiling in zap calls")                   \
                                                                            \
  develop(intx, CompileZapLast, -1,                                         \
          "If +ZapDeadCompiledLocals, "                                     \
          "compile this many after skipping (incl. skip count, -1 = all)")  \
                                                                            \
  develop(intx, ZapDeadCompiledLocalsFirst, 0,                              \
          "If +ZapDeadCompiledLocals, "                                     \
          "skip this many before really doing it")                          \
                                                                            \
  develop(intx, ZapDeadCompiledLocalsLast, -1,                              \
          "If +ZapDeadCompiledLocals, "                                     \
          "do this many after skipping (incl. skip count, -1 = all)")       \
                                                                            \
  develop(intx, OptoPrologueNops, 0,                                        \
          "Insert this many extra nop instructions "                        \
          "in the prologue of every nmethod")                               \
                                                                            \
  develop_pd(intx, InteriorEntryAlignment,                                  \
          "Code alignment for interior entry points "                       \
          "in generated code (in bytes)")                                   \
                                                                            \
  develop_pd(intx, OptoLoopAlignment,                                       \
          "Align inner loops to zero relative to this modulus")             \
                                                                            \
  develop(intx, IndexSetWatch, 0,                                           \
          "Trace all operations on this IndexSet (-1 means all, 0 none)")   \
                                                                            \
  develop(intx, OptoNodeListSize, 4,                                        \
          "Starting allocation size of Node_List data structures")          \
                                                                            \
  develop(intx, OptoBlockListSize, 8,                                       \
          "Starting allocation size of Block_List data structures")         \
                                                                            \
  develop(intx, OptoPeepholeAt, -1,                                         \
          "Apply peephole optimizations to this peephole rule")             \
                                                                            \
  develop(bool, PrintOpto, false,                                           \
          "Print compiler2 attempts")                                       \
                                                                            \
  develop(bool, PrintOptoInlining, false,                                   \
          "Print compiler2 inlining decisions")                             \
                                                                            \
  develop(bool, VerifyOpto, false,                                          \
          "Apply more time consuming verification during compilation")      \
                                                                            \
  develop(bool, VerifyOptoOopOffsets, false,                                \
          "Check types of base addresses in field references")              \
                                                                            \
  develop(bool, IdealizedNumerics, false,                                   \
          "Check performance difference allowing FP "                       \
          "associativity and commutativity...")                             \
                                                                            \
  develop(bool, OptoBreakpoint, false,                                      \
          "insert breakpoint at method entry")                              \
                                                                            \
  develop(bool, OptoBreakpointOSR, false,                                   \
          "insert breakpoint at osr method entry")                          \
                                                                            \
  develop(bool, OptoBreakpointC2I, false,                                   \
          "insert breakpoint at C2IAdaptor entry")                          \
                                                                            \
  develop(bool, OptoBreakpointI2C, false,                                   \
          "insert breakpoint at I2CAdaptor entry")                          \
                                                                            \
  develop(intx, BreakAtNode, 0,                                             \
          "Break at construction of this Node (either _idx or _debug_idx)") \
                                                                            \
  develop(bool, OptoBreakpointC2N, false,                                   \
          "insert breakpoint at native stub entry")                         \
                                                                            \
  develop(bool, OptoBreakpointC2R, false,                                   \
          "insert breakpoint at runtime stub entry")                        \
                                                                            \
  develop(bool, OptoNoExecute, false,                                       \
          "Attempt to parse and compile but do not execute generated code") \
                                                                            \
  develop(bool, PrintOptoStatistics, false,                                 \
          "Print New compiler statistics")                                  \
                                                                            \
  develop(bool, PrintOptoAssembly, false,                                   \
          "Print New compiler assembly output")                             \
                                                                            \
  develop(bool, OptoRuntimeCalleeSavedFloats, false,                        \
          "Runtime saves/restores callee-saved floats or doubles")          \
                                                                            \
  develop_pd(bool, OptoPeephole,                                            \
          "Apply peephole optimizations after register allocation")         \
                                                                            \
  develop(bool, OptoRemoveUseless, true,                                    \
          "Remove useless nodes after parsing")                             \
                                                                            \
  develop(bool, PrintFrameConverterAssembly, false,                         \
          "Print New compiler assembly output for frame converters")        \
                                                                            \
  develop(bool, PrintOptoParseInfo, false,                                  \
          "Print New compiler parse info per method")                       \
                                                                            \
  develop(bool, PrintParseStatistics, false,                                \
          "Print nodes, transforms and new values made per bytecode parsed")\
                                                                            \
  develop(bool, PrintOptoPeephole, false,                                   \
          "Print New compiler peephole replacements")                       \
                                                                            \
  develop(bool, TraceOptoParse, false,                                      \
          "Trace bytecode parse and control-flow merge")                    \
                                                                            \
  develop(bool, OptoNoGC, false,                                            \
          "Skip GC for new compiler")                                       \
                                                                            \
  product_pd(intx,  LoopUnrollLimit,                                        \
          "Unroll loop bodies with node count less than this")              \
                                                                            \
  develop(bool, OptoCoalesce, true,                                         \
          "Use Conservative Copy Coalescing in the Register Allocator")     \
                                                                            \
  develop(bool, UseUniqueSubclasses, true,                                  \
          "Narrow an abstract reference to the unique concrete subclass")   \
                                                                            \
  develop(bool, UseExactTypes, true,                                        \
          "Use exact types to eliminate array store checks and v-calls")    \
                                                                            \
  develop(bool, OptoReorgOffsets, false,                                    \
          "Reorganize offsets to lower register pressure")                  \
                                                                            \
  develop_pd(intx, RegisterCostAreaRatio,                                   \
          "Spill selection in reg allocator: scale area by (X/64K) before " \
          "adding cost")                                                    \
                                                                            \
  develop_pd(bool, UseCISCSpill,                                            \
          "Use ADLC supplied cisc instructions during allocation")          \
                                                                            \
  develop(bool, VerifyGraphEdges , false,                                   \
          "Verify Bi-directional Edges")                                    \
                                                                            \
  develop(bool, VerifyDUIterators, false,                                   \
          "Verify the safety of all iterations of Bi-directional Edges")    \
                                                                            \
  develop(bool, VerifyHashTableKeys, true,                                  \
          "Verify the immutability of keys in the VN hash tables")          \
                                                                            \
  develop_pd(intx, FLOATPRESSURE,                                           \
          "Number of float LRG's that constitute high register pressure")   \
                                                                            \
  develop_pd(intx, INTPRESSURE,                                             \
          "Number of integer LRG's that constitute high register pressure") \
                                                                            \
  develop(bool, TraceOptoPipelining, false,                                 \
          "Trace pipelining information")                                   \
                                                                            \
  develop(bool, TraceOptoOutput, false,                                     \
          "Trace pipelining information")                                   \
                                                                            \
  product_pd(bool, OptoScheduling,                                          \
          "Instruction Scheduling after register allocation")               \
                                                                            \
  product_pd(bool, OptoBundling,                                            \
          "Generate nops to fill i-cache lines")                            \
                                                                            \
  product_pd(intx, ConditionalMoveLimit,                                    \
          "Limit of ops to make speculative when using CMOVE")              \
                                                                            \
  /* Set BranchOnRegister == false. See 4965987. */                         \
  product(bool, BranchOnRegister, false,                                    \
          "Use Sparc V9 branch-on-register opcodes")                        \
                                                                            \
  develop(bool, SparcV9RegsHiBitsZero, true,                                \
          "Assume Sparc V9 I&L registers on V8+ systems are zero-extended") \
                                                                            \
  product_pd(intx, UseSSE,                                                  \
          "0=fpu stack,1=SSE for floats,2=SSE/SSE2 for all")                \
                                                                            \
  product_pd(intx,  AllocatePrefetchStyle,                                  \
          "0=no prefetch, 1=dead load, 2=prefetch instruction")             \
                                                                            \
  product_pd(intx,  AllocatePrefetchDistance,                               \
          "Distance to prefetch ahead of allocation pointer")               \
                                                                            \
  product(intx, CompilationPolicyChoice, 1,                                 \
          "which compilation policy")                                       \
                                                                            \
  product(bool, UseOldInlining, true,                                       \
          "Enable the 1.3 inlining strategy")                               \
                                                                            \
  /* controls for tier 1 compilations */                                    \
                                                                            \
  develop(bool, Tier1UpdateMethodData, true,                                \
          "Generate code, during tier 1, to update the methodDataOop")      \
                                                                            \
  develop(bool, Tier1CountInvocations, true,                                \
          "Generate code, during tier 1, to update invocation counter")     \
                                                                            \
  product(intx, Tier1Inline, false,                                         \
          "enable inlining during tier 1")                                  \
                                                                            \
  product(intx, Tier1MaxInlineSize, 8,                                      \
          "maximum bytecode size of a method to be inlined, during tier 1") \
                                                                            \
  product(intx, Tier1FreqInlineSize, 35,                                    \
          "max bytecode size of a frequent method to be inlined, tier 1")   \
                                                                            \
  develop(intx, ImplicitNullCheckThreshold, 3,                              \
          "Don't do implicit null checks if NPE's in a method exceeds limit") \
                                                                            \
 /* controls for loop optimization */                                       \
  product(intx, Tier1LoopOptsCount, 0,                                      \
          "Set level of loop optimization for tier 1 compiles")             \
                                                                            \
  product(intx, LoopOptsCount, 43,                                          \
          "Set level of loop optimization for tier 1 compiles")             \
                                                                            \
  /* controls for heat-based inlining */                                    \
                                                                            \
  develop(intx, NodeCountInliningCutoff, 18000,                             \
          "If parser node generation exceeds limit stop inlining")          \
                                                                            \
  develop(intx, NodeCountInliningStep, 1000,                                \
          "Target size of warm calls inlined between optimization passes")  \
                                                                            \
  develop(bool, InlineWarmCalls, false,                                     \
          "Use a heat-based priority queue to govern inlining")             \
                                                                            \
  develop(intx, HotCallCountThreshold, 999999,                              \
          "large numbers of calls (per method invocation) force hotness")   \
                                                                            \
  develop(intx, HotCallProfitThreshold, 999999,                             \
          "highly profitable inlining opportunities force hotness")         \
                                                                            \
  develop(intx, HotCallTrivialWork, -1,                                     \
          "trivial execution time (no larger than this) forces hotness")    \
                                                                            \
  develop(intx, HotCallTrivialSize, -1,                                     \
          "trivial methods (no larger than this) force calls to be hot")    \
                                                                            \
  develop(intx, WarmCallMinCount, -1,                                       \
          "number of calls (per method invocation) to enable inlining")     \
                                                                            \
  develop(intx, WarmCallMinProfit, -1,                                      \
          "number of calls (per method invocation) to enable inlining")     \
                                                                            \
  develop(intx, WarmCallMaxWork, 999999,                                    \
          "execution time of the largest inlinable method")                 \
                                                                            \
  develop(intx, WarmCallMaxSize, 999999,                                    \
          "size of the largest inlinable method")                           \
                                                                            \
  develop(bool, EagerInitialization, false,                                 \
          "Eagerly initialize classes if possible")                         \
                                                                            \
  product(intx, CICompilerCount, 2,                                         \
          "Number of compiler threads to run")                              \
                                                                            \
  product(intx, MaxNodeLimit, 65000,                                        \
          "Maximum number of nodes")                                        \
                                                                            \
  product(intx, NodeLimitFudgeFactor, 1000,                                 \
          "Fudge Factor for certain optimizations")                         \
                                                                            \
  product(bool, UseJumpTables, false,                                       \
          "Use JumpTables instead of a binary search tree for switches")    \
                                                                            \
  product(intx, MinJumpTableSize, 8,                                        \
          "Minimum number of targets in a generated jump table")            \
                                                                            \
  product(intx, MaxJumpTableSize, 65000,                                    \
          "Maximum number of targets in a generated jump table")            \
                                                                            \
  product(intx, MaxJumpTableSparseness, 5,                                  \
          "Maximum sparseness for jumptables")                              \

C2_FLAGS(DECLARE_DEVELOPER_FLAG, DECLARE_PD_DEVELOPER_FLAG, DECLARE_PRODUCT_FLAG, DECLARE_PD_PRODUCT_FLAG)
