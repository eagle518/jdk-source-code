#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Runtime1.hpp	1.117 04/02/17 20:30:02 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class StubAssembler;

// iEntries is a holder for interpreter entry points entered from
// compiled code. Before invoking the interpreter they check if
// the callee has been compiled in the meantime (besides inline
// cache check, if necessary). If the callee has been compiled,
// the call site needs to be updated.

class iEntries VALUE_OBJ_CLASS_SPEC {
 private:
  address _base;
  int _static_call_offset;
  int _optimized_call_offset;
  int _virtual_call_offset;
  int _virtual_final_call_offset;

 public:
  // creation
  iEntries();
  iEntries(int static_call_offset, int optimized_call_offset, int virtual_call_offset, int virtual_final_call_offset);
  void set_base(address base);

  // accessors
  address static_call_entry() const              { return _base + _static_call_offset; }
  address optimized_call_entry() const           { return _base + _optimized_call_offset; }
  address virtual_call_entry() const             { return _base + _virtual_call_offset;  }
  address virtual_final_call_entry() const       { return _base + _virtual_final_call_offset; }
};


// The Runtime1 holds all assembly stubs and VM
// runtime routines needed by code code generated
// by the Compiler1.

#define RUNTIME1_STUBS(stub, last_entry) \
  stub(alignment_frame_return)       \
  stub(resolve_invokestatic)         \
  stub(resolve_invokevirtual)        \
  stub(resolve_invoke_opt_virtual)   \
  stub(handle_ic_miss)               \
  stub(handle_wrong_static_method)   \
  stub(handle_wrong_method)          \
  stub(must_compile_method)          \
  stub(entering_non_entrant)         \
  stub(range_check_failed)      /* throws ArrayIndexOutOfBoundsException */ \
  stub(throw_index_exception)   /* throws IndexOutOfBoundsException */ \
  stub(throw_div0_exception)         \
  stub(throw_null_pointer_exception) \
  stub(new_instance)                 \
  stub(fast_new_instance)            \
  stub(fast_new_instance_init_check) \
  stub(new_type_array)               \
  stub(new_object_array)             \
  stub(new_multi_array)              \
  stub(handle_exception)             \
  stub(unwind_exception)             \
  stub(throw_array_store_exception)  \
  stub(throw_class_cast_exception)   \
  stub(throw_incompatible_class_change_error)   \
  stub(slow_subtype_check)           \
  stub(monitorenter)                 \
  stub(monitorenter_with_jvmpi)      \
  stub(monitorexit)                  \
  stub(interpreter_entries)          \
  stub(init_check_patching)          \
  stub(load_klass_patching)          \
  stub(osr_unwind_exception)         \
  stub(illegal_instruction_handler)  \
  stub(polling_page_safepoint_handler) \
  stub(polling_page_return_handler)  \
  stub(check_safepoint_and_suspend_for_native_trans)  \
  stub(jvmdi_exception_throw)        \
  stub(jvmpi_method_entry)           \
  stub(jvmpi_method_exit)            \
  stub(ldiv_stub)                    \
  stub(lrem_stub)                    \
  stub(f2i_is32bit_stub)             \
  stub(f2i_not32bit_stub)            \
  stub(f2l_stub)                     \
  stub(d2l_stub)                     \
  last_entry(number_of_ids)

#define DECLARE_STUB_ID(x)       x ## _id ,
#define DECLARE_LAST_STUB_ID(x)  x
#define STUB_NAME(x)             #x " Runtime1 stub",
#define LAST_STUB_NAME(x)        #x " Runtime1 stub"

class Runtime1: public AllStatic {
  friend class VMStructs;
  friend class ArrayCopyStub;
 private:
  enum Constants {
#ifdef _LP64
    desired_max_code_buffer_size = 500*K,
#else
    desired_max_code_buffer_size = 256*K,
#endif
    desired_max_locs_buffer_size =   8*K 
  };

  // Note: These buffers are allocated once ince allocation for
  //       each compilation seems to be too expensive (at least on Intel win32).
  static BufferBlob* _code_buffer;
  static address     _locs_buffer;
  static int         _code_buffer_size;
  static int         _locs_buffer_size;

 public:
  enum StubID {
    RUNTIME1_STUBS(DECLARE_STUB_ID, DECLARE_LAST_STUB_ID)
  };

 private:
  static bool      _is_initialized;
  static CodeBlob* _blobs[number_of_ids];
  static const char* _blob_names[];
  static iEntries  _ientries[AbstractInterpreter::number_of_method_entries];

  // statistics
  static int _resolve_invoke_cnt;
  static int _handle_wrong_method_cnt;
  static int _ic_miss_cnt;
  static int _generic_arraycopy_cnt;
  static int _primitive_arraycopy_cnt;
  static int _oop_arraycopy_cnt;
  static int _arraycopy_slowcase_cnt;

  // stub generation
  static void generate_blob_for(StubID id);
  static OopMapSet* generate_code_for(StubID id, StubAssembler* masm, int* frame_size);
  static OopMapSet* generate_exception_throw (StubAssembler* sasm, address target, Register arg1 = noreg);
  static OopMapSet* generate_patching(StubAssembler* sasm, address target);
  static OopMapSet* generate_handler(StubAssembler* sasm, address target, bool force_return);
  static OopMapSet* generate_polling_page_handler(StubAssembler* sasm, address target);
  static OopMapSet* generate_deoptimization_handler(StubAssembler* sasm, int* unpack_exception_offset, int* frame_size, int* reexecute_offset);

  // runtime entry points
  static void new_instance    (JavaThread* thread, klassOop klass);
  static void new_type_array  (JavaThread* thread, klassOop klass, jint length);
  static void new_object_array(JavaThread* thread, klassOop klass, jint length);
  static void new_multi_array (JavaThread* thread, klassOop klass, int rank, jint* dims);

  static address resolve_virtual_call(JavaThread* thread, oop receiver);
  static address resolve_opt_virtual_call(JavaThread* thread, oop receiver);
  static address resolve_static_call(JavaThread* thread, oop receiver);

  static address handle_ic_miss     (JavaThread* thread, oop recv);
  static address handle_wrong_method(JavaThread* thread, oop recv);
  static void unimplemented_entry   (JavaThread* thread, StubID id);

  static address exception_handler_for_pc(JavaThread* thread, oop exception, address pc);
  static void post_jvmti_exception_throw(JavaThread* thread, oop exception);

  static void throw_range_check_exception(JavaThread* thread, int index);
  static void throw_index_exception(JavaThread* thread, int index);
  static void throw_div0_exception(JavaThread* thread);
  static void throw_null_pointer_exception(JavaThread* thread);
  static void throw_class_cast_exception(JavaThread* thread, oop obect);
  static void throw_incompatible_class_change_error(JavaThread* thread);
  static void throw_array_store_exception(JavaThread* thread);

  static void monitorenter(JavaThread* thread, oop obj, BasicObjectLock* lock);
  static void monitorexit (JavaThread* thread, BasicObjectLock* lock);

  static int init_check_patching(JavaThread* thread);
  static int move_klass_patching(JavaThread* thread);

   //static void patch_code(JavaThread* thread, StubID stub_id, TRAPS);

  static void prepare_interpreter_call(JavaThread* thread, methodOop method);

  static void jvmpi_method_entry_after_deopt(JavaThread* thread, oop receiver);

 public:
  static CodeBuffer* new_code_buffer();

  // initialization
  static bool is_initialized()                   { return _is_initialized; }
  static void initialize();
  static void initialize_pd();

  // stubs
  static CodeBlob* blob_for (StubID id);
  static address   entry_for(StubID id)          { return blob_for(id)->instructions_begin(); }
  static const char* name_for (StubID id);

  // interpreter entry points for compiled code
  static iEntries* ientries_for(methodHandle m);

  // method tracing
  static void trace_method_entry(jint v1, jint v2);
  static void trace_method_exit(jint value);

  static void trace_block_entry(jint block_id);

  // directly accessible leaf routine
  static int  arraycopy(oop src, int src_pos, oop dst, int dst_pos, int length);
  static void primitive_arraycopy(HeapWord* src, HeapWord* dst, int length);
  static void oop_arraycopy(HeapWord* src, HeapWord* dst, int length);

  static void print_statistics()                 PRODUCT_RETURN;
};


class GC_Support: public AllStatic {
 public:
  static void  preserve_callee_argument_oops(const frame fr, symbolHandle signature, bool is_static, OopClosure* f);
  static intptr_t* get_argument_addr_at         (const frame fr, ArgumentLocation location);
  static intptr_t* get_stack_argument_addr_at   (const frame fr, int offset_from_sp_in_words);
  static intptr_t* get_register_argument_addr_at(const frame fr, Register r);
};
