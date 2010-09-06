#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)codeCache.hpp	1.53 03/12/23 16:39:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The CodeCache implements the code cache for various pieces of generated
// code, e.g., compiled java methods, runtime stubs, transition frames, etc.
// The entries in the CodeCache are all CodeBlob's.

// Implementation:
//   - Each CodeBlob occupies one chunk of memory.
//   - Like the offset table in oldspace the zone has at table for
//     locating a method given a addess of an instruction.

class OopClosure;

class CodeCache : AllStatic {
  friend class VMStructs;
 private:
  // CodeHeap is malloc()'ed at startup and never deleted during shutdown,
  // so that the generated assembly code is always there when it's needed.
  // This may cause memory leak, but is necessary, for now. See 4423824,
  // 4422213 or 4436291 for details.
  static CodeHeap * _heap;
  static int _number_of_blobs;
  static int _number_of_nmethods_with_dependencies;
  
  static void verify_if_often() PRODUCT_RETURN;
 public:     

  // Initialization
  static void initialize();

  // Allocation/administration
  static CodeBlob* allocate(int size);              // allocates a new CodeBlob
  static void commit(CodeBlob* cb);                 // called when the allocated CodeBlob has been filled
  static int alignment_unit();                      // guaranteed alignment of all CodeBlobs
  static int alignment_offset();                    // guaranteed offset of first CodeBlob byte within alignment unit (i.e., allocation header)
  static void free(CodeBlob* cb);                   // frees a CodeBlob
  static void flush();                              // flushes all CodeBlobs
  static bool contains(void *p);                    // returns whether p is included  
  static void blobs_do(void f(CodeBlob* cb));       // iterates over all CodeBlobs 
  static void nmethods_do(void f(nmethod* nm));     // iterates over all nmethods 

  // Lookup
  static CodeBlob* find_blob(void* start);         
  static nmethod*  find_nmethod(void* start);

  // Lookup that does not fail if you lookup a zombie method (if you call this, be sure to know
  // what you are doing)
  static CodeBlob* find_blob_unsafe(void* start);

  // Iteration
  static CodeBlob* first();
  static CodeBlob* next (CodeBlob* cb);  
  static CodeBlob* alive(CodeBlob *cb);
  static nmethod* alive_nmethod(CodeBlob *cb);
  static int       nof_blobs()                 { return _number_of_blobs; }

  // GC support
  static void gc_epilogue();
  static void gc_prologue();
  // If "unloading_occurred" is true, then unloads (i.e., breaks root links
  // to) any unmarked codeBlobs in the cache.  Sets "marked_for_unloading"
  // to "true" iff some code got unloaded.
  static void do_unloading(BoolObjectClosure* is_alive,
                                   OopClosure* keep_alive,
                                   bool unloading_occurred,
                                   bool& marked_for_unloading);
  static void oops_do(OopClosure* f);
  
  // Printing/debugging
  static void print()   PRODUCT_RETURN;          // prints summary 
  static void print_internals();
  static void verify()  PRODUCT_RETURN;          // verifies the code cache
  
  // Profiling
  static address first_address();                // first address used for CodeBlobs
  static address last_address();                 // last  address used for CodeBlobs
  static size_t  capacity()                      { return _heap->capacity(); }
  static size_t  max_capacity()                  { return _heap->max_capacity(); }
  static size_t  unallocated_capacity()          { return _heap->unallocated_capacity(); }  

#ifndef CORE
  static void clear_inline_caches()                       CORE_RETURN;         // clear all inline caches

  // Deoptimization
  static int  mark_for_deoptimization(klassOop dependee)  CORE_RETURN;
#ifdef HOTSWAP
  static int  mark_for_evol_deoptimization(instanceKlassHandle dependee)  CORE_RETURN;
#endif HOTSWAP
  static int  mark_for_deoptimization(methodOop dependee) CORE_RETURN;
  static void make_marked_nmethods_zombies()              CORE_RETURN;
  static void make_marked_nmethods_not_entrant()          CORE_RETURN;

    // tells how many nmethods have dependencies
  static int number_of_nmethods_with_dependencies();
#endif
};
