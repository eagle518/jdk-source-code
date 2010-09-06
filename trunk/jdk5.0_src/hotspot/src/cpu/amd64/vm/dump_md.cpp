#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)dump_md.cpp	1.2 04/07/29 16:35:59 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_dump_md.cpp.incl"



// Generate the self-patching vtable method:
//
// This method will be called (as any other Klass virtual method) with
// the Klass itself as the first argument.  Example:
//
// 	oop obj;
// 	int size = obj->klass()->klass_part()->oop_size(this);
//
// for which the virtual method call is Klass::oop_size();
//
// The dummy method is called with the Klass object as the first
// operand, and an object as the second argument.
//

//=====================================================================

// All of the dummy methods in the vtable are essentially identical,
// differing only by an ordinal constant, and they bear no releationship
// to the original method which the caller intended. Also, there needs
// to be 'vtbl_list_size' instances of the vtable in order to
// differentiate between the 'vtable_list_size' original Klass objects.

#define __ masm->

void CompactingPermGenGen::generate_vtable_methods(void** vtbl_list,
                                                   void** vtable,
                                                   char** md_top,
                                                   char* md_end,
                                                   char** mc_top,
                                                   char* mc_end) {

  intptr_t vtable_bytes = (num_virtuals * vtbl_list_size) * sizeof(void*);
  *(intptr_t *)(*md_top) = vtable_bytes;
  *md_top += sizeof(intptr_t);
  void** dummy_vtable = (void**)*md_top;
  *vtable = dummy_vtable;
  *md_top += vtable_bytes;

  // Get ready to generate dummy methods.

  CodeBuffer* cb = new CodeBuffer((unsigned char*)*mc_top, mc_end - *mc_top);
  MacroAssembler* masm = new MacroAssembler(cb);

  Label common_code;
  for (int i = 0; i < vtbl_list_size; ++i) {
    for (int j = 0; j < num_virtuals; ++j) {
      dummy_vtable[num_virtuals * i + j] = (void*)masm->pc();

      // Load eax with a value indicating vtable/offset pair.
      // -- bits[ 7..0]  (8 bits) which virtual method in table?
      // -- bits[12..8]  (5 bits) which virtual method table?
      // -- must fit in 13-bit instruction immediate field.
      __ movl(rax, (i << 8) + j);
      __ jmp(common_code);
    }
  }

  __ bind(common_code);

  // Expecting to be called with "thiscall" convections -- the arguments
  // are on the stack and the "this" pointer is in ecx. In addition, eax
  // was set (above) to the offset of the method in the table.

  __ pushq(rarg1);			// save & free register
  __ pushq(rarg0);			// save "this"
  __ movq(rarg0, rax);
  __ shrq(rarg0, 8);			// isolate vtable identifier.
  __ shlq(rarg0, LogBytesPerWord);
  __ movq(rarg1, (int64_t)vtbl_list);	// ptr to correct vtable list.
  __ addq(rarg1, rarg0);		// ptr to list entry.
  __ movq(rarg1, Address(rarg1));	// get correct vtable address.
  __ popq(rarg0);			// restore "this"
  __ movq(Address(rarg0, 0), rarg1);	// update vtable pointer.

  __ andq(rax, 0x00ff);			// isolate vtable method index
  __ shlq(rax, LogBytesPerWord);
  __ addq(rax, rarg1);			// address of real method pointer.
  __ popq(rarg1);			// restore register.
  __ movq(rax, Address(rax, 0));	// get real method pointer.
  __ jmp(rax);				// jump to the real method.

  __ flush();

  *mc_top = (char*)__ pc();
}

