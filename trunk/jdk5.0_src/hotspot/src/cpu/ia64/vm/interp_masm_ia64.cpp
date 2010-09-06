#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interp_masm_ia64.cpp	1.18 03/12/23 16:36:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interp_masm_ia64.cpp.incl"

// Implementation of InterpreterMacroAssembler

#ifndef CORE

void InterpreterMacroAssembler::increment_invocation_counter(const Register counter) {
  assert(UseCompiler, "incrementing must be useful");

  const Register backedge_counter        = GR2_SCRATCH;
  const Register backedge_counter_addr   = backedge_counter;
  const Register invocation_counter_addr = GR3_SCRATCH;

  add(invocation_counter_addr, method_(invocation_counter) + in_bytes(InvocationCounter::counter_offset()));
  add(backedge_counter_addr, method_(backedge_counter) + in_bytes(InvocationCounter::counter_offset()));

  // Load each counter
  ld4(counter, invocation_counter_addr);
  ld4(backedge_counter, backedge_counter_addr);

  // Add the delta to the invocation counter
  add(counter, counter, InvocationCounter::count_increment);

  // Mask the backedge counter
  and3(backedge_counter, backedge_counter, InvocationCounter::count_mask_value);

  // Store new counter value
  st4(invocation_counter_addr, counter);

  // Return invocation counter + backedge counter
  add(counter, counter, backedge_counter);
}

#endif // !CORE


// Lock object.
//
// On entry:
//   monitor - Points to the BasicObjectLock to be used for locking,
//             which must be initialized with the object to lock.
//   obj     - Points to the object to be locked.
//
void InterpreterMacroAssembler::lock_object(Register monitor, Register obj) {
  Label locked;

  // markOop displaced_header = obj->mark().set_unlocked();
  // monitor->lock()->set_displaced_header(displaced_header);
  // if (Atomic::cmpxchg_ptr(monitor, obj->mark_addr(), displaced_header) == displaced_header) {
  //   // We stored the monitor address into the object's mark word.
  // } else if (THREAD->is_lock_owned((address)displaced_header))
  //   // Simple recursive case.
  //   monitor->lock()->set_displaced_header(NULL);
  // } else {
  //   // Slow path.
  //   InterpreterRuntime::monitorenter(THREAD, monitor);
  // }

  if (!UseHeavyMonitors) {
    const Register mark_addr             = GR2_SCRATCH;
    const Register displaced_header_addr = GR3_SCRATCH;
    const Register lock                  = GR_RET;
    const Register mark                  = GR_RET1;

    const PredicateRegister own_lock     = PR15_SCRATCH;

    add(mark_addr, obj, oopDesc::mark_offset_in_bytes());
    add(displaced_header_addr, monitor, BasicObjectLock::lock_offset_in_bytes() +
                                        BasicLock::displaced_header_offset_in_bytes());

    // Load markOop from oop into mark
    ld8(mark, mark_addr);
    eog();

    // Set mark to markOop | markOopDesc::unlocked_value
    or3(mark, markOopDesc::unlocked_value, mark);
    eog();

    // Load Compare Value application register
    mov(AR_CCV, mark);

    // Initialize the box.  Must happen before we update the object mark!
    st8(displaced_header_addr, mark);

    // Must fence, otherwise, preceding store(s) may float below cmpxchg
    mf();
    eog();

    // Compare object markOop with mark and if equal exchange scratch1 with object markOop
    cmpxchg8(lock, mark_addr, monitor, Assembler::acquire);

    // If the compare-and-exchange succeeded, then we found an unlocked object and we
    // have now locked it.  But if the values are not equal, then we failed, and
    // need to take the slow path.
    cmp(own_lock, PR0, mark, lock, Assembler::equal);
    br(own_lock, locked);

    // These can be done in parallel
    sub(lock, lock, SP);
    mov(mark, ~(os::vm_page_size()-1) | markOopDesc::lock_mask_in_place);
    eog();

    // We did not find an unlocked object, so see if this is a recursive case
    and3(lock, mark, lock);
    eog();

    st8(displaced_header_addr, lock, Assembler::ordered_release);
    cmp(own_lock, PR0, lock, GR0, Assembler::equal);
    br(own_lock, locked);
  }

  // Slow path
  call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorenter), monitor);

  bind(locked);
}


// Unlock object.
//
// On entry:
//   monitor - Points to the BasicObjectLock for lock
//
// Throw IllegalMonitorException if object is not locked by current thread
//
void InterpreterMacroAssembler::unlock_object(Register monitor) {
  Label unlocked;

  // if ((displaced_header = monitor->displaced_header()) == NULL) {
  //   // Recursive unlock.  Mark the monitor unlocked by setting the object field to NULL.
  //   monitor->set_obj(NULL);
  // } else if (Atomic::cmpxchg_ptr(displaced_header, obj->mark_addr(), monitor) == monitor) {
  //   // We swapped the unlocked mark in displaced_header into the object's mark word.
  //   monitor->set_obj(NULL);
  // } else {
  //   // Slow path.
  //   InterpreterRuntime::monitorexit(THREAD, monitor);
  // }

  if (!UseHeavyMonitors) {
    const Register displaced_header      = GR2_SCRATCH;
    const Register lock                  = displaced_header;

    const Register displaced_header_addr = displaced_header; // Address is dead after use.
    const Register lock_obj_addr         = GR3_SCRATCH;
    const Register obj                   = GR_RET1;
    const Register mark_addr             = obj;

    const PredicateRegister is_recursive_unlock = PR15_SCRATCH;
    const PredicateRegister is_lightweight      = PR15_SCRATCH;

    // Test first if we are in the fast recursive case.  If so, we mark the
    // monitor unlocked (store zero in the monitor's object field) and are done.

    add(displaced_header_addr, monitor, BasicObjectLock::lock_offset_in_bytes() +
                                        BasicLock::displaced_header_offset_in_bytes());
    add(lock_obj_addr, monitor, BasicObjectLock::obj_offset_in_bytes());

    ld8(displaced_header, displaced_header_addr);
    ld8(obj, lock_obj_addr);

    // If the displaced header is zero, we have a recursive unlock.
    cmp(is_recursive_unlock, PR0, 0, displaced_header, Assembler::equal);

    st8(is_recursive_unlock, lock_obj_addr, GR0, Assembler::ordered_release);
    br(is_recursive_unlock, unlocked);

    // If we still have a lightweight lock, unlock the object and be done.

    // The object address from the monitor is in obj.
    add(mark_addr, obj, oopDesc::mark_offset_in_bytes());
    mov(AR_CCV, monitor);

    // We have the displaced header in displaced_header.  If the lock is still
    // lightweight, it will contain the monitor address and we'll store the
    // displaced header back into the object's mark word.
    cmpxchg8(lock, mark_addr, displaced_header, Assembler::release);

    cmp(is_lightweight, PR0, lock, monitor, Assembler::equal);
    st8(is_lightweight, lock_obj_addr, GR0, Assembler::ordered_release);
    br(is_lightweight, unlocked);

    // The lock has been converted into a heavy lock, go down the slow path.
  }

  // Slow path
  call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorexit), monitor);

  bind(unlocked);
}


// if (thread in interp_only_mode) {
//   InterpreterRuntime::post_method_entry();
// }
// if (*jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY ) ||
//     *jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY2)) {
//   SharedRuntime::jvmpi_method_entry(method, receiver);
// }
//
void InterpreterMacroAssembler::notify_method_entry() {
  const PredicateRegister no_post   = PR15_SCRATCH;
  const PredicateRegister is_static = PR14_SCRATCH;

  // Whenever JVMTI puts a thread in interp_only_mode, method
  // entry/exit events are sent for that thread to track stack
  // depth.  If it is possible to enter interp_only_mode we add
  // the code to check if the event should be sent.
  if (JvmtiExport::can_post_interpreter_events()) {
    Label jvmti_post_done;

    const Register interp_only_mode   = GR2_SCRATCH;
    const Register interp_only_mode_addr   = interp_only_mode;

    add(interp_only_mode_addr, thread_(interp_only_mode));
    ld4(interp_only_mode, interp_only_mode_addr);
    cmp4(no_post, PR0, 0, interp_only_mode, Assembler::equal);
    br(no_post, jvmti_post_done);
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::post_method_entry));
    ld8(GR27_method, GR_Lmethod_addr);                // Reload GR27_method, call killed it
    bind(jvmti_post_done);
  }

  Label jvmpi_post_done;

  const Register access_flags_addr = GR2_SCRATCH;
  const Register event_flag_addr   = GR3_SCRATCH;
  const Register event2_flag_addr  = GR_RET;
  const Register access_flags      = access_flags_addr;// Address is dead after use
  const Register event_flag        = GR_RET1;
  const Register event2_flag       = event2_flag_addr; // "
  const Register receiver          = GR_O2;            // Where it'll end up anyway

  add(access_flags_addr, method_(access_flags));
  mova(event_flag_addr,
       (address)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY));

  ld4(access_flags, access_flags_addr);
  ld4(event_flag, event_flag_addr);
  add(event2_flag_addr,
      event_flag_addr,
      (int)((intptr_t)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY2) -
	    (intptr_t)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_ENTRY)));

  ld4(event2_flag, event2_flag_addr);
  ld8(receiver, GR_Ilocals);
  cmp4(no_post, PR0, GR0, GR0, Assembler::equal);   // no_post = 1;

  cmp4(no_post, PR0, JVMPI_EVENT_ENABLED, event_flag,  Assembler::notEqual, Assembler::And_);
  cmp4(no_post, PR0, JVMPI_EVENT_ENABLED, event2_flag, Assembler::notEqual, Assembler::And_);
  br(no_post, jvmpi_post_done);

  // Notify method entry
  tbit(is_static, PR0, access_flags, JVM_ACC_STATIC_BIT, Assembler::notEqual);
  mov(is_static, receiver, GR0);
  call_VM(noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_entry), GR27_method, receiver);
  ld8(GR27_method, GR_Lmethod_addr);                  // Reload GR27_method, call killed it
  bind(jvmpi_post_done);
}

 
void InterpreterMacroAssembler::notify_method_exit() {
  const PredicateRegister no_post = PR15_SCRATCH;

  // Whenever JVMTI puts a thread in interp_only_mode, method
  // entry/exit events are sent for that thread to track stack
  // depth.  If it is possible to enter interp_only_mode we add
  // the code to check if the event should be sent.
  if (JvmtiExport::can_post_interpreter_events()) {
    Label jvmti_post_done;

    const Register interp_only_mode   = GR2_SCRATCH;
    const Register interp_only_mode_addr   = interp_only_mode;

    add(interp_only_mode_addr, thread_(interp_only_mode));
    ld4(interp_only_mode, interp_only_mode_addr);
    cmp4(no_post, PR0, 0, interp_only_mode, Assembler::equal);
    br(no_post, jvmti_post_done);
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::post_method_exit));
    ld8(GR27_method, GR_Lmethod_addr);                // Reload GR27_method, call killed it
    bind(jvmti_post_done);
  }

  Label jvmpi_post_done;

  const Register event_flag_addr = GR2_SCRATCH;
  const Register event_flag      = event_flag_addr; // Address is dead after use

  // Notify method exit
  mova(event_flag_addr,
       (address)jvmpi::event_flags_array_at_addr(JVMPI_EVENT_METHOD_EXIT));
  ld4(event_flag, event_flag_addr);
  cmp4(no_post, PR0, JVMPI_EVENT_ENABLED, event_flag,  Assembler::notEqual);
  br(no_post, jvmpi_post_done);
  call_VM(noreg, CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_exit), GR27_method);
  ld8(GR27_method, GR_Lmethod_addr);                // Reload GR27_method, call killed it
  bind(jvmpi_post_done);
}
