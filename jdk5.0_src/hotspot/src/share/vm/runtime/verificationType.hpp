#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)verificationType.hpp	1.6 03/12/23 16:44:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Abstract types used by the byte code verifier. 
enum Tag {
  ITEM_Bogus,             // Unused 
  ITEM_Integer,
  ITEM_Float,
  ITEM_Double,
  ITEM_Long,
  ITEM_Null,              // Result of aconst_null 
  ITEM_UninitializedThis, // "this" is in <init> method, before call to super() 
  ITEM_Object,
  ITEM_Uninitialized,

  /* The following codes are used by the verifier but don't actually occur in
   * class files.
   */
  ITEM_Long_2,      // 2nd word of long
  ITEM_Double_2,    // 2nd word of double

  ITEM_Category1,
  ITEM_Category2,
  ITEM_Category2_2nd,
  ITEM_Reference,
  ITEM_Bool,    // bool, byte, char, short can be array elements
  ITEM_Byte,
  ITEM_Char,
  ITEM_Short,
  ITEM_Array
};

class RefType;
class ArrType;
class ObjType;
class UninitializedType;

// represents primary types
// super class of all other inference types
class VerificationType : public CHeapObj {
  friend class ArrType;
  friend class ObjType;
  friend class RefType;
  friend class StackMapFrame;
  friend class StackMapReader;
  friend class ClassVerifier;
private:
  // Only used as element types in array inference types.
  static VerificationType*  _bool_type;
  static VerificationType*  _byte_type;
  static VerificationType*  _char_type;
  static VerificationType*  _short_type;
protected:
  u1 _tag;

  // static variables represent special inference types
  static VerificationType*  _bogus_type;
  static VerificationType*  _integer_type; 
  static VerificationType*  _float_type; 
  static VerificationType*  _double_type; 
  static VerificationType*  _long_type;
  static VerificationType*  _long2_type; 
  static VerificationType*  _double2_type; 
  static VerificationType*  _category1_type; 
  static VerificationType*  _category2_type; 
  static VerificationType*  _category2_2nd_type; 
  static RefType*           _null_type; 
  static RefType*           _reference_type; 
  static RefType*           _array_type; 
  static UninitializedType* _uninitialized_this; 

  static VerificationType* get_primary_type(const BasicType t);
  static VerificationType* get_primary_type(const Tag t);
  static int change_sig_to_verificationType(SignatureStream* sig_type, VerificationType** inference_type, 
                                            ClassVerifier* verifier, TRAPS);

  VerificationType(u1 tag) : _tag(tag) { }

  inline u1 tag() const    { return _tag; }
  inline bool is_bogus() const         { return _tag == ITEM_Bogus; }
  inline bool is_int() const           { return _tag == ITEM_Integer; }
  inline bool is_float() const         { return _tag == ITEM_Float; }
  inline bool is_double() const        { return _tag == ITEM_Double; }
  inline bool is_long() const          { return _tag == ITEM_Long; }
  inline bool is_long2() const         { return _tag == ITEM_Long_2; }
  inline bool is_double2() const       { return _tag == ITEM_Double_2; }
  inline bool is_byte() const          { return _tag == ITEM_Byte; }
  inline bool is_bool() const          { return _tag == ITEM_Bool; }
  inline bool is_short() const         { return _tag == ITEM_Short; }
  inline bool is_char() const          { return _tag == ITEM_Char; }
  inline bool is_null() const	         { return _tag == ITEM_Null; }
  inline bool is_array() const         { return _tag == ITEM_Array; }
  inline bool is_object() const        { return _tag == ITEM_Object; }
  inline bool is_uninitialized() const { return _tag == ITEM_Uninitialized; }

  inline bool is_reference() const { 
    return (_tag == ITEM_Reference || _tag == ITEM_Object || _tag == ITEM_Array || _tag == ITEM_Uninitialized);
  }

  inline bool is_category1() const {
    return (_tag == ITEM_Integer || _tag == ITEM_Float || _tag == ITEM_Null 
          || _tag == ITEM_Byte || _tag == ITEM_Bool || _tag == ITEM_Short || _tag == ITEM_Char
          || _tag == ITEM_Object || _tag == ITEM_Array || _tag == ITEM_Uninitialized); 
  }
  inline bool is_category2() const {
    return (_tag == ITEM_Double || _tag == ITEM_Long);
  }
  inline bool is_category2_2nd() const {
    return (_tag == ITEM_Double_2 || _tag == ITEM_Long_2);
  }
  inline virtual bool equals(const VerificationType* t) const { return _tag == t->tag(); }

  // Returns true if this type is assignable from t.
  virtual bool is_assignable_from(const VerificationType* t, TRAPS);

  // Debugging
  virtual void print_on(outputStream* st) const PRODUCT_RETURN;

public:
  static void initialize();
  static void finalize();
};

class RefType : public VerificationType {
  friend class VerificationType;
  friend class ClassVerifier;
  friend class ClassTypeHashtable;
protected:
  RefType(u1 tag) : VerificationType(tag) { }
  virtual bool is_assignable_from(const VerificationType* from, TRAPS);
};

class ObjType : public RefType {
  friend class VerificationType;
  friend class ArrType;
  friend class StackMapFrame;
  friend class ClassVerifier;
  friend class ClassTypeHashtable;

  symbolHandle _name;
  instanceKlassHandle _klass; 
  ClassVerifier* _verifier;

  ObjType(symbolHandle name, ClassVerifier* verifier) : 
      _name(name), _klass(instanceKlassHandle()), _verifier(verifier),
      RefType(ITEM_Object) { }
  ObjType(symbolHandle name, instanceKlassHandle klass, ClassVerifier* verifier) : 
      _name(name), _klass(klass), _verifier(verifier), RefType(ITEM_Object) { }

  inline symbolOop name() const { return _name(); }
  inline klassOop klass() const { return _klass(); }
  inline void set_klass(instanceKlassHandle klass) { _klass = klass; }
  inline bool same_name(const symbolOop name) const { 
    return _name->fast_compare(name) ? false : true;
  }
  inline bool equals(const VerificationType* t) const {
    return this == t;
  }     
  bool is_assignable_from(const VerificationType* from, TRAPS);

  // Debugging
  void print_on(outputStream* st) const PRODUCT_RETURN;
};

class ArrType : public RefType {
  friend class VerificationType;
  friend class ClassVerifier;
  friend class ClassTypeHashtable;

  int                _dimensions;
  VerificationType*  _element_type;
  symbolHandle       _sig;
  ClassVerifier*     _verifier;

  ArrType(symbolHandle signature, ClassVerifier* verifier, TRAPS);

  inline VerificationType* element_type() const  { return _element_type; }
  inline int dimensions() const               { return _dimensions; }
  inline symbolOop signature() const          { return _sig(); }

  // We need to check is_null() first because null is assignable to array types.
  // In certain cases such as checking iaload or iastore, null type casts
  // to ArrTypes for convenient type checking.
  inline bool is_byte_array() const      { 
    return is_null() || (_dimensions == 1 && _element_type->is_byte()); 
  }
  inline bool is_bool_array() const      { 
    return is_null() || (_dimensions == 1 && _element_type->is_bool()); 
  }
  inline bool is_int_array() const       { 
    return is_null() || (_dimensions == 1 && _element_type->is_int()); 
  }
  inline bool is_char_array() const      { 
    return is_null() || (_dimensions == 1 && _element_type->is_char()); 
  }
  inline bool is_short_array() const     { 
    return is_null() || (_dimensions == 1 && _element_type->is_short()); 
  }
  inline bool is_long_array() const      { 
    return is_null() || (_dimensions == 1 && _element_type->is_long()); 
  }
  inline bool is_float_array() const     { 
    return is_null() || (_dimensions == 1 && _element_type->is_float()); 
  }
  inline bool is_double_array() const    { 
    return is_null() || (_dimensions == 1 && _element_type->is_double()); 
  }
  inline bool is_reference_array() const { 
    return is_null() || _dimensions > 1 || _element_type->is_object(); 
  }
  inline bool same_signature(const symbolOop sig) const { 
    return _sig->fast_compare(sig) ? false : true;
  }
  inline bool equals(const VerificationType *t) const {
    return this == t;
  }
  
  // Return a type one dimension less than this one
  VerificationType* get_component(TRAPS) const;

  bool is_assignable_from(const VerificationType* from, TRAPS);

  // Debugging
  void print_on(outputStream* st) const PRODUCT_RETURN;
};

class UninitializedType : public RefType {
  friend class VerificationType;
  friend class StackMapFrame;
  friend class StackMapReader;
  friend class ClassVerifier;

  int _offset;

  UninitializedType(int offset) : _offset(offset), RefType(ITEM_Uninitialized) { }
  inline int offset() const { return _offset; }
  inline bool equals(const VerificationType* t) const {
    return (VerificationType::equals(t) &&
            _offset == ((UninitializedType*)t)->offset());
  }
  bool is_assignable_from(const VerificationType* from, TRAPS);

  // Debugging
  void print_on(outputStream* st) const PRODUCT_RETURN;
};

