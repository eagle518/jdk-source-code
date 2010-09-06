#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)verificationType.cpp	1.6 03/12/23 16:44:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_verificationType.cpp.incl"

// Predefined verification types
VerificationType* VerificationType::_bogus_type = NULL;
VerificationType* VerificationType::_integer_type = NULL;
VerificationType* VerificationType::_float_type = NULL;
VerificationType* VerificationType::_double_type = NULL;
VerificationType* VerificationType::_long_type = NULL;
VerificationType* VerificationType::_long2_type = NULL;
VerificationType* VerificationType::_double2_type = NULL;
VerificationType* VerificationType::_category1_type = NULL;
VerificationType* VerificationType::_category2_type = NULL;
VerificationType* VerificationType::_category2_2nd_type = NULL;
VerificationType* VerificationType::_bool_type = NULL;
VerificationType* VerificationType::_byte_type = NULL;
VerificationType* VerificationType::_char_type = NULL;
VerificationType* VerificationType::_short_type = NULL;
RefType*          VerificationType::_null_type = NULL;
RefType*          VerificationType::_reference_type = NULL;
RefType*          VerificationType::_array_type = NULL;
UninitializedType* VerificationType::_uninitialized_this = NULL;

void verificationType_init() {
  VerificationType::initialize();
}

void verificationType_exit() {
  VerificationType::finalize();
}

void VerificationType::initialize() {
  _bogus_type = new VerificationType(ITEM_Bogus);
  _integer_type = new VerificationType(ITEM_Integer);
  _float_type = new VerificationType(ITEM_Float);
  _double_type = new VerificationType(ITEM_Double);
  _long_type = new VerificationType(ITEM_Long);
  _long2_type = new VerificationType(ITEM_Long_2);
  _double2_type = new VerificationType(ITEM_Double_2);
  _category1_type = new VerificationType(ITEM_Category1);
  _category2_type = new VerificationType(ITEM_Category2);
  _category2_2nd_type = new VerificationType(ITEM_Category2_2nd);
  _bool_type = new VerificationType(ITEM_Bool);
  _byte_type = new VerificationType(ITEM_Byte);
  _char_type = new VerificationType(ITEM_Char);
  _short_type = new VerificationType(ITEM_Short);
  _null_type = new RefType(ITEM_Null);
  _reference_type = new RefType(ITEM_Reference);
  _array_type = new RefType(ITEM_Array);
  _uninitialized_this = new UninitializedType(-1);
}

void VerificationType::finalize() {
  delete _bogus_type;
  delete _integer_type;
  delete _float_type;
  delete _double_type;
  delete _long_type;
  delete _long2_type;
  delete _double2_type;
  delete _category1_type;
  delete _category2_type;
  delete _category2_2nd_type;
  delete _bool_type;
  delete _byte_type;
  delete _char_type;
  delete _short_type;
  delete _null_type;
  delete _reference_type;
  delete _array_type;
  delete _uninitialized_this;
}

VerificationType* VerificationType::get_primary_type(const BasicType t_type) {
  switch(t_type) {
    case T_INT:     return VerificationType::_integer_type;
    case T_BOOLEAN: return VerificationType::_bool_type;
    case T_BYTE:    return VerificationType::_byte_type;
    case T_CHAR:    return VerificationType::_char_type;
    case T_SHORT:   return VerificationType::_short_type;
    case T_FLOAT:   return VerificationType::_float_type;
    case T_DOUBLE:  return VerificationType::_double_type;
    case T_LONG:    return VerificationType::_long_type;
    case T_VOID:
    case T_OBJECT: 
    case T_ARRAY:   return NULL;  // do nothing
    default: ShouldNotReachHere();
  }
  return NULL;
}

VerificationType* VerificationType::get_primary_type(const Tag t) {
  switch(t) {
    case ITEM_Bogus:  return VerificationType::_bogus_type;
    case ITEM_Integer:return VerificationType::_integer_type;
    case ITEM_Float:  return VerificationType::_float_type;
    case ITEM_Double: return VerificationType::_double_type;
    case ITEM_Long:   return VerificationType::_long_type;
    case ITEM_Null:   return RefType::_null_type;
    default:          ShouldNotReachHere();
  }
  return NULL;
}

bool VerificationType::is_assignable_from(const VerificationType* t, TRAPS) {
  if (t == NULL) { return false; }
  if (_tag == ITEM_Category1) {
    return t->is_category1();
  }
  if (_tag == ITEM_Category2) {
    return t->is_category2();
  }
  if (_tag == ITEM_Category2_2nd) {
    return t->is_category2_2nd();
  }
 
  if (equals(t) || is_bogus()) { return true; }
  if (t->is_int()) {
    // An int value can be assigned to a variable with type
    // boolean, byte, char, or short.
    return (is_bool() || is_byte() || 
            is_char() || is_short());
  } 
  return false;
}

int VerificationType::change_sig_to_verificationType(SignatureStream* sig_type,
                                               VerificationType** inference_type,
                                               ClassVerifier* verifier,
                                               TRAPS) {
  switch (sig_type->type()) {
    case T_INT:
    case T_BOOLEAN:
    case T_BYTE:
    case T_CHAR:
    case T_SHORT:
      *inference_type = VerificationType::_integer_type;
      return 1;
    case T_FLOAT:
      *inference_type = VerificationType::_float_type;
      return 1;
    case T_DOUBLE:
      *inference_type = VerificationType::_double_type;
      *++inference_type = VerificationType::_double2_type;
      return 2;
    case T_LONG:
      *inference_type = VerificationType::_long_type;
      *++inference_type = VerificationType::_long2_type;
      return 2;
    case T_OBJECT: case T_ARRAY: {
      // put this type into local objtype table
      symbolOop name = sig_type->as_symbol(CHECK_0);
      *inference_type = verifier->_local_class_type_table->get_class_type_from_name(symbolHandle(THREAD, name), CHECK_0);
      return 1;
    }
    default:
      // something wrong
      THROW_MSG_0(vmSymbols::java_lang_ClassFormatError(), "Unexpected type in signature");               
  }
}

#ifndef PRODUCT

void VerificationType::print_on(outputStream* st) const {
  switch (_tag) {
    case ITEM_Bogus:              st->print(" bogus "); break;
    case ITEM_Integer:            st->print(" int "); break;
    case ITEM_Float:              st->print(" float "); break;
    case ITEM_Double:             st->print(" double "); break;
    case ITEM_Long:               st->print(" long "); break;
    case ITEM_Null:               st->print(" null "); break;            
    case ITEM_UninitializedThis:  st->print(" uninitializedThis "); break;
    case ITEM_Object:             st->print(" class "); break;
    case ITEM_Uninitialized:      st->print(" uninitialized "); break;
    case ITEM_Long_2:             st->print(" long_2 "); break;
    case ITEM_Double_2:           st->print(" double_2 "); break;
    case ITEM_Category1:          st->print(" category1 "); break;
    case ITEM_Category2:          st->print(" category2 "); break;
    case ITEM_Category2_2nd:      st->print(" category2_2nd "); break;
    case ITEM_Reference:          st->print(" reference "); break;
    case ITEM_Bool:               st->print(" bool "); break;
    case ITEM_Byte:               st->print(" byte "); break;
    case ITEM_Char:               st->print(" char "); break;
    case ITEM_Short:              st->print(" short "); break;
    case ITEM_Array:              st->print(" array "); break;
    default:   ShouldNotReachHere();
  }
}

void ObjType::print_on(outputStream* st) const {
  st->print(" class %s ", _name->as_klass_external_name()); 
}

void ArrType::print_on(outputStream* st) const {
  st->print(" class %s ", _sig->as_klass_external_name()); 
}

void UninitializedType::print_on(outputStream* st) const {
  st->print(" uninitialized %d ", _offset); 
}

#endif


// RefType is only used for type check convenience.
// It's not really stored as inference type in stackmap table.
// Currently there are _null_type, _reference_type and _array_type
// allocated as RefType objects.
bool RefType::is_assignable_from(const VerificationType* from, TRAPS) {
  if (from == NULL) { return false; }

  if (_tag == ITEM_Reference) {
    return (from->tag() == ITEM_Reference // never true
         || from->is_null() || from->is_object() 
         || from->is_array() || from->is_uninitialized());
  } else if (is_null()) {
    return from->is_null();
  } else if (is_array()) {
    return from->is_array() || from->is_null();
  } else if (is_object()) { // never true
    return from->is_object() || from->is_null();
  } else {
    return false;
  }
}


bool ObjType::is_assignable_from(const VerificationType* from, TRAPS) {
  if (from == NULL) { return false; }
  if (equals(from)) { return true; }

  // Any object class is assignable from null
  if (from->is_null()) { return true; }

  // Any object or array type is assignable to java.lang.Object.
  if (this == _verifier->_object_class_type) {
    return (from->is_object() || from->is_array());
  }

  // If klass is null, load it first.
  if (_klass.is_null()) {
    klassOop k = _verifier->load_class(_name, CHECK_0);
    _klass = instanceKlassHandle(THREAD, k);
  }

  // We treat interfaces as java.lang.Object, 
  // including java.lang.Cloneable and java.io.Serializable.
  if (_klass->is_interface()) {
    return (from->is_object() || from->is_array());
  }

  // Now from has to be an object type.
  if (from->is_object()) {
    klassOop from_klass = ((ObjType*)from)->klass();
    if (from_klass == NULL) {
      // Need to load from_klass first
      from_klass = _verifier->load_class(((ObjType*)from)->name(), CHECK_0);
    }
    return instanceKlass::cast(from_klass)->is_subclass_of(_klass());
  }
  return false;
}

ArrType::ArrType(symbolHandle sig, ClassVerifier* v, TRAPS) : _verifier(v), RefType(ITEM_Array) {
  symbolOop object_key;

  // dimension and object_key are assigned as a side-effect of this call
  BasicType t = FieldType::get_array_info(sig(), &_dimensions, &object_key, CHECK);
  assert(t != T_ARRAY, "element type can not be array type");
  if (t == T_OBJECT) {
    _element_type = v->_local_class_type_table->get_class_type_from_name(symbolHandle(THREAD, object_key), CHECK);
  } else {
    _element_type = get_primary_type(t);
  } 
  _sig = sig;
}

VerificationType* ArrType::get_component(TRAPS) const {
  if(_dimensions == 1) { return _element_type; }
  else {
    ResourceMark rm(THREAD);
    symbolOop component = oopFactory::new_symbol(_sig->as_C_string() + 1, CHECK_0);  
    return _verifier->_local_class_type_table->get_class_type_from_name(symbolHandle(THREAD, component), CHECK_0);
  }
}


bool ArrType::is_assignable_from(const VerificationType* from, TRAPS) {
  if (from == NULL) { return false; }
  if (equals(from)) { return true; }

  // Any array type is assignable from null.
  if (from->is_null()) { return true; }

  // An array type is not assignable from a non-array type.
  if (!from->is_array()) { return false; }

  // Now both are array types.
  int from_dim = ((ArrType*)from)->dimensions();
  assert((_dimensions > 0 && from_dim > 0), "array can not have 0 dimensions");
  VerificationType* from_component = ((ArrType*)from)->get_component(CHECK_0);
  VerificationType* component = get_component(CHECK_0);
  return component->is_assignable_from(from_component, CHECK_0);
}

bool UninitializedType::is_assignable_from(const VerificationType* from, TRAPS) {
  if (from == NULL) { return false; }
  if (this == from) { return true; }
  return (VerificationType::equals(from));
}
