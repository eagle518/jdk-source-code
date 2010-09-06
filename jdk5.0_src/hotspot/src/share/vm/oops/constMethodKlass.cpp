#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)constMethodKlass.cpp	1.4 03/12/23 16:41:45 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_constMethodKlass.cpp.incl"


klassOop constMethodKlass::create_klass(TRAPS) {
  constMethodKlass o;
  KlassHandle h_this_klass(THREAD, Universe::klassKlassObj());  
  KlassHandle k = base_create_klass(h_this_klass, header_size(),
                                    o.vtbl_value(), CHECK_0);
  // Make sure size calculation is right
  assert(k()->size() == align_object_size(header_size()),
         "wrong size for object");
  //java_lang_Class::create_mirror(k, CHECK_0); // Allocate mirror
  return k();
}


int constMethodKlass::oop_size(oop obj) const {
  assert(obj->is_constMethod(), "must be constMethod oop");
  return constMethodOop(obj)->object_size();
}


constMethodOop constMethodKlass::allocate(int byte_code_size,
                                          int compressed_line_number_size,
                                          int localvariable_table_length,
                                          int checked_exceptions_length,
                                          TRAPS) {

  int size = constMethodOopDesc::object_size(byte_code_size,
                                             compressed_line_number_size,
                                             localvariable_table_length,
                                             checked_exceptions_length);
  KlassHandle h_k(THREAD, as_klassOop());
  constMethodOop cm = (constMethodOop)
    CollectedHeap::permanent_obj_allocate(h_k, size, CHECK_0);
  cm->set_interpreter_kind(AbstractInterpreter::invalid);
  cm->init_fingerprint();
  cm->set_method(NULL);
  cm->set_exception_table(NULL);
  cm->set_stackmap_u1(NULL);
  cm->set_stackmap_u2(NULL);
  cm->set_code_size(byte_code_size);
  cm->set_constMethod_size(size);
  cm->set_inlined_tables_length(checked_exceptions_length,
                                compressed_line_number_size,
                                localvariable_table_length);
  assert(cm->size() == size, "wrong size for object");
  NOT_PRODUCT(cm->set_partially_loaded());     
  return cm;
}


void constMethodKlass::oop_follow_contents(oop obj) {
  assert (obj->is_constMethod(), "object must be constMethod");
  constMethodOop cm = constMethodOop(obj);
  MarkSweep::mark_and_push(cm->adr_method());
  MarkSweep::mark_and_push(cm->adr_exception_table());
  MarkSweep::mark_and_push(cm->adr_stackmap_u1());
  MarkSweep::mark_and_push(cm->adr_stackmap_u2());
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constMethodKlassObj never moves.
}


int constMethodKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert (obj->is_constMethod(), "object must be constMethod");
  constMethodOop cm = constMethodOop(obj);
  blk->do_oop(cm->adr_method());
  blk->do_oop(cm->adr_exception_table());
  blk->do_oop(cm->adr_stackmap_u1());
  blk->do_oop(cm->adr_stackmap_u2());
  // Get size before changing pointers. 
  // Don't call size() or oop_size() since that is a virtual call.
  int size = cm->object_size();  
  return size;
}


int constMethodKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  assert (obj->is_constMethod(), "object must be constMethod");
  constMethodOop cm = constMethodOop(obj);
  oop* adr;
  adr = cm->adr_method();
  if (mr.contains(adr)) blk->do_oop(adr);
  adr = cm->adr_exception_table();
  if (mr.contains(adr)) blk->do_oop(adr);
  adr = cm->adr_stackmap_u1();
  if (mr.contains(adr)) blk->do_oop(adr);
  adr = cm->adr_stackmap_u2();
  if (mr.contains(adr)) blk->do_oop(adr);
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = cm->object_size();  
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constMethodKlassObj never moves.
  return size;
}


int constMethodKlass::oop_adjust_pointers(oop obj) {
  assert(obj->is_constMethod(), "should be constMethod");
  constMethodOop cm = constMethodOop(obj);
  MarkSweep::adjust_pointer(cm->adr_method());
  MarkSweep::adjust_pointer(cm->adr_exception_table());
  MarkSweep::adjust_pointer(cm->adr_stackmap_u1());
  MarkSweep::adjust_pointer(cm->adr_stackmap_u2());
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = cm->object_size();  
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constMethodKlassObj never moves.
  return size;
}


void constMethodKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert(obj->is_constMethod(), "should be constMethod");
}

#ifndef PRODUCT

// Printing

void constMethodKlass::oop_print_on(oop obj, outputStream* st) {
  ResourceMark rm;
  assert(obj->is_constMethod(), "must be constMethod");
  Klass::oop_print_on(obj, st);
  constMethodOop m = constMethodOop(obj);
  st->print(" - method:       " INTPTR_FORMAT " ", m->method());
  m->method()->print_value_on(st); st->cr();
  st->print(" - exceptions:   " INTPTR_FORMAT "\n", m->exception_table());
  st->print(" - stackmap_u1:  " INTPTR_FORMAT "\n", m->stackmap_u1());
  st->print(" - stackmap_u2:  " INTPTR_FORMAT "\n", m->stackmap_u2());
}


// Short version of printing constMethodOop - just print the name of the
// method it belongs to.
void constMethodKlass::oop_print_value_on(oop obj, outputStream* st) {
  assert(obj->is_constMethod(), "must be constMethod");
  constMethodOop m = constMethodOop(obj);
  st->print(" const part of method " );
  m->method()->print_value_on(st);
}


const char* constMethodKlass::internal_name() const {
  return "{constMethod}";
}


// Verification

void constMethodKlass::oop_verify_on(oop obj, outputStream* st) {
  Klass::oop_verify_on(obj, st);
  guarantee(obj->is_constMethod(), "object must be constMethod");
  constMethodOop m = constMethodOop(obj);
  guarantee(m->is_perm(),                            "should be in permspace");

  // Verification can occur during oop construction before the method or 
  // other fields have been initialized.
  if (!obj->partially_loaded()) {
    guarantee(m->method()->is_perm(), "should be in permspace");
    guarantee(m->method()->is_method(), "should be method");
    guarantee(m->exception_table()->is_perm(), "should be in permspace");
    guarantee(m->exception_table()->is_typeArray(), "should be type array");
    guarantee(m->stackmap_u1()->is_perm(), "should be in permspace");
    guarantee(m->stackmap_u1()->is_typeArray(), "should be type array");
    guarantee(m->stackmap_u2()->is_perm(), "should be in permspace");
    guarantee(m->stackmap_u2()->is_typeArray(), "should be type array");

    address m_end = (address)((oop*) m + m->size());
    address compressed_table_start = m->code_end();
    guarantee(compressed_table_start <= m_end, "invalid method layout");
    address compressed_table_end = compressed_table_start;
    // Verify line number table
    if (m->has_linenumber_table()) {
      CompressedLineNumberReadStream stream(m->compressed_linenumber_table());
      while (stream.read_pair()) {
        guarantee(stream.bci() >= 0 && stream.bci() <= m->code_size(), "invalid bci in line number table");
      }
      compressed_table_end += stream.position();
    }
    guarantee(compressed_table_end <= m_end, "invalid method layout");
    // Verify checked exceptions and local variable tables
    if (m->has_checked_exceptions()) {
      u2* addr = m->checked_exceptions_length_addr();
      guarantee(*addr > 0 && (address) addr >= compressed_table_end && (address) addr < m_end, "invalid method layout");
    }
    if (m->has_localvariable_table()) {
      u2* addr = m->localvariable_table_length_addr();
      guarantee(*addr > 0 && (address) addr >= compressed_table_end && (address) addr < m_end, "invalid method layout");
    }
    // Check compressed_table_end relative to uncompressed_table_start
    u2* uncompressed_table_start;
    if (m->has_localvariable_table()) {
      uncompressed_table_start = (u2*) m->localvariable_table_start();
    } else {
      if (m->has_checked_exceptions()) {
        uncompressed_table_start = (u2*) m->checked_exceptions_start();
      } else {
        uncompressed_table_start = (u2*) m_end;
      }
    }
    int gap = (intptr_t) uncompressed_table_start - (intptr_t) compressed_table_end;
    int max_gap = align_object_size(1)*BytesPerWord;
    guarantee(gap >= 0 && gap < max_gap, "invalid method layout");
  }
}

bool constMethodKlass::oop_partially_loaded(oop obj) const {
  assert(obj->is_constMethod(), "object must be klass");
  constMethodOop m = constMethodOop(obj);
  return m->stackmap_u2() == (typeArrayOop)obj; 
  // check whether stackmap_u2 points to self (flag for partially loaded)
}


// The stackmap_u2 is the last field set when loading an object.
void constMethodKlass::oop_set_partially_loaded(oop obj) {
  assert(obj->is_constMethod(), "object must be klass");
  constMethodOop m = constMethodOop(obj);
  m->set_stackmap_u2((typeArrayOop)obj);
  // Temporarily set stackmap_u2 to point to self
}

#endif // PRODUCT

