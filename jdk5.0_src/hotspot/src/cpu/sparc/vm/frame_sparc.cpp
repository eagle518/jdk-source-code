#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)frame_sparc.cpp	1.146 03/12/23 16:37:10 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_frame_sparc.cpp.incl"

#ifndef CORE
void RegisterMap::pd_clear() {
  if (_thread->has_last_Java_frame()) {
    frame fr = _thread->last_frame(); 
    _window = fr.sp();
  } else {
    _window = NULL;
  }
  _younger_window = NULL;
}

address RegisterMap::pd_location(VMReg::Name regname) const {
  assert(0 <= regname && regname < reg_count, "sanity check");
  int second_word = 0;
  int enc = regname;
#ifdef COMPILER2
  enc = Matcher::_regEncode[regname];
  if( enc == 255 ) return NULL; // Unencodable high-half of doubles
  if( enc > RegisterImpl::number_of_registers ) {
#ifdef _LP64
    enc -= 128;                 // Other half of 64-bit long registers
    assert( enc >= 0, "sanity check on ad file register encoding" );
#else // _LP64
    int index = enc - Matcher::_regEncode[OptoReg::Name(R_O0H_num)];
    enc -= 128;                 // Other half of 64-bit long registers
    assert( enc >= 0, "sanity check on ad file register encoding" );
    assert( enc < RegisterImpl::number_of_registers, "" );
#endif // _LP64
    second_word = sizeof(jint);
  }

  assert( enc < RegisterImpl::number_of_registers, "" );
#endif // #ifdef COMPILER2
  
  if (enc < RegisterImpl::number_of_registers) {
    Register reg = as_Register(enc);
    if (reg->is_out()) {
      assert(_younger_window != NULL, "Younger window should be available");
      return second_word + (address)&_younger_window[reg->after_save()->sp_offset_in_saved_window()];
    }
    if (reg->is_local() || reg->is_in()) {
      assert(_window != NULL, "Window should be available");
      return second_word + (address)&_window[reg->sp_offset_in_saved_window()];
    }
  }
  return NULL;
}


// We are shifting windows.  That means we are moving all %i to %o,
// getting rid of all current %l, and keeping all %g.  This is only
// complicated if any of the location pointers for these are valid.
// The normal case is that everything is in its standard register window
// home, and _location_valid[0] is zero.  In that case, this routine
// does exactly nothing.
void RegisterMap::shift_individual_registers() {
  if (!update_map())  return;  // this only applies to maps with locations

  LocationValidType lv = _location_valid[0];
  LocationValidType lv0 = lv;
  int i;

#ifdef COMPILER1
  const static int R_L_nums[] = {0+020,1+020,2+020,3+020,4+020,5+020,6+020,7+020};
  const static int R_I_nums[] = {0+030,1+030,2+030,3+030,4+030,5+030,6+030,7+030};
  const static int R_O_nums[] = {0+010,1+010,2+010,3+010,4+010,5+010,6+010,7+010};
  const static int R_G_nums[] = {0+000,1+000,2+000,3+000,4+000,5+000,6+000,7+000};
#endif

#ifdef COMPILER2
  const static int R_L_nums[] = {R_L0_num,R_L1_num,R_L2_num,R_L3_num,R_L4_num,R_L5_num,R_L6_num,R_L7_num};
  const static int R_I_nums[] = {R_I0_num,R_I1_num,R_I2_num,R_I3_num,R_I4_num,R_I5_num,R_FP_num,R_I7_num};
  const static int R_O_nums[] = {R_O0_num,R_O1_num,R_O2_num,R_O3_num,R_O4_num,R_O5_num,R_SP_num,R_O7_num};
  const static int R_G_nums[] = {R_G0_num,R_G1_num,R_G2_num,R_G3_num,R_G4_num,R_G5_num,R_G6_num,R_G7_num};
#endif

#ifdef ASSERT
  static bool once = false;
  if (!once) {
    once = true;
    for (i = 0; i < 8; i++) {
      assert(R_L_nums[i] < location_valid_type_size, "in first chunk");
      assert(R_I_nums[i] < location_valid_type_size, "in first chunk");
      assert(R_O_nums[i] < location_valid_type_size, "in first chunk");
      assert(R_G_nums[i] < location_valid_type_size, "in first chunk");
    }
  }

  static LocationValidType bad_mask = 0;
  if (bad_mask == 0) {
    LocationValidType m = 0;
    m |= (1LL << R_O_nums[6]); // SP
    m |= (1LL << R_O_nums[7]); // cPC
    m |= (1LL << R_I_nums[6]); // FP
    m |= (1LL << R_I_nums[7]); // rPC
    m |= (1LL << R_G_nums[2]); // TLS
    m |= (1LL << R_G_nums[7]); // reserved by libthread
    bad_mask = m;
  }
  assert((lv & bad_mask) == 0, "cannot have special locations for SP,FP,TLS,etc.");
#endif // ASSERT

  static LocationValidType R_LIO_mask = 0;
  LocationValidType mask = R_LIO_mask;
  if (mask == 0) {
    for (i = 0; i < 8; i++) {
      mask |= (1LL << R_L_nums[i]);
      mask |= (1LL << R_I_nums[i]);
      mask |= (1LL << R_O_nums[i]);
    }
    R_LIO_mask = mask;
  }

  lv &= ~mask;  // clear %l, %o, %i regs

  // if we cleared some non-%g locations, we may have to do some shifting
  if (lv != lv0) {
    // copy %i0-%i5 to %o0-%o5, if they have special locations
    // This can happen in within stubs which spill argument registers
    // around a dynamic link operation, such as resolve_opt_virtual_call.
    for (i = 0; i < 8; i++) {
      if (lv0 & (1LL << R_I_nums[i])) {
        _location[R_O_nums[i]] = _location[R_I_nums[i]];
        lv |=  (1LL << R_O_nums[i]);
      }
    }
  }

  _location_valid[0] = lv;
}
#endif  // !CORE


// Profiling/safepoint support
bool JavaThread::get_top_frame(frame* _fr, ExtendedPC* _addr, bool for_profile_only) {
  intptr_t*   sp;
  intptr_t*   younger_sp;
  u_char* pc;

  ExtendedPC addr = os::fetch_top_frame(this, &younger_sp, &sp);

  if ((addr.contained_pc() == NULL) || (sp == NULL))
    return false;

  if ((addr.npc() != addr.pc() + 4)) {
    // caught a signal at a delay slot -- bail for safepoints (ok for profiling)
    if (!for_profile_only)
      return false;
  }

  if (_addr != NULL) *_addr = addr;

  frame fr(sp, frame::unpatchable, addr.pc());
  *_fr = fr;

  return true;
}

bool frame::safe_for_sender(JavaThread *thread) {
  bool safe = false;
  address   sp = (address)_sp;
  if (sp != NULL && 
      (sp <= thread->stack_base() && sp >= thread->stack_base() - thread->stack_size())) {
      safe = true;
  }
  return safe;
}

bool frame::is_interpreted_frame() const  {
  return Interpreter::contains(pc());
}


bool frame::is_entry_frame() const {
  return StubRoutines::returns_to_call_stub(pc());
}


// constructor

frame::frame(intptr_t* sp, intptr_t* younger_sp, intptr_t pc_adjustment, bool younger_frame_is_interpreted) { 
  _sp = sp;
  _younger_sp = younger_sp;
  if (younger_sp == NULL) {
    // make a deficient frame which doesn't know where its PC is
    _pc = NULL;
  } else {
    _pc = (address)younger_sp[I7->sp_offset_in_saved_window()] + pc_return_offset + pc_adjustment;
    assert( (intptr_t*)younger_sp[FP->sp_offset_in_saved_window()] == (intptr_t*)((intptr_t)sp - STACK_BIAS), "younger_sp must be valid");
    // In case of native stubs, the pc retrieved here might be 
    // wrong.  (the _last_native_pc will have the right value)
    // So do not put add any asserts on the _pc here.
  }
  if (younger_frame_is_interpreted) {
    // compute adjustment to this frame's SP made by its interpreted callee
    _interpreter_sp_adjustment = (intptr_t*)((intptr_t)younger_sp[IsavedSP->sp_offset_in_saved_window()] +
                                             STACK_BIAS) - sp;
  } else {
    _interpreter_sp_adjustment = 0;
  }
}


// sender_sp

intptr_t* frame::interpreter_frame_sender_sp() const {
  assert(is_interpreted_frame(), "interpreted frame expected");
  return fp();
}

void frame::set_interpreter_frame_sender_sp(intptr_t* sender_sp) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  Unimplemented();
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
#ifndef CORE
  bool is_deopted;
  if (f.is_compiled_frame(&is_deopted)) {
    if (is_deopted)
      printf("deoptimized frame 1\n");
    else
      printf("compiled frame 1\n");
  }
#endif

  return f;
}
#endif


frame frame::sender_for_entry_frame(RegisterMap *map) const {
  assert(map != NULL, "map must be set");
  // Java frame called from C; skip all C frames and return top C
  // frame of that chunk as the sender
  JavaFrameAnchor* jfa = entry_frame_call_wrapper()->anchor();
  assert(!entry_frame_is_first(), "next Java fp must be non zero");
  assert(jfa->last_Java_sp() > _sp, "must be above this frame on stack");
  intptr_t* last_Java_sp = jfa->last_Java_sp();
  // Since we are walking the stack now this nested anchor is obviously walkable
  // even if it wasn't when it was stacked.
  if (!jfa->walkable()) {
    // Capture _last_Java_pc (if needed) and mark anchor walkable.
    jfa->capture_last_Java_pc(_sp);
  }
  assert(jfa->last_Java_pc() != NULL, "No captured pc!");
  map->clear(jfa->not_at_call_id());  
#ifndef CORE
  map->make_integer_regs_unsaved();
  map->shift_window(last_Java_sp, NULL);
#endif
  assert(map->include_argument_oops(), "should be set by clear");
  return frame(last_Java_sp, frame::unpatchable, jfa->last_Java_pc());
}

frame frame::sender_for_interpreter_frame(RegisterMap *map) const {
  ShouldNotCallThis();
  return sender(map);
}

frame frame::sender_for_raw_compiled_frame(RegisterMap* map) const {
#ifndef CORE
  CodeBlob* stub_cb = CodeCache::find_blob(pc());
  assert(stub_cb != NULL, "wrong pc");

  // Do not adjust the sender_pc if it points into a temporary codebuffer.

  return sender_for_compiled_frame(map, stub_cb, false);
#else
  ShouldNotReachHere();
  return frame();
#endif
}

#ifndef CORE
frame frame::sender_for_compiled_frame(RegisterMap *map, CodeBlob* cb, bool adjusted) const {  
  return sender_with_pc_adjustment(map, NULL, adjusted);
}

#endif

frame frame::sender(RegisterMap* map, CodeBlob* cb) const {
  return sender_with_pc_adjustment(map, cb, true);
}

frame frame::sender_with_pc_adjustment(RegisterMap* map, CodeBlob* cb, bool adjust_pc) const {
  assert(map != NULL, "map must be set");

  // Default is not to follow arguments; sender_for_xxx will update it accordingly
  map->set_include_argument_oops(false);

  if (cb == NULL && is_entry_frame()) return sender_for_entry_frame(map);

  intptr_t* younger_sp     = sp();
  intptr_t* sp             = sender_sp();
  intptr_t  pc_adjustment  = 0;     // not used in CORE system
  bool      is_interpreted = false; // not used in CORE system

  // Note:  The version of this operation on any platform with callee-save
  //        registers must update the register map (if not null).
  //        In order to do this correctly, the various subtypes of
  //        of frame (interpreted, compiled, glue, native),
  //        must be distinguished.  There is no need on SPARC for
  //        such distinctions, because all callee-save registers are
  //        preserved for all frames via SPARC-specific mechanisms.
  //
  //        *** HOWEVER, *** if and when we make any floating-point
  //        registers callee-saved, then we will have to copy over
  //        the RegisterMap update logic from the Intel code.

#ifndef CORE
  // The constructor of the sender must know whether this frame is interpreted so it can set the
  // sender's _interpreter_sp_adjustment field.  An osr adapter frame was originally
  // interpreted but its pc is in the code cache (for c1 -> osr_frame_return_id stub), so it must be
  // explicitly recognized. 
  if (Interpreter::contains(pc()) || is_osr_adapter_frame()) {
    is_interpreted = true;
    COMPILER1_ONLY(map->make_integer_regs_unsaved();
                   map->shift_window(sp, younger_sp);)
  } else {
    // Find a CodeBlob containing this frame's pc or elide the lookup and use the
    // supplied blob which is already known to be associated with this frame.
    assert(cb == NULL || (!cb->caller_must_gc_arguments(map->thread()) && !map->include_argument_oops()),
           "Assumed that we don't need "
           "map->set_include_argument_oops(cb->caller_must_gc_arguments(map->thread())) "
           "(for sake of performance)");
    if (cb == NULL) {
      cb = CodeCache::find_blob(pc());
    }
    if (cb != NULL) {
      if (adjust_pc) {
        address sender_pc_0 = this->sender_pc();
        address sender_pc = map->thread()->safepoint_state()->compute_adjusted_pc(sender_pc_0);
        // Adjust the sender_pc if it points into a temporary codebuffer.      
        pc_adjustment = sender_pc - sender_pc_0;
      }
      if (cb->caller_must_gc_arguments(map->thread())) {
        map->set_include_argument_oops(true);
      }

      // Update the locations of implicitly saved registers to be their addresses in the register save area.
      // For %o registers, the addresses of %i registers in the next younger frame are used.
      map->shift_window(sp, younger_sp);
      if (map->update_map()) {
        if (cb->oop_maps() != NULL) {
          OopMapSet::update_register_map(this, cb, map);
        }
      }
    }
  }
#endif
  return frame(sp, younger_sp, pc_adjustment, is_interpreted);
}


void frame::patch_pc(Thread* thread, address pc) {
  if(thread == Thread::current()) {
   StubRoutines::sparc::flush_callers_register_windows_func()();
  } 
  if (TracePcPatching) {
    assert(_pc == *O7_addr() + pc_return_offset, "frame has wrong pc");
    tty->print_cr("patch_pc at address  0x%x [0x%x -> 0x%x] ", O7_addr(), _pc, pc);
  }
  _pc = pc;
  *O7_addr() = pc - pc_return_offset;
}


static bool sp_is_valid(intptr_t* old_sp, intptr_t* young_sp, intptr_t* sp) {
  return (((intptr_t)sp & (2*wordSize-1)) == 0 && 
          sp <= old_sp && 
          sp >= young_sp);
}


/*
  Find the (biased) sp that is just younger than old_sp starting at sp.
  If not found return NULL. Register windows are assumed to be flushed.
*/
intptr_t* frame::next_younger_sp_or_null(intptr_t* old_sp, intptr_t* sp) {

  intptr_t* previous_sp = NULL;
  intptr_t* orig_sp = sp;

  int max_frames = (old_sp - sp) / 16; // Minimum frame size is 16
  int max_frame2 = max_frames;
  while(sp != old_sp && sp_is_valid(old_sp, orig_sp, sp)) {
    if (max_frames-- <= 0) 
      // too many frames have gone by; invalid parameters given to this function 
      break; 
    previous_sp = sp;
    sp = (intptr_t*)sp[FP->sp_offset_in_saved_window()];
    sp = (intptr_t*)((intptr_t)sp + STACK_BIAS);
  }

  return (sp == old_sp ? previous_sp : NULL);
}

/*
  Determine if "sp" is a valid stack pointer. "sp" is assumed to be younger than
  "valid_sp". So if "sp" is valid itself then it should be possible to walk frames
  from "sp" to "valid_sp". The assumption is that the registers windows for the
  thread stack in question are flushed.
*/
bool frame::is_valid_stack_pointer(intptr_t* valid_sp, intptr_t* sp) {
  return next_younger_sp_or_null(valid_sp, sp) != NULL;
}


bool frame::interpreter_frame_equals_unpacked_fp(intptr_t* fp) {
  assert(is_interpreted_frame(), "must be interpreter frame");
  return this->fp() == fp;
}


void frame::pd_gc_epilog() {
  if (is_interpreted_frame()) {
    // set constant pool cache entry for interpreter
    methodOop m = interpreter_frame_method();

    *interpreter_frame_cpoolcache_addr() = m->constants()->cache();
  }
}


bool frame::is_interpreted_frame_valid() const {
  assert(is_interpreted_frame(), "Not an interpreted frame");
  // These are reasonable sanity checks
  if (fp() == 0 || (intptr_t(fp()) & (2*wordSize-1)) != 0) {
    return false;
  }
  if (sp() == 0 || (intptr_t(sp()) & (2*wordSize-1)) != 0) {
    return false;
  }
  const intptr_t interpreter_frame_initial_sp_offset = interpreter_frame_vm_local_words;
  if (fp() + interpreter_frame_initial_sp_offset < sp()) {
    return false;
  }
  // These are hacks to keep us out of trouble.
  // The problem with these is that they mask other problems
  if (fp() <= sp()) {        // this attempts to deal with unsigned comparison above
    return false;
  }
  if (fp() - sp() > 4096) {  // stack frames shouldn't be large.
    return false;
  }
  return true;
}


#ifndef CORE

#ifdef COMPILER1
address frame::frameless_stub_return_addr() {
  return (address)(sp()[I5->sp_offset_in_saved_window()]);
}


void frame::patch_frameless_stub_return_addr(Thread* thread, address return_addr) {
  if (thread == Thread::current()) {
    StubRoutines::sparc::flush_callers_register_windows_func()();
  }
  sp()[I5->sp_offset_in_saved_window()] = (intptr_t)return_addr;
}
#endif // COMPILER1
#endif

#ifdef COMPILER2
int frame::pd_compute_variable_size(int frame_size_in_words, CodeBlob *code) {
  if (code->is_osr_adapter()) {
    // osr adapters used to be interpreter frames, so undo any
    // adjustment to the frame size done by the interpreter.
    return (sp()[IsavedSP->sp_offset_in_saved_window()] + STACK_BIAS - (intptr_t)fp()) / wordSize;
  }
  return 0;
}
#endif

// Windows have been flushed on entry (but not marked). Capture the pc that
// is the return address to the frame that contains "sp" as its stack pointer. 
// This pc resides in the called of the frame corresponding to "sp". 
// As a side effect we mark this JavaFrameAnchor as having flushed the windows.
// This side effect lets us mark stacked JavaFrameAnchors (stacked in the 
// call_helper) as flushed when we have flushed the windows for the most
// recent (i.e. current) JavaFrameAnchor. This saves useless flushing calls
// and lets us find the pc just once rather than multiple times as it did
// in the bad old _post_Java_state days.
//
void JavaFrameAnchor::capture_last_Java_pc(intptr_t* sp) {
  if (last_Java_sp() != NULL && last_Java_pc() == NULL) {
    // try and find the sp just younger than _last_Java_sp
    intptr_t* _post_Java_sp = frame::next_younger_sp_or_null(last_Java_sp(), sp);
    // Really this should never fail otherwise VM call must have non-standard
    // frame linkage (bad) or stack is not properly flushed (worse).
    guarantee(_post_Java_sp != NULL, "bad stack!");
    _last_Java_pc = (address) _post_Java_sp[ I7->sp_offset_in_saved_window()] + frame::pc_return_offset;

  }
  set_window_flushed();
}

void JavaFrameAnchor::make_walkable(JavaThread* thread) {
  if (walkable()) return;
  // Eventually make an assert
  guarantee(Thread::current() == (Thread*)thread, "only current thread can flush its registers");
  // We always flush in case the profiler wants it but we won't mark
  // the windows as flushed unless we have a last_Java_frame
  intptr_t* sp = StubRoutines::sparc::flush_callers_register_windows_func()();
  if (last_Java_sp() != NULL ) {
    capture_last_Java_pc(sp);
  }
}


BasicType frame::interpreter_frame_result(oop* oop_result, jvalue* value_result) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  methodOop method = interpreter_frame_method();
  BasicType type = method->result_type();
  
  if (method->is_native()) {
    // Prior to notifying the runtime of the method_exit the possible result
    // value is saved to l_scratch and d_scratch.

    intptr_t* l_scratch = fp() + interpreter_frame_l_scratch_fp_offset;
    intptr_t* d_scratch = fp() + interpreter_frame_d_scratch_fp_offset;

    address l_addr = (address)l_scratch;
#ifdef _LP64
    // On 64-bit the result for 1/8/16/32-bit result types is in the other
    // word half
    l_addr += wordSize/2;
#endif

    switch (type) {
      case T_OBJECT:
      case T_ARRAY: {
	oop* obj_p = *(oop**)l_scratch;
	oop obj = (obj_p == NULL) ? NULL : *obj_p;
	assert(obj == NULL || Universe::heap()->is_in(obj), "sanity check");
	*oop_result = obj;
	break;
      }

      case T_BOOLEAN : { jint* p = (jint*)l_addr; value_result->z = (jboolean)((*p) & 0x1); break; }
      case T_BYTE    : { jint* p = (jint*)l_addr; value_result->b = (jbyte)((*p) & 0xff); break; }
      case T_CHAR    : { jint* p = (jint*)l_addr; value_result->c = (jchar)((*p) & 0xffff); break; }
      case T_SHORT   : { jint* p = (jint*)l_addr; value_result->s = (jshort)((*p) & 0xffff); break; }
      case T_INT     : value_result->i = *(jint*)l_addr; break;
      case T_LONG    : value_result->j = *(jlong*)l_scratch; break;
      case T_FLOAT   : value_result->f = *(jfloat*)d_scratch; break;
      case T_DOUBLE  : value_result->d = *(jdouble*)d_scratch; break;
      case T_VOID    : /* Nothing to do */ break;
      default        : ShouldNotReachHere();
    }
  } else {  
    intptr_t* tos_addr = interpreter_frame_tos_address();

    switch(type) {
      case T_OBJECT:
      case T_ARRAY: {
	oop obj = (oop)*tos_addr;
 	assert(obj == NULL || Universe::heap()->is_in(obj), "sanity check");	
	*oop_result = obj;
	break;
      }
      case T_BOOLEAN : { jint* p = (jint*)tos_addr; value_result->z = (jboolean)((*p) & 0x1); break; }
      case T_BYTE    : { jint* p = (jint*)tos_addr; value_result->b = (jbyte)((*p) & 0xff); break; }
      case T_CHAR    : { jint* p = (jint*)tos_addr; value_result->c = (jchar)((*p) & 0xffff); break; }
      case T_SHORT   : { jint* p = (jint*)tos_addr; value_result->s = (jshort)((*p) & 0xffff); break; }
      case T_INT     : value_result->i = *(jint*)tos_addr; break;
      case T_LONG    : value_result->j = *(jlong*)tos_addr; break;
      case T_FLOAT   : value_result->f = *(jfloat*)tos_addr; break;
      case T_DOUBLE  : value_result->d = *(jdouble*)tos_addr; break;
      case T_VOID    : /* Nothing to do */ break;
      default        : ShouldNotReachHere();
    }
  };

  return type;
}
