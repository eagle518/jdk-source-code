/*
 * Copyright (c) 1997, 2009, Oracle and/or its affiliates. All rights reserved.
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

class Compile;

//------------------------------Phase------------------------------------------
// Most optimizations are done in Phases.  Creating a phase does any long
// running analysis required, and caches the analysis in internal data
// structures.  Later the analysis is queried using transform() calls to
// guide transforming the program.  When the Phase is deleted, so is any
// cached analysis info.  This basic Phase class mostly contains timing and
// memory management code.
class Phase : public StackObj {
public:
  enum PhaseNumber {
    Compiler,                   // Top-level compiler phase
    Parser,                     // Parse bytecodes
    Remove_Useless,             // Remove useless nodes
    Optimistic,                 // Optimistic analysis phase
    GVN,                        // Pessimistic global value numbering phase
    Ins_Select,                 // Instruction selection phase
    CFG,                        // Build a CFG
    BlockLayout,                // Linear ordering of blocks
    Register_Allocation,        // Register allocation, duh
    LIVE,                       // Dragon-book LIVE range problem
    StringOpts,                 // StringBuilder related optimizations
    Interference_Graph,         // Building the IFG
    Coalesce,                   // Coalescing copies
    Ideal_Loop,                 // Find idealized trip-counted loops
    Macro_Expand,               // Expand macro nodes
    Peephole,                   // Apply peephole optimizations
    last_phase
  };
protected:
  enum PhaseNumber _pnum;       // Phase number (for stat gathering)

#ifndef PRODUCT
  static int _total_bytes_compiled;

  // accumulated timers
  static elapsedTimer _t_totalCompilation;
  static elapsedTimer _t_methodCompilation;
  static elapsedTimer _t_stubCompilation;
#endif

// The next timers used for LogCompilation
  static elapsedTimer _t_parser;
  static elapsedTimer _t_escapeAnalysis;
  static elapsedTimer _t_optimizer;
  static elapsedTimer   _t_idealLoop;
  static elapsedTimer   _t_ccp;
  static elapsedTimer _t_matcher;
  static elapsedTimer _t_registerAllocation;
  static elapsedTimer _t_output;

#ifndef PRODUCT
  static elapsedTimer _t_graphReshaping;
  static elapsedTimer _t_scheduler;
  static elapsedTimer _t_blockOrdering;
  static elapsedTimer _t_macroExpand;
  static elapsedTimer _t_peephole;
  static elapsedTimer _t_codeGeneration;
  static elapsedTimer _t_registerMethod;
  static elapsedTimer _t_temporaryTimer1;
  static elapsedTimer _t_temporaryTimer2;
  static elapsedTimer _t_idealLoopVerify;

// Subtimers for _t_optimizer
  static elapsedTimer   _t_iterGVN;
  static elapsedTimer   _t_iterGVN2;

// Subtimers for _t_registerAllocation
  static elapsedTimer   _t_ctorChaitin;
  static elapsedTimer   _t_buildIFGphysical;
  static elapsedTimer   _t_computeLive;
  static elapsedTimer   _t_regAllocSplit;
  static elapsedTimer   _t_postAllocCopyRemoval;
  static elapsedTimer   _t_fixupSpills;

// Subtimers for _t_output
  static elapsedTimer   _t_instrSched;
  static elapsedTimer   _t_buildOopMaps;
#endif
public:
  Compile * C;
  Phase( PhaseNumber pnum );
#ifndef PRODUCT
  static void print_timers();
#endif
};
