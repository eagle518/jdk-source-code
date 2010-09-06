#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)phase.hpp	1.44 03/12/23 16:42:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
    Copy_Elimination,           // Copy Elimination
    Dead_Code_Elimination,      // DCE and compress Nodes
    Conditional_Constant,       // Conditional Constant Propagation
    CFG,                        // Build a CFG
    DefUse,                     // Build Def->Use chains
    Register_Allocation,        // Register allocation, duh
    LIVE,                       // Dragon-book LIVE range problem
    Interference_Graph,         // Building the IFG
    Coalesce,                   // Coalescing copies
    Conditional_CProp,          // Conditional Constant Propagation
    Ideal_Loop,                 // Find idealized trip-counted loops
    Peephole,                   // Apply peephole optimizations
    last_phase
  };
protected:
  static const char * const names[last_phase];
  enum PhaseNumber _pnum;       // Phase number (for stat gathering)

  // accumulated timers
  static elapsedTimer _t_totalCompilation;
  static elapsedTimer _t_methodCompilation;
  static elapsedTimer _t_nativeCompilation;
  static elapsedTimer _t_stubCompilation;
  static elapsedTimer _t_adapterCompilation;

  static elapsedTimer _t_parser;
  static elapsedTimer _t_optimizer;
  static elapsedTimer   _t_iterGVN;
  static elapsedTimer   _t_idealLoop;
  static elapsedTimer   _t_ccp;
  static elapsedTimer   _t_iterGVN2;
  static elapsedTimer _t_graphReshaping;
  static elapsedTimer _t_matcher;
  static elapsedTimer _t_scheduler;
  static elapsedTimer _t_registerAllocation;
  static elapsedTimer   _t_ctorChaitin;
  static elapsedTimer   _t_buildIFGphysical;
  static elapsedTimer   _t_computeLive;
  static elapsedTimer   _t_regAllocSplit;
  static elapsedTimer   _t_postAllocCopyRemoval;
  static elapsedTimer   _t_fixupSpills;
  static elapsedTimer _t_removeEmptyBlocks;
  static elapsedTimer _t_peephole;
  static elapsedTimer _t_output;
  static elapsedTimer   _t_instrSched;
  static elapsedTimer   _t_buildOopMaps;
  static elapsedTimer _t_codeGeneration;
  static elapsedTimer _t_registerMethod;
  static elapsedTimer _t_temporaryTimer1;
  static elapsedTimer _t_temporaryTimer2;
  static int _total_bytes_compiled;
  static int _total_nmethods_size;
  static int _total_code_size;
public:
  Compile * C;
  Phase( PhaseNumber pnum );
  static void add_code_size(nmethod* nm);
#ifndef PRODUCT
  static void print_timers();
#endif
};

