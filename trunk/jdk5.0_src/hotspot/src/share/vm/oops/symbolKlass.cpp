#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)symbolKlass.cpp	1.50 03/12/23 16:42:07 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_symbolKlass.cpp.incl"

symbolOop symbolKlass::allocate_symbol(u1* name, int len, TRAPS) {
  // Don't allow symbol oops to be created which cannot fit in a symbolOop.
  if (len > symbolOopDesc::max_length()) {
    THROW_MSG_0(vmSymbols::java_lang_InternalError(), 
                "name is too long to represent");
  }
  int size = symbolOopDesc::object_size(len);
  symbolKlassHandle h_k(THREAD, as_klassOop());
  symbolOop sym = (symbolOop)
    CollectedHeap::permanent_obj_allocate(h_k, size, CHECK_0);
  assert(!sym->is_parsable(), "not expecting parsability yet.");
  sym->set_utf8_length(len);
  for (int i = 0; i < len; i++) {
    sym->byte_at_put(i, name[i]);
  }
  // Let the first emptySymbol be created and
  // ensure only one is ever created.
  assert(sym->is_parsable() || Universe::emptySymbol() == NULL,
         "should be parsable here.");
  return sym;
}

klassOop symbolKlass::create_klass(TRAPS) {
  symbolKlass o;
  KlassHandle h_this_klass(THREAD, Universe::klassKlassObj());
  KlassHandle k = base_create_klass(h_this_klass, header_size(), o.vtbl_value(), CHECK_0);
  // Make sure size calculation is right
  assert(k()->size() == align_object_size(header_size()), "wrong size for object");
//  java_lang_Class::create_mirror(k, CHECK_0); // Allocate mirror
  return k();
}

int symbolKlass::oop_size(oop obj) const { 
  assert(obj->is_symbol(),"must be a symbol");
  symbolOop s = symbolOop(obj);
  int size = s->object_size();
  return size; 
}

bool symbolKlass::oop_is_parsable(oop obj) const {
  assert(obj->is_symbol(),"must be a symbol");
  symbolOop s = symbolOop(obj);
  return s->object_is_parsable();
}

void symbolKlass::oop_follow_contents(oop obj) {
  assert (obj->is_symbol(), "object must be symbol");
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::symbolKlassObj never moves.
  // Note: do not follow next link here (see SymbolTable::follow_contents)
}


int symbolKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert(obj->is_symbol(), "object must be symbol");
  symbolOop s = symbolOop(obj);
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = s->object_size();
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::symbolKlassObj never moves.
  return size;
}


int symbolKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  assert(obj->is_symbol(), "object must be symbol");
  symbolOop s = symbolOop(obj);
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = s->object_size();
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::symbolKlassObj never moves.
  return size;
}


int symbolKlass::oop_adjust_pointers(oop obj) {
  assert(obj->is_symbol(), "should be symbol");
  symbolOop s = symbolOop(obj);
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = s->object_size();
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::symbolKlassObj never moves.
  return size;
}


void symbolKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert(obj->is_symbol(), "should be symbol");
}


#ifndef PRODUCT
// Printing

void symbolKlass::oop_print_on(oop obj, outputStream* st) {
  st->print("Symbol: '");
  symbolOop sym = symbolOop(obj);
  for (int i = 0; i < sym->utf8_length(); i++) {
    st->print("%c", sym->byte_at(i));
  }
  st->print("'");
}

void symbolKlass::oop_print_value_on(oop obj, outputStream* st) {
  symbolOop sym = symbolOop(obj);
  st->print("'");
  for (int i = 0; i < sym->utf8_length(); i++) {
    st->print("%c", sym->byte_at(i));
  }
  st->print("'");
}

const char* symbolKlass::internal_name() const {
  return "{symbol}";
}
#endif //PRODUCT
