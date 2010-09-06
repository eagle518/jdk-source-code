#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)sweeper.hpp	1.19 03/12/23 16:44:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// An NmethodSweeper is an incremental cleaner for:
//    - cleanup inline caches
//    - reclamation of unreferences zombie nmethods
//

class NMethodSweeper : public AllStatic {  
  static long      _traversals;   // Stack traversal count
  static CodeBlob* _current;      // Current nmethod
  static int       _seen;         // Nof. nmethod we have currently processed in current pass of CodeCache
  static int       _invocations;  // No. of invocations left until we are completed with this pass

  static jint      _nof_zombies;       // No. of zombie methods in the CodeCache
  static jint      _nof_not_entrants;  // No. of not_entrant methods in the CodeCache
  static jint      _nof_unloaded;      // No. of not_entrant methods in the CodeCache

  static void process_nmethod(nmethod *nm);    
 public:      
  static long traversal_count() { return _traversals; }  

  static void sweep();  // Invoked at the end of each safepoint

  static void notify(nmethod *nm) { // Tell the sweeper there's work to do
    // Inlined in order to prevent a tail call to increment.  On some platforms,
    // the call to increment can be patched at execution time with more efficient
    // code.  Compiler-generated code that does a tail call makes patching
    // impossible, since we can't tell where the call site is from inside the
    // increment method.
    if (nm->is_zombie()) {
      Atomic::inc(&_nof_zombies);
    } else if (nm->is_not_entrant()) {
      Atomic::inc(&_nof_not_entrants);
    } else if (nm->is_unloaded()) {
      Atomic::inc(&_nof_unloaded);  
    } else {
      ShouldNotReachHere();
    }  
  }
};
