#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)dump_md.cpp	1.2 04/07/29 16:36:04 JVM"
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

  // Allocate space for the dummy vtables.

  int num_methods = num_virtuals * vtbl_list_size;
  intptr_t vtable_bytes = num_methods * sizeof(void*);
  intptr_t descriptor_bytes = num_methods * sizeof(FuncDesc);
  *(intptr_t *)(*md_top) = vtable_bytes + descriptor_bytes;
  *md_top += sizeof(intptr_t);

  void** dummy_vtable = (void**)*md_top;
  *vtable = dummy_vtable;
  *md_top += vtable_bytes;

  // Allocate space for the dummy function descriptors.  Since one
  // function descriptor is needed per vtable entry, and they are twice
  // the size, allocate twice as much memory.

  FuncDesc* dummy_descriptors = (FuncDesc*)*md_top;
  *md_top += descriptor_bytes;

  guarantee(*md_top <= md_end, "Insufficient space for vtables.");

  // Get ready to generate dummy methods.

  CodeBuffer* cb = new CodeBuffer((unsigned char*)*mc_top, mc_end - *mc_top);
  MacroAssembler* masm = new MacroAssembler(cb);

  Label common_code;
  for (int i = 0; i < vtbl_list_size; ++i) {
    for (int j = 0; j < num_virtuals; ++j) {
      int index = num_virtuals * i + j;
      dummy_descriptors[index].set_entry(masm->pc());
      dummy_descriptors[index].set_gp(0);
      dummy_vtable[index] = &dummy_descriptors[index];

      // Load GR14 with a value indicating vtable/offset pair.
      // -- bits[ 7..0]  (8 bits) which virtual method in table?
      // -- bits[12..8]  (5 bits) which virtual method table?
      // -- must fit in 13-bit instruction immediate field.
      __ mov(GR14, (i << 8) + j);
      __ br(common_code);
      __ flush_bundle();

    }
  }

  __ bind(common_code);

  // Expecting to be called with the "this" pointer in O0/I0 (where
  // "this" is a Klass object).  In addition, L0 was set (above) to
  // identify the method and table.

  // Look up the correct vtable pointer.

  __ mova(GR15, (address)vtbl_list);
  __ shru(GR16, GR14, 8);		// Isolate GR16 = vtable identifier.
  __ shl(GR16, GR16, LogBytesPerWord);
  __ add(GR16, GR15, GR16);		// GR16 = new (correct) vtable pointer.
  __ ld8(GR15, GR16);			// GR16 = new (correct) vtable pointer.
  __ st8(GR_I0, GR15);			// Save correct vtable ptr in entry.

  // Where to jump to?

  __ mov(GR17, 255);
  __ and3(GR17, GR17, GR14);		// Isolate GR16 = method offset.
  __ shl(GR17, GR17, LogBytesPerWord);
  __ add(GR17, GR15, GR17);

  __ ld8(GR17, GR17);			// Get address of correct descriptor.
  __ adds(GR16, in_bytes(FuncDesc::gp_offset()), GR17); // Get address of GP
  __ ld8(GR17, GR17);			// Get address of function
  __ ld8(GP, GR16);			// Load GP for intended function.

  __ mov(BR6, GR17);
  __ br(BR6);				// Jump to correct method.

  __ flush_bundle();
  __ flush();

  *mc_top = (char*)__ pc();

  guarantee(*mc_top <= mc_end, "Insufficient space for method wrappers.");
}
