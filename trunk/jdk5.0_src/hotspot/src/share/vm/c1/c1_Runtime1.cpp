#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Runtime1.cpp	1.207 04/06/09 09:32:58 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

#include "incls/_precompiled.incl"
#include "incls/_c1_Runtime1.cpp.incl"


// Implementation of StubAssembler

StubAssembler::StubAssembler(CodeBuffer* code) : C1_MacroAssembler(code) {
  _name = NULL;
  _must_gc_arguments = false;
}


void StubAssembler::set_info(const char* name, bool must_gc_arguments) {
  _name = name;
  _must_gc_arguments = must_gc_arguments;
}


// Implementation of iEntries

iEntries::iEntries() {
  _static_call_offset    = 0;
  _optimized_call_offset = 0;
  _virtual_call_offset   = 0;
  _virtual_final_call_offset  = 0;
}


iEntries::iEntries(int static_call_offset, int optimized_call_offset, int virtual_call_offset, int virtual_final_call_offset) {
  _static_call_offset    = static_call_offset;
  _optimized_call_offset = optimized_call_offset;
  _virtual_call_offset   = virtual_call_offset;
  _virtual_final_call_offset  = virtual_final_call_offset;
}


void iEntries::set_base(address base) {
  assert(base != NULL, "base must exist");
  _base = base;
}


// Implementation of Runtime1

bool      Runtime1::_is_initialized = false;
CodeBlob* Runtime1::_blobs[Runtime1::number_of_ids];
const char *Runtime1::_blob_names[] = {
  RUNTIME1_STUBS(STUB_NAME, LAST_STUB_NAME)
};
iEntries  Runtime1::_ientries[AbstractInterpreter::number_of_method_entries];

#ifndef PRODUCT
// statistics
int Runtime1::_generic_arraycopy_cnt = 0;
int Runtime1::_primitive_arraycopy_cnt = 0;
int Runtime1::_oop_arraycopy_cnt = 0;
int Runtime1::_arraycopy_slowcase_cnt = 0;
#endif

int Runtime1::_code_buffer_size = 0;
int Runtime1::_locs_buffer_size = 0;
BufferBlob* Runtime1::_code_buffer      = NULL;
address Runtime1::_locs_buffer      = NULL;


CodeBuffer* Runtime1::new_code_buffer() {
  CodeBuffer* code = new CodeBuffer(_code_buffer_size, _locs_buffer_size, 0, 0, 0, true,
                                    _code_buffer, (relocInfo *)_locs_buffer, NULL, false, 
                                    NULL, "Compiler1 temporary CodeBuffer");
  // Allocate code buffer space only once
  if (_code_buffer == NULL ) _code_buffer = code->get_blob();
  return code;
}


void Runtime1::generate_blob_for(StubID id) {
  assert(0 <= id && id < number_of_ids, "illegal stub id");
  ResourceMark rm;
  // create code buffer for code storage
#ifdef _LP64
  int code_size = CodeBuffer::insts_memory_size(VerifyOops ? 11*K : 7*K);
#else
  int code_size = CodeBuffer::insts_memory_size(VerifyOops ? 8*K : 4*K);
#endif
  int locs_size = CodeBuffer::locs_memory_size(1*K);

  CodeBuffer* code = new_code_buffer();

  // create assembler for code generation
  StubAssembler* sasm = new StubAssembler(code);
  // generate code for runtime stub
  int frame_size;
  OopMapSet* oop_maps;
  oop_maps = generate_code_for(id, sasm, &frame_size);
  // align so printing shows nop's instead of random code at the end (SimpleStubs are aligned)
  sasm->align(BytesPerWord);
  // make sure all code is in code buffer
  sasm->flush();
  // create blob - distinguish a few special cases
  CodeBlob* blob = NULL;
  switch (id) {
    case illegal_instruction_handler_id:
    case polling_page_safepoint_handler_id:
    case polling_page_return_handler_id:
      blob = SafepointBlob::create(code, oop_maps, no_frame_size);
      break;

    default:
      {
        RuntimeStub* stub = RuntimeStub::new_runtime_stub(name_for(id), code, no_frame_size, oop_maps, sasm->must_gc_arguments());
        blob = stub;
      }
      break;
  }
  // install blob
  assert(blob != NULL, "blob must exist");
  _blobs[id] = blob;
}


void Runtime1::initialize() {
  // Warning: If we have more than one compilation running in parallel, we
  //          need a lock here with the current setup (lazy initialization).
  if (!is_initialized()) {
    _is_initialized = true;

    // setup CodeBuffer
    assert(_code_buffer == NULL, "initialization should happen only once");
    _code_buffer_size = CodeBuffer::insts_memory_size(desired_max_code_buffer_size);
    _locs_buffer_size = CodeBuffer::locs_memory_size (desired_max_locs_buffer_size);
    _code_buffer = NULL;
    _locs_buffer = NEW_C_HEAP_ARRAY(u_char, _locs_buffer_size);

    // platform-dependent initialization
    initialize_pd();
    // generate stubs
    SharedRuntime::generate_deopt_blob();
    for (int id = 0; id < number_of_ids; id++) generate_blob_for((StubID)id);
    // 'relocate' interpreter entries
    address base = blob_for(interpreter_entries_id)->instructions_begin();
    for (int j = 0; j < AbstractInterpreter::number_of_method_entries; j++) {
      _ientries[j].set_base(base);
    }
    // printing
    if (PrintSimpleStubs) {
      ResourceMark rm;
      for (int id = 0; id < number_of_ids; id++) {
        _blobs[id]->print();
      }
    }
  }
}


CodeBlob* Runtime1::blob_for(StubID id) {
  assert(0 <= id && id < number_of_ids, "illegal stub id");
  if (!is_initialized()) initialize();
  return _blobs[id];
}


const char* Runtime1::name_for(StubID id) {
  assert(0 <= id && id < number_of_ids, "illegal stub id");
  return _blob_names[id];
}


iEntries* Runtime1::ientries_for(methodHandle m) {
  if (!is_initialized()) initialize();
  AbstractInterpreter::MethodKind kind = AbstractInterpreter::method_kind(m);
  assert(0 <= kind && kind < AbstractInterpreter::number_of_method_entries, "illegal index");
  return &_ientries[kind];
}


// runtime entry points for call resolution

JRT_BLOCK_ENTRY(address, Runtime1::resolve_virtual_call(JavaThread* thread, oop receiver))
  methodHandle callee_method;
  JRT_BLOCK
    Handle recv(THREAD, receiver);
    callee_method = SharedRuntime::resolve_helper(thread, true, false, CHECK_0);
    // return callee methodOop, recv and interpreter or verified entry point
    // (resolution was exact)
    thread->set_vm_result(callee_method());
    thread->set_vm_result_2(recv());
  JRT_BLOCK_END
  // return compiled code entry point after potential safepoints
  return callee_method->verified_code_entry();
JRT_END


JRT_BLOCK_ENTRY(address, Runtime1::resolve_opt_virtual_call(JavaThread* thread, oop receiver))
  methodHandle callee_method;
  JRT_BLOCK
    Handle recv(THREAD, receiver);
    callee_method = SharedRuntime::resolve_helper(thread, true, true, CHECK_0);
    // return callee methodOop, recv and interpreter or verified entry point
    // (resolution was exact)
    thread->set_vm_result(callee_method());
    thread->set_vm_result_2(recv());
  JRT_BLOCK_END
  // return compiled code entry point after potential safepoints
  return callee_method->verified_code_entry();
JRT_END


JRT_BLOCK_ENTRY(address, Runtime1::resolve_static_call(JavaThread* thread, oop receiver))
  methodHandle callee_method;
  JRT_BLOCK
    Handle recv(THREAD, receiver);
    callee_method = SharedRuntime::resolve_helper(thread, false, false, CHECK_0);
    // return callee methodOop, recv and interpreter or verified entry point
    // (resolution was exact)
    thread->set_vm_result(callee_method());
    thread->set_vm_result_2(recv());
  JRT_BLOCK_END
  // return compiled code entry point after potential safepoints
  return callee_method->verified_code_entry();
JRT_END

JRT_BLOCK_ENTRY(address, Runtime1::handle_ic_miss(JavaThread* thread, oop receiver))
  methodHandle callee_method;
  JRT_BLOCK
    Handle recv(thread, receiver);
    callee_method = SharedRuntime::handle_ic_miss_helper(thread, CHECK_0);
    // Return methodOop & recv through TLS
    thread->set_vm_result(callee_method());
    thread->set_vm_result_2(recv());
  JRT_BLOCK_END
  // return compiled code entry point after potential safepoints
  return callee_method->verified_code_entry();
JRT_END


JRT_BLOCK_ENTRY(address, Runtime1::handle_wrong_method(JavaThread* thread, oop receiver))
  methodHandle callee_method;
  JRT_BLOCK
    Handle recv(thread, receiver);
    //Force resolving of caller (if we called from compiled frame)
    callee_method = SharedRuntime::reresolve_call_site(thread, CHECK_0);
    assert(recv.is_null() || !callee_method->is_static(), "non-null receiver for static call?");

    thread->set_vm_result(callee_method());
    thread->set_vm_result_2(recv());
  JRT_BLOCK_END
  // return compiled code entry point after potential safepoints
  return callee_method->verified_code_entry();
JRT_END


JRT_ENTRY(void, Runtime1::new_instance(JavaThread* thread, klassOop klass))
  assert(oop(klass)->is_klass(), "not a class");
  instanceKlassHandle h(thread, klass);
  h->check_valid_for_instantiation(true, CHECK);
  // make sure klass is initialized
  h->initialize(CHECK);
  // allocate instance and return via TLS
  oop obj = h->allocate_instance(CHECK);
  thread->set_vm_result(obj);
JRT_END


JRT_ENTRY(void, Runtime1::new_type_array(JavaThread* thread, klassOop klass, jint length))
  // Note: no handle for klass needed since they are not used
  //       anymore after new_typeArray() and no GC can happen before.
  //       (This may have to change if this code changes!)
  assert(oop(klass)->is_klass(), "not a class");
  BasicType elt_type = typeArrayKlass::cast(klass)->type();
  oop obj = oopFactory::new_typeArray(elt_type, length, CHECK);
  thread->set_vm_result(obj);
  // This is pretty rare but this runtime patch is stressful to deoptimization
  // if we deoptimize here so force a deopt to stress the path.
  if (DeoptimizeALot) {
    RegisterMap map(thread, false);
    frame caller =  thread->last_frame().sender(&map);
    CodeBlob* cb = CodeCache::find_blob_unsafe(caller.pc());
    if (!((nmethod*)cb)->is_patched_for_deopt()) {
      VM_DeoptimizeFrame deopt(thread, caller.id());
      VMThread::execute(&deopt);
      assert(((nmethod*)cb)->is_patched_for_deopt(), "Must be patched");
    }
  }

JRT_END


JRT_ENTRY(void, Runtime1::new_object_array(JavaThread* thread, klassOop array_klass, jint length))
  // Note: no handle for klass needed since they are not used
  //       anymore after new_objArray() and no GC can happen before.
  //       (This may have to change if this code changes!)
  assert(oop(array_klass)->is_klass(), "not a class");
  klassOop elem_klass = objArrayKlass::cast(array_klass)->element_klass();
  objArrayOop obj = oopFactory::new_objArray(elem_klass, length, CHECK);
  thread->set_vm_result(obj);
  // This is pretty rare but this runtime patch is stressful to deoptimization
  // if we deoptimize here so force a deopt to stress the path.
  if (DeoptimizeALot) {
    RegisterMap map(thread, false);
    frame caller =  thread->last_frame().sender(&map);
    CodeBlob* cb = CodeCache::find_blob_unsafe(caller.pc());
    if (!((nmethod*)cb)->is_patched_for_deopt()) {
      VM_DeoptimizeFrame deopt(thread, caller.id());
      VMThread::execute(&deopt);
      assert(((nmethod*)cb)->is_patched_for_deopt(), "Must be patched");
    }
  }
JRT_END


JRT_ENTRY(void, Runtime1::new_multi_array(JavaThread* thread, klassOop klass, int rank, jint* dims))
  assert(oop(klass)->is_klass(), "not a class");
  assert(rank >= 1, "rank must be nonzero");
#ifdef _LP64
// In 64 bit mode, the sizes are stored in the top 32 bits
// of each 64 bit stack entry.
// dims is actually an intptr_t * because the arguments
// are pushed onto a 64 bit stack.
// We must create an array of jints to pass to multi_allocate.
// We reuse the current stack because it will be popped
// after this bytecode is completed.
  if ( rank > 1 ) {
    int index;
    for ( index = 1; index < rank; index++ ) {  // First size is ok
        dims[index] = dims[index*2];
    }
  }
#endif
  oop obj = arrayKlass::cast(klass)->multi_allocate(rank, dims, 1, CHECK);
  thread->set_vm_result(obj);
JRT_END


JRT_ENTRY(void, Runtime1::unimplemented_entry(JavaThread* thread, StubID id))
  tty->print_cr("Runtime1::entry_for(%d) returned unimplemented entry point", id);
JRT_END


JRT_ENTRY(void, Runtime1::throw_array_store_exception(JavaThread* thread))
  THROW(vmSymbolHandles::java_lang_ArrayStoreException());
JRT_END


JRT_ENTRY(void, Runtime1::post_jvmti_exception_throw(JavaThread* thread, oop exception))
  if (JvmtiExport::can_post_exceptions()) {
    vframeStream vfst(thread, true);
    address bcp = vfst.method()->bcp_from(vfst.bci());
    JvmtiExport::post_exception_throw(thread, vfst.method(), bcp, exception);
  }
JRT_END

extern void vm_exit(int code);

// Enter this method from compiled code handler below. This is where we transition
// to VM mode. This is done as a helper routine so that the method called directly
// from compiled code does not have to transition to VM. This allows the entry
// method to see if the nmethod that we have just looked up a handler for has
// been deoptimized while we were in the vm. This simplifies the assembly code
// cpu directories.
//
// We are entering here from exception stub (via the entry method below)
// If there is a compiled exception handler in this method, we will continue there;
// otherwise we will unwind the stack and continue at the caller of top frame method
// Note: we enter in Java using a special JRT wrapper. This wrapper allows us to
// control the area where we can allow a safepoint. After we exit the safepoint area we can
// check to see if the handler we are going to return is now in a nmethod that has
// been deoptimized. If that is the case we return the deopt blob
// unpack_with_exception entry instead. This makes life for the exception blob easier
// because making that same check and diverting is painful from assembly language.
//


JRT_ENTRY_NO_ASYNC(static address, exception_handler_for_pc_helper(JavaThread* thread, oop ex, address pc, nmethod*& nm))

  address continuation = NULL;
  Handle exception(thread, ex);
  pc = thread->safepoint_state()->compute_adjusted_pc(pc);
  nm = CodeCache::find_nmethod(pc);
  assert(nm != NULL, "this is not an nmethod");
#ifdef ASSERT
  assert(exception.not_null(), "NULL exceptions should be handled by throw_exception");
  assert(exception->is_oop(), "just checking");
  // Check that exception is a subclass of Throwable, otherwise we have a VerifyError
  if (!(exception->is_a(SystemDictionary::throwable_klass()))) {
    if (ExitVMOnVerifyError) vm_exit(-1);
    ShouldNotReachHere();
  }
#endif
  // too noisy event
  // Events::log("exception_handler_for_pc @ " INTPTR_FORMAT " oop " INTPTR_FORMAT " impl %d", pc, ex, (intptr_t)entered_as_implicit);
  // ExceptionCache is used only for exceptions at call and not for implicit exceptions

  // Check the stack guard pages and reenable them if necessary and there is
  // enough space on the stack to do so.  Use fast exceptions only if the guard
  // pages are enabled.
  bool guard_pages_enabled = thread->stack_yellow_zone_enabled();
  if (!guard_pages_enabled) guard_pages_enabled = thread->reguard_stack();
  if (UseFastExceptionHandling && guard_pages_enabled && !JvmtiExport::can_post_exceptions()) {
    Handle h_ex(thread, ex);
    address fast_continuation = nm->handler_for_exception_and_pc(h_ex, pc);
    if (fast_continuation != NULL) {
      if (fast_continuation == ExceptionCache::unwind_handler()) fast_continuation = NULL;
      thread->set_vm_result(exception());
      return fast_continuation;
    }
  }

  // JVMTI only
  address handler_bcp;
  methodHandle method;

    // If the stack guard pages are enabled, check whether there is a handler in
    // the current method.  Otherwise (guard pages disabled), force an unwind and
    // skip the exception cache update (i.e., just leave continuation==NULL).
    if (guard_pages_enabled) {

      // New exception handling mechanism can support inlined methods
      // with exception handlers since the mappings are from PC to PC

      // debugging support
      // tracing
      if (TraceExceptions) {
        ttyLocker ttyl;
        ResourceMark rm;
        tty->print_cr("Exception <%s> (0x%x) thrown in compiled method <%s> at PC " PTR_FORMAT " for thread 0x%x",
                      exception->print_value_string(), exception(), nm->method()->print_value_string(), pc, thread);
      }
      // for AbortVMOnException flag
      NOT_PRODUCT(Exceptions::debug_check_abort(exception));

      int pco = ExceptionRangeTable::compute_modified_at_call_pco(pc - nm->instructions_begin(), true);
      ExceptionRangeTable* table = nm->exception_range_table();
      assert(table != NULL, "");
      int idx = table->entry_index_for_pco(pco);
      if (idx != -1) {
        ResourceMark rm(thread);
        ScopeDesc* sd = nm->scope_desc_at(pc, true);
        int scope_count = 0;
        assert(sd != NULL, "must have scope description at exception sites");
        while (idx < table->length()) {
          ExceptionRangeEntry* entry = table->entry_at(idx);
          if (entry->covers(pco)) {
            assert(entry->scope_count() >= scope_count,
                   "scopes must be added to ExceptionRangeTable in inner-to-outer order");
            while (scope_count < entry->scope_count()) {
              sd = sd->sender();
              assert(sd != NULL, "out of scopes - bug in ExceptionRangeTable generation");
              ++scope_count;
            }
            KlassHandle klass;
            if (entry->exception_type() != 0) {
              klassOop k = sd->method()->constants()->klass_at(entry->exception_type(), THREAD);
              if (HAS_PENDING_EXCEPTION) {
                // We threw an exception while trying to find the
                // exception handler.  The exception modeling in the
                // parse doesn't provide enough information for us to
                // do the redispatch we'd do in the interpreter case,
                // so deoptimize and reexecute with the original
                // exception pending.  This will cause the dispatch to
                // restart in the interpreter.
                CLEAR_PENDING_EXCEPTION;

                if (!nm->is_patched_for_deopt()) {
                  RegisterMap map(thread, false);
                  frame caller =  thread->last_frame().sender(&map);
                  VM_DeoptimizeFrame deopt(thread, caller.id());
                  VMThread::execute(&deopt);
                }

                THREAD->set_pending_exception(exception(), __FILE__, __LINE__);
                thread->set_vm_result(exception());
                return NULL;
              }
              klass = KlassHandle(thread, k);
            }
            if (entry->exception_type() == 0 || exception()->is_a(klass())) {
              continuation = nm->instructions_begin() + entry->handler_pco();
              if (JvmtiExport::can_post_exceptions()) {
                handler_bcp = sd->method()->bcp_from(entry->handler_bci());
                method = sd->method();
              }
#ifdef ASSERT
              KlassHandle ek (thread, exception->klass());
              int i_bci = sd->method()->fast_exception_handler_bci_for(ek,
                                                    sd->bci(), CHECK_(0));
              assert(entry->handler_bci() == i_bci, "dispatching to the wrong exception handler");
#endif // ASSERT
              break;
            }
          } else {
            if (entry->start_pco() > pco) break;
          }
          ++idx;
        }
      }

      if (UseFastExceptionHandling && !JvmtiExport::can_post_exceptions()) {
        // the exception cache is used only by non-implicit exceptions
        if (continuation == NULL) {
          nm->add_handler_for_exception_and_pc(exception, pc, ExceptionCache::unwind_handler());
        } else {
          nm->add_handler_for_exception_and_pc(exception, pc, continuation);
        }
      }
    }

  // notify debugger of an exception catch
  // (this is good for exceptions caught in native methods as well)
  if (JvmtiExport::can_post_exceptions() && continuation != NULL) {
    //Pass true for in_handler_frame because the handler frame is known by C1 at this point.
    JvmtiExport::notice_unwind_due_to_exception(thread, method(), handler_bcp, exception(), true);
  }

  thread->set_vm_result(exception());

  if (TraceExceptions) {
    ttyLocker ttyl;
    ResourceMark rm;
    tty->print_cr("Thread " PTR_FORMAT " continuing at PC " PTR_FORMAT " for exception thrown at PC " PTR_FORMAT,
                  thread, continuation, pc);
  }

  return continuation;
JRT_END

// Enter this method from compiled code only if there is a Java exception handler
// in the method handling the exception
// We are entering here from exception stub. We don't do a normal VM transition here.
// We do it in a helper. This is so we can check to see if the nmethod we have just
// searched for an exception handler has been deoptimized in the meantime.
address  Runtime1::exception_handler_for_pc(JavaThread* thread, oop ex, address pc) {

  // Still in Java mode
  debug_only(ResetNoHandleMark rnhm);
  nmethod* nm = NULL;
  address continuation = NULL;
  {
    // Enter VM mode by calling the helper

    ResetNoHandleMark rnhm;
    continuation = exception_handler_for_pc_helper(thread, ex, pc, nm);
  }
  // Back in JAVA, use no oops DON'T safepoint

  // Now check to see if the nmethod we were called from is now deoptimized.
  // If so we must return to the deopt blob and de
  // deoptimized nmethod

  if (nm != NULL && nm->is_patched_for_deopt()) {
    continuation = SharedRuntime::deopt_blob()->unpack_with_exception();
  }

  return continuation;
}


JRT_ENTRY(void, Runtime1::throw_range_check_exception(JavaThread* thread, int index))
  Events::log("throw_range_check");
  char message[jintAsStringSize];
  sprintf(message, "%d", index);
  SharedRuntime::throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_ArrayIndexOutOfBoundsException(), message);
JRT_END


JRT_ENTRY(void, Runtime1::throw_index_exception(JavaThread* thread, int index))
  Events::log("throw_index");
  char message[16];
  sprintf(message, "%d", index);
  SharedRuntime::throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_IndexOutOfBoundsException(), message);
JRT_END


JRT_ENTRY(void, Runtime1::throw_div0_exception(JavaThread* thread))
  SharedRuntime::throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_ArithmeticException(), "/ by zero");
JRT_END


JRT_ENTRY(void, Runtime1::throw_null_pointer_exception(JavaThread* thread))
  SharedRuntime::throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_NullPointerException());
JRT_END


JRT_ENTRY(void, Runtime1::throw_class_cast_exception(JavaThread* thread, oop object))
  ResourceMark rm(thread);
  Handle obj(thread, object);
  SharedRuntime::throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_ClassCastException(), Klass::cast(obj->klass())->external_name());
JRT_END


JRT_ENTRY(void, Runtime1::throw_incompatible_class_change_error(JavaThread* thread))
  ResourceMark rm(thread);
  SharedRuntime::throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_IncompatibleClassChangeError());
JRT_END


JRT_ENTRY_NO_ASYNC(void, Runtime1::monitorenter(JavaThread* thread, oop obj, BasicObjectLock* lock))
  Handle h_obj(thread, obj);
  assert(h_obj()->is_oop(), "must be NULL or an object");
  if (UseFastLocking) {
    // When using fast locking, the compiled code has already tried the fast case
    assert(obj == lock->obj(), "must match");
    ObjectSynchronizer::slow_enter(h_obj, lock->lock(), THREAD);
  } else {
    lock->set_obj(obj);
    ObjectSynchronizer::fast_enter(h_obj, lock->lock(), THREAD);
  }
JRT_END


JRT_LEAF(void, Runtime1::monitorexit(JavaThread* thread, BasicObjectLock* lock))
  assert(thread == JavaThread::current(), "threads must correspond");
  assert(thread->last_Java_sp(), "last_Java_sp must be set");
  // monitorexit is non-blocking (leaf routine) => no exceptions can be thrown
  EXCEPTION_MARK;

  oop obj = lock->obj();
  assert(obj->is_oop(), "must be NULL or an object");
  if (UseFastLocking) {
    // When using fast locking, the compiled code has already tried the fast case
    ObjectSynchronizer::slow_exit(obj, lock->lock(), THREAD);
  } else {
    ObjectSynchronizer::fast_exit(obj, lock->lock(), THREAD);
  }
JRT_END


static int resolve_field_return_offset(methodHandle caller, int bci, TRAPS) {
  Bytecode_field* field_access = Bytecode_field_at(caller(), caller->bcp_from(bci));
  // This can be static or non-static field access
  Bytecodes::Code code       = field_access->code();

  // We must load class, initialize class and resolvethe field
  FieldAccessInfo result; // initialize class if needed
  constantPoolHandle constants(THREAD, caller->constants());
  LinkResolver::resolve_field(result, constants, field_access->index(), Bytecodes::java_code(code), false, CHECK_0);
  return result.field_offset();
}


static klassOop resolve_field_return_klass(methodHandle caller, int bci, TRAPS) {
  Bytecode_field* field_access = Bytecode_field_at(caller(), caller->bcp_from(bci));
  // This can be static or non-static field access
  Bytecodes::Code code       = field_access->code();

  // We must load class, initialize class and resolvethe field
  FieldAccessInfo result; // initialize class if needed
  constantPoolHandle constants(THREAD, caller->constants());
  LinkResolver::resolve_field(result, constants, field_access->index(), Bytecodes::java_code(code), false, CHECK_0);
  return result.klass()();
}


//
// This routine patches sites where a class wasn't loaded or
// initialized at the time the code was generated.  It handles
// references to classes, fields and forcing of initialization.  Most
// of the cases are straightforward and involving simply forcing
// resolution of a class, rewriting the instruction stream with the
// needed constant and replacing the call in this function with the
// patched code.  The case for static field is more complicated since
// the thread which is in the process of initializing a class can
// access it's static fields but other threads can't so the code
// either has to deoptimize when this case is detected or execute a
// check that the current thread is the initializing thread.  The
// current
//
// Patches basically look like this:
//
//
// patch_site: jmp patch stub     ;; will be patched
// continue:   ...
//             ...
//             ...
//             ...
//
// They have a stub which looks like this:
//
//             ;; patch body
//             movl <const>, reg           (for class constants)
//        <or> movl [reg1 + <const>], reg  (for field offsets)
//        <or> movl reg, [reg1 + <const>]  (for field offsets)
//             <being_init offset> <bytes to copy> <bytes to skip>
// patch_stub: call Runtime1::patch_code (through a runtime stub)
//             jmp patch_site
//
// 
// A normal patch is done by rewriting the patch body, usually a move,
// and then copying it into place over top of the jmp instruction
// being careful to flush caches and doing it in an MP-safe way.  The
// constants following the patch body are used to find various pieces
// of the patch relative to the call site for Runtime1::patch_code.
// The case for getstatic and putstatic is more complicated because
// getstatic and putstatic have special semantics when executing while
// the class is being initialized.  getstatic/putstatic on a class
// which is being_initialized may be executed by the initializing
// thread but other threads have to block when they execute it.  This
// is accomplished in compiled code by executing a test of the current
// thread against the initializing thread of the class.  It's emitted
// as boilerplate in their stub which allows the patched code to be
// executed before it's copied back into the main body of the nmethod.
//
// being_init: get_thread(<tmp reg>
//             cmpl [reg1 + <init_thread_offset>], <tmp reg>
//             jne patch_stub
//             movl [reg1 + <const>], reg  (for field offsets)  <or>
//             movl reg, [reg1 + <const>]  (for field offsets)
//             jmp continue
//             <being_init offset> <bytes to copy> <bytes to skip>
// patch_stub: jmp Runtim1::patch_code (through a runtime stub)
//             jmp patch_site
//
// If the class is being initialized the patch body is rewritten and
// the patch site is rewritten to jump to being_init, instead of
// patch_stub.  Whenever this code is executed it checks the current
// thread against the intializing thread so other threads will enter
// the runtime and end up blocked waiting the class to finish
// initializing inside the calls to resolve_field below.  The
// initializing class will continue on it's way.  Once the class is
// fully_initialized, the intializing_thread of the class becomes
// NULL, so the next thread to execute this code will fail the test,
// call into patch_code and complete the patching process by copying
// the patch body back into the main part of the nmethod and resume
// executing.
//
//

JRT_ENTRY(static nmethod*, patch_code(JavaThread* thread, Runtime1::StubID stub_id ))

  ResourceMark rm(thread);
  RegisterMap reg_map(thread, false);
  frame runtime_frame = thread->last_frame();
  frame caller_frame = runtime_frame.sender(&reg_map);
  nmethod* caller_code = NULL;

  // last java frame on stack
  vframeStream vfst(thread, true);
  assert(!vfst.at_end(), "Java frame must exist");

  methodHandle caller_method(THREAD, vfst.method());
  // Note that caller_method->code() may not be same as caller_code because of OSR's
  // Note also that in the presence of inlining it is not guaranteed
  // that caller_method() == caller_code->method()
  caller_code = CodeCache::find_nmethod(caller_frame.pc());
  assert(caller_code != NULL, "nmethod not found");
  // make sure the nmethod doesn't get swept before we're done with it
  // not possible with lazy deopt
  // nmethodLocker locker(caller_code);


  int bci = vfst.bci();

  Events::log("patch_code @ " INTPTR_FORMAT , caller_frame.pc());

  Bytecodes::Code code = Bytecode_at(caller_method->bcp_from(bci))->java_code();

#ifndef PRODUCT
  // this is used by assertions in the init_check_patching_id
  BasicType patch_field_type = T_ILLEGAL;
#endif // PRODUCT
  int patch_field_offset = -1;
  KlassHandle init_klass(THREAD, klassOop(NULL)); // klass needed by init_check_patching code
  Handle load_klass(THREAD, NULL);                // oop needed by load_klass_patching code
  if (stub_id == Runtime1::init_check_patching_id) {

    Bytecode_field* field_access = Bytecode_field_at(caller_method(), caller_method->bcp_from(bci));
    FieldAccessInfo result; // initialize class if needed
    Bytecodes::Code code = field_access->code();
    constantPoolHandle constants(THREAD, caller_method->constants());
    LinkResolver::resolve_field(result, constants, field_access->index(), Bytecodes::java_code(code), false, CHECK_(caller_code));
    patch_field_offset = result.field_offset();
    init_klass = result.klass();
#ifndef PRODUCT
    patch_field_type = result.field_type();
#endif
  } else if (stub_id == Runtime1::load_klass_patching_id) {
    oop k;
    switch (code) {
      case Bytecodes::_putstatic:
      case Bytecodes::_getstatic:
        { k = resolve_field_return_klass(caller_method, bci, CHECK_(caller_code));
        }
        break;
      case Bytecodes::_new:
        { Bytecode_new* bnew = Bytecode_new_at(caller_method->bcp_from(bci));
          k = caller_method->constants()->klass_at(bnew->index(), CHECK_(caller_code));
        }
        break;
      case Bytecodes::_multianewarray:
        { Bytecode_multianewarray* mna = Bytecode_multianewarray_at(caller_method->bcp_from(bci));
          k = caller_method->constants()->klass_at(mna->index(), CHECK_(caller_code));
        }
        break;
      case Bytecodes::_instanceof:
        { Bytecode_instanceof* io = Bytecode_instanceof_at(caller_method->bcp_from(bci));
          k = caller_method->constants()->klass_at(io->index(), CHECK_(caller_code));
        }
        break;
      case Bytecodes::_checkcast:
        { Bytecode_checkcast* cc = Bytecode_checkcast_at(caller_method->bcp_from(bci));
          k = caller_method->constants()->klass_at(cc->index(), CHECK_(caller_code));
        }
        break;
      case Bytecodes::_anewarray:
        { Bytecode_anewarray* anew = Bytecode_anewarray_at(caller_method->bcp_from(bci));
          klassOop ek = caller_method->constants()->klass_at(anew->index(), CHECK_(caller_code));
          k = Klass::cast(ek)->array_klass(CHECK_(caller_code));
        }
        break;
      case Bytecodes::_ldc:
      case Bytecodes::_ldc_w:
        {
          Bytecode_loadconstant* cc = Bytecode_loadconstant_at(caller_method(),
                                                               caller_method->bcp_from(bci));
          klassOop resolved = caller_method->constants()->klass_at(cc->index(), CHECK_(caller_code));
          // ldc wants the java mirror.
          k = resolved->klass_part()->java_mirror();
        }
        break;
      default: Unimplemented();
    }
    // convert to handle
    load_klass = Handle(THREAD, k);
  } else {
    ShouldNotReachHere();
  }

  // Now copy code back

  {
    MutexLockerEx ml_patch (Patching_lock, Mutex::_no_safepoint_check_flag);
    //
    // Deoptimization may have happened while we waited for the lock.
    // In that case we don't bother to do any patching we just return
    // and let the deopt happen
    if (!caller_code->is_patched_for_deopt()) {
      NativeGeneralJump* jump = nativeGeneralJump_at(caller_frame.pc());
      address instr_pc = jump->jump_destination();
      NativeInstruction* ni = nativeInstruction_at(instr_pc);
      if (ni->is_jump() ) {
        // the jump has not been patched yet
        // The jump destination is slow case and therefore not part of the stubs
        // (stubs are only for StaticCalls)

        // format of buffer
        //    ....
        //    instr byte 0     <-- copy_buff
        //    instr byte 1
        //    ..
        //    instr byte n-1
        //      n
        //    ....             <-- call destination

        address stub_location = caller_frame.pc() + PatchingStub::patch_info_offset();
        unsigned char* byte_count = (unsigned char*) (stub_location - 1);
        unsigned char* byte_skip = (unsigned char*) (stub_location - 2);
        unsigned char* being_initialized_entry_offset = (unsigned char*) (stub_location - 3);
        address copy_buff = stub_location - *byte_skip - *byte_count;
        address being_initialized_entry = stub_location - *being_initialized_entry_offset;
        if (TracePatching) {
          tty->print_cr(" Patching %s at bci %d at address 0x%x  (%s)", Bytecodes::name(code), bci,
                        instr_pc, (stub_id == Runtime1::init_check_patching_id) ? "field" : "klass");
          OopMap* map = caller_code->oop_map_for_return_address(caller_frame.pc(), true);
          assert(map != NULL, "null check");
          map->print();
          tty->cr();
        }
        // depending on the code below, do_patch says whether to copy the patch body back into the nmethod
        bool do_patch = true;
        if (stub_id == Runtime1::init_check_patching_id) {
          do_patch = instanceKlass::cast(init_klass())->is_initialized() ||
            (code != Bytecodes::_getstatic && code != Bytecodes::_putstatic);
          NativeGeneralJump* jump = nativeGeneralJump_at(instr_pc);
          if (jump->jump_destination() == being_initialized_entry) {
            assert(do_patch == true, "initialization must be complete at this point");
          } else {
            // The offset may not be correct if the class was not loaded at code generation time.
            // Set it now.
            NativeMovRegMem* n_move = nativeMovRegMem_at(copy_buff);
            assert(n_move->offset() == 0 || (n_move->offset() == 4 && (patch_field_type == T_DOUBLE || patch_field_type == T_LONG)), "illegal offset for type");
            assert(patch_field_offset >= 0, "illegal offset");
            n_move->add_offset_in_bytes(patch_field_offset);
          }
        } else if (stub_id == Runtime1::load_klass_patching_id) {
          // patch the instruction <move reg, klass>
          NativeMovConstReg* n_copy = nativeMovConstReg_at(copy_buff);
          assert(n_copy->data() == 0, "illegal init value");
          assert(load_klass() != NULL, "klass not set");
          n_copy->set_data((intx) (load_klass()));

          // update relocInfo to oop
          nmethod* nm = CodeCache::find_nmethod(instr_pc);
          assert(nm != NULL, "invalid nmethod_pc");

          RelocIterator iter(nm, (address)instr_pc, (address)(instr_pc + 1));
          relocInfo::change_reloc_info_for_address(&iter, (address) instr_pc, relocInfo::none, relocInfo::oop_type);
  #ifdef SPARC
          { address instr_pc2 = instr_pc + NativeMovConstReg::add_offset;
            RelocIterator iter2(nm, instr_pc2, instr_pc2 + 1);
            relocInfo::change_reloc_info_for_address(&iter2, (address) instr_pc2, relocInfo::none, relocInfo::oop_type);
          }
  #endif
        } else {
          ShouldNotReachHere();
        }
        if (do_patch) {
          // replace instructions
          // first replace the tail, then the call
          for (int i = NativeCall::instruction_size; i < *byte_count; i++) {
            address ptr = copy_buff + i;
            int a_byte = (*ptr) & 0xFF;
            address dst = instr_pc + i;
            *(unsigned char*)dst = (unsigned char) a_byte;
          }
          ICache::invalidate_range(instr_pc, *byte_count);
          NativeGeneralJump::replace_mt_safe(instr_pc, copy_buff);
  #ifdef SPARC
          if (stub_id == Runtime1::load_klass_patching_id) {
            // Factor this out!
            // update relocInfo to oop
            nmethod* nm = CodeCache::find_nmethod(instr_pc);
            assert(nm != NULL, "invalid nmethod_pc");

            RelocIterator oops(nm, instr_pc, instr_pc + 1);
            bool found = false;
            while (oops.next() && !found) {
              if (oops.type() == relocInfo::oop_type) {
                oop_Relocation* r = oops.oop_reloc();
                oop* oop_adr = r->oop_addr();
                *oop_adr = load_klass();
                r->fix_oop_relocation();
                found = true;
              }
            }
            assert(found, "the oop must exist!");
          }
  #endif
        } else {
          ICache::invalidate_range(copy_buff, *byte_count);
          NativeGeneralJump::insert_unconditional(instr_pc, being_initialized_entry);
        }
      }
    }
  }
  return caller_code;
JRT_END

//
// Entry point for compiled code. We want to patch a nmethod.
// We don't do a normal VM transition here because we want to
// know after the patching is complete and any safepoint(s) are taken
// if the calling nmethod was deoptimized. We do this by calling a
// helper method which does the normal VM transition and when it
// completes we can check for deoptimization. This simplifies the
// assembly code in the cpu directories.
//
int Runtime1::move_klass_patching(JavaThread* thread) {
//
// NOTE: we are still in Java 
//
  Thread* THREAD = thread;
  debug_only(NoHandleMark nhm;)
  nmethod* caller_code = NULL;
  {
    // Enter VM mode

    ResetNoHandleMark rnhm;
    caller_code = patch_code(thread, load_klass_patching_id);
  }
  // Back in JAVA, use no oops DON'T safepoint

  // Return true if calling code is deoptimized

  return caller_code->is_patched_for_deopt();
}

//
// Entry point for compiled code. We want to patch a nmethod.
// We don't do a normal VM transition here because we want to
// know after the patching is complete and any safepoint(s) are taken
// if the calling nmethod was deoptimized. We do this by calling a
// helper method which does the normal VM transition and when it
// completes we can check for deoptimization. This simplifies the
// assembly code in the cpu directories.
//

int Runtime1::init_check_patching(JavaThread* thread) {
//
// NOTE: we are still in Java 
//
  Thread* THREAD = thread;
  debug_only(NoHandleMark nhm;)
  nmethod* caller_code = NULL;
  {
    // Enter VM mode

    ResetNoHandleMark rnhm;
    caller_code = patch_code(thread, init_check_patching_id);
  }
  // Back in JAVA, use no oops DON'T safepoint

  // Return true if calling code is deoptimized

  return caller_code->is_patched_for_deopt();
JRT_END


JRT_LEAF(void, Runtime1::trace_method_entry(jint v1, jint v2))
  tty->print_cr("Entering a method 0x%x 0x%x", v1, v2);
JRT_END


JRT_LEAF(void, Runtime1::trace_method_exit(jint value))
  Unimplemented();
JRT_END


JRT_LEAF(void, Runtime1::trace_block_entry(jint block_id))
  // for now we just print out the block id
  tty->print("%d ", block_id);
JRT_END


// fast and direct copy of arrays; returning -1, means that an exception may be thrown
// and we did not copy anything
JRT_LEAF(int, Runtime1::arraycopy(oop src, int src_pos, oop dst, int dst_pos, int length))
#ifndef PRODUCT
  _generic_arraycopy_cnt++;        // Slow-path oop array copy
#endif

  enum {
    ac_failed = -1, // arraycopy failed
    ac_ok = 0       // arraycopy succeeded
  };

  if (src == NULL || dst == NULL || src_pos < 0 || dst_pos < 0 || length < 0) return ac_failed;
  if (!dst->is_array() || !src->is_array()) return ac_failed;
  if ((unsigned int) arrayOop(src)->length() < (unsigned int)src_pos + (unsigned int)length) return ac_failed;
  if ((unsigned int) arrayOop(dst)->length() < (unsigned int)dst_pos + (unsigned int)length) return ac_failed;

  if (length == 0) return ac_ok;
  if (src->is_typeArray()) {
    const klassOop klass_oop = src->klass();
    if (klass_oop != dst->klass()) return ac_failed;
    typeArrayKlass* klass = typeArrayKlass::cast(klass_oop);
    const int sc  = klass->scale();
    const int ihs = klass->array_header_in_bytes() / wordSize;
    char* src_addr = (char*) ((oop*)src + ihs) + (src_pos * sc);
    char* dst_addr = (char*) ((oop*)dst + ihs) + (dst_pos * sc);
    // Potential problem: memmove is not guaranteed to be word atomic
    // Revisit in Merlin
    memmove(dst_addr, src_addr, length * sc);
    return ac_ok;
  } else {
    assert(src->is_objArray(), "what array type is it else?");
    oop* src_addr = objArrayOop(src)->obj_at_addr(src_pos);
    oop* dst_addr = objArrayOop(dst)->obj_at_addr(dst_pos);
    // For performance reasons, we assume we are using a card marking write
    // barrier. The assert will fail if this is not the case.
    // Note that we use the non-virtual inlineable variant of write_ref_array.
    BarrierSet* bs = Universe::heap()->barrier_set();
    if (src == dst) {
      // same object, no check
      Copy::conjoint_oops_atomic(src_addr, dst_addr, length);
      bs->write_ref_array(MemRegion((HeapWord*)dst_addr,
                                    (HeapWord*)(dst_addr + length)));
      return ac_ok;
    } else {
      klassOop bound = objArrayKlass::cast(dst->klass())->element_klass();
      klassOop stype = objArrayKlass::cast(src->klass())->element_klass();
      if (stype == bound || Klass::cast(stype)->is_subtype_of(bound)) {
        // Elements are guaranteed to be subtypes, so no check necessary
        Copy::conjoint_oops_atomic(src_addr, dst_addr, length);
        bs->write_ref_array(MemRegion((HeapWord*)dst_addr,
                                      (HeapWord*)(dst_addr + length)));
        return ac_ok;
      }
    }
  }
  return ac_failed;
JRT_END


JRT_LEAF(void, Runtime1::primitive_arraycopy(HeapWord* src, HeapWord* dst, int length))
#ifndef PRODUCT
  _primitive_arraycopy_cnt++;
#endif

  if (length == 0) return;
  // Not guaranteed to be word atomic, but that doesn't matter
  // for anything but an oop array, which is covered by oop_arraycopy.
  Copy::conjoint_bytes(src, dst, length);
JRT_END

JRT_LEAF(void, Runtime1::oop_arraycopy(HeapWord* src, HeapWord* dst, int num))
#ifndef PRODUCT
  _oop_arraycopy_cnt++;
#endif

  if (num == 0) return;
  Copy::conjoint_oops_atomic((oop*) src, (oop*) dst, num);
  BarrierSet* bs = Universe::heap()->barrier_set();
  bs->write_ref_array(MemRegion(dst, dst + num));
JRT_END

JRT_ENTRY(void, Runtime1::jvmpi_method_entry_after_deopt(JavaThread* thread, oop receiver))
  // Perform JVMPI method entry notification for a synchronized method activation
  // that was deoptimized after its monitorenter operation.  First, get the method:
  vframeStream vfst(thread, true);  // Do not skip and javaCalls
  assert(!vfst.at_end(), "Java frame must exist");
  methodHandle method (THREAD, vfst.method());
  if (method()->is_static()) {
    // clear the "receiver", which is really the method's class that was passed to monitorenter
    receiver = NULL;
  }
  SharedRuntime::jvmpi_method_entry_work(thread, method(), receiver);
JRT_END


#ifndef PRODUCT
void Runtime1::print_statistics() {
  tty->print_cr("C1 Runtime statistics:");
  tty->print_cr(" _resolve_invoke_virtual_cnt:     %d", SharedRuntime::_resolve_virtual_ctr);
  tty->print_cr(" _resolve_invoke_opt_virtual_cnt: %d", SharedRuntime::_resolve_opt_virtual_ctr);
  tty->print_cr(" _resolve_invoke_static_cnt:      %d", SharedRuntime::_resolve_static_ctr);
  tty->print_cr(" _handle_wrong_method_cnt:        %d", SharedRuntime::_wrong_method_ctr);
  tty->print_cr(" _ic_miss_cnt:                    %d", SharedRuntime::_ic_miss_ctr);
  tty->print_cr(" _generic_arraycopy_cnt:          %d", _generic_arraycopy_cnt);
  tty->print_cr(" _primitive_arraycopy_cnt:        %d", _primitive_arraycopy_cnt);
  tty->print_cr(" _oop_arraycopy_cnt:              %d", _oop_arraycopy_cnt);
  tty->print_cr(" _arraycopy_slowcase_cnt:         %d", _arraycopy_slowcase_cnt);
  SharedRuntime::print_ic_miss_histogram();
  tty->cr();
}
#endif // PRODUCT
