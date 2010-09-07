/*
 * Copyright (c) 1997, 2005, Oracle and/or its affiliates. All rights reserved.
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

// An NmethodSweeper is an incremental cleaner for:
//    - cleanup inline caches
//    - reclamation of unreferences zombie nmethods
//

class NMethodSweeper : public AllStatic {
  static long      _traversals;   // Stack traversal count
  static CodeBlob* _current;      // Current nmethod
  static int       _seen;         // Nof. nmethod we have currently processed in current pass of CodeCache
  static int       _invocations;  // No. of invocations left until we are completed with this pass

  static bool      _rescan;          // Indicates that we should do a full rescan of the
                                     // of the code cache looking for work to do.
  static int       _locked_seen;     // Number of locked nmethods encountered during the scan
  static int       _not_entrant_seen_on_stack; // Number of not entrant nmethod were are still on stack

  static bool      _was_full;        // remember if we did emergency unloading
  static jint      _advise_to_sweep; // flag to indicate code cache getting full
  static jlong     _last_was_full;   // timestamp of last emergency unloading
  static uint      _highest_marked;   // highest compile id dumped at last emergency unloading
  static long      _was_full_traversal;   // trav number at last emergency unloading

  static void process_nmethod(nmethod *nm);
 public:
  static long traversal_count() { return _traversals; }

  static void sweep();  // Invoked at the end of each safepoint

  static void notify(nmethod* nm) {
    // Perform a full scan of the code cache from the beginning.  No
    // need to synchronize the setting of this flag since it only
    // changes to false at safepoint so we can never overwrite it with false.
     _rescan = true;
  }

  static void handle_full_code_cache(bool is_full); // Called by compilers who fail to allocate
  static void speculative_disconnect_nmethods(bool was_full);   // Called by vm op to deal with alloc failure

  static void set_was_full(bool state) { _was_full = state; }
  static bool was_full() { return _was_full; }
};
