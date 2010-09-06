#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_win32_amd64.cpp	1.6 04/03/22 10:32:07 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// do not include  precompiled  header file
# include "incls/_os_win32_amd64.cpp.incl"

// Install a win32 structured exception handler around thread.
// AMD64 uses Vectored Exceptions so this isn't necessary.
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
  // On Intel Win32, virtual addresses never have the sign bit set.
  return (char*) -1;
}

void os::initialize_thread() {
// Nothing to do.
}


// Atomics and Stub Functions

typedef jint      xchg_func_t            (jint,     volatile jint*);
typedef intptr_t  xchg_ptr_func_t        (intptr_t, volatile intptr_t*);
typedef jint      cmpxchg_func_t         (jint,     volatile jint*,  jint);
typedef jlong     cmpxchg_long_func_t    (jlong,    volatile jlong*, jlong);
typedef jint      add_func_t             (jint,     volatile jint*);
typedef intptr_t  add_ptr_func_t         (intptr_t, volatile intptr_t*);
typedef void      fence_func_t           ();


jint os::atomic_xchg_bootstrap(jint exchange_value, volatile jint* dest) {
  // try to use the stub:
  xchg_func_t* func = CAST_TO_FN_PTR(xchg_func_t*, StubRoutines::atomic_xchg_entry());

  if (func != NULL) {
    os::atomic_xchg_func = func;
    return (*func)(exchange_value, dest);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jint old_value = *dest;
  *dest = exchange_value;
  return old_value;
}


intptr_t os::atomic_xchg_ptr_bootstrap(intptr_t exchange_value, volatile intptr_t* dest) {
  // try to use the stub:
  xchg_ptr_func_t* func = CAST_TO_FN_PTR(xchg_ptr_func_t*, StubRoutines::atomic_xchg_ptr_entry());

  if (func != NULL) {
    os::atomic_xchg_ptr_func = func;
    return (*func)(exchange_value, dest);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  intptr_t old_value = *dest;
  *dest = exchange_value;
  return old_value;
}


jint os::atomic_cmpxchg_bootstrap(jint exchange_value, volatile jint* dest, jint compare_value) {
  // try to use the stub:
  cmpxchg_func_t* func = CAST_TO_FN_PTR(cmpxchg_func_t*, StubRoutines::atomic_cmpxchg_entry());

  if (func != NULL) {
    os::atomic_cmpxchg_func = func;
    return (*func)(exchange_value, dest, compare_value);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jint old_value = *dest;
  if (old_value == compare_value)
    *dest = exchange_value;
  return old_value;
}

jlong os::atomic_cmpxchg_long_bootstrap(jlong exchange_value, volatile jlong* dest, jlong compare_value) {
  // try to use the stub:
  cmpxchg_long_func_t* func = CAST_TO_FN_PTR(cmpxchg_long_func_t*, StubRoutines::atomic_cmpxchg_long_entry());

  if (func != NULL) {
    os::atomic_cmpxchg_long_func = func;
    return (*func)(exchange_value, dest, compare_value);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jlong old_value = *dest;
  if (old_value == compare_value)
    *dest = exchange_value;
  return old_value;
}

jint os::atomic_add_bootstrap(jint add_value, volatile jint* dest) {
  // try to use the stub:
  add_func_t* func = CAST_TO_FN_PTR(add_func_t*, StubRoutines::atomic_add_entry());

  if (func != NULL) {
    os::atomic_add_func = func;
    return (*func)(add_value, dest);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  return (*dest) += add_value;
}

intptr_t os::atomic_add_ptr_bootstrap(intptr_t add_value, volatile intptr_t* dest) {
  // try to use the stub:
  add_ptr_func_t* func = CAST_TO_FN_PTR(add_ptr_func_t*, StubRoutines::atomic_add_ptr_entry());

  if (func != NULL) {
    os::atomic_add_ptr_func = func;
    return (*func)(add_value, dest);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  return (*dest) += add_value;
}

void os::fence_bootstrap() {
  // try to use the stub:
  fence_func_t* func = CAST_TO_FN_PTR(fence_func_t*, StubRoutines::fence_entry());

  if (func != NULL) {
    os::fence_func = func;
    (*func)();
    return;
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  // don't have to do anything for a single thread
}


xchg_func_t*         os::atomic_xchg_func         = os::atomic_xchg_bootstrap;
xchg_ptr_func_t*     os::atomic_xchg_ptr_func     = os::atomic_xchg_ptr_bootstrap;
cmpxchg_func_t*      os::atomic_cmpxchg_func      = os::atomic_cmpxchg_bootstrap;
cmpxchg_long_func_t* os::atomic_cmpxchg_long_func = os::atomic_cmpxchg_long_bootstrap;
add_func_t*          os::atomic_add_func          = os::atomic_add_bootstrap;
add_ptr_func_t*      os::atomic_add_ptr_func      = os::atomic_add_ptr_bootstrap;
fence_func_t*        os::fence_func               = os::fence_bootstrap;


ExtendedPC os::fetch_frame_from_context(void* ucVoid, 
                    intptr_t** ret_sp, intptr_t** ret_fp) {

  ExtendedPC  epc;
  CONTEXT* uc = (CONTEXT*)ucVoid;

  if (uc != NULL) {
    epc = ExtendedPC((address)uc->Rip);
    if (ret_sp) *ret_sp = (intptr_t*)uc->Rsp;
    if (ret_fp) *ret_fp = (intptr_t*)uc->Rbp;
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
  return frame(sp, fp, epc.pc());
}

// VC++ does not save frame pointer on stack in optimized build. It
// can be turned off by /Oy-. If we really want to walk C frames,
// we can use the StackWalk() API.
frame os::get_sender_for_C_frame(frame* fr) {
  return frame(fr->sender_sp(), fr->link(), fr->sender_pc());
}

frame os::current_frame() {
  intptr_t* fp = (*CAST_TO_FN_PTR( intptr_t* (*)(void), StubRoutines::amd64::get_previous_fp_entry()))();
  frame myframe((intptr_t*)os::current_stack_pointer(), 
                (intptr_t*)fp,
                CAST_FROM_FN_PTR(address, os::current_frame));
  if (os::is_first_C_frame(&myframe)) {
    // stack is not walkable
    return frame(NULL, NULL, NULL);
  } else {
    return os::get_sender_for_C_frame(&myframe);
  }
}

void os::print_context(outputStream *st, void *context) {
  if (context == NULL) return;

  CONTEXT* uc = (CONTEXT*)context;

  st->print_cr("Registers:");
  st->print(  "EAX=" INTPTR_FORMAT, uc->Rax);
  st->print(", EBX=" INTPTR_FORMAT, uc->Rbx);
  st->print(", ECX=" INTPTR_FORMAT, uc->Rcx);
  st->print(", EDX=" INTPTR_FORMAT, uc->Rdx);
  st->cr();
  st->print(  "ESP=" INTPTR_FORMAT, uc->Rsp);
  st->print(", EBP=" INTPTR_FORMAT, uc->Rbp);
  st->print(", ESI=" INTPTR_FORMAT, uc->Rsi);
  st->print(", EDI=" INTPTR_FORMAT, uc->Rdi);
  st->cr();
  st->print(  "EIP=" INTPTR_FORMAT, uc->Rip);
  st->print(", EFLAGS=" INTPTR_FORMAT, uc->EFlags);
  st->cr();
  st->cr();

  intptr_t *sp = (intptr_t *)uc->Rsp;
  st->print_cr("Top of Stack: (sp=" PTR_FORMAT ")", sp);
  print_hex_dump(st, (address)sp, (address)(sp + 32), sizeof(intptr_t));
  st->cr();

  // Note: it may be unsafe to inspect memory near pc. For example, pc may
  // point to garbage if entry point in an nmethod is corrupted. Leave
  // this at the end, and hope for the best.
  address pc = (address)uc->Rip;
  st->print_cr("Instructions: (pc=" PTR_FORMAT ")", pc);
  print_hex_dump(st, pc - 16, pc + 16, sizeof(char));
  st->cr();
}

