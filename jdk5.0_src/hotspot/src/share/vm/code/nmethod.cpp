#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)nmethod.cpp	1.318 04/04/20 16:28:32 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_nmethod.cpp.incl"


// The _unwind_handler is a special marker address, which says that
// for given exception oop and address, the frame should be removed
// as the tuple cannot be caught in the nmethod
address ExceptionCache::_unwind_handler = (address) -1;


ExceptionCache::ExceptionCache(Handle exception, address pc, address handler) {
  assert(pc != NULL, "Must be non null");
  assert(exception.not_null(), "Must be non null");
  assert(handler != NULL, "Must be non null");

  _count = 0;
  _exception_type = exception->klass();
  _next = NULL;

  add_address_and_handler(pc,handler);
}


address ExceptionCache::match(Handle exception, address pc) {
  assert(pc != NULL,"Must be non null");
  assert(exception.not_null(),"Must be non null");
  if (exception->klass() == exception_type()) {
    return (test_address(pc));
  }

  return NULL;
}


bool ExceptionCache::match_exception_with_space(Handle exception) {
  assert(exception.not_null(),"Must be non null");
  if (exception->klass() == exception_type() && count() < cache_size) {
    return true;
  }
  return false;
}


address ExceptionCache::test_address(address addr) {
  for (int i=0; i<count(); i++) {
    if (pc_at(i) == addr) {
      return handler_at(i);
    }
  }
  return NULL;
}


bool ExceptionCache::add_address_and_handler(address addr, address handler) {
  if (test_address(addr) == handler) return true;
  if (count() < cache_size) {
    set_pc_at(count(),addr);
    set_handler_at(count(), handler);
    increment_count();
    return true;
  }
  return false;
}


// private method for handling exception cache
// These methods are private, and used to manipulate the exception cache
// directly.
ExceptionCache* nmethod::exception_cache_entry_for_exception(Handle exception) {
  ExceptionCache* ec = exception_cache();
  while (ec != NULL) {
    if (ec->match_exception_with_space(exception)) {
      return ec;
    }
    ec = ec->next();
  }
  return NULL;
}


//-------------------------------------------------------------------------------

PcDescCache::PcDescCache() {
  for (int i = 0; i < cache_size; i++) _pc_descs[i] = NULL;
}


PcDesc* PcDescCache::pc_desc_at(nmethod* nm, address pc, bool at_call) const {
  for (int i = 0; i < cache_size; i++) {
    PcDesc* p = _pc_descs[i];
    if (p != NULL && p->real_pc(nm) == pc && p->at_call() == at_call) {
      return p;
    }
  }
  return NULL;
}

PcDesc* PcDescCache::pc_desc_at(nmethod* nm, address pc) const {
  for (int i = 0; i < cache_size; i++) {
    PcDesc* p = _pc_descs[i];
    if (p != NULL && p->real_pc(nm) == pc) {
      return p;
    }
  }
  return NULL;
}

void PcDescCache::add_pc_desc(PcDesc* pc_desc) {
  for (int i = cache_size -1 ; i > 0; i--) {
    // shift
    _pc_descs[i]  = _pc_descs[i-1];
  }
  _pc_descs[0] = pc_desc;
}

//-------------------------------------------------------------------------------




void nmethod::add_exception_cache_entry(ExceptionCache* new_entry) {
  assert(ExceptionCache_lock->owned_by_self(),"Must hold the ExceptionCache_lock");
  assert(new_entry != NULL,"Must be non null");
  assert(new_entry->next() == NULL, "Must be null");

  if (exception_cache() != NULL) {
    new_entry->set_next(exception_cache());
  }
  set_exception_cache(new_entry);
}


// public method for accessing the exception cache
// These are the public access methods.
address nmethod::handler_for_exception_and_pc(Handle exception, address pc) {
  // We never grab a lock to read the exception cache, so we may
  // have false negatives. This is okay, as it can only happen during
  // the first few exception lookups for a given nmethod.
  ExceptionCache* ec = exception_cache();
  while (ec != NULL) {
    address ret_val;
    if ((ret_val = ec->match(exception,pc)) != NULL) {
      return ret_val;
    }
    ec = ec->next();
  }
  return NULL;
}


void nmethod::add_handler_for_exception_and_pc(Handle exception, address pc, address handler) {
  // There are potential race conditions during exception cache updates, so we
  // must own the ExceptionCache_lock before doing ANY modifications. Because
  // we dont lock during reads, it is possible to have several threads attempt
  // to update the cache with the same data. We need to check for already inserted
  // copies of the current data before adding it.

  MutexLocker ml(ExceptionCache_lock);
  ExceptionCache* target_entry = exception_cache_entry_for_exception(exception);

  if (target_entry == NULL || !target_entry->add_address_and_handler(pc,handler)) {
    target_entry = new ExceptionCache(exception,pc,handler);
    add_exception_cache_entry(target_entry);
  }
}


//-------------end of code for ExceptionCache--------------


void nmFlags::clear() {
  assert(sizeof(nmFlags) == sizeof(int), "using more than one word for nmFlags");
  *(jint*)this = 0;
}

int nmethod::total_size() const {
  return 
    code_size()          +
    exception_size()     +
    stub_size()          +
    scopes_data_size()   +
    scopes_pcs_size()    +
    handler_table_size() +
    nul_chk_table_size();
}

const char* nmethod::compile_kind() const {
  if (is_native_method())  return "c2n";
  if (is_osr_method())     return "osr";
  return NULL;
}

// %%% This variable is no longer used?
int nmethod::_zombie_instruction_size = NativeJump::instruction_size;


nmethod* nmethod::new_nmethod(methodHandle method,
  int entry_bci,
  int iep_offset,
  int ep_offset,
  int vep_offset,
  int code_offset,
  int osr_offset,
  DebugInformationRecorder* recorder, 
  CodeBuffer *code_buffer, int frame_size, 
  OopMapSet* oop_maps, 
  ExceptionHandlerTable* handler_table, 
  ImplicitExceptionTable* nul_chk_table,
  ExceptionRangeTable* exception_range_table,
  AbstractCompiler* compiler
)
{
  // create nmethod
  nmethod* nm = NULL;
  { MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    int nmethod_size =
      allocation_size(code_buffer, sizeof(nmethod), recorder->oop_size())
      + round_to(recorder->pcs_size()          , oopSize)
#ifdef COMPILER2
      + round_to(handler_table->size_in_bytes(), oopSize)
#endif
#ifdef COMPILER1
      + round_to(exception_range_table->size_in_bytes(), oopSize)
#endif
      + round_to(nul_chk_table->size_in_bytes(), oopSize)
      + round_to(recorder->data_size()         , oopSize);
    nm = new (nmethod_size) nmethod(method(), nmethod_size, entry_bci,
                                    iep_offset, ep_offset, vep_offset, code_offset, osr_offset,
                                    recorder, code_buffer, frame_size, 
                                    oop_maps, 
                                    handler_table,
                                    nul_chk_table,
                                    exception_range_table,
                                    compiler );
    if (FastNMethodDependencies && nm != NULL) {
      // To make dependency checking during class loading fast, record 
      // the nmethod dependencies in the classes it is dependent on.
      // This allows the dependency checking code to simply walk the
      // class hierarchy above the loaded class, checking only nmethods 
      // which are dependent on those classes.  The slow way is to
      // check every nmethod for dependencies which makes it linear in 
      // the number of methods compiled.  For applications with a lot
      // classes the slow way is too slow.
      for (int index = nm->first_dependent(); index < nm->dependent_limit(); index += 2) {
        klassOop  klass  = klassOop(nm->oop_at(index));
        if (klass == NULL)  // ignore this entry - it is "extended" dependency
          continue;         // which is used only in evolution
        assert(klass->is_klass(), "type check");
        
        // record this nmethod as dependent on this klass
        instanceKlass::cast(klass)->add_dependent_nmethod(nm);
      }
    }
  }

  // All buffers in the CodeBuffer are allocated in the CodeCache. 
  // If the code buffer is created on each compile attempt
  // as in C2, then it must be freed.
  if ( code_buffer->auto_free_blob() && code_buffer->get_blob() ) 
    delete code_buffer;

  // verify nmethod
  debug_only(if (nm) nm->verify();) // might block

  // done
  return nm;
}


void* nmethod::operator new(size_t size, int nmethod_size) {  
  // Always leave some room in the CodeCache for I2C/C2I adapters  
  if (CodeCache::unallocated_capacity() < CodeCacheMinimumFreeSpace) return NULL;
  return CodeCache::allocate(nmethod_size);    
}


nmethod::nmethod(
  methodOop method,
  int nmethod_size,
  int entry_bci,
  int iep_offset,
  int ep_offset,
  int vep_offset,
  int code_offset,
  int osr_offset,  
  DebugInformationRecorder* recorder,
  CodeBuffer *code_buffer,
  int frame_size,
  OopMapSet* oop_maps,
  ExceptionHandlerTable* handler_table,
  ImplicitExceptionTable* nul_chk_table,
  ExceptionRangeTable* exception_range_table,
  AbstractCompiler* compiler
)
: CodeBlob("nmethod", code_buffer, sizeof(nmethod), nmethod_size, frame_size, oop_maps, recorder->oop_size())
{
  {
    debug_only(No_Safepoint_Verifier nsv;)
    assert_locked_or_safepoint(CodeCache_lock);

    NOT_PRODUCT(_has_debug_info = false; )
    _method                  = method;
    _entry_bci               = entry_bci;
    _link                    = NULL;
    _c2i_adapter             = NULL;
    _compiler                = compiler;
    _exception_offset        = instructions_offset() + code_buffer->exception_offset();
    _stub_offset             = instructions_offset() + (code_buffer->stub_begin() - code_buffer->code_begin());
    _scopes_data_offset      = data_offset();
    _scopes_pcs_offset       = _scopes_data_offset   + round_to(recorder->data_size         (), oopSize);
    _handler_table_offset    = _scopes_pcs_offset    + round_to(recorder->pcs_size          (), oopSize);
    
#ifdef COMPILER2
    _nul_chk_table_offset    = _handler_table_offset + round_to(handler_table->size_in_bytes(), oopSize);
#endif
#ifdef COMPILER1
    _nul_chk_table_offset    = _handler_table_offset + round_to(exception_range_table->size_in_bytes(), oopSize);
#endif
    _nmethod_end_offset      = _nul_chk_table_offset + round_to(nul_chk_table->size_in_bytes(), oopSize);
    _first_dependent         = recorder->first_dependent();
    _number_of_dependents    = recorder->number_of_dependents();
    _compile_id              = 0;  // default
 
#ifdef COMPILER1
    assert(UseFPConstTables || ep_offset == 0, "ep_offset must be NULL if we do not use FP constants tables");
#endif
    _entry_point             = instructions_begin() + ep_offset;
    _verified_entry_point    = instructions_begin() + vep_offset;
    _frame_start_offset      = code_offset;
    _osr_entry_point         = instructions_begin() + osr_offset;
    _exception_cache         = NULL;
    _cached_pcdesc0          = scopes_pcs_begin();

#ifdef COMPILER1
    _interpreter_entry_point = instructions_begin() + iep_offset;
#else
    _interpreter_entry_point = NULL;
#endif

    flags.clear();
    flags.state              = alive;
    _patched_for_deopt       = false;
    _markedForDeoptimization = 0;

    _unload_reported	     = false;		// jvmti state

    _lock_count = 0;
    _stack_traversal_mark    = 0;

    // Copy contents of ScopeDescRecorder to nmethod
    recorder->copy_to(this);

    resolve_JNIHandles();
    debug_only(check_store();)

    CodeCache::commit(this);
  
    VTune::create_nmethod(this);

#ifdef COMPILER2
    // Copy contents of ExceptionHandlerTable to nmethod
    handler_table->copy_to(this);
#endif
    nul_chk_table->copy_to(this);
    // we use the information of entry points to find out if a method is
    // static or non static

#ifdef COMPILER1
    exception_range_table->copy_to(this);
    assert(_method->is_static() == (_entry_point == _verified_entry_point), " entry points must be same for static methods and vice versa");
#endif
  }

  bool printmethod = (PrintNMethods && !method->is_native()) || (PrintNativeNMethods && method->is_native());
  if (printmethod || PrintDebugInfo || PrintRelocations || PrintDependencies || PrintExceptionHandlers) {
    print_nmethod(printmethod);
  }

  // Make sure all the entry points are correctly aligned for patching.
  NativeJump::check_verified_entry_alignment(entry_point(), verified_entry_point());

  // Note: Do not verify in here as the CodeCache_lock is
  //       taken which would conflict with the CompiledIC_lock
  //       which taken during the verification of call sites.
  //       (was bug - gri 10/25/99)

  Events::log("Create nmethod " INTPTR_FORMAT, this);
}


#ifndef PRODUCT
void nmethod::print_nmethod(bool printmethod) {
  ttyLocker ttyl;  // keep the following output all in one block
  // This output goes directly to the tty, not the compiler log.
  // To enable tools to match it up with the compilation activity,
  // be sure to tag this tty output with the compile ID.
  if (xtty != NULL) {
    xtty->begin_head("print_nmethod compile_id='%d'", compile_id());
    const char* nm_kind = compile_kind();
    if (nm_kind != NULL)  xtty->print(" compile_kind='%s'", nm_kind);
    xtty->method(_method);
    xtty->stamp();
    xtty->end_head(" address='" INTPTR_FORMAT "'", (intptr_t) this);
  }
  // print the header part first
  print();
  // then print the requested information
  if (printmethod) {
    print_code();
    print_pcs();
    oop_maps()->print();
  }
  if (PrintDebugInfo) {
    print_scopes();
  }
  if (PrintRelocations) {
    print_relocations();
  }
  if (PrintDependencies) {
    print_dependencies();
  }
  if (PrintExceptionHandlers) {
    print_handler_table();
    print_nul_chk_table();
  }
  if (xtty != NULL) {
    xtty->tail("print_nmethod");
  }
}
#endif


void nmethod::set_version(int v) {
  flags.version = v;
}

ScopeDesc* nmethod::scope_desc_at(address pc, bool at_call) {
  PcDesc* pd = pc_desc_at(pc, at_call);
  guarantee(pd != NULL, "scope must be present");
  return new ScopeDesc(this, pd->scope_decode_offset());
}


ScopeDesc* nmethod::scope_desc_at(address pc) {
  PcDesc* pd = pc_desc_at(pc);
  guarantee(pd != NULL, "scope must be present");
  return new ScopeDesc(this, pd->scope_decode_offset());
}


void nmethod::clear_inline_caches() {  
  assert(SafepointSynchronize::is_at_safepoint(), "cleaning of IC's only allowed at safepoint");
  if (is_zombie()) {
    return;
  }

  // When a nmethod is 1st deoptimized it's inline caches are cleaned. They
  // should not be able to be updated after that (we assert in the patching
  // code) so we just skip out here.
  // Note: we can't even check the caches here if nmethod has been deopt because
  // the call instructions have indeterminate instructions in them and the
  // IC constructors will assert and die if they don't see what they expect.
  if (is_patched_for_deopt()) return;

  RelocIterator iter(this);
  while (iter.next()) {
    iter.reloc()->clear_inline_cache();
  }
}


void nmethod::cleanup_inline_caches() {

  assert(SafepointSynchronize::is_at_safepoint() && 
        !CompiledIC_lock->is_locked() &&
        !Patching_lock->is_locked(), "no threads must be updating the inline caches by them selfs");

  // If the method is not entrant or zombie then a JMP is plastered over the
  // first few bytes.  If an oop in the old code was there, that oop
  // should not get GC'd.  Skip the first few bytes of oops on
  // not-entrant methods.
  address low_boundary = verified_entry_point();
  if (!is_in_use()) {
    low_boundary += NativeJump::instruction_size;
    // %%% Note:  On SPARC we patch only a 4-byte trap, not a full NativeJump.
    // This means that the low_boundary is going to be a little too high.
    // This shouldn't matter, since oops of non-entrant methods are never used.
    // In fact, why are we bothering to look at oops in a non-entrant method??
  }

  // When a nmethod is 1st deoptimized it's inline caches are cleaned. They
  // should not be able to be updated after that (we assert in the patching
  // code) so we just skip out here.
  // Note: we can't even check the caches here if nmethod has been deopt because
  // the call instructions have indeterminate instructions in them and the
  // IC constructors will assert and die if they don't see what they expect.

  if (is_patched_for_deopt()) return;

  // Find all calls in an nmethod, and clear the ones that points to zombie methods
  ResourceMark rm;
  RelocIterator iter(this, low_boundary);
  while(iter.next()) {
    switch(iter.type()) {
      case relocInfo::virtual_call_type:
      case relocInfo::opt_virtual_call_type: {  
	CompiledIC *ic = CompiledIC_at(iter.reloc());
	// Ok, to lookup references to zombies here
	CodeBlob *cb = CodeCache::find_blob_unsafe(ic->ic_destination());
	if( cb != NULL && cb->is_nmethod() ) {
	  nmethod* nm = (nmethod*)cb;
	  // Clean inline caches pointing to both zombie and not_entrant methods
	  if (!nm->is_in_use()) ic->set_to_clean();
	}                                             
        break;
      }
      case relocInfo::static_call_type: {        
	CompiledStaticCall *csc = compiledStaticCall_at(iter.reloc());
	CodeBlob *cb = CodeCache::find_blob_unsafe(csc->destination());
	if( cb != NULL && cb->is_nmethod() ) {
	  nmethod* nm = (nmethod*)cb;
	  // Clean inline caches pointing to both zombie and not_entrant methods
	  if (!nm->is_in_use()) csc->set_to_clean();
	}                                             
        break;
      }
    }    
  }
}

void nmethod::mark_as_seen_on_stack() {
  assert(is_not_entrant(), "must be a non-entrant method");
  set_stack_traversal_mark(NMethodSweeper::traversal_count());
}

// Tell if a non-entrant method can be converted to a zombie (i.e., there is no activations on the stack)
bool nmethod::can_not_entrant_be_converted() {
  assert(is_not_entrant(), "must be a non-entrant method");
  assert(SafepointSynchronize::is_at_safepoint(), "must be called during a safepoint");
  
  // Since the nmethod sweeper only does partial sweep the sweeper's traversal
  // count can be greater than the stack traversal count before it hits the
  // nmethod for the second time.
  return stack_traversal_mark()+1 < NMethodSweeper::traversal_count();
}

// Make a class unloaded - i.e., change state and notify sweeper
void nmethod::make_unloaded() { 
  check_safepoint();
  flags.state = unloaded;

  // The methodOop is gone at this point
  assert(_method == NULL, "method field should have been cleared by follow_roots_or_mark_for_unloading" );

  set_link(NULL);  
  NMethodSweeper::notify(this);  
}


void nmethod::mark_for_unloading(BoolObjectClosure* is_alive) {
  flags.markedForUnloading = 1;

  // Since this nmethod is being unloaded, make sure that dependencies
  // recorded in instanceKlasses get flushed and pass non-NULL closure to 
  // indicate that this work is being done during a GC.
  assert(Universe::heap()->is_gc_active(), "should only be called during gc");
  assert(is_alive != NULL, "Should be non-NULL");
  // A non-NULL is_alive closure indicates that this is being called during GC.
  flush_dependencies(is_alive);
}


static void inc_decompile_count(nmethod* self) {
#ifdef COMPILER2
  // Could be gated by ProfileTraps, but do not bother...
  methodOop m = self->method();
  if (m == NULL)  return;
  methodDataOop mdo = m->method_data();
  if (mdo == NULL)  return;
  // There is a benign race here.  See comments in methodDataOop.hpp.
  mdo->inc_decompile_count();
#endif
}


void nmethod::invalidate_osr_method() { 
  assert(_entry_bci != InvocationEntryBci, "wrong kind of nmethod");   
  if (_entry_bci != InvalidOSREntryBci)
    inc_decompile_count(this);
  // Remove from list of active nmethods
  if (method() != NULL) 
    instanceKlass::cast(method()->method_holder())->remove_osr_nmethod(this);
  // Set entry as invalid
  _entry_bci = InvalidOSREntryBci;   
}

// Common functionality for both make_not_entrant and make_zombie
void nmethod::make_not_entrant_or_zombie(int state) {
  assert(state == zombie || state == not_entrant, "must be zombie or not_entrant");

  if ((((LogCompilation || TraceDeoptimization) && state == not_entrant)
       || TraceCreateZombies)
      && (xtty != NULL)) {
    HandleMark hm;
    ttyLocker ttyl;
    xtty->begin_elem("make_not_entrant%s thread='%d' compile_id='" UINTX_FORMAT "'",
                     (state == zombie ? " zombie='1'" : ""),
                     os::current_thread_id(), compile_id());
    const char* nm_kind = compile_kind();
    if (nm_kind != NULL)  xtty->print(" compile_kind='%s'", nm_kind);
    xtty->method(method());
    xtty->stamp();
    xtty->end_elem();
  }

  // Code for an on-stack-replacement nmethod is removed when a class gets unloaded.
  // They never become zombie/non-entrant, so the nmethod sweeper will never remove
  // them. Instead the entry_bci is set to InvalidOSREntryBci, so the osr nmethod
  // will never be used anymore. That the nmethods only gets removed when class unloading
  // happens, make life much simpler, since the nmethods are not just going to disappear
  // out of the blue.
  if (is_osr_only_method()) {
    invalidate_osr_method();  
    return;
  }

  // If the method is already zombie or set to the state we want, nothing to do
  if (is_zombie() || (state == not_entrant && is_not_entrant())) {
    return;
  }

  // Make sure the nmethod is not flushed in case of a safepoint in code below.
  nmethodLocker nml(this);

  {
    // Enter critical section.  Does not block for safepoint.
    MutexLockerEx pl(Patching_lock, Mutex::_no_safepoint_check_flag);
    // The caller can be calling the method statically or through an inline
    // cache call.
    // We check is_patched_for_deopt because even though the nmethod goes
    // non-entrant when deoptimized it can later be marked unloaded
    // and once a nmethod is deoptimized we don't want an spurious updates
    // to it and even though the updates here are harmless they would prevent
    // us from asserting to catch spurious and dangerous patching of an nmethod.
    //
    if (!is_not_entrant() && !is_patched_for_deopt()) {
#ifdef COMPILER1
      // Note: we cannot ask method()->is_static() here because nmethod::_method
      // is set to NULL when nmethod is marked as unloaded 
      if (entry_point() == verified_entry_point()) {
        // methodOop was static
        NativeJump::patch_verified_entry(entry_point(), verified_entry_point(),
                  Runtime1::entry_for(Runtime1::handle_wrong_static_method_id));
      } else {
        // methodOop was not static
        NativeJump::patch_verified_entry(entry_point(), verified_entry_point(), 
                  Runtime1::entry_for(Runtime1::handle_wrong_method_id));
      }  
#else
      NativeJump::patch_verified_entry(entry_point(), verified_entry_point(),
                  OptoRuntime::handle_wrong_method_stub());
#endif
      assert (NativeJump::instruction_size == nmethod::_zombie_instruction_size, "");      
    }
  
    if (FastNMethodDependencies) {
      // When the nmethod becomes zombie it is no longer alive so the
      // dependencies must be flushed.  nmethods in the not_entrant
      // state will be flushed later when the transition to zombie
      // happens or they get unloaded.
      if (state == zombie) {
        assert(SafepointSynchronize::is_at_safepoint(), "must be done at safepoint");
        flush_dependencies(NULL);
      } else {
        assert(state == not_entrant, "other cases may need to be handled differently");
      }
    }
  
    // Change state
    flags.state = state;
  } // leave critical region under Patching_lock
    
  if (state == not_entrant) {
    Events::log("Make nmethod not entrant " INTPTR_FORMAT, this);
  } else {
    Events::log("Make nmethod zombie " INTPTR_FORMAT, this);    
  }

  if (TraceCreateZombies) {    
    tty->print_cr("nmethod <" INTPTR_FORMAT "> code made %s", this, (state == not_entrant) ? "not entrant" : "zombie");
  }
  
  // Make sweeper aware that there is a zombie method that needs to be removed  
  NMethodSweeper::notify(this);  

  // not_entrant only stuff
  if (state == not_entrant) {    
    mark_as_seen_on_stack();
  }

  // It's a true state change, so mark the method as decompiled.
  inc_decompile_count(this);

  // zombie only - if a JVMTI agent has enabled the CompiledMethodUnload event
  // and it hasn't already been reported for this nmethod then report it now.
  // (the event may have been reported earilier if the GC marked it for unloading).
  if (state == zombie && JvmtiExport::should_post_compiled_method_unload() && !unload_reported()) {
    assert(method() != NULL, "checking");
    {
      HandleMark hm;
      JvmtiExport::post_compiled_method_unload_at_safepoint(method()->jmethod_id(), code_begin());    
    }
    set_unload_reported();    
  }


  // Zombie only stuff
  if (state == zombie) {    
    VTune::delete_nmethod(this);
  }

  // Check whether method got unloaded at a safepoint before this,
  // if so we can skip the flushing steps below
  if (method() == NULL) return;

  // Remove nmethod from method.
  // We need to check if both the _code and _from_compiled_code_entry_point
  // refer to this nmethod because there is a race in setting these two fields
  // in methodOop as seen in bugid 4947125.
  // If the vep() points to the zombie nmethod, the memory for the nmethod
  // could be flushed and the compiler and vtable stubs could still call
  // through it.
  if (method()->code() == this || 
      method()->from_compiled_code_entry_point() == verified_entry_point()) {
    HandleMark hm;
    method()->set_code(NULL);
    method()->invocation_counter()->reset();    
    method()->backedge_counter()->reset();
  }
}


#ifndef PRODUCT
void nmethod::check_safepoint() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");
}
#endif

 
void nmethod::flush() {
  // Note that there are no valid oops in the nmethod anymore.
  assert(is_zombie() || (is_osr_method() && is_unloaded()), "must be a zombie method");
  assert(is_marked_for_reclamation() || (is_osr_method() && is_unloaded()), "must be marked for reclamation");  

  assert (!is_locked_by_vm(), "locked methods shouldn't be flushed");
  check_safepoint();

  // completely deallocate this method
  EventMark m("flushing nmethod " INTPTR_FORMAT " %s", this, "");
  if (PrintMethodFlushing) {
    tty->print_cr("*flushing nmethod " INTPTR_FORMAT ". Live blobs: %d", this, CodeCache::nof_blobs());
  }

  // We need to deallocate any ExceptionCache data.
  // Note that we do not need to grab the nmethod lock for this, it
  // better be thread safe if we're disposing of it!
  ExceptionCache* ec = exception_cache();
  set_exception_cache(NULL);
  while(ec != NULL) {
    ExceptionCache* next = ec->next();
    delete ec;
    ec = next;
  }

  ((CodeBlob*)(this))->flush();

  CodeCache::free(this);
}


//
// Notify all classes this nmethod is dependent on that it is no
// longer dependent. This should only be called in two situations.
// First, when a nmethod transitions to a zombie all dependents need
// to be clear.  Since zombification happens at a safepoint there's no
// synchronization issues.  The second place is a little more tricky.
// During phase 1 of mark sweep class unloading may happen and as a
// result some nmethods may get unloaded.  In this case the flushing
// of dependencies must happen during phase 1 since after GC any
// dependencies in the unloaded nmethod won't be updated, so
// traversing the dependency information in unsafe.  In that case this
// function is called with a non-NULL argument and this function only 
// notifies instanceKlasses that are reachable

void nmethod::flush_dependencies(BoolObjectClosure* is_alive) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be done at safepoint");
  assert(Universe::heap()->is_gc_active() == (is_alive != NULL), 
  "is_alive is non-NULL if and only if we are called during GC");
  if (FastNMethodDependencies && !has_flushed_dependencies()) {
    set_has_flushed_dependencies();
    for (int index = first_dependent(); index < dependent_limit(); index += 2) {
      klassOop  klass  = klassOop(oop_at(index));
      if (klass == NULL)  // ignore this entry - it is "extended" dependency,
        continue;         // which is used only in evolution
      assert(klass->is_klass(), "type check");

      // During GC the is_alive closure is non-NULL, and is used to 
      // determine liveness of dependees that need to be updated.
      if (is_alive == NULL || is_alive->do_object_b(klass)) {
        instanceKlass::cast(klass)->remove_dependent_nmethod(this);
      }
    }
  }
}


// Follow individual root in nmethod, and mark nmethod for unloading if this root
// is the only thing keeping the referred object alive

void nmethod::follow_root_or_mark_for_unloading(
  BoolObjectClosure* is_alive, OopClosure* keep_alive,
  oop* root, bool unloading_occurred, bool& marked_for_unloading) {
  assert(root != NULL, "just checking");
  oop obj = *root;
  if (obj != NULL) {
    if (obj->being_unloaded(is_alive)) {
      assert(unloading_occurred,
             "should not reach here if no classes got unloaded");
      // We should not keep this object alive
      mark_for_unloading(is_alive);
      *root = NULL;
      marked_for_unloading = true;
    } else {
      // $$$ DLD: why should this be necessary?  Is this again to
      // ensure a full enumeration of the roots?
      keep_alive->do_oop(root);
    }
  }
}

void nmethod::remove_from_exception_cache(ExceptionCache* ec) {
  ExceptionCache* prev = NULL;
  ExceptionCache* curr = exception_cache();
  assert(curr != NULL, "nothing to remove");
  // find the previous and next entry of ec
  while (curr != ec) {
    prev = curr;
    curr = curr->next();
    assert(curr != NULL, "ExceptionCache not found");
  }
  // now: curr == ec
  ExceptionCache* next = curr->next();
  if (prev == NULL) {
    set_exception_cache(next);
  } else {
    prev->set_next(next);
  }
  delete curr;
}

void nmethod::post_compiled_method_unload(BoolObjectClosure* is_alive) {
  if (_method != NULL && !is_marked_for_unloading() && _method->being_unloaded(is_alive)) {
    if (jvmpi::is_event_enabled(JVMPI_EVENT_COMPILED_METHOD_UNLOAD)) {
      jvmpi::post_compiled_method_unload_event(_method);
    }

    // If a JVMTI agent has enabled the CompiledMethodUnload event then 
    // post the event. Sometime later this nmethod will be made a zombie by
    // the sweeper but the methodOop will not be valid at that point.
    if (JvmtiExport::should_post_compiled_method_unload()) {
      assert(!unload_reported(), "already unloaded");
      HandleMark hm;
      JvmtiExport::post_compiled_method_unload_at_safepoint(method()->jmethod_id(), code_begin());
    }

    // The JVMTI CompiledMethodUnload event can be enabled or disabled at
    // any time. As the nmethod is being unloaded now we mark it has 
    // having the unload event reported - this will ensure that we don't
    // attempt to report the event in the unlikely scenario where the 
    // event is enabled at the time the nmethod is made a zombie.
    set_unload_reported();
  }
}

// This is called at the end of MarkSweep::phase1. Follow all roots in 
// in this nmethod, unless they are the only ones keeping the class 
// alive. In that case, NULL out the root, mark the this nmethod for 
// unloading and set the boolean flag marked_for_unloading to true.

void nmethod::follow_roots_or_mark_for_unloading(
  BoolObjectClosure* is_alive, OopClosure* keep_alive,
  bool unloading_occurred, bool& marked_for_unloading) {
  // make sure the oops ready to receive visitors
  assert(!is_zombie() && !is_marked_for_unloading(),
         "should not call follow on zombie nmethod or one that"
         " it marked for unloading");

  // If the method is not entrant then a JMP is plastered over the
  // first few bytes.  If an oop in the old code was there, that oop
  // should not get GC'd.  Skip the first few bytes of oops on
  // not-entrant methods.
  address low_boundary = verified_entry_point();
  if (is_not_entrant()) {
    low_boundary += NativeJump::instruction_size;
    // %%% Note:  On SPARC we patch only a 4-byte trap, not a full NativeJump.
    // (See comment above.)
  }

  if (unloading_occurred) {
    post_compiled_method_unload(is_alive);
  }

  // Follow methodOop
  follow_root_or_mark_for_unloading(
    is_alive, keep_alive,
    (oop*) &_method, unloading_occurred, marked_for_unloading);
  
  // Exception cache
  ExceptionCache* ec = exception_cache();
  while (ec != NULL) {
    oop* ex_addr = (oop*)ec->exception_type_addr();
    oop ex = *ex_addr;
    ExceptionCache* next_ec = ec->next();
    if (ex != NULL && ex->being_unloaded(is_alive)) {
      remove_from_exception_cache(ec);
    } else {
      keep_alive->do_oop(ex_addr);
    }
    ec = next_ec;
  }

  // If nmethod is patched for deopt then oops embedded in the code are dead

  if (!is_patched_for_deopt()) {
    // If class unloading occurred we first iterate over all inline caches and clear ICs where
    // the cached oop is referring to an unloaded klass or method. The remaining live cached 
    // oops will be followed in the relocInfo::oop_type iteration below.
    if (unloading_occurred) {
      RelocIterator iter(this, low_boundary);
      while(iter.next()) {
        if (iter.type() == relocInfo::virtual_call_type) {
          CompiledIC *ic = CompiledIC_at(iter.reloc());
          oop ic_oop = ic->cached_oop();
          if (ic_oop != NULL && ic_oop->being_unloaded(is_alive)) {
            ic->set_to_clean();
            assert(ic->cached_oop() == NULL, "cached oop in IC should be cleared")
          }
        }
      }
    }

    // Compiled code
    RelocIterator iter(this, low_boundary);
    while (iter.next()) {
      if (iter.type() == relocInfo::oop_type) {
        oop_Relocation* r = iter.oop_reloc();
        // In this loop, we must only follow those oops directly embedded in
        // the code.  Other oops (oop_index>0) are seen as part of scopes_oops.
        assert(1 == (r->oop_index() == 0) + (r->oop_addr() >= oops_begin() && r->oop_addr() < oops_end()), "oop must be found in exactly one place");
	if (r->oop_index() == 0 && r->oop_value() != NULL) {
	  follow_root_or_mark_for_unloading(is_alive, keep_alive,
	    r->oop_addr(), unloading_occurred, marked_for_unloading);
	}      
      }
    }
  }

  // Scopes
  for (oop* p = oops_begin(); p < oops_end(); p++) {
    if (*p == Universe::non_oop_word())  continue;  // skip non-oops
    follow_root_or_mark_for_unloading(
      is_alive, keep_alive,
      p, unloading_occurred, marked_for_unloading);
  }

  // Break cycle between nmethod & method
  if( is_marked_for_unloading() ) {
    if (TraceClassUnloading && WizardMode) {
      tty->print_cr("[Class unloading: Marking nmethod " INTPTR_FORMAT " for unloading], methodOop " INTPTR_FORMAT, this, _method);
    }
    // If _method is already NULL the methodOop is about to be unloaded, 
    // so we don't have to break the cycle. Note that it is possible to
    // have the methodOop live here, in case we unload the nmethod because
    // it is pointing to some oop (other than the methodOop) being unloaded.
    if (_method != NULL) {
      // OSR methods point to the methodOop, but the methodOop does not
      // point back!
      if (_method->code() == this) {
	_method->set_code(NULL); // Break a cycle
      }
      inc_decompile_count(this); // Last chance to make a mark on the MDO
      _method = NULL;		 // Clear the method of this dead nmethod
    }
  }
}

void nmethod::oops_do(OopClosure* f) {
  // make sure the oops ready to receive visitors
  assert(!is_zombie(), "should not call follow on zombie nmethod");

  // If the method is not entrant or zombie then a JMP is plastered over the
  // first few bytes.  If an oop in the old code was there, that oop
  // should not get GC'd.  Skip the first few bytes of oops on
  // not-entrant methods.
  address low_boundary = verified_entry_point();
  if (is_not_entrant()) {
    low_boundary += NativeJump::instruction_size;
    // %%% Note:  On SPARC we patch only a 4-byte trap, not a full NativeJump.
    // (See comment above.)
  }

  // Compiled code
  f->do_oop((oop*) &_method);
  ExceptionCache* ec = exception_cache();
  while(ec != NULL) {
    f->do_oop((oop*)ec->exception_type_addr());
    ec = ec->next();
  }

  // If nmethod is patched then all embedded oops are now dead
  if (!is_patched_for_deopt()) {

    RelocIterator iter(this, low_boundary);
    while (iter.next()) {
      if (iter.type() == relocInfo::oop_type ) {
	oop_Relocation* r = iter.oop_reloc();      
	// In this loop, we must only follow those oops directly embedded in
	// the code.  Other oops (oop_index>0) are seen as part of scopes_oops.
	assert(1 == (r->oop_index() == 0) + (r->oop_addr() >= oops_begin() && r->oop_addr() < oops_end()), "oop must be found in exactly one place");
	if (r->oop_index() == 0 && r->oop_value() != NULL) {
	  f->do_oop(r->oop_addr());
	}      
      }
    }
  }

  // Scopes
  for (oop* p = oops_begin(); p < oops_end(); p++) {
    if (*p == Universe::non_oop_word())  continue;  // skip non-oops
    f->do_oop(p);
  }
}

// Method that knows how to preserve outgoing arguments at call. This method must be
// called with a frame corresponding to a Java invoke
void nmethod::preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map, OopClosure* f) {  
  if (!method()->is_native() && reg_map->is_pc_at_call(fr.id())) {
    SimpleScopeDesc ssd(this,fr.pc(),true);
    Bytecode_invoke* call = Bytecode_invoke_at(ssd.method(), ssd.bci());
    bool is_static = call->is_invokestatic();
    symbolOop signature = call->signature();
    fr.oops_compiled_arguments_do(signature, is_static, reg_map, f);
  }
}


oop nmethod::embeddedOop_at(u_char* p) {
  RelocIterator iter(this, p, p + oopSize);
  assert(!is_patched_for_deopt(), "oop is dead!");
  while (iter.next())
    if (iter.type() == relocInfo::oop_type) {
      return iter.oop_reloc()->oop_value();
    }
  return NULL;
}


inline bool includes(void* p, void* from, void* to) {
  return from <= p && p < to;
}


void nmethod::copy_pc_at(int index, PcDesc* pc) {
  // must be sorted and unique; we do a binary search in pc_desc_at()
  assert(index >= 0, "must be positive");
  assert(index == 0 ||
         scopes_pcs_begin()[index-1].pc_offset() < pc->pc_offset() ||
         (scopes_pcs_begin()[index-1].pc_offset() == pc->pc_offset() &&
          scopes_pcs_begin()[index-1].at_call() &&
          !pc->at_call()),
         "PcDesc's must be sorted");
  scopes_pcs_begin()[index] = *pc;
}

void nmethod::copy_scopes_data(u_char* buffer, int size) {
  memcpy(scopes_data_begin(), buffer, size);
}


// Finds a PcDesc with real-pc equal to "pc"
PcDesc* nmethod::pc_desc_at(address pc, bool at_call) {
  // First step: check if the last PcDesc found is the one that we need
  // In order to prevent race conditions do not dereference _cached_pcdesc0,
  // but use a (unmutable) copy of _cached_pcdesc0
  PcDesc* cached_pcdesc = _cached_pcdesc0;
  if ( cached_pcdesc->real_pc(this) == pc && cached_pcdesc->at_call() == at_call) {
      return cached_pcdesc;
  }
  
  // Second step: check the PcDesc cache if it contains the desired PcDesc (almost 100% hit rate)
  { PcDesc* pp = _pc_desc_cache.pc_desc_at(this, pc, at_call);
    if (pp != NULL) {
      _cached_pcdesc0 = pp;
      return pp;
    }
  }
  
  // Third step: linear search for the PcDesc
  for (PcDesc* p = scopes_pcs_begin(); p < scopes_pcs_end(); p++) {
    if ((p->real_pc(this) == pc) && (p->at_call() == at_call)) {
      _pc_desc_cache.add_pc_desc(_cached_pcdesc0);
      _cached_pcdesc0 = p;
      return p;
    }
  }

  return NULL;
}


PcDesc* nmethod::pc_desc_at(address pc) {
  // First step: check if the last PcDesc found is the one that we need
  // In order to prevent race conditions do not dereference _cached_pcdesc0,
  // but use a (unmutable) copy of _cached_pcdesc0
  PcDesc* cached_pcdesc = _cached_pcdesc0;
  if (cached_pcdesc->real_pc(this) == pc) {
      return cached_pcdesc;
  }
  
  // Second step: check the PcDesc cache if it contains the desired PcDesc (almost 100% hit rate)
  { PcDesc* pp = _pc_desc_cache.pc_desc_at(this, pc);
    if (pp != NULL) {
      _cached_pcdesc0 = pp;
      return pp;
    }
  }
  
  // Third step: linear search for the PcDesc
  for (PcDesc* p = scopes_pcs_begin(); p < scopes_pcs_end(); p++) {
    if (p->real_pc(this) == pc) {
      _pc_desc_cache.add_pc_desc(_cached_pcdesc0);
      _cached_pcdesc0 = p;
      return p;
    }
  }
  return NULL;
}

bool nmethod::is_dependent_on_entry(klassOop dependee, klassOop klass, methodOop method) {
  // check if klass is involved at all
  if (!instanceKlass::cast(klass)->is_marked_dependent()) return false;
  // If we have no method, then this is a case where we optimized on 
  // 'dependee' having no subklasses, i.e., a instance-of or check-cast
  // check uses a simple pointer compare instead of the subtype-cache.
  if( !method ) return true;
  methodOop overriding = instanceKlass::cast(dependee)->find_method(method->name(), method->signature());
  return overriding != NULL;
}

bool nmethod::is_dependent_on(klassOop dependee) {
  if (number_of_dependents() == 0 ) return false;
  // What has happened:
  // 1) a new class dependee has been added
  // 2) dependee and all its super classes have been marked
  for (int index = first_dependent(); index < dependent_limit(); index += 2) {
    klassOop  klass  = klassOop(oop_at(index));
    if (klass == NULL)  // ignore this entry - it is "extended" dependency,
      continue;         // which is used only in evolution or fast breakpoints
    assert(klass->is_klass(), "type check");

    // if we are dependent on an interface and
    // the interface has more than 1 implementors we should deoptimize
    if (Klass::cast(klass)->is_interface()
        && instanceKlass::cast(klass)->nof_implementors() > 1) return true;

    methodOop method = methodOop(oop_at(index+1));
    assert( !method || method->is_method(), "type check" );
  
    if (is_dependent_on_entry(dependee, klass, method)) 
      return true;
  }
  return false;
}

bool nmethod::is_evol_dependent_on(klassOop dependee) {
  if (number_of_dependents() == 0) return false;
  instanceKlass *dependee_ik = instanceKlass::cast(dependee);
  objArrayOop dependee_methods = dependee_ik->methods();

  for (int index = first_dependent()+1; index < dependent_limit(); index += 2) {
    methodOop method = methodOop(oop_at(index));
    if (method == NULL)
      continue;
    for (int j = 0; j < dependee_methods->length(); j++) {
      if ((methodOop) dependee_methods->obj_at(j) == method) {
	if (PrintHotSwap)
	  tty->print_cr("Found evol dependency of nmethod %s.%s on method %s.%s",
			_method->method_holder()->klass_part()->external_name(),
			_method->name()->as_C_string(),
			method->method_holder()->klass_part()->external_name(),
			method->name()->as_C_string());
	return true;
      }
    }
  }
  return false;
}

bool nmethod::is_dependent_on_method(methodOop dependee) {
  if (method() == dependee) return true;
  if (number_of_dependents() == 0) return false;

  for (int index = first_dependent()+1; index < dependent_limit(); index += 2) {
    methodOop method = methodOop(oop_at(index));
    if (method == dependee) {
      return true;
    }
  }
  return false;
}


bool nmethod::is_patchable_at(address instr_addr) {
  assert (code_contains(instr_addr), "wrong nmethod used");
  if (is_zombie()) {
    // a zombie may never be patched
    return false;
#if 0 // %%% remove _zombie_instruction_size ?
    // a zombie nmethod does not allow patching of the zombie-jump instruction
    address protected_code_limit = verified_entry_point() + nmethod::_zombie_instruction_size;
    return protected_code_limit >= instr_addr;
#endif
  }
  return true;
}


// generates an interpreter entry point, if neccesary. GC can happen during this call.
address nmethod::interpreter_entry_point() {
  if (_interpreter_entry_point == NULL) {
#ifdef COMPILER1
    _interpreter_entry_point = verified_entry_point();
#else
    _interpreter_entry_point = I2CAdapterGenerator::std_verified_entry(method());    
#endif // COMPILER1
  }
  assert(_interpreter_entry_point != NULL, "entry point must be set");
  return _interpreter_entry_point;
}


void resolve_and_patch(oop* v) {
  // As a special case, IC oops are initialized to 1 or -1.
  if (*v != NULL && *v != (oop)Universe::non_oop_word()) {
    oop obj = JNIHandles::resolve((jobject)*v);
    *v = obj;
  }
}


void nmethod::resolve_JNIHandles() {
  assert(!is_patched_for_deopt(), " shouldn't be calling this?");
  RelocIterator iter(this);
  while (iter.next()) {
    if (iter.type() == relocInfo::oop_type) {
      oop_Relocation *reloc = iter.oop_reloc();
      if (reloc->oop_index() == 0) {
	if (!is_patched_for_deopt())
	  reloc->oops_do(resolve_and_patch);
      } else {
        assert(reloc->oop_value() == NULL || reloc->oop_value()->is_oop(), "should be oop");
        reloc->set_value(reloc->value());
      }
    }
  }
}


address nmethod::continuation_for_implicit_exception(address pc) {
  // Exception happened outside inline-cache check code => we are inside
  // an active nmethod => use cpc to determine a return address
  int exception_offset = pc - instructions_begin();
  int cont_offset = ImplicitExceptionTable(this).at( exception_offset );
#ifdef ASSERT
  if (cont_offset == 0) {
    Thread* thread = ThreadLocalStorage::get_thread_slow();
    ResetNoHandleMark rnm; // Might be called from LEAF/QUICK ENTRY
    HandleMark hm(thread);
    ResourceMark rm(thread);
    CodeBlob* cb = CodeCache::find_blob(pc);
    assert(cb != NULL && cb == this, "");
    tty->print_cr("implicit exception happened at " INTPTR_FORMAT, pc);
    print();
    method()->print_codes();
    print_code();
    print_pcs();
  }
#endif
  guarantee(cont_offset != 0, "unhandled implicit exception in compiled code");
  return instructions_begin() + cont_offset;
}



void nmethod_init() {
  // make sure you didn't forget to adjust the filler fields
  assert(sizeof(nmFlags) <= 4,           "nmFlags occupies more than a word");
  assert(sizeof(nmethod) % oopSize == 0, "nmethod size must be multiple of a word");
}


//-------------------------------------------------------------------------------------------


nmethodLocker::nmethodLocker(address pc) {
  CodeBlob *cb = CodeCache::find_blob(pc);
  _nm = (nmethod*)cb;
  Atomic::inc(&_nm->_lock_count);
  // The guarantee is *after* the increment in order to prevent a tail call
  // to the latter.  On some platforms, the call to increment can be patched
  // at execution time with more efficient code.  Compiler-generated code that
  // does a tail call makes patching impossible, since we can't tell where the
  // call site is from inside the increment method.
  guarantee(cb != NULL && cb->is_nmethod() && !_nm->is_zombie(), "bad pc for a nmethod found");  
}

nmethodLocker::nmethodLocker(nmethod *nm) {
  _nm = nm;
  if (_nm != NULL) {
    Atomic::inc(&_nm->_lock_count);
  }
  // See above.
  guarantee(nm == NULL || !_nm->is_zombie(), "cannot lock a zombie method");
}

nmethodLocker::~nmethodLocker() {
  if (_nm != NULL) {
    Atomic::dec(&_nm->_lock_count);
  }
  guarantee(_nm == NULL || _nm->_lock_count >= 0, "unmatched nmethod lock/unlock");
}


// -----------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

void nmethod::verify() {

  // Hmm. OSR methods can be deopted but not marked as zombie or not_entrant
  // seems odd.

  if( is_zombie() || is_patched_for_deopt() || 
      is_not_entrant() ) 
    return;

  assert(method()->is_oop(), "must be valid");

  ResourceMark rm;

  if (!CodeCache::contains(this)) {
    fatal1("nmethod at " INTPTR_FORMAT " not in zone", this);
  }

  nmethod* nm = CodeCache::find_nmethod(verified_entry_point());
  if (nm != this) {
    fatal1("findNMethod did not find this nmethod (" INTPTR_FORMAT ")", this);
  }

  for (PcDesc* p = scopes_pcs_begin(); p < scopes_pcs_end(); p++) {
    if (! p->verify(this)) {
      tty->print_cr("\t\tin nmethod at " INTPTR_FORMAT " (pcs)", this);
    }
  }

  verify_scopes();
}


void nmethod::verify_interrupt_point(address call_site) {
#ifdef ASSERT
  // This code does not work in release mode since
  // owns_lock only is available in debug mode.
  CompiledIC* ic = NULL;
  Thread *cur = Thread::current();
  if (cur->is_VM_thread() || CompiledIC_lock->owner() == cur) {
    ic = CompiledIC_at(call_site);
  } else {    
    MutexLocker ml_verify (CompiledIC_lock);
    ic = CompiledIC_at(call_site);
  }
  PcDesc* pd = pc_desc_at(ic->end_of_call(),true);
  for (ScopeDesc* sd = new ScopeDesc(this, pd->scope_decode_offset()); !sd->is_top(); sd = sd->sender()) {
    sd->verify();
  }
#endif
}

void nmethod::verify_scopes() {
  if( !method() ) return;	// Runtime stubs have no scope
  if (method()->is_native()) return; // Ignore stub methods.
  // iterate through all interrupt point
  // and verify the debug information is valid.
  RelocIterator iter((nmethod*)this);
  while (iter.next()) {
    address stub = NULL;
    switch (iter.type()) {
      case relocInfo::virtual_call_type:
        verify_interrupt_point(iter.addr());
        break;
      case relocInfo::opt_virtual_call_type:
        stub = iter.opt_virtual_call_reloc()->static_stub();
        verify_interrupt_point(iter.addr());
        break;
      case relocInfo::static_call_type:
        stub = iter.static_call_reloc()->static_stub();          
        //verify_interrupt_point(iter.addr());
        break;
      case relocInfo::runtime_call_type: 
        address destination = iter.reloc()->value();
        // Right now there is no way to find out which entries support
        // an interrupt point.  It would be nice if we had this
        // information in a table.
        break;
    }
    assert(stub == NULL || stub_contains(stub), "static call stub outside stub section");
  }
}


void nmethod::check_store() {
  // Make sure all oops in the compiled code are tenured
  RelocIterator iter(this);
  while (iter.next()) {
    if (iter.type() == relocInfo::oop_type) {
      // oops embedded in the code are dead if patched for deopt
      if (!is_patched_for_deopt() || iter.oop_reloc()->oop_index() != 0) {
	oop obj = iter.oop_reloc()->oop_value();
	if (obj != NULL && !obj->is_perm()) {
	  fatal("must be permanent oop in compiled code");
	}
      }
    }
  }
}


// Printing operations

void nmethod::print() const {
  ResourceMark rm;
  
  tty->print("Compiled ");
  if (is_osr_method()) tty->print("(osr)");
  if (is_native_method()) tty->print("(native)");
  method()->print_value_on(tty);
  tty->cr();

  if (WizardMode) {
    tty->print("((nmethod*) "INTPTR_FORMAT ") ", this);
    tty->print(" for method " INTPTR_FORMAT , method());
    tty->print(" { ");
    if (version())        tty->print("v%d ", version());
    if (level())          tty->print("l%d ", level());
    if (is_in_use())      tty->print("in_use ");
    if (is_not_entrant()) tty->print("not_entrant ");
    if (is_zombie())      tty->print("zombie ");
    if (is_unloaded())    tty->print("unloaded ");
    tty->print_cr("}:");  
  }
  if (code_size         () > 0) tty->print_cr(" main code      [" INTPTR_FORMAT "," INTPTR_FORMAT "] = %d", 
					      code_begin(),
					      code_end(),
					      code_size());
  if (exception_size    () > 0) tty->print_cr(" exception code [" INTPTR_FORMAT "," INTPTR_FORMAT "] = %d",
					      exception_begin(),
					      exception_end(),
					      exception_size());
  if (stub_size         () > 0) tty->print_cr(" stub code      [" INTPTR_FORMAT "," INTPTR_FORMAT "] = %d",
					      stub_begin(),
					      stub_end(),
					      stub_size());
  if (relocation_size   () > 0) tty->print_cr(" relocation     [" INTPTR_FORMAT "," INTPTR_FORMAT "] = %d",
					      relocation_begin(),
					      relocation_end(),
					      relocation_size());
  if (oops_size         () > 0) tty->print_cr(" oops           [" INTPTR_FORMAT "," INTPTR_FORMAT "] = %d",
					      oops_begin(),
					      oops_end(),
					      oops_size());
  if (scopes_data_size  () > 0) tty->print_cr(" scopes data    [" INTPTR_FORMAT "," INTPTR_FORMAT "] = %d",
					      scopes_data_begin(),
					      scopes_data_end(),
					      scopes_data_size());
  if (scopes_pcs_size   () > 0) tty->print_cr(" scopes pcs     [" INTPTR_FORMAT "," INTPTR_FORMAT "] = %d",
					      scopes_pcs_begin(),
					      scopes_pcs_end(),
					      scopes_pcs_size());
  if (handler_table_size() > 0) tty->print_cr(" handler table  [" INTPTR_FORMAT "," INTPTR_FORMAT "] = %d",
					      handler_table_begin(),
					      handler_table_end(),
					      handler_table_size());
  if (nul_chk_table_size() > 0) tty->print_cr(" nul chk table  [" INTPTR_FORMAT "," INTPTR_FORMAT "] = %d",
					      nul_chk_table_begin(),
					      nul_chk_table_end(),
					      nul_chk_table_size());
  tty->print_cr("total size = %d", size());
}


void nmethod::print_scopes() {
  // Find the first pc desc for all scopes in the code and print it.
  ResourceMark rm;
  for (PcDesc* p = scopes_pcs_begin(); p < scopes_pcs_end(); p++) {
    ScopeDesc* sd = scope_desc_at(p->real_pc(this), p->at_call());
    sd->print();
  }
}

void nmethod::print_dependencies() {
  ResourceMark rm;
  if (number_of_dependents()  == 0) {
    tty->print_cr("No dependencies");
    return;
  }

  tty->print_cr("Dependencies:");
  for (int index = first_dependent(); index < dependent_limit(); index += 2) {
    klassOop  klass  = klassOop(oop_at(index));
    assert((klass == NULL) || klass->is_klass(), "type check");

    methodOop method = methodOop(oop_at(index+1));
    assert(method == NULL || method->is_method(), "type check");

    if (klass == NULL)
      tty->print("(no class), ");
    else
      tty->print("  %s, ", instanceKlass::cast(klass)->external_name());
    if (method == NULL)
      tty->print("(no method)");
    else
      method->print_name(tty);
    tty->cr();
  }
}


void nmethod::print_code() {
  HandleMark hm;
  ResourceMark m;
  Disassembler().decode(this);
}


void nmethod::print_relocations() {
  ResourceMark m;       // in case methods get printed via the debugger
  tty->print_cr("relocations:");
  RelocIterator iter(this);
  iter.print();
  if (UseRelocIndex) {
    jint* index_end   = (jint*)relocation_end() - 1;
    jint  index_size  = *index_end;
    jint* index_start = (jint*)( (address)index_end - index_size );
    tty->print_cr("    index @" INTPTR_FORMAT ": index_size=%d", index_start, index_size);
    if (index_size > 0) {
      jint* ip;
      for (ip = index_start; ip+2 <= index_end; ip += 2)
	tty->print_cr("  (%d %d) addr=" INTPTR_FORMAT " @" INTPTR_FORMAT,
		      ip[0],
		      ip[1],
		      header_end()+ip[0],
		      relocation_begin()-1+ip[1]);
      for (; ip < index_end; ip++)
	tty->print_cr("  (%d ?)", ip[0]);
      tty->print_cr("          @" INTPTR_FORMAT ": index_size=%d", ip, *ip++);
      tty->print_cr("reloc_end @" INTPTR_FORMAT ":", ip);
    }
  }
}


void nmethod::print_pcs() {
  ResourceMark m;       // in case methods get printed via debugger
  tty->print_cr("pc-bytecode offsets:");
  for (PcDesc* p = scopes_pcs_begin(); p < scopes_pcs_end(); p++) {
    p->print(this);
  }
}


const char* nmethod::reloc_string_for(u_char* begin, u_char* end) {
  RelocIterator iter(this, begin, end);
  bool have_one = false;
  while (iter.next()) {
    have_one = true;
    switch (iter.type()) {
        case relocInfo::none:                  return "no_reloc";
        case relocInfo::oop_type: {
          stringStream st;
          oop_Relocation* r = iter.oop_reloc();
	  if (is_patched_for_deopt() && iter.oop_reloc()->oop_index() == 0) {
	    st.print("dead oop in code");
	  } else {
	    oop obj = r->oop_value();
	    st.print("oop(");
	    if (obj == NULL) st.print("NULL");
	    else obj->print_value_on(&st);
	    st.print(")");
	  }
          return st.as_string();
        }
        case relocInfo::virtual_call_type:     return "virtual_call";
        case relocInfo::opt_virtual_call_type: return "optimized virtual_call";
        case relocInfo::static_call_type:      return "static_call";
        case relocInfo::static_stub_type:      return "static_stub";        
        case relocInfo::runtime_call_type:     return "runtime_call";
        case relocInfo::external_word_type:    return "external_word";
        case relocInfo::internal_word_type:    return "internal_word";
        case relocInfo::safepoint_type:        return "safepoint";
        case relocInfo::return_type:           return "return";
        case relocInfo::poll_type:             return "poll";
        case relocInfo::poll_return_type:      return "poll_return";
        case relocInfo::type_mask:             return "type_bit_mask";
    }
  }
  return have_one ? "other" : NULL;
}


ScopeDesc* nmethod::scope_desc_in(address begin, address end) {
  for (PcDesc* p = scopes_pcs_begin(); p < scopes_pcs_end(); p++) {
    if (p->real_pc(this) == end && p->at_call()) return new ScopeDesc(this, p->scope_decode_offset());
  }
  return NULL;
}

void nmethod::print_code_comment_on(outputStream* st, int column, u_char* begin, u_char* end) {
  ScopeDesc* sd  = scope_desc_in(begin, end);  
  if (sd != NULL) {
    st->fill_to(column);
    if (sd->bci() == SynchronizationEntryBCI) {
      st->print(";*synchronization entry");
    } else {
      if (sd->bci() == SynchronizationEntryBCI) {
        st->print("; synchronization");
      } else {
        if (sd->method() == NULL) {
          tty->print("method is NULL");
        } else if (sd->method()->is_native()) {
          tty->print("method is native");
        } else {
          address bcp  = sd->method()->bcp_from(sd->bci());
          Bytecodes::Code bc = Bytecodes::java_code(Bytecodes::cast(*bcp));
          st->print(";*%s", Bytecodes::name(bc));
          if ( bc == Bytecodes::_invokevirtual
            || bc == Bytecodes::_invokespecial
            || bc == Bytecodes::_invokestatic
            || bc == Bytecodes::_invokeinterface) {
            Bytecode_invoke* invoke = Bytecode_invoke_at(sd->method(), sd->bci());
            st->print(" ");
            if (invoke->name())
              invoke->name()->print_symbol_on(st);
            else
              st->print("UNKNOWN SYMBOL");
          }
        }
      }
    }
    st->cr();
    // Print all scopes
    for (;sd != NULL; sd = sd->sender()) {
      st->fill_to(column);
      st->print("; -");
      if (sd->method() == NULL) {
        tty->print("method is NULL");
      } else {
        sd->method()->print_short_name(st);
      }
      int lineno = sd->method()->line_number_from_bci(sd->bci());
      if (lineno != -1) {
        st->print("@%d (line %d)", sd->bci(), lineno);
      } else {
        st->print("@%d", sd->bci());
      }
      st->cr();
    }
  }
  // Print relocation information
  const char* str = reloc_string_for(begin, end);
  if (str != NULL) {
    if (sd != NULL) st->cr();
    st->fill_to(column);
    st->print(";   {%s}", str);
  }
  int cont_offset = ImplicitExceptionTable(this).at(begin - instructions_begin());
  if (cont_offset != 0) {
    st->fill_to(column);
    st->print("; implicit exception: dispatches to " INTPTR_FORMAT, instructions_begin() + cont_offset);
  }

}

void nmethod::print_value_on(outputStream* st) const {
  st->print("nmethod");  
  if (WizardMode) st->print(" (" INTPTR_FORMAT ")", this);
  st->print(":");
  if (is_osr_method()) st->print("(osr)");
  method()->print_value_on(st);  
}

void nmethod::print_calls(outputStream* st) {
  RelocIterator iter(this);
  while (iter.next()) {
    switch (iter.type()) {
    case relocInfo::virtual_call_type:
    case relocInfo::opt_virtual_call_type: {
      VerifyMutexLocker mc(CompiledIC_lock);
      CompiledIC_at(iter.reloc())->print();
      break;
    }
    case relocInfo::static_call_type:
      st->print_cr("Static call at " INTPTR_FORMAT, iter.reloc()->addr());
      compiledStaticCall_at(iter.reloc())->print();
      break;
    }
  }
}

void nmethod::print_handler_table() {
  COMPILER1_ONLY(exception_range_table()->print(instructions_begin()););
  COMPILER2_ONLY(ExceptionHandlerTable(this).print();)
}

void nmethod::print_nul_chk_table() {
  ImplicitExceptionTable(this).print(instructions_begin());
}

#endif // PRODUCT
