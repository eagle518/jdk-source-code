#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_win32_ia64.cpp	1.9 03/06/06 11:57:32 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// do not include  precompiled  header file
# include "incls/_os_win32_ia64.cpp.incl"

// Install a win32 structured exception handler around thread.
// IA64 uses Vectored Exceptions so this isn't necessary.
void os::os_exception_wrapper(java_call_t f, JavaValue* value, methodHandle* method, JavaCallArguments* args, Thread* thread) {
    f(value, method, args, thread);
}

char* os::reserve_memory(size_t bytes, char* addr) {
  assert((size_t) addr % os::vm_allocation_granularity() == 0, "reserve alignment");
  assert(bytes % os::vm_allocation_granularity() == 0, "reserve block size");
  char* res = (char*) VirtualAlloc(addr, bytes, MEM_RESERVE,
                                   PAGE_EXECUTE_READWRITE);
  assert(addr == NULL || addr == res, "Unexpected address from reserve.");
  return res;
}

char* os::non_memory_address_word() {
  // Must never look like an address returned by reserve_memory,
  // even in its subfields (as defined by the CPU immediate fields,
  // if the CPU splits constants across multiple instructions).
  return (char*) -1;
}


// OS specific thread initialization
//
// For Itanium, we calculate and store the limits of the 
// register and memory stacks.  
//
void os::initialize_thread() {
  address mem_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  // Initialize our register stack limit which is our guard
  JavaThread::enable_register_stack_guard();

  // Initialize our memory stack limit 
  mem_stk_limit = thread->stack_base() - thread->stack_size() +
                  ((StackShadowPages + StackYellowPages +
                  StackRedPages) * os::vm_page_size());
 
  thread->set_memory_stack_limit( mem_stk_limit );
}


// Atomic operations.

typedef jint     xchg_func_t        (jint,     volatile jint*);
typedef intptr_t xchg_ptr_func_t    (intptr_t, volatile intptr_t*);
typedef jint     cmpxchg_func_t     (jint,     volatile jint*,     jint);
typedef intptr_t cmpxchg_ptr_func_t (intptr_t, volatile intptr_t*, intptr_t);
typedef jlong    cmpxchg_long_func_t(jlong,    volatile jlong*,    jlong);
typedef jint     add_func_t         (jint,     volatile jint*);
typedef intptr_t add_ptr_func_t     (intptr_t, volatile intptr_t*);
typedef void     fence_func_t       (void);

jint Atomic::xchg(jint exchange_value, volatile jint* dest) {
  // See if the stub is there.
  address func = StubRoutines::atomic_xchg_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    jint old_value = *dest;
    *dest = exchange_value;
    return old_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(xchg_func_t*, func))(exchange_value, dest);
}

intptr_t Atomic::xchg_ptr(intptr_t exchange_value, volatile intptr_t* dest) {
  // See if the stub is there.
  address func = StubRoutines::atomic_xchg_ptr_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    intptr_t old_value = *dest;
    *dest = exchange_value;
    return old_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(xchg_ptr_func_t*, func))(exchange_value, dest);
}

jint Atomic::cmpxchg(jint exchange_value, volatile jint* dest, jint compare_value) {
  // See if the stub is there.
  address func = StubRoutines::atomic_cmpxchg_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    jint old_value = *dest;
    if (old_value == compare_value)
      *dest = exchange_value;
    return old_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(cmpxchg_func_t*, func))(exchange_value, dest, compare_value);
}

jlong Atomic::cmpxchg(jlong exchange_value, volatile jlong* dest, jlong compare_value) {
  // See if the stub is there.
  address func = StubRoutines::atomic_cmpxchg_long_entry();

  if (func == NULL) {
    // No stub.
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    jlong old_value = *dest;
    if (old_value == compare_value)
       *dest = exchange_value;
    return old_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(cmpxchg_long_func_t*, func))(exchange_value, dest, compare_value);
}

jint Atomic::add(jint add_value, volatile jint* dest) {
  // See if the stub is there.
  address func = StubRoutines::atomic_add_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    return (*dest) += add_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(add_func_t*, func))(add_value, dest);
}

intptr_t Atomic::add_ptr(intptr_t add_value, volatile intptr_t* dest) {
  // See if the stub is there.
  address func = StubRoutines::atomic_add_ptr_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    return (*dest) += add_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(add_ptr_func_t*, func))(add_value, dest);
}

void OrderAccess::fence(void) {
  // See if the stub is there.
  address func = StubRoutines::fence_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    return;
  }

  // Do the operation.
  (*CAST_TO_FN_PTR(fence_func_t*, func))();
}

// Register Stack Management Routines

// Check to see if the current BSP is within our current guard
// page area.
bool JavaThread::register_stack_overflow()  {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return false;

  reg_stk_limit = thread->stack_base() + thread->stack_size() -
                  ((StackShadowPages + StackYellowPages + 
                  StackRedPages) * os::vm_page_size());

  if ( StubRoutines::ia64::get_backing_store_pointer() > reg_stk_limit )
    return true;
  else
    return false;
}

// Set the guard page address to it's normal guarded position.
// Compiled code and interpreter entry compares the current
// BSP to this address to check for overflow.

void JavaThread::enable_register_stack_guard() {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  // We assume that the register stack is the same size as the memory stack and that
  // it starts at the beginning stack address and grows higher.  The memory stack
  // grows to lower addresses.

  reg_stk_limit = thread->stack_base() + thread->stack_size() -
                  ((StackShadowPages + StackYellowPages + 
                  StackRedPages) * os::vm_page_size());
  
  thread->set_register_stack_limit( reg_stk_limit );
}

// Reduce the guard page by YellowZonePages to allow for the processing
// of register stack overflow exceptions.
void JavaThread::disable_register_stack_guard() {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  // We assume that the register stack is the same size as the memory stack and that
  // it starts at the beginning stack address and grows higher.  The memory stack
  // grows to lower addresses.

  reg_stk_limit = thread->stack_base() + thread->stack_size() -
                  ((StackShadowPages + StackRedPages) * os::vm_page_size());
 
  thread->set_register_stack_limit( reg_stk_limit );
}

ExtendedPC os::fetch_frame_from_context(void* ucVoid,
                    intptr_t** ret_sp, intptr_t** ret_fp) {
  ExtendedPC  epc;
  CONTEXT* uc = (CONTEXT*)ucVoid;
  
  if (uc != NULL) {
    epc = ExtendedPC((address)uc->StIIP);
    if (ret_sp) *ret_sp = (intptr_t*)uc->IntSp;
    if (ret_fp) *ret_fp = (intptr_t*)uc->RsBSP;
  } else {
    // construct empty ExtendedPC for return value checking
    epc = ExtendedPC(NULL);
    if (ret_sp) *ret_sp = (intptr_t *)NULL;
    if (ret_fp) *ret_fp = (intptr_t *)NULL;
  }
  
  return epc;
}

frame os::fetch_frame_from_context(void* ucVoid) {
  intptr_t* sp;
  intptr_t* fp;
  ExtendedPC epc = fetch_frame_from_context(ucVoid, &sp, &fp);
  frame fr(sp, fp);
  fr.set_pc(epc.pc());
  return fr;
}

void os::print_context(outputStream *st, void *context) {
  if (context == NULL) return;

  st->print_cr("Registers:");

  CONTEXT* uc = (CONTEXT*)context;

  st->print   ("unat=" INTPTR_FORMAT "  ", uc->ApUNAT); // User NaT collection
  st->print   ("lc="   INTPTR_FORMAT "  ", uc->ApLC);   // Loop counter
  st->print_cr("ccv="  INTPTR_FORMAT "  ", uc->ApCCV);  // CMPXCHG register

  st->print   ("pfs=" INTPTR_FORMAT "  ", uc->RsPFS);   // Previous function state
  st->print   ("bsp=" INTPTR_FORMAT "  ", uc->RsBSP);   // Backing store pointer
  st->print_cr("bspstore=" INTPTR_FORMAT "  ", uc->RsBSPSTORE);
  st->print   ("rsc=" INTPTR_FORMAT "  ", uc->RsRSC);   // RSE config
  st->print_cr("rnat=" INTPTR_FORMAT "  ", uc->RsRNAT); // RSE NaT collection

  st->print   ("ip=" INTPTR_FORMAT "  ", uc->StIIP);    // IP
  st->print_cr("pr=" INTPTR_FORMAT "  ", uc->Preds);    // Predicates

  // b0-b7
  st->print   ("b0=" INTPTR_FORMAT "  ", uc->BrRp);
  st->print   ("b1=" INTPTR_FORMAT "  ", uc->BrS0);
  st->print_cr("b2=" INTPTR_FORMAT "  ", uc->BrS1);
  st->print   ("b3=" INTPTR_FORMAT "  ", uc->BrS2);
  st->print   ("b4=" INTPTR_FORMAT "  ", uc->BrS3);
  st->print_cr("b5=" INTPTR_FORMAT "  ", uc->BrS4);
  st->print   ("b6=" INTPTR_FORMAT "  ", uc->BrT0);
  st->print   ("b7=" INTPTR_FORMAT "  ", uc->BrT1);
  st->cr();

  // r0-r31 (sp is r12)
  st->print   ("r0=" INTPTR_FORMAT "  ", (intptr_t)0);
  st->print   ("r1=" INTPTR_FORMAT "  ", uc->IntGp);
  st->print_cr("r2=" INTPTR_FORMAT "  ", uc->IntT0);
  st->print   ("r3=" INTPTR_FORMAT "  ", uc->IntT1);
  st->print   ("r4=" INTPTR_FORMAT "  ", uc->IntS0);
  st->print_cr("r5=" INTPTR_FORMAT "  ", uc->IntS1);
  st->print   ("r6=" INTPTR_FORMAT "  ", uc->IntS2);
  st->print   ("r7=" INTPTR_FORMAT "  ", uc->IntS3);
  st->print_cr("r8=" INTPTR_FORMAT "  ", uc->IntV0);
  st->print   ("r9=" INTPTR_FORMAT "  ", uc->IntT2);
  st->print   ("r10=" INTPTR_FORMAT " ", uc->IntT3);
  st->print_cr("r11=" INTPTR_FORMAT " ", uc->IntT4);
  st->print   ("r12=" INTPTR_FORMAT " ", uc->IntSp);
  st->print   ("r13=" INTPTR_FORMAT " ", uc->IntTeb);
  st->print_cr("r14=" INTPTR_FORMAT " ", uc->IntT5);
  st->print   ("r15=" INTPTR_FORMAT " ", uc->IntT6);
  st->print   ("r16=" INTPTR_FORMAT " ", uc->IntT7);
  st->print_cr("r17=" INTPTR_FORMAT " ", uc->IntT8);
  st->print   ("r18=" INTPTR_FORMAT " ", uc->IntT9);
  st->print   ("r19=" INTPTR_FORMAT " ", uc->IntT10);
  st->print_cr("r20=" INTPTR_FORMAT " ", uc->IntT11);
  st->print   ("r21=" INTPTR_FORMAT " ", uc->IntT12);
  st->print   ("r22=" INTPTR_FORMAT " ", uc->IntT13);
  st->print_cr("r23=" INTPTR_FORMAT " ", uc->IntT14);
  st->print   ("r24=" INTPTR_FORMAT " ", uc->IntT15);
  st->print   ("r25=" INTPTR_FORMAT " ", uc->IntT16);
  st->print_cr("r26=" INTPTR_FORMAT " ", uc->IntT17);
  st->print   ("r27=" INTPTR_FORMAT " ", uc->IntT18);
  st->print   ("r28=" INTPTR_FORMAT " ", uc->IntT19);
  st->print_cr("r29=" INTPTR_FORMAT " ", uc->IntT20);
  st->print   ("r30=" INTPTR_FORMAT " ", uc->IntT21);
  st->print   ("r31=" INTPTR_FORMAT " ", uc->IntT22);
  st->cr();
  st->cr();

  intptr_t *sp = (intptr_t *)uc->IntSp;
  st->print_cr("Top of Stack: (sp=" PTR_FORMAT ")", sp);
  print_hex_dump(st, (address)sp, (address)(sp + 32), sizeof(intptr_t));
  st->cr();

  intptr_t *fp = (intptr_t *)uc->RsBSP;
  st->print_cr("Top of Register Stack: (bsp=" PTR_FORMAT ")", fp);
  print_hex_dump(st, (address)(fp - 31), (address)(fp + 1), sizeof(intptr_t));
  st->cr();

  address pc = (address)uc->StIIP;
  st->print_cr("Instructions: (pc=" PTR_FORMAT ")", pc);
  print_hex_dump(st, pc - 16, pc + 16, sizeof(char));
}

// IA64 does not support C stack walking
frame os::get_sender_for_C_frame(frame* fr) {
  frame f(NULL, NULL);
  f.set_pc(NULL);
  return f;
}

frame os::current_frame() {
  frame fr(NULL, NULL);
  fr.set_pc(NULL);
  return fr;
}

