#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jniHandles.hpp	1.33 04/02/11 18:45:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class JNIHandleBlock;


// Interface for creating and resolving local/global JNI handles

class JNIHandles : AllStatic {
  friend class VMStructs;
 private:
  static JNIHandleBlock* _global_handles;             // First global handle block
  static JNIHandleBlock* _weak_global_handles;        // First weak global handle block
  static oop _deleted_handle;                         // Sentinel marking deleted handles
  
 public:
  // Resolve handle into oop
  inline static oop resolve(jobject handle);
  // Resolve externally provided handle into oop with some guards
  inline static oop resolve_external_guard(jobject handle);
  // Resolve handle into oop, result guaranteed not to be null
  inline static oop resolve_non_null(jobject handle);

  // Local handles
  static jobject make_local(oop obj);
  static jobject make_local(JNIEnv* env, oop obj);    // Fast version when env is known
  static jobject make_local(Thread* thread, oop obj); // Even faster version when current thread is known
  inline static void destroy_local(jobject handle);

  // Global handles
  static jobject make_global(Handle  obj,    bool post_jvmpi_event = true);
  static void destroy_global(jobject handle, bool post_jvmpi_event = true);

  // Weak global handles
  static jobject make_weak_global(Handle obj);
  static void destroy_weak_global(jobject handle);

  // Sentinel marking deleted handles in block. Note that we cannot store NULL as 
  // the sentinel, since clearing weak global JNI refs are done by storing NULL in
  // the handle. The handle may not be reused before destroy_weak_global is called.
  static oop deleted_handle()   { return _deleted_handle; }

  // Initialization
  static void initialize();
  
  // Debugging
  static void verify();
  static bool is_local_handle(Thread* thread, jobject handle);
  static bool is_global_handle(jobject handle);
  static bool is_weak_global_handle(jobject handle);
  static long global_memory_usage();

  // Garbage collection support(global handles only, local handles are traversed from thread)
  // Traversal of regular global handles
  static void oops_do(OopClosure* f);
  // Traversal of weak global handles. Unreachable oops are cleared.
  static void weak_oops_do(BoolObjectClosure* is_alive, OopClosure* f);
};



// JNI handle blocks holding local/global JNI handles

class JNIHandleBlock : public CHeapObj {
  friend class VMStructs;
 private:
  enum SomeConstants {     
    block_size_in_oops  = 32                    // Number of handles per handle block
  };

  oop             _handles[block_size_in_oops]; // The handles
  int             _top;                         // Index of next unused handle
  JNIHandleBlock* _next;                        // Link to next block

  // The following instance variables are only used by the first block in a chain. 
  // Having two types of blocks complicates the code and the space overhead in negligble.
  JNIHandleBlock* _last;                        // Last block in use 
  JNIHandleBlock* _pop_frame_link;              // Block to restore on PopLocalFrame call
  oop*            _free_list;                   // Handle free list
  int             _allocate_before_rebuild;     // Number of blocks to allocate before rebuilding free list

  #ifndef PRODUCT
  JNIHandleBlock* _block_list_link;             // Link for list below
  static JNIHandleBlock* _block_list;           // List of all allocated blocks (for debugging only)
  #endif

  static JNIHandleBlock* _block_free_list;      // Free list of currently unused blocks
  static int      _blocks_allocated;            // For debugging/printing

  // Fill block with bad_handle values
  void zap();

  // No more handles in the both the current and following blocks
  void clear() { _top = NULL; }

  // Free list computation
  void rebuild_free_list();

 public:
  // Handle allocation
  jobject allocate_handle(oop obj);

  // Block allocation and block free list management
  static JNIHandleBlock* allocate_block(Thread* thread = NULL);
  static void release_block(JNIHandleBlock* block, Thread* thread = NULL);

  // JNI PushLocalFrame/PopLocalFrame support
  JNIHandleBlock* pop_frame_link() const          { return _pop_frame_link; }
  void set_pop_frame_link(JNIHandleBlock* block)  { _pop_frame_link = block; }

  // Stub generator support
  static int top_offset_in_bytes()                { return (intptr_t)(&((JNIHandleBlock*)NULL)->_top); }

  // Garbage collection support
  // Traversal of regular handles
  void oops_do(OopClosure* f);
  // Traversal of weak handles. Unreachable oops are cleared.
  void weak_oops_do(BoolObjectClosure* is_alive, OopClosure* f);

  // Debugging
  bool chain_contains(jobject handle) const;    // Does this block or following blocks contain handle
  bool contains(jobject handle) const;          // Does this block contain handle
  #ifndef PRODUCT
  int length() const;                           // Length of chain starting with this block
  static bool any_contains(jobject handle);     // Does any block currently in use contain handle
  long memory_usage() const;
  static void print_statictics();
  #endif
};


inline oop JNIHandles::resolve(jobject handle) {
  oop result = (handle == NULL ? NULL : *(oop*)handle);
  assert(result != NULL || (handle == NULL || !CheckJNICalls || is_weak_global_handle(handle)), "Invalid value read from jni handle");
  assert(result != badJNIHandle, "Pointing to zapped jni handle area");
  return result;
};


inline oop JNIHandles::resolve_external_guard(jobject handle) {
  if (handle == NULL) return NULL;
  oop result = *(oop*)handle;
  if (result == NULL || result == badJNIHandle) return NULL;
  return result;
};


inline oop JNIHandles::resolve_non_null(jobject handle) {
  assert(handle != NULL, "JNI handle should not be null");
  oop result = *(oop*)handle;
  assert(result != NULL, "Invalid value read from jni handle");
  assert(result != badJNIHandle, "Pointing to zapped jni handle area");
  // Don't let that private _deleted_handle object escape into the wild.
  assert(result != deleted_handle(), "Used a deleted global handle.");
  return result;
};


inline void JNIHandles::destroy_local(jobject handle) {
  if (handle != NULL) {
    *((oop*)handle) = deleted_handle(); // Mark the handle as deleted, allocate will reuse it
  }
}


