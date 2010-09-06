#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciField.cpp	1.23 03/12/23 16:39:28 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciField.cpp.incl"

// ciField
//
// This class represents the result of a field lookup in the VM.
// The lookup may not succeed, in which case the information in
// the ciField will be incomplete.

// ------------------------------------------------------------------
// ciField::ciField
ciField::ciField(ciInstanceKlass* klass, int index): _known_to_link_with(NULL) {
  ASSERT_IN_VM;
  Thread *thread = Thread::current();

  assert(klass->get_instanceKlass()->is_linked(), "must be linked before using its constan-pool");

  _cp_index = index;
  constantPoolHandle cpool(thread, klass->get_instanceKlass()->constants());

  // Get the field's name, signature, and type.
  symbolHandle name  (thread, cpool->name_ref_at(index));

  int nt_index = cpool->name_and_type_ref_index_at(index);
  int sig_index = cpool->signature_ref_index_at(nt_index);
  symbolHandle signature (thread, cpool->symbol_at(sig_index));

  BasicType field_type = FieldType::basic_type(signature());

  // If the field is a pointer type, get the klass of the
  // field.
  if (field_type == T_OBJECT || field_type == T_ARRAY) {
    bool ignore;
    // This is not really a class reference; the index always refers to the
    // field's type signature, as a symbol.  Linkage checks do not apply.
    _type = ciEnv::current(thread)->get_klass_by_index(klass, sig_index, ignore);
  } else {
    _type = ciType::make(field_type);
  }

  // Get the field's declared holder.
  //
  // Note: we actually create a ciInstanceKlass for this klass,
  // even though we may not need to.
  int holder_index = cpool->klass_ref_index_at(index);
  bool holder_is_accessible;
  ciInstanceKlass* declared_holder =
    ciEnv::current(thread)->get_klass_by_index(klass, holder_index,
                                               holder_is_accessible)
      ->as_instance_klass();

  // The declared holder of this field may not have been loaded.
  // Bail out with partial field information.
  if (!holder_is_accessible) {
    // _cp_index and _type have already been set.
    // The default values for _flags and _constant_value will suffice.
    // We need values for _holder, _offset,  and _is_constant,
    _holder = declared_holder;
    _offset = -1;
    _is_constant = false;
    return;
  }

  instanceKlass* loaded_decl_holder = declared_holder->get_instanceKlass();

  // Perform the field lookup.
  fieldDescriptor field_desc;
  klassOop canonical_holder = 
    loaded_decl_holder->find_field(name(), signature(), &field_desc);
  if (canonical_holder == NULL) {
    // Field lookup failed.  Will be detected by will_link.
    _holder = declared_holder;
    _offset = -1;
    _is_constant = false;
    return;
  }

  assert(canonical_holder == field_desc.field_holder(), "just checking");
  initialize_from(&field_desc);
}

ciField::ciField(fieldDescriptor *fd): _known_to_link_with(NULL) {
  ASSERT_IN_VM;

  _cp_index = -1;

  // Get the field's name, signature, and type.
  symbolOop name = fd->name();
  symbolOop signature = fd->signature();

  BasicType field_type = fd->field_type();

  // If the field is a pointer type, get the klass of the
  // field.
  if (field_type == T_OBJECT || field_type == T_ARRAY) {
    ciEnv *env = CURRENT_ENV;
    ciKlass* klass = env->get_object(fd->field_holder())->as_klass();
    ciSymbol* sig = env->get_object(fd->signature())->as_symbol();
    _type = env->get_klass_by_name_impl(klass, sig, false);
  } else {
    _type = ciType::make(field_type);
  }

  initialize_from(fd);
}

void ciField::initialize_from(fieldDescriptor* fd) {
  // Get the flags, offset, and canonical holder of the field.
  _flags = ciFlags(fd->access_flags());
  _offset = fd->offset();
  _holder = CURRENT_ENV->get_object(fd->field_holder())->as_instance_klass();
  
  // Check to see if the field is constant.
  if (_holder->is_initialized() &&
      this->is_final() && this->is_static()) {
    // This field just may be constant.  The only cases where it will
    // not be constant are:
    //
    // 1. The field holds a non-perm-space oop.  The field is, strictly
    //    speaking, constant but we cannot embed non-perm-space oops into
    //    generated code.  For the time being we need to consider the
    //    field to be not constant.
    // 2. The field is a *special* static&final field whose value
    //    may change.  The three examples are java.lang.System.in, 
    //    java.lang.System.out, and java.lang.System.err.

    klassOop k = _holder->get_klassOop();
    assert( SystemDictionary::system_klass() != NULL, "Check once per vm");
    if( k == SystemDictionary::system_klass() ) {
      // Check offsets for case 2: System.in, System.out, or System.err 
      if( _offset == java_lang_System::in_offset_in_bytes()  ||
          _offset == java_lang_System::out_offset_in_bytes() ||
          _offset == java_lang_System::err_offset_in_bytes() ) {
        _is_constant = false;
        return;
      }
    }

    _is_constant = true;
    switch(type()->basic_type()) {
    case T_BYTE: 
      _constant_value = ciConstant(type()->basic_type(), k->byte_field(_offset));
      break;
    case T_CHAR: 
      _constant_value = ciConstant(type()->basic_type(), k->char_field(_offset));
      break;
    case T_SHORT: 
      _constant_value = ciConstant(type()->basic_type(), k->short_field(_offset));
      break;
    case T_BOOLEAN:
      _constant_value = ciConstant(type()->basic_type(), k->bool_field(_offset));
      break;
    case T_INT:
      _constant_value = ciConstant(type()->basic_type(), k->int_field(_offset));
      break;
    case T_FLOAT:
      _constant_value = ciConstant(k->float_field(_offset));
      break;
    case T_DOUBLE:
      _constant_value = ciConstant(k->double_field(_offset));
      break;
    case T_LONG:
      _constant_value = ciConstant(k->long_field(_offset));
      break;
    case T_OBJECT:
    case T_ARRAY:
      {
	oop o = k->obj_field(_offset);

	// A field will be "constant" if it is known always to be
	// a non-null reference to an instance of a particular class,
	// or to a particular array.  This can happen even if the instance
	// or array is not perm.  In such a case, an "unloaded" ciArray
	// or ciInstance is created.  The compiler may be able to use
	// information about the object's class (which is exact) or length.

	if (o == NULL) {
	  _constant_value = ciConstant(type()->basic_type(), ciNullObject::make());
	} else {
	  _constant_value = ciConstant(type()->basic_type(), CURRENT_ENV->get_object(o));
	  assert(_constant_value.as_object() == CURRENT_ENV->get_object(o), "check interning");
	}
      }
    }
  } else {
    _is_constant = false;
  }
}

// ------------------------------------------------------------------
// ciField::will_link
//
// Can a specific access to this field be made without causing
// link errors?
bool ciField::will_link(ciInstanceKlass* accessing_klass,
			Bytecodes::Code bc) {
  VM_ENTRY_MARK;
  if (_known_to_link_with == accessing_klass) {
    return true;
  }

  FieldAccessInfo result;
  constantPoolHandle c_pool(THREAD,
                         accessing_klass->get_instanceKlass()->constants());
  LinkResolver::resolve_field(result, c_pool, _cp_index,
                              Bytecodes::java_code(bc),
			      true, false, KILL_COMPILE_ON_FATAL_(false));
  _known_to_link_with = accessing_klass;
  return true;
}

// ------------------------------------------------------------------
// ciField::print
void ciField::print() {
  tty->print("<ciField holder=");
  _holder->print_name();
  tty->print(" offset=%d type=", _offset);
  _type->print_name();
  tty->print(" is_constant=%s", bool_to_str(_is_constant));
  if (_is_constant) {
    tty->print(" constant_value=");
    _constant_value.print();
  }
  tty->print(">");
}

