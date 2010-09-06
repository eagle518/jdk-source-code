#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Items.cpp	1.29 03/12/23 16:39:08 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_Items.cpp.incl"


HintItem HintItem::_no_hint(NULL, true);

bool HintItem::is_same(Item* item) {
  return Item::is_same(item) && _type == item->as_hint()->_type;
}


void HintItem::set_from_item(const Item* item) { 
  Item::set_from_item(item);
  _type = item->type();
}


//---------------------------------------------------------------------

bool Item::is_object() const {
  return _instr == NULL 
            ? false
            : type()->is_object_kind();
}

// _instr points to a root and the root does not point to this,
// we copy the content of root-item into this
void Item::update() {
  assert(!is_root_item(), "");
  if (_instr->is_root() && this != _instr->item() ) {
    set_from_item(_instr->item());
  }
}


bool Item::is_same(Item* item) {
  bool f =
    is_hint_item() == item->is_hint_item() &&
    _mode == item->_mode &&
    _loc == item->_loc &&
    _spill_ix == item->_spill_ix &&
    _instr == item->_instr &&
    _round32 == item->_round32;
  assert(f, "");
  return f;
}


ciObject* Item::get_jobject_constant() const {
  assert(is_constant() && _instr != NULL, "");
  if (type()->as_ObjectConstant() != NULL) {
    return type()->as_ObjectConstant()->value();
  } else  if (type()->as_InstanceConstant() != NULL) {
    return type()->as_InstanceConstant()->value();
  } else if (type()->as_ClassConstant() != NULL) {
    return type()->as_ClassConstant()->value();
  } else {
    assert(type()->as_ArrayConstant() != NULL, "wrong access");
    return type()->as_ArrayConstant()->value();
  }
}


jint Item::get_jint_constant() const {
  assert(is_constant() && _instr != NULL, "");
  assert(type()->as_IntConstant() != NULL, "type check");
  return type()->as_IntConstant()->value();
}


jint Item::get_address_constant() const {
  assert(is_constant() && _instr != NULL, "");
  assert(type()->as_AddressConstant() != NULL, "type check");
  return type()->as_AddressConstant()->value();
}


jfloat Item::get_jfloat_constant() const {
  assert(is_constant() && _instr != NULL, "");
  assert(type()->as_FloatConstant() != NULL, "type check");
  return type()->as_FloatConstant()->value();
}


jdouble Item::get_jdouble_constant() const {
  assert(is_constant() && _instr != NULL, "");
  assert(type()->as_DoubleConstant() != NULL, "type check");
  return type()->as_DoubleConstant()->value();
}


jlong Item::get_jlong_constant() const {
  assert(is_constant() && _instr != NULL, "");
  assert(type()->as_LongConstant() != NULL, "type check");
  return type()->as_LongConstant()->value();
}


#ifndef PRODUCT

void Item::print() {
  if (is_root_item()) {
    tty->print(" RootItem: ");
  } else if (is_hint_item()) {
    tty->print(" HintItem: ");
  } else {
    tty->print(" Item:     ");
  }

  if (is_register()) {
    tty->print(" [register] ");
    get_register().print();
  } else if (is_stack()) {
    if (is_spilled()) {
      tty->print(" [stack-spilled] spill-idx=%d ", get_spilled_index());
    } else {
      tty->print(" [stack-local] local-idx=%d ", get_stack());
    }
  } else if (is_constant()) {
    tty->print(" [constant] ");
  } else {
    assert(!is_valid(), "must be an invalid item");
  }

  if (destroys_register()) {
    tty->print(" (destroys register) ");
  }
  if (is_round32()) {
    tty->print(" (round32) ");
  }
  tty->print(" value: ");
  if (value() == NULL) {
    tty->print(" null ");
  } else {
    value()->print();
  }
  tty->cr();
}

#endif


