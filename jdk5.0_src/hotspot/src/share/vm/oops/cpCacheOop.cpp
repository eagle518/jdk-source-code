#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)cpCacheOop.cpp	1.53 04/05/04 14:58:52 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_cpCacheOop.cpp.incl"


// Implememtation of ConstantPoolCacheEntry

void ConstantPoolCacheEntry::set_initial_state(int index) {
  assert(0 <= index && index < 0x10000, "sanity check");
  _indices = index;
}


int ConstantPoolCacheEntry::as_flags(TosState state, bool is_final,
                    bool is_vfinal, bool is_volatile,
                    bool is_method_interface, bool is_method) {
  int f = state;

  assert( state < number_of_states, "Invalid state in as_flags");

  f <<= 1;
  if (is_final) f |= 1;
  f <<= 1;
  if (is_vfinal) f |= 1;
  f <<= 1;
  if (is_volatile) f |= 1;
  f <<= 1;
  if (is_method_interface) f |= 1;
#ifdef HOTSWAP
  f <<= 1;
  if (is_method) f |= 1;
  f <<= ConstantPoolCacheEntry::hotSwapBit;
#else
  f <<= ConstantPoolCacheEntry::methodInterface;
#endif HOTSWAP
  // Preserve existing flag bit values
#ifdef ASSERT
  int old_state = ((_flags >> tosBits) & 0x0F);
  assert(old_state == 0 || old_state == state,
         "inconsistent cpCache flags state");
#endif
  return (_flags | f) ;
}

void ConstantPoolCacheEntry::set_bytecode_1(Bytecodes::Code code) {
#ifdef ASSERT
  // Read once.
  volatile Bytecodes::Code c = bytecode_1();
  assert(c == 0 || c == code || code == 0, "update must be consistent");
#endif
  // Need to flush pending stores here before bytecode is written.
  OrderAccess::release_store_ptr(&_indices, _indices | ((u_char)code << 16));
}

void ConstantPoolCacheEntry::set_bytecode_2(Bytecodes::Code code) {
#ifdef ASSERT
  // Read once.
  volatile Bytecodes::Code c = bytecode_2();
  assert(c == 0 || c == code || code == 0, "update must be consistent");
#endif
  // Need to flush pending stores here before bytecode is written.
  OrderAccess::release_store_ptr(&_indices, _indices | ((u_char)code << 24));
}

#ifdef ASSERT
// It is possible to have two different dummy methodOops created
// when the resolve code for invoke interface executes concurrently
// Hence the assertion below is weakened a bit for the invokeinterface
// case.
bool ConstantPoolCacheEntry::same_methodOop(oop cur_f1, oop f1) {
  return (cur_f1 == f1 || ((methodOop)cur_f1)->name() ==
	 ((methodOop)f1)->name() || ((methodOop)cur_f1)->signature() == 
	 ((methodOop)f1)->signature());
}
#endif

// Note that concurrent update of both bytecodes can leave one of them
// reset to zero.  This is harmless; the interpreter will simply re-resolve
// the damaged entry.  More seriously, the memory synchronization is needed
// to flush other fields (f1, f2) completely to memory before the bytecodes
// are updated, lest other processors see a non-zero bytecode but zero f1/f2.
void ConstantPoolCacheEntry::set_field(Bytecodes::Code get_code, 
                                       Bytecodes::Code put_code,
                                       KlassHandle field_holder, 
                                       int field_index, 
                                       int field_offset, 
                                       TosState field_type, 
                                       bool is_final,
                                       bool is_volatile) {
  set_f1(field_holder());
  set_f2(field_offset);
  assert(field_index <= 0xFFFF, "field index does not fit in low flag bits");
  set_flags(as_flags(field_type, is_final, false, is_volatile, false, false) | (field_index & 0xFFFF));
  set_bytecode_1(get_code);
  set_bytecode_2(put_code);
  verify(tty);
}

void ConstantPoolCacheEntry::set_method(Bytecodes::Code invoke_code,
                                        methodHandle method,
                                        int vtable_index) {

  assert(method->interpreter_entry() != NULL, "should have been set at this point");
  assert(!method->is_old_version(),  "attempt to write old method to cpCache");
  bool change_to_virtual = (invoke_code == Bytecodes::_invokeinterface);

  int byte_no = -1;
  bool needs_vfinal_flag = false;
  switch (invoke_code) {
    case Bytecodes::_invokevirtual:
    case Bytecodes::_invokeinterface: {
        if (Klass::can_be_statically_bound(method())) {
          set_f2((intptr_t)method());
	  needs_vfinal_flag = true;
        } else {
          set_f2(vtable_index);
        }
        byte_no = 2;
        break;
    }
    case Bytecodes::_invokespecial:
      // Preserve the value of the vfinal flag on invokevirtual bytecode
      // which may be shared with this constant pool cache entry.
      needs_vfinal_flag = is_resolved(Bytecodes::_invokevirtual) && is_vfinal();
      // fall through
    case Bytecodes::_invokestatic:
      set_f1(method());
      byte_no = 1;
      break;
    default:
      ShouldNotReachHere();
      break;
  }

  set_flags(as_flags(as_TosState(method->result_type()),
                     method->is_final_method(), 
                     needs_vfinal_flag, 
                     false, 
                     change_to_virtual,
                     true)|
            method()->size_of_parameters());

  // Note:  byte_no also appears in TemplateTable::resolve.
  if (byte_no == 1) {
    set_bytecode_1(invoke_code);
  } else if (byte_no == 2)  {
    if (change_to_virtual) {
      // NOTE: THIS IS A HACK - BE VERY CAREFUL!!!
      //
      // Workaround for the case where we encounter an invokeinterface, but we
      // should really have an _invokevirtual since the resolved method is a 
      // virtual method in java.lang.Object. This is a corner case in the spec
      // but is presumably legal. javac does not generate this code.
      //
      // We set bytecode_1() to _invokeinterface, because that is the
      // bytecode # used by the interpreter to see if it is resolved.
      // We set bytecode_2() to _invokevirtual.
      // See also interpreterRuntime.cpp. (8/25/2000)
      set_bytecode_1(invoke_code);
      set_bytecode_2(Bytecodes::_invokevirtual);
    } else {
      set_bytecode_2(invoke_code);
    }
  } else {
    ShouldNotReachHere();
  }
  verify(tty);
}


void ConstantPoolCacheEntry::set_interface_call(methodHandle method, int index) {
  klassOop interf = method->method_holder();
  assert(instanceKlass::cast(interf)->is_interface(), "must be an interface");
  set_f1(interf);
  set_f2(index);
  set_flags(as_flags(as_TosState(method->result_type()), method->is_final_method(), false, false, false, true) | method()->size_of_parameters());
  set_bytecode_1(Bytecodes::_invokeinterface);  
}


class LocalOopClosure: public OopClosure {
 private:
  void (*_f)(oop*);

 public:
  LocalOopClosure(void f(oop*))        { _f = f; }
  virtual void do_oop(oop* o)          { _f(o); }
};


void ConstantPoolCacheEntry::oops_do(void f(oop*)) {
  LocalOopClosure blk(f);
  oop_iterate(&blk);
}


void ConstantPoolCacheEntry::oop_iterate(OopClosure* blk) {
  assert(in_words(size()) == 4, "check code below - may need adjustment");
  // field[1] is always oop or NULL
  blk->do_oop((oop*)&_f1);
  if (is_vfinal()) {
    blk->do_oop((oop*)&_f2);
  }
}


void ConstantPoolCacheEntry::oop_iterate_m(OopClosure* blk, MemRegion mr) {
  assert(in_words(size()) == 4, "check code below - may need adjustment");
  // field[1] is always oop or NULL
  if (mr.contains((oop *)&_f1)) blk->do_oop((oop*)&_f1);
  if (is_vfinal()) {
    if (mr.contains((oop *)&_f2)) blk->do_oop((oop*)&_f2);
  }
}


void ConstantPoolCacheEntry::follow_contents() {
  assert(in_words(size()) == 4, "check code below - may need adjustment");
  // field[1] is always oop or NULL
  MarkSweep::mark_and_push((oop*)&_f1);
  if (is_vfinal()) {
    MarkSweep::mark_and_push((oop*)&_f2);
  }
}


void ConstantPoolCacheEntry::adjust_pointers() {
  assert(in_words(size()) == 4, "check code below - may need adjustment");
  // field[1] is always oop or NULL
  MarkSweep::adjust_pointer((oop*)&_f1);
  if (is_vfinal()) {
    MarkSweep::adjust_pointer((oop*)&_f2);
  }
}

#ifdef HOTSWAP
// Evolution support

void ConstantPoolCacheEntry::adjust_method_entry(methodOop old_method, methodOop new_method) {
   
  // virtual, final 
  if (is_vfinal()) {
    if (f2() == (intptr_t)old_method) {
      _f2 = (intptr_t)new_method;
    }
    return;
  }

  if (_f1 == NULL)  // Virtual call. So far we assume the vtable indices don't change
    return;
  if (_f1 == old_method) {
    _f1 = new_method;
  }
}
#endif HOTSWAP

void ConstantPoolCacheEntry::print(outputStream* st, int index) const {
  // print separator
  if (index == 0) tty->print_cr("                 -------------");
  // print entry
  tty->print_cr("%3d  (%08x)  [%02x|%02x|%5d]", index, this, bytecode_2(), bytecode_1(), constant_pool_index());
  tty->print_cr("                 [   %08x]", _f1);
  tty->print_cr("                 [   %08x]", _f2);
  tty->print_cr("                 [   %08x]", _flags);
  tty->print_cr("                 -------------");
}


void ConstantPoolCacheEntry::verify(outputStream* st) const {
  // not implemented yet
}


// Implementation of ConstantPoolCache

void constantPoolCacheOopDesc::initialize(intArray& inverse_index_map) {
  assert(inverse_index_map.length() == length(), "inverse index map must have same length as cache");
  for (int i = 0; i < length(); i++) entry_at(i)->set_initial_state(inverse_index_map[i]);
}

#ifdef HOTSWAP
void constantPoolCacheOopDesc::adjust_method_entries(objArrayOop old_methods,
						     objArrayOop new_methods) {
  for (int i = 0; i < old_methods->length(); i++) {
    methodOop old_method = (methodOop) old_methods->obj_at(i);
    if (! old_method->is_old_version())
      continue;
    for (int j = 0; j < length(); j++) {
      if (entry_at(j)->is_method_entry()) {
	entry_at(j)->adjust_method_entry(old_method, (methodOop) new_methods->obj_at(i));
      }
    }
  }
}
#endif HOTSWAP
