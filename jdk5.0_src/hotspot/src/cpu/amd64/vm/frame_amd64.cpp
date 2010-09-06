#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)frame_amd64.cpp	1.4 03/12/23 16:35:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_frame_amd64.cpp.incl"


// Profiling/safepoint support

bool JavaThread::get_top_frame(frame* _fr, 
                               ExtendedPC* _addr, 
                               bool for_profile_only)
{
  intptr_t* sp;
  intptr_t* fp;

  ExtendedPC addr = os::fetch_top_frame(this, &sp, &fp);

  if (addr.contained_pc() != NULL) {
    /* Some return paths do not require valid fp or sp values,
       so they are not checked here */
      frame fr(sp, fp, addr.pc());
      *_fr = fr;
      if (_addr != NULL) {
        *_addr = addr;
      }
      return true;
   }
   return false;
}

bool frame::safe_for_sender(JavaThread *thread)
{
  address sp = (address) _sp;
  address fp = (address) _fp;
  return
    sp != NULL && fp != NULL && 
    sp <= thread->stack_base() &&
    sp >= thread->stack_base() - thread->stack_size() &&
    fp <= thread->stack_base() && 
    fp >= thread->stack_base() - thread->stack_size();
}

void frame::patch_pc(Thread* thread, address pc)
{
  if (TracePcPatching) {
    tty->print_cr("patch_pc at address 0x%lx [0x%lx -> 0x%lx] ", 
                  &((address*) _sp)[-1], ((address*) _sp)[-1], pc);
  }
  ((address*) _sp)[-1] = _pc = pc; 
}

void frame::set_sender_pc(address addr)
{ 
  if (TracePcPatching) {
    tty->print_cr("set_sender_pc at address 0x%lx [0x%lx -> 0x%lx] ", 
                  sender_pc_addr(), *sender_pc_addr(), addr);
  }
  *sender_pc_addr() = addr; 
}

bool frame::is_interpreted_frame() const
{
  return Interpreter::contains(pc());
}

bool frame::is_entry_frame() const
{
  return StubRoutines::returns_to_call_stub(pc());
}

// sender_sp

intptr_t* frame::interpreter_frame_sender_sp() const
{
  assert(is_interpreted_frame(), "interpreted frame expected");
  return (intptr_t*) at(interpreter_frame_sender_sp_offset);
}

void frame::set_interpreter_frame_sender_sp(intptr_t* sender_sp)
{
  assert(is_interpreted_frame(), "interpreted frame expected");
  long_at_put(interpreter_frame_sender_sp_offset, (jlong) sender_sp);
}

// monitor elements

BasicObjectLock* frame::interpreter_frame_monitor_begin() const 
{
  return 
    (BasicObjectLock*) addr_at(interpreter_frame_monitor_block_bottom_offset);
}

BasicObjectLock* frame::interpreter_frame_monitor_end() const
{
  BasicObjectLock* result =
    (BasicObjectLock*) *addr_at(interpreter_frame_monitor_block_top_offset);
  // make sure the pointer points inside the frame
  assert((intptr_t) fp() > (intptr_t) result,
         "result must <  than frame pointer");
  assert((intptr_t) sp() <= (intptr_t) result,
         "result must >= than stack pointer");
  return result;
}

void frame::interpreter_frame_set_monitor_end(BasicObjectLock* value)
{
  *((BasicObjectLock**) addr_at(interpreter_frame_monitor_block_top_offset)) =
    value;
}

frame frame::sender_for_entry_frame(RegisterMap* map) const 
{
  assert(map != NULL, "map must be set");
  // Java frame called from C; skip all C frames and return top C
  // frame of that chunk as the sender
  JavaFrameAnchor* jfa = entry_frame_call_wrapper()->anchor();
  assert(!entry_frame_is_first(), "next Java fp must be non zero");
  assert(jfa->last_Java_sp() > _sp, "must be above this frame on stack");  
  frame fr(jfa->last_Java_sp(), jfa->last_Java_fp());  
  map->clear(jfa->not_at_call_id()); 
  assert(map->include_argument_oops(), "should be set by clear");
  return fr;
}

frame frame::sender_for_interpreter_frame(RegisterMap* map) const
{
  // This basically makes sp in a frame the original sp before the
  // interpreter adjusted it.  This is handled by
  // _interpreter_sp_adjustment handling on sparc.  This is much less
  // visible.
  intptr_t* sp = (intptr_t*) at(interpreter_frame_sender_sp_offset);
  // We do not need to update the callee-save register mapping because above
  // us is either another interpreter frame or a converter-frame, but never
  // directly a compiled frame.
  return frame(sp, link(), sender_pc());
}

frame frame::sender_for_raw_compiled_frame(RegisterMap* map) const 
{
#ifndef CORE
  CodeBlob* stub_cb = CodeCache::find_blob(pc());
  assert(stub_cb != NULL, "wrong pc");
  return sender_for_compiled_frame(map, stub_cb, false);
#else
  ShouldNotReachHere();
  return frame();
#endif
}

#ifndef CORE
#ifdef COMPILER2

//------------------------------link_offset------------------------------------
// Find the where RBP got saved in the frame.  Cached and lazily computed.
int CodeBlob::link_offset( ) 
{
  // Magic value -2 means Not Yet Computed; other values are the actual result.
  if (_link_offset == not_yet_computed) {

    // No prior RBP saved for nmethods (prior frame is NOT an interpreted).
    if (is_nmethod()) {
      _link_offset = undefined;
    } else {
      // The Adaptor spills RBP so it must be in a stack location
      // somewhere.  Find it.
      OopMap* map = oop_maps()->at(0);// Should be in first map

      ResourceMark rm;
      OopMapValue omv;
      for (OopMapStream oms(map, OopMapValue::callee_saved_value); 
           !oms.is_done();
           oms.next()) {
	omv = oms.current();
	if (omv.content_reg() == Matcher::interpreter_frame_pointer_reg()) {
	  _link_offset = omv.stack_offset() >> 1;  // 4B slots -> 8B slots
	  break;
	}
      }
      assert(_link_offset != not_yet_computed,
             "didn't find RBP in first oopmap?");
    }
  }
  return _link_offset;
}
#endif // COMPILER2

//-----------------------------sender_for_compiled_frame-----------------------
frame frame::sender_for_compiled_frame(RegisterMap* map, 
                                       CodeBlob* cb, 
                                       bool adjusted) const 
{
  assert(map != NULL, "map must be set");

  // frame owned by optimizing compiler 
  intptr_t* sender_sp = NULL;

#ifdef COMPILER1
  sender_sp = fp() + frame::sender_sp_offset;
  assert(sender_sp == sp() + cb->frame_size() || cb->frame_size() == -1,
         "must match");
#endif

#ifdef COMPILER2
  assert(cb->frame_size() >= 0, "Compiled by Compiler1: do not use");
  sender_sp = sp() + cb->frame_size();
  if (cb->is_i2c_adapter()) {
    // Sender's SP is stored at the end of the frame
    sender_sp = (intptr_t*) *(sender_sp - (VerifyStackAtCalls ? 0 : 1)) + 1;
  }
#endif

  // On Intel the return_address is always the word on the stack
  address sender_pc = (address) *(sender_sp - 1);

  if (map->update_map() && cb->oop_maps() != NULL) {
    OopMapSet::update_register_map(this, cb, map);
  }

  // Move this here for C1 and collecting oops in arguments (According to Rene)
  COMPILER1_ONLY(map->set_include_argument_oops(
                   cb->caller_must_gc_arguments(map->thread()));)

  intptr_t *saved_fp = (intptr_t*) *(sender_sp - frame::sender_sp_offset);

#ifdef COMPILER1
  assert(saved_fp == (intptr_t*) (*fp()), "should match");
#endif
#ifdef COMPILER2
  if (!cb->is_osr_adapter()) {
    int llink_offset = cb->link_offset();
    if (llink_offset >= 0) {    
      // Restore base-pointer, since next frame might be an
      // interpreter frame.
      intptr_t* fp_addr = sp() + llink_offset;
#ifdef ASSERT
      // Check to see if regmap has same info
      if (map->update_map()) {
        address fp2_addr = map->location(VMReg::Name(RBP_num));
        assert(fp2_addr == NULL || fp2_addr == (address) fp_addr,
               "inconsistent framepointer info.");          
      }
#endif       
      saved_fp = (intptr_t*) *fp_addr;
    }
  }
#endif // COMPILER2

  // Update sp (e.g. osr adapter needs to get saved_esp)
  if (cb->is_osr_adapter()) {
    // See in frame_amd64.hpp the interpreter's frame layout.
    // Currently, sender's sp points just past the 'return pc' like normal.
    // We need to load up the real 'sender_sp' from the interpreter's frame.
    // This is different from normal, because the current interpreter frame
    // (which has been mangled into an OSR-adapter) pushed a bunch of space
    // on the caller's frame to make space for Java locals.  
    intptr_t* sp_addr = 
      sender_sp - frame::sender_sp_offset + 
      frame::interpreter_frame_sender_sp_offset;
    sender_sp = (intptr_t*) *sp_addr;
  }

#ifndef CORE
  if (adjusted && !SafepointPolling) {
    // Adjust the sender_pc if it points into a temporary codebuffer.      
    sender_pc = 
      map->thread()->safepoint_state()->compute_adjusted_pc(sender_pc);
  }
#endif

  assert(sender_sp != sp(), "must have changed");
  return frame(sender_sp, saved_fp, sender_pc);
}

#endif /* ndef CORE */

frame frame::sender(RegisterMap* map, CodeBlob *cb) const 
{
  // Default is we done have to follow them. The sender_for_xxx will
  // update it accordingly
  map->set_include_argument_oops(false);

  if (is_entry_frame()) {
    return sender_for_entry_frame(map);
  }
  if (is_interpreted_frame()) {
    return sender_for_interpreter_frame(map);
  }
#ifndef CORE  
  if(cb == NULL) {
    cb = CodeCache::find_blob(pc());
  } else {
    assert(cb == CodeCache::find_blob(pc()),"Must be the same");
  }
  if (cb != NULL) {
    // Returns adjusted pc if it was pointing into a temp. safepoint
    // codebuffer.
    return sender_for_compiled_frame(map, cb, true);
  }
#endif // CORE
  // Must be native-compiled frame, i.e. the marshaling code for native
  // methods that exists in the core system.
  return frame(sender_sp(), link(), sender_pc());
}

bool frame::interpreter_frame_equals_unpacked_fp(intptr_t* fp)
{
  assert(is_interpreted_frame(), "must be interpreter frame");
  methodOop method = interpreter_frame_method();
  // When unpacking an optimized frame the frame pointer is
  // adjusted with: 
  int diff = method->max_locals() - method->size_of_parameters();
  return _fp == (fp - diff);
}

void frame::pd_gc_epilog()
{
  // nothing done here now
}

bool frame::is_interpreted_frame_valid() const
{
  assert(is_interpreted_frame(), "Not an interpreted frame");

  // These are reasonable sanity checks
  if (fp() == 0 || (intptr_t(fp()) & 0xF) != 0) {
    return false;
  }
  if (sp() == 0 || (intptr_t(sp()) & 0xF) != 0) {
    return false;
  }
  if (fp() + interpreter_frame_initial_sp_offset < sp()) {
    return false;
  }
  // These are hacks to keep us out of trouble.
  // The problem with these is that they mask other problems
  if (fp() <= sp()) { // this attempts to deal with unsigned comparison above
    return false;
  }
  if (fp() - sp() > 4096) {  // stack frames shouldn't be large.
    return false;
  }
  return true;
}

#ifdef COMPILER1
address frame::frameless_stub_return_addr()
{
  // To call jvmpi_method_exit, the call pushes two arguments on the stack so
  // the return address is stored two words below its frame's top-of-stack.
  return (address) *(sp() + 2);
}


void frame::patch_frameless_stub_return_addr(Thread* thread,
                                             address return_addr)
{
  *(address*) (sp() + 2) = return_addr;
}
#endif /* COMPILER1 */


#ifdef COMPILER2

int frame::pd_compute_variable_size(int frame_size_in_words, CodeBlob *code)
{
  if (code->is_osr_adapter()) {
    intptr_t* sender_sp = sp() + (code->frame_size());
    // See in frame_amd64.hpp the interpreter's frame layout.
    // Currently, sender's sp points just past the 'return pc' like normal.
    // We need to load up the real 'sender_sp' from the interpreter's frame.
    // This is different from normal, because the current interpreter frame
    // (which has been mangled into an OSR-adapter) pushed a bunch of space
    // on the caller's frame to make space for Java locals.
    intptr_t* sp_addr = 
      sender_sp - frame::sender_sp_offset + 
      frame::interpreter_frame_sender_sp_offset;
    intptr_t* new_sp = (intptr_t*) *sp_addr;
    frame_size_in_words = new_sp - sp();    
  }
  return frame_size_in_words;
}

#endif /* COMPILER2 */

BasicType frame::interpreter_frame_result(oop* oop_result, jvalue* value_result) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  methodOop method = interpreter_frame_method();
  BasicType type = method->result_type();

  intptr_t* tos_addr;
  if (method->is_native()) {
    // Prior to calling into the runtime to report the method_exit the
    // registers with possible result values (XMM0 and RAX) are pushed to 
    // the native stack. For non floating point return types the return
    // value is at ESP + 2 (words).
    tos_addr = (intptr_t*)sp();
    if (type != T_FLOAT && type != T_DOUBLE) {
      tos_addr += 2;
    }
  } else {
    tos_addr = interpreter_frame_tos_address();
  }

  switch (type) {
    case T_OBJECT  : 
    case T_ARRAY   : {
      oop* obj_p;
      if (method->is_native()) {
        obj_p = *(oop**)tos_addr;
      } else {
        obj_p = (oop*)tos_addr;
      }
      oop obj = (obj_p == NULL) ? NULL : *obj_p;
      assert(obj == NULL || Universe::heap()->is_in(obj), "sanity check");
      *oop_result = obj;
      break;
    }
    case T_BOOLEAN : value_result->z = *(jboolean*)tos_addr; break;
    case T_BYTE    : value_result->b = *(jbyte*)tos_addr; break;
    case T_CHAR    : value_result->c = *(jchar*)tos_addr; break;
    case T_SHORT   : value_result->s = *(jshort*)tos_addr; break;
    case T_INT     : value_result->i = *(jint*)tos_addr; break;
    case T_LONG    : value_result->j = *(jlong*)tos_addr; break;
    case T_FLOAT   : value_result->f = *(jfloat*)tos_addr; break;
    case T_DOUBLE  : value_result->d = *(jdouble*)tos_addr; break;
  }

  return type;
}

