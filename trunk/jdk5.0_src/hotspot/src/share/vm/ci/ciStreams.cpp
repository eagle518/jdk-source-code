#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciStreams.cpp	1.18 03/12/23 16:39:39 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciStreams.cpp.incl"

// ciExceptionHandlerStream
//
// Walk over some selected set of a methods exception handlers.

// ------------------------------------------------------------------
// ciExceptionHandlerStream::count
//
// How many exception handlers are there in this stream?
//
// Implementation note: Compiler2 needs this functionality, so I had
int ciExceptionHandlerStream::count() {
  int save_pos = _pos;
  int save_end = _end;

  int count = 0;

  _pos = -1;
  _end = _method->_handler_count;


  next();
  while (!is_done()) {
    count++;
    next();
  }

  _pos = save_pos;
  _end = save_end;

  return count;
}

int ciExceptionHandlerStream::count_remaining() {
  int save_pos = _pos;
  int save_end = _end;

  int count = 0;

  while (!is_done()) {
    count++;
    next();
  }

  _pos = save_pos;
  _end = save_end;

  return count;
}

// ciByteCodeStream
//
// The class is used to iterate over the bytecodes of a method.
// It hides the details of constant pool structure/access by
// providing accessors for constant pool items.

// ------------------------------------------------------------------
// ciByteCodeStream::wide
//
// Special handling for the wide bytcode
Bytecodes::Code ciByteCodeStream::wide()
{
  // Get following bytecode; do not return wide
  Bytecodes::Code bc = (Bytecodes::Code)_pc[1];	
  _pc += 2;			// Skip both bytecodes
  _pc += 2;			// Skip index always
  if( bc == Bytecodes::_iinc ) 
    _pc += 2;			// Skip optional constant
  _was_wide = _pc;		// Flag last wide bytecode found
  return bc;
}

// ------------------------------------------------------------------
// ciByteCodeStream::table
//
// Special handling for switch ops
Bytecodes::Code ciByteCodeStream::table( Bytecodes::Code bc ) {
  switch( bc ) {		// Check for special bytecode handling
    
  case Bytecodes::_fast_linearswitch: 
  case Bytecodes::_fast_binaryswitch: 
  case Bytecodes::_lookupswitch:
    _pc++;			// Skip wide bytecode
    _pc += (_start-_pc)&3;	// Word align
    _table_base = (jint*)_pc;	// Capture for later usage
				// table_base[0] is default far_dest
    // Table has 2 lead elements (default, length), then pairs of u4 values.
    // So load table length, and compute address at end of table
    _pc = (address)&_table_base[2+ 2*Bytes::get_Java_u4((address)&_table_base[1])];
    break;

  case Bytecodes::_tableswitch: { 
    _pc++;			// Skip wide bytecode
    _pc += (_start-_pc)&3;	// Word align
    _table_base = (jint*)_pc;	// Capture for later usage
				// table_base[0] is default far_dest
    int lo = Bytes::get_Java_u4((address)&_table_base[1]);// Low bound
    int hi = Bytes::get_Java_u4((address)&_table_base[2]);// High bound
    int len = hi - lo + 1;	// Dense table size
    _pc = (address)&_table_base[3+len];	// Skip past table
    break;
  }

  default:			 
    fatal("unhandled bytecode");	 
  }
  return bc;
}

// ------------------------------------------------------------------
// ciByteCodeStream::EOBCs
//
// Handle End-Of-Bytecodes.  Special handling for _go_native here.
Bytecodes::Code ciByteCodeStream::EOBCs() {
  return (Bytecodes::Code)EOBC;
}

// ------------------------------------------------------------------
// ciByteCodeStream::reset_to_bci
void ciByteCodeStream::reset_to_bci( int bci ) {
  _bc_start=_was_wide=0; 
  _pc = _start+bci; 
}

// ------------------------------------------------------------------
// ciByteCodeStream::force_bci
void ciByteCodeStream::force_bci(int bci) {
  if (bci < 0) {
    reset_to_bci(0);
    _bc_start = _start + bci;
    _bc = EOBCs();
  } else {
    reset_to_bci(bci);
    next();
  }
}

// ------------------------------------------------------------------
// ciByteCodeStream::java
//
// Convert internal opcodes to standard

Bytecodes::Code ciByteCodeStream::java( Bytecodes::Code bc )
{
  // Handle fast variants
  // The java_code in the bytecode definition is what the original
  // bytecode was before it was rewritten.  Since the rest of the
  // bytecode and pair stays the same, all we have to do is return the
  // original bytecode and it's original length.
  bc = Bytecodes::java_code(bc);
  _pc += Bytecodes::java_length_at(_pc);
  return bc;
}

// ------------------------------------------------------------------
// Constant pool access
// ------------------------------------------------------------------

// ------------------------------------------------------------------
// ciByteCodeStream::get_klass_index
//
// If this bytecodes references a klass, return the index of the 
// referenced klass.
int ciByteCodeStream::get_klass_index() {
  assert(cur_bc() == Bytecodes::_checkcast ||
         cur_bc() == Bytecodes::_instanceof ||
         cur_bc() == Bytecodes::_anewarray ||
         cur_bc() == Bytecodes::_multianewarray ||
         cur_bc() == Bytecodes::_new ||
         cur_bc() == Bytecodes::_newarray, "wrong bc");
  return get_index_big();
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_klass
//
// If this bytecode is a new, newarray, multianewarray, instanceof,
// or checkcast, get the referenced klass.
ciKlass* ciByteCodeStream::get_klass(bool& will_link) {
  return CURRENT_ENV->get_klass_by_index(_holder, get_klass_index(),
                                         will_link);
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_constant_index
//
// If this bytecode is one of the ldc variants, get the index of the
// referenced constant.
int ciByteCodeStream::get_constant_index() const {
  switch(cur_bc()) {
  case Bytecodes::_ldc:
    return get_index();
  case Bytecodes::_ldc_w:
  case Bytecodes::_ldc2_w:
    return get_index_big();
  default:
    ShouldNotReachHere();
    return 0;
  }
}
// ------------------------------------------------------------------
// ciByteCodeStream::get_constant
//
// If this bytecode is one of the ldc variants, get the referenced
// constant.
ciConstant ciByteCodeStream::get_constant() {
  return CURRENT_ENV->get_constant_by_index(_holder, get_constant_index());
}

// ------------------------------------------------------------------
bool ciByteCodeStream::is_unresolved_string() const {
  return CURRENT_ENV->is_unresolved_string(_holder, get_constant_index());
}

// ------------------------------------------------------------------
bool ciByteCodeStream::is_unresolved_klass() const {
  return CURRENT_ENV->is_unresolved_klass(_holder, get_constant_index());
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_field_index
//
// If this is a field access bytecode, get the constant pool
// index of the referenced field.
int ciByteCodeStream::get_field_index() {
  assert(cur_bc() == Bytecodes::_getfield ||
         cur_bc() == Bytecodes::_putfield ||
         cur_bc() == Bytecodes::_getstatic ||
         cur_bc() == Bytecodes::_putstatic, "wrong bc");
  return get_index_big();
}


// ------------------------------------------------------------------
// ciByteCodeStream::get_field
//
// If this bytecode is one of get_field, get_static, put_field,
// or put_static, get the referenced field.
ciField* ciByteCodeStream::get_field(bool& will_link) {
  ciField* f = CURRENT_ENV->get_field_by_index(_holder, get_field_index());
  will_link = f->will_link(_holder, _bc);
  return f;
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_declared_field_holder
//
// Get the declared holder of the currently referenced field.
//
// Usage note: the holder() of a ciField class returns the canonical
// holder of the field, rather than the holder declared in the
// bytecodes.
//
// There is no "will_link" result passed back.  The user is responsible
// for checking linkability when retrieving the associated field.
ciInstanceKlass* ciByteCodeStream::get_declared_field_holder() {
  int holder_index = get_field_holder_index();
  bool ignore;
  return CURRENT_ENV->get_klass_by_index(_holder, holder_index, ignore)
      ->as_instance_klass();
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_field_holder_index
//
// Get the constant pool index of the declared holder of the field
// referenced by the current bytecode.  Used for generating
// deoptimization information.
int ciByteCodeStream::get_field_holder_index() {
  VM_ENTRY_MARK;
  constantPoolOop cpool = _holder->get_instanceKlass()->constants();
  return cpool->klass_ref_index_at(get_field_index());
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_field_signature_index
//
// Get the constant pool index of the signature of the field
// referenced by the current bytecode.  Used for generating
// deoptimization information.
int ciByteCodeStream::get_field_signature_index() {
  VM_ENTRY_MARK;
  constantPoolOop cpool = _holder->get_instanceKlass()->constants();
  int nt_index = cpool->name_and_type_ref_index_at(get_field_index());
  return cpool->signature_ref_index_at(nt_index);
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_method_index
//
// If this is a method invocation bytecode, get the constant pool
// index of the invoked method.
int ciByteCodeStream::get_method_index() {
  switch (cur_bc()) {
  case Bytecodes::_invokeinterface:
    return Bytes::get_Java_u2(_pc-4);
  case Bytecodes::_invokevirtual:
  case Bytecodes::_invokespecial:
  case Bytecodes::_invokestatic:
    return get_index_big();
  default:
    ShouldNotReachHere();
    return 0;
  }  
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_method
//
// If this is a method invocation bytecode, get the invoked method.
ciMethod* ciByteCodeStream::get_method(bool& will_link) {
  ciMethod* m = CURRENT_ENV->get_method_by_index(_holder, get_method_index(),cur_bc());
  will_link = m->is_loaded();
  return m;
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_declared_method_holder
//
// Get the declared holder of the currently referenced method.
//
// Usage note: the holder() of a ciMethod class returns the canonical
// holder of the method, rather than the holder declared in the
// bytecodes.
//
// There is no "will_link" result passed back.  The user is responsible
// for checking linkability when retrieving the associated method.
ciKlass* ciByteCodeStream::get_declared_method_holder() {
  bool ignore;
  return CURRENT_ENV->get_klass_by_index(_holder, get_method_holder_index(), ignore);
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_method_holder_index
//
// Get the constant pool index of the declared holder of the method
// referenced by the current bytecode.  Used for generating
// deoptimization information.
int ciByteCodeStream::get_method_holder_index() {
  VM_ENTRY_MARK;
  constantPoolOop cpool = _holder->get_instanceKlass()->constants();
  return cpool->klass_ref_index_at(get_method_index());
}

// ------------------------------------------------------------------
// ciByteCodeStream::get_method_signature_index
//
// Get the constant pool index of the signature of the method
// referenced by the current bytecode.  Used for generating
// deoptimization information.
int ciByteCodeStream::get_method_signature_index() {
  VM_ENTRY_MARK;
  constantPoolOop cpool = _holder->get_instanceKlass()->constants();
  int method_index = get_method_index();
  int name_and_type_index = cpool->name_and_type_ref_index_at(method_index);
  return cpool->signature_ref_index_at(name_and_type_index);
}


