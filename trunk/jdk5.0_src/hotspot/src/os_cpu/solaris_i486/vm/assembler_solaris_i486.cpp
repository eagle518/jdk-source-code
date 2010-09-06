#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_solaris_i486.cpp	1.14 03/12/23 16:38:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_solaris_i486.cpp.incl"


void Assembler::int3() {
  pushl(eax);
  pushl(edx);
  pushl(ecx);
  call(CAST_FROM_FN_PTR(address, os::breakpoint), relocInfo::runtime_call_type);
  popl(ecx);
  popl(edx);
  popl(eax);
}

void MacroAssembler::get_thread(Register thread) {  

  // Try to emit a Solaris-specific fast TSD/TLS accessor.  
  ThreadLocalStorage::pd_tlsAccessMode tlsMode = ThreadLocalStorage::pd_getTlsAccessMode () ; 
  if (tlsMode == ThreadLocalStorage::pd_tlsAccessIndirect) { 		// T1
     // Use thread as a temporary: mov r, gs:[0]; mov r, [r+tlsOffset]
     emit_byte (Assembler::GS_segment) ; 			
     movl (thread, Address(0, relocInfo::none)) ; 
     movl (thread, Address(thread, ThreadLocalStorage::pd_getTlsOffset())) ; 
     return ; 
  } else 
  if (tlsMode == ThreadLocalStorage::pd_tlsAccessDirect) { 		// T2 
     // mov r, gs:[tlsOffset]
     emit_byte (Assembler::GS_segment) ; 			
     movl (thread, Address(ThreadLocalStorage::pd_getTlsOffset(), relocInfo::none)) ; 
     return ; 
  }

  // slow call to of thr_getspecific
  // int thr_getspecific(thread_key_t key, void **value);  
  // Consider using pthread_getspecific instead.  

  pushl(0);								// allocate space for return value
  if (thread != eax) pushl(eax);					// save eax if caller still wants it
  pushl(ecx);							        // save caller save
  pushl(edx);							        // save caller save
  if (thread != eax) {
    leal(thread, Address(esp, 3 * sizeof(int)));	                // address of return value
  } else {
    leal(thread, Address(esp, 2 * sizeof(int)));	                // address of return value
  }
  pushl(thread);							// and pass the address
  pushl(ThreadLocalStorage::thread_index());				// the key
  call(CAST_FROM_FN_PTR(address, thr_getspecific), relocInfo::runtime_call_type);
  increment(esp, 2 * wordSize);
  popl(edx);
  popl(ecx);
  if (thread != eax) popl(eax);
  popl(thread);
}

bool MacroAssembler::needs_explicit_null_check(int offset) {
  // Identical to Sparc/Solaris code
  bool offset_in_first_page =   0 <= offset  &&  offset < os::vm_page_size();
  return !offset_in_first_page;
}



