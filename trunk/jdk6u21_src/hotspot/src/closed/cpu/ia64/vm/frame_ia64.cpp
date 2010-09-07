/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_frame_ia64.cpp.incl"


#ifdef ASSERT
void RegisterMap::check_location_valid() {
}
#endif /* ASSERT */

void RegisterMap::pd_clear() {
  if (_thread->has_last_Java_frame()) {
    frame fr = _thread->last_frame();
    _bsp = fr.fp();
  } else {
    _bsp = NULL;
  }
  _extra1 = NULL;
  _extra2 = NULL;
  _extra3 = NULL;
#ifdef ASSERT
    for(int i = 0; i < reg_count; i++) {
      _location[i] = NULL;
    }
#endif
}

void RegisterMap::pd_initialize_from(const RegisterMap* map) {
  _bsp = map->_bsp;
  _extra1 = map->_extra1;
  _extra2 = map->_extra2;
  _extra3 = map->_extra3;

  // We have more 96 stacked + 32 unstacked register to worry about.
  _location_valid[0] = 0;
  _location_valid[1] = 0;
}

void RegisterMap::pd_initialize() {
  _bsp = NULL;
  _extra1 = NULL;
  _extra2 = NULL;
  _extra3 = NULL;

  // We have more 96 stacked + 32 unstacked register to worry about.
  _location_valid[0] = 0;
  _location_valid[1] = 0;
}

void RegisterMap::make_integer_regs_unsaved(void) {
  // We have more 96 stacked + 32 unstacked register to worry about.
  _location_valid[0] = 0;
  _location_valid[1] = 0;
}

void RegisterMap::shift_window(frame& caller, frame &callee) {
  _bsp = caller.fp();
  // This will cause is to
  if (_location_valid[0] != 0 || _location_valid[1] != 0) shift_individual_registers(caller, callee);
}

// We are shifting windows.  That means we are moving all INs to OUTs.
// This is simple by comparison on sparc where the sizes of the windows
// is fixed. On IA64 the INs of the callee frame become OUTs in the caller
// but the register number is determined by the value in the frame
// mask cfm/pfs. Basically we always hope that _location_valid[0..1]
// are zero so we can skip this entire action.
void RegisterMap::shift_individual_registers(frame& caller, frame& callee) {
  if (!update_map())  return;  // this only applies to maps with locations

  uintptr_t lv[2];
  lv[0] = (uintptr_t) _location_valid[0];
  lv[1] = (uintptr_t) _location_valid[1];
  int i;
  BitMap orig(lv, 128);

  // The number of in's in the callee (current map) is the
  // number of out's in the caller
  int caller_size_of_frame = frame::size_of_frame((uint64_t) *callee.register_addr(GR_Lsave_PFS));
  int caller_size_of_locals = frame::size_of_locals((uint64_t) *callee.register_addr(GR_Lsave_PFS));
  int out_count = caller_size_of_frame - caller_size_of_locals;

  // Assume that reg 0..31 are already valid (or clear). Need a mask to clear
  // the rest

  static uintptr_t R_static_mask = 0;
  uintptr_t mask1 = R_static_mask;
  if (mask1 == 0) {
    BitMap bits(&mask1, 64);
    for (i = 0; i < 32; i++) {
      bits.set_bit(i);
    }
    R_static_mask = mask1;
  }

  _location_valid[0] &= mask1;
  _location_valid[1] = 0;
  BitMap updated((uintptr_t*) _location_valid, 128);

  // Get the first register number of the new output registers

  int out_base_idx = 32 + caller_size_of_locals;
  intptr_t* locations[96]; // At most 96 stacked locations

  // Copy the output locations to temporary space while setting the
  // new valid bits
  for ( i = 0 ; i < out_count ; i++ ) {
    // Is the input register valid??
    if (orig.at(i + 32)) {
      // Save the location and mark output register as valid
      locations[i] = _location[i + 32];
      updated.set_bit(out_base_idx + i);
    }
  }

  // Now move the locations to their final spot

  for ( i = 0 ; i < out_count ; i++ ) {
    if (updated.at(out_base_idx + i)) {
      _location[out_base_idx + i] = locations[i];
    }
  }

#ifdef ASSERT
  // Verify no valid bits are on in the stacked register bit range except the in's -> out's
  // we just moved
  for ( i = 32 ; i < out_base_idx ; i++) {
    assert(!updated.at(i), "Invalid bit");
  }

  for ( i = out_count ; i < 128 - out_base_idx ; i ++ ) {
    assert(!updated.at( i+ out_base_idx), "Invalid bit");
  }
#endif
}

address RegisterMap::pd_location(VMReg regname) const {
  //
  // If regname is a windowed register find the rse location of it.
  // If not windowed then return NULL.
  // Unlike sparc where the window storage location is split across
  // caller and callee (i.e. to find a caller's outs we must find
  // the callee's ins). On ia64 the this is simpler (well something
  // had to be) since all the windowed registers are contiguous
  // (well except for NATs) so we can find any frames registers
  // just by knowing its bsp.
  //
  assert(regname->is_reg(), " must be");
  if (regname->is_Register()) {
    Register reg = regname->as_Register();
    if (reg->is_stacked()) {
      assert(_bsp != NULL, "Window should be available");
      return (address) frame::register_addr(reg, _bsp);
    }
    return NULL;
  }
  return NULL;
}

bool frame::safe_for_sender(JavaThread *thread) {
  address   cursp = (address)sp();
  address   curfp = (address)fp();
  if ((cursp != NULL && curfp != NULL &&
      (cursp <= thread->stack_base() && cursp >= thread->stack_base() - thread->stack_size())) &&
      (curfp <= thread->stack_base() && curfp >= thread->stack_base() - thread->stack_size())) {
      CodeBlob *cb = CodeCache::find_blob_unsafe(_pc);
      // Unfortunately we can only check frame complete for runtime stubs and nmethod
      // other generic buffer blobs are more problematic so we just assume they are
      // ok. adapter blobs never have a frame complete and are never ok.
      if (cb != NULL && !cb->is_frame_complete_at(_pc)) {
        if (cb->is_nmethod() || cb->is_adapter_blob() || cb->is_runtime_stub()) {
          return false;
        }
      }
      return true;
  }
  return false;
}

bool frame::is_interpreted_frame() const  {
  return InterpreterGenerator::is_interpreter_return(pc());
}

bool frame::is_entry_frame() const {
  return StubRoutines::returns_to_call_stub(pc());
}


// sender_sp

intptr_t* frame::interpreter_frame_sender_sp() const {
  assert(is_interpreted_frame(), "interpreted frame expected");
  return (intptr_t*) (*register_addr(GR_Lsave_SP));
}

void frame::set_interpreter_frame_sender_sp(intptr_t* sender_sp) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  Unimplemented(); // From SPARC, we can do this...
  // x86 would do this
  *register_addr(GR_Lsave_SP) = (intptr_t) sender_sp;
}

static bool contains_nat(intptr_t* from, intptr_t* to) { return frame::contains_nat_collection(from, to); }

intptr_t* frame::link() const {
  intptr_t* bsp = fp();
  uint64_t fsize = size_of_locals((uint64_t) *register_addr(GR_Lsave_PFS));
  intptr_t* result = bsp - fsize;
  if (fsize > 63) {
    assert(false, "2 nat collections possible !");
  } else {
    if (contains_nat(result, bsp)) result--;
  }

  return result;

}

#ifdef ASSERT
// Debugging aid
static frame nth_sender(int n) {
  frame f = JavaThread::current()->last_frame();

  for(int i = 0; i < n; ++i)
    f = f.sender((RegisterMap*)NULL);

  printf("first frame %d\n",          f.is_first_frame()       ? 1 : 0);
  printf("interpreted frame %d\n",    f.is_interpreted_frame() ? 1 : 0);
  printf("java frame %d\n",           f.is_java_frame()        ? 1 : 0);
  printf("entry frame %d\n",          f.is_entry_frame()       ? 1 : 0);
  printf("native frame %d\n",         f.is_native_frame()      ? 1 : 0);
  bool is_deopted;
  if (f.is_compiled_frame(&is_deopted)) {
    if (is_deopted)
      printf("deoptimized frame 1\n");
    else
      printf("compiled frame 1\n");
  }

  return f;
}
#endif

frame frame::sender_for_entry_frame(RegisterMap *map) const {
  assert(map != NULL, "map must be set");
  // Java frame called from C; skip all C frames and return top C
  // frame of that chunk as the sender
  JavaFrameAnchor* jfa = entry_frame_call_wrapper()->anchor();
  assert(!entry_frame_is_first(), "next Java fp must be non zero");
  intptr_t* last_Java_sp = jfa->last_Java_sp();
  intptr_t* last_Java_fp = jfa->last_Java_fp();
  address last_Java_pc = jfa->last_Java_pc();
  assert(last_Java_sp > _raw_sp, "must be above this frame on stack");
  assert(last_Java_fp < _bsp, "RSE must be below this frame on stack");
  map->clear();
  frame caller_frame(last_Java_sp, last_Java_fp, NULL, last_Java_pc);
  map->make_integer_regs_unsaved();
  // HACK
  frame callee(*this);
  map->shift_window(caller_frame, callee);
  assert(map->include_argument_oops(), "should be set by clear");
  return caller_frame;
}

frame frame::sender_for_interpreter_frame(RegisterMap *map) const {

  // SPARC did not call this because of interpreter_sp_adjustment concerns

  frame caller(sender_sp(), link(), sender_pc_addr(), sender_pc());

  // What about the map? seems like at the very least we should do
  map->make_integer_regs_unsaved();
  // HACK
  frame callee(*this);
  map->shift_window(caller, callee);
  return caller;
}

frame frame::sender_for_compiled_frame(RegisterMap *map, CodeBlob* cb) const {

  assert(map != NULL, "map must be set");

  // frame owned by  compiler

  address* pc_addr = compiled_sender_pc_addr(cb);
  address pc = *pc_addr;

  frame caller(compiled_sender_sp(cb), compiled_sender_fp(cb), pc_addr, pc);

  // Now adjust the map

  // HACK
  frame callee(*this);
  // Move callee ins to caller outs
  map->shift_window(caller, callee);

  // Get the rest
  if (map->update_map()) {
    // Tell GC to use argument oopmaps for some runtime stubs that need it.
    map->set_include_argument_oops(cb->caller_must_gc_arguments(map->thread()));
    if (cb->oop_maps() != NULL) {
      OopMapSet::update_register_map(this, cb, map);
    }
  }

  return caller;
}

intptr_t* frame::compiled_sender_sp(CodeBlob* cb) const {
  return sp() + cb->frame_size();
}

address*   frame::compiled_sender_pc_addr(CodeBlob* cb) const {
  // SPARC/X86 don't do this
  return (address*) (register_addr(GR_Lsave_RP));
}

intptr_t* frame::compiled_sender_fp(CodeBlob* cb) const {
  // This needs to be in link() once we're convinced it works properly here
  // and for all of our frames.

  intptr_t* bsp = fp();
  uint64_t fsize = size_of_locals((uint64_t) *register_addr(GR_Lsave_PFS));
  intptr_t* sender_bsp = bsp - fsize;
  if (fsize > 63) {
    assert(false, "2 nat collections possible !");
  } else {
    if (contains_nat(sender_bsp, bsp)) sender_bsp--;
  }
  return sender_bsp;
}

frame frame::sender(RegisterMap* map, CodeBlob* cb) const {

  // Default is we done have to follow them. The sender_for_xxx will
  // update it accordingly
  map->set_include_argument_oops(false);

  if (is_entry_frame())       return sender_for_entry_frame(map);
  if (is_interpreted_frame()) return sender_for_interpreter_frame(map);
  if(cb == NULL) {
    cb = CodeCache::find_blob(pc());
  } else {
    assert(cb == CodeCache::find_blob(pc()),"Must be the same");
  }

  if (cb  != NULL) {
    return sender_for_compiled_frame(map, cb);
  }
  // Must be native-compiled frame, i.e. the marshaling code for native
  // methods that exists in the core system.
  // link() will blow here since not interpreted....
  return frame(sender_sp(), link(), NULL, sender_pc());
}

// Adjust index for the way a compiler frame is layed out in SPARC

int frame::adjust_offset(methodOop method, int index) {
  // SPARC
return 0;
}

void frame::patch_pc(Thread* thread, address pc) {
  guarantee(_pc_address != NULL, "Must have address to patch");
  // We should croak if _pc_address is in the register window area and the flushed
  // bit is not set (so should sparc)
  if(thread == Thread::current()) {
   (void)StubRoutines::ia64::flush_register_stack()();
  }
  if (TracePcPatching) {
    assert(_pc == *_pc_address + pc_return_offset, "frame has wrong pc");
    tty->print_cr("patch_pc at address  " INTPTR_FORMAT " [" INTPTR_FORMAT " -> " INTPTR_FORMAT "] ",
                 _pc_address, _pc, pc);
  }
  _pc = pc;
  *_pc_address = pc - pc_return_offset;
}

void frame::pd_gc_epilog() {
  // QQQ why is this needed on sparc but not x86???
  if (is_interpreted_frame()) {
    // set constant pool cache entry for interpreter
    methodOop m = interpreter_frame_method();

    *interpreter_frame_cpoolcache_addr() = m->constants()->cache();
  }
}


bool frame::is_interpreted_frame_valid() const {
  // Is there anything to do?
  assert(is_interpreted_frame(), "Not an interpreted frame");
  return true;
}


void JavaFrameAnchor::make_walkable(JavaThread* thread) {
  if (walkable()) return;
  // Eventually make an assert
  guarantee(Thread::current() == (Thread*)thread, "only current thread can flush its registers");
  (void)StubRoutines::ia64::flush_register_stack()();
  if (!has_last_Java_frame()) {
    // don't mark as flushed unless there is a last_Java_sp
    return;
  }
  _flags |= flushed;
}


BasicType frame::interpreter_frame_result(oop* oop_result, jvalue* value_result) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  methodOop method = interpreter_frame_method();
  BasicType type = method->result_type();

  if (method->is_native()) {
    // Prior to calling into the runtime to notify the method exit the possible
    // result value is saved into the interpreter frame.
    interpreterState istate = get_interpreterState();
    address lresult = (address)istate + in_bytes(cInterpreter::native_lresult_offset());
    address fresult = (address)istate + in_bytes(cInterpreter::native_fresult_offset());

    switch (method->result_type()) {
      case T_OBJECT:
      case T_ARRAY: {
        oop* obj_p = *(oop**)lresult;
        oop obj = (obj_p == NULL) ? NULL : *obj_p;
        assert(obj == NULL || Universe::heap()->is_in(obj), "sanity check");
        *oop_result = obj;
        break;
      }
      case T_BOOLEAN : value_result->z = *(jboolean*)lresult; break;
      case T_INT     : value_result->i = *(jint*)lresult; break;
      case T_CHAR    : value_result->c = *(jchar*)lresult; break;
      case T_SHORT   : value_result->s = *(jshort*)lresult; break;
      case T_BYTE    : value_result->z = *(jbyte*)lresult; break;
      case T_LONG    : value_result->j = *(jlong*)lresult; break;
      case T_FLOAT   :
      case T_DOUBLE  : {
        // 16-byte alignment - see interpreter_ia64.cpp
        fresult += 0x8;
        fresult = (address)((intptr_t)fresult & ~(intptr_t)0xf);

        // FR_RET was spill'ed to *fresult
        jdouble res = StubRoutines::ia64::ldffill()(fresult);
        if (type == T_DOUBLE) {
          value_result->d = res;
        } else {
          value_result->f = (jfloat)res;
        }
        break;
      }
      case T_VOID    : /* Nothing to do */ break;
      default        : ShouldNotReachHere();
    }
  } else {
    intptr_t* tos_addr = interpreter_frame_tos_address();
    switch (method->result_type()) {
      case T_OBJECT:
      case T_ARRAY: {
        oop obj = *(oop*)tos_addr;
        assert(obj == NULL || Universe::heap()->is_in(obj), "sanity check");
        *oop_result = obj;
      }
      case T_BOOLEAN : value_result->z = *(jboolean*)tos_addr; break;
      case T_BYTE    : value_result->b = *(jbyte*)tos_addr; break;
      case T_CHAR    : value_result->c = *(jchar*)tos_addr; break;
      case T_SHORT   : value_result->s = *(jshort*)tos_addr; break;
      case T_INT     : value_result->i = *(jint*)tos_addr; break;
      case T_LONG    : value_result->j = *(jlong*)tos_addr; break;
      case T_FLOAT   : value_result->f = *(jfloat*)tos_addr; break;
      case T_DOUBLE  : value_result->d = *(jdouble*)tos_addr; break;
      case T_VOID    : /* Nothing to do */ break;
      default        : ShouldNotReachHere();
    }
  }

  return type;
}


intptr_t* frame::interpreter_frame_tos_at(jint offset) const {
  int index = Interpreter::expr_index_at(offset);
  return &interpreter_frame_tos_address()[index];
}

intptr_t* frame::entry_frame_argument_at(int offset) const {
  // Since an entry frame always calls the interpreter first,
  // the parameters are on the stack and relative to known register in the
  // entry frame.
  intptr_t* tos = (intptr_t*) *register_addr(GR_entry_frame_TOS);
  return &tos[offset + Interpreter::stackElementWords()]; // prepushed tos
}

// top of expression stack
intptr_t* frame::interpreter_frame_tos_address() const {
  interpreterState istate = get_interpreterState();
  return istate->_stack + Interpreter::stackElementWords();
}
