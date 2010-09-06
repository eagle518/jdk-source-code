#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_ValueType.hpp	1.28 03/12/23 16:39:22 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// type hierarchy
class ValueType;
class   VoidType;
class   IntType;
class     IntConstant;
class     IntInterval;
class   LongType;
class     LongConstant;
class   FloatType;
class     FloatConstant;
class   DoubleType;
class     DoubleConstant;
class   ObjectType;
class     ObjectConstant;
class     ArrayType;
class       ArrayConstant;
class     InstanceType;
class       InstanceConstant;
class     ClassType;
class       ClassConstant;
class   AddressType;
class     AddressConstant;
class   IllegalType;


// predefined types
extern VoidType*       voidType;
extern IntType*        intType;
extern LongType*       longType;
extern FloatType*      floatType;
extern DoubleType*     doubleType;
extern ObjectType*     objectType;
extern ArrayType*      arrayType;
extern InstanceType*   instanceType;
extern ClassType*      classType;
extern AddressType*    addressType;
extern IllegalType*    illegalType;


// predefined constants
extern IntConstant*    intZero;
extern IntConstant*    intOne;
extern ObjectConstant* objectNull;


// tags
enum ValueTag {
  // all legal tags must come first
  intTag,
  longTag,
  floatTag,
  doubleTag,
  objectTag,
  addressTag,
  number_of_legal_tags,
  // all other tags must follow afterwards
  voidTag = number_of_legal_tags,
  illegalTag,
  number_of_tags
};


class ValueType: public CompilationResourceObj {
 public:
  // initialization
  static void initialize();

  // accessors
  virtual ValueType* base() const                = 0; // the 'canonical' type (e.g., intType for an IntConstant)
  virtual ValueTag tag() const                   = 0; // the 'canonical' tag  (useful for type matching) 
  virtual int size() const                       = 0; // the size of an object of the type in words
  virtual const char tchar() const               = 0; // the type 'character' for printing
  virtual const char* name() const               = 0; // the type name for printing
  virtual bool is_constant() const               { return false; }

  // testers
  bool is_void()                                 { return as_VoidType()     != NULL; }
  bool is_int()                                  { return as_IntType()      != NULL; }
  bool is_long()                                 { return as_LongType()     != NULL; }
  bool is_float()                                { return as_FloatType()    != NULL; }
  bool is_double()                               { return as_DoubleType()   != NULL; }
  bool is_object()                               { return as_ObjectType()   != NULL; }
  bool is_array()                                { return as_ArrayType()    != NULL; }
  bool is_instance()                             { return as_InstanceType() != NULL; }
  bool is_class()                                { return as_ClassType()    != NULL; }
  bool is_address()                              { return as_AddressType()  != NULL; }
  bool is_illegal()                              { return as_IllegalType()  != NULL; }

  bool is_int_kind() const                       { return tag() == intTag || tag() == longTag; }
  bool is_float_kind() const                     { return tag() == floatTag || tag() == doubleTag; }
  bool is_object_kind() const                    { return tag() == objectTag; }

  bool is_single_word() const                    { return size() == 1; }
  bool is_double_word() const                    { return size() == 2; }

  // casting
  virtual VoidType*         as_VoidType()        { return NULL; }
  virtual IntType*          as_IntType()         { return NULL; }
  virtual LongType*         as_LongType()        { return NULL; }
  virtual FloatType*        as_FloatType()       { return NULL; }
  virtual DoubleType*       as_DoubleType()      { return NULL; }
  virtual ObjectType*       as_ObjectType()      { return NULL; }
  virtual ArrayType*        as_ArrayType()       { return NULL; }
  virtual InstanceType*     as_InstanceType()    { return NULL; }
  virtual ClassType*        as_ClassType()       { return NULL; }
  virtual AddressType*      as_AddressType()     { return NULL; }
  virtual IllegalType*      as_IllegalType()     { return NULL; }

  virtual IntConstant*      as_IntConstant()     { return NULL; }
  virtual LongConstant*     as_LongConstant()    { return NULL; }
  virtual FloatConstant*    as_FloatConstant()   { return NULL; }
  virtual DoubleConstant*   as_DoubleConstant()  { return NULL; }
  virtual ObjectConstant*   as_ObjectConstant()  { return NULL; }
  virtual InstanceConstant* as_InstanceConstant(){ return NULL; }
  virtual ClassConstant*    as_ClassConstant()   { return NULL; }
  virtual ArrayConstant*    as_ArrayConstant()   { return NULL; }
  virtual AddressConstant*  as_AddressConstant() { return NULL; }

  // type operations
  ValueType* meet(ValueType* y) const;
  ValueType* join(ValueType* y) const;

  // debugging
  void print(outputStream* s = tty)              { s->print(name()); }
};


class VoidType: public ValueType {
 public:
  virtual ValueType* base() const                { return voidType; }
  virtual ValueTag tag() const                   { return voidTag; }
  virtual int size() const                       { return 0; }
  virtual const char tchar() const               { return 'v'; }
  virtual const char* name() const               { return "void"; }
  virtual VoidType* as_VoidType()                { return this; }
};


class IntType: public ValueType {
 public:
  virtual ValueType* base() const                { return intType; }
  virtual ValueTag tag() const                   { return intTag; }
  virtual int size() const                       { return 1; }
  virtual const char tchar() const               { return 'i'; }
  virtual const char* name() const               { return "int"; }
  virtual IntType* as_IntType()                  { return this; }
};


class IntConstant: public IntType {
 private:
  jint _value;

 public:
  IntConstant(jint value)                        { _value = value; }

  jint value() const                             { return _value; }

  virtual bool is_constant() const               { return true; }
  virtual IntConstant* as_IntConstant()          { return this; }
};


class IntInterval: public IntType {
 private:
  jint _beg;
  jint _end;

 public:
  IntInterval(jint beg, jint end) {
    assert(beg <= end, "illegal interval");
    _beg = beg;
    _end = end;
  }

  jint beg() const                               { return _beg; }
  jint end() const                               { return _end; }

  virtual bool is_interval() const               { return true; }
};


class LongType: public ValueType {
 public:
  virtual ValueType* base() const                { return longType; }
  virtual ValueTag tag() const                   { return longTag; }
  virtual int size() const                       { return 2; }
  virtual const char tchar() const               { return 'l'; }
  virtual const char* name() const               { return "long"; }
  virtual LongType* as_LongType()                { return this; }
};


class LongConstant: public LongType {
 private:
  jlong _value;

 public:
  LongConstant(jlong value)                      { _value = value; }

  jlong value() const                            { return _value; }

  virtual bool is_constant() const               { return true; }
  virtual LongConstant* as_LongConstant()        { return this; }
};


class FloatType: public ValueType {
 public:
  virtual ValueType* base() const                { return floatType; }
  virtual ValueTag tag() const                   { return floatTag; }
  virtual int size() const                       { return 1; }
  virtual const char tchar() const               { return 'f'; }
  virtual const char* name() const               { return "float"; }
  virtual FloatType* as_FloatType()              { return this; }
};


class FloatConstant: public FloatType {
 private:
  jfloat _value;

 public:
  FloatConstant(jfloat value)                    { _value = value; }

  jfloat value() const                           { return _value; }

  virtual bool is_constant() const               { return true; }
  virtual FloatConstant* as_FloatConstant()      { return this; }
};


class DoubleType: public ValueType {
 public:
  virtual ValueType* base() const                { return doubleType; }
  virtual ValueTag tag() const                   { return doubleTag; }
  virtual int size() const                       { return 2; }
  virtual const char tchar() const               { return 'd'; }
  virtual const char* name() const               { return "double"; }
  virtual DoubleType* as_DoubleType()            { return this; }
};


class DoubleConstant: public DoubleType {
 private:
  jdouble _value;

 public:
  DoubleConstant(jdouble value)                  { _value = value; }

  jdouble value() const                          { return _value; }

  virtual bool is_constant() const               { return true; }
  virtual DoubleConstant* as_DoubleConstant()    { return this; }
};


class ObjectType: public ValueType {
 public:
  virtual ValueType* base() const                { return objectType; }
  virtual ValueTag tag() const                   { return objectTag; }
  virtual int size() const                       { return 1; }
  virtual const char tchar() const               { return 'a'; }
  virtual const char* name() const               { return "object"; }
  virtual ObjectType* as_ObjectType()            { return this; }
  virtual jobject encoding() const               { ShouldNotReachHere(); return NULL; }
};


class ObjectConstant: public ObjectType {
 private:
  ciObject* _value;

 public:
  ObjectConstant(ciObject* value)                { _value = value; }

  ciObject* value() const                        { return _value; }

  virtual bool is_constant() const               { return true; }
  virtual ObjectConstant* as_ObjectConstant()    { return this; }
  virtual jobject encoding() const;
};


class ArrayType: public ObjectType {
 public:
  virtual ArrayType* as_ArrayType()              { return this; }
};


class ArrayConstant: public ArrayType {
 private:
  ciArray* _value;

 public:
  ArrayConstant(ciArray* value)                  { _value = value; }

  ciArray* value() const                         { return _value; }

  virtual bool is_constant() const               { return true; }

  virtual ArrayConstant* as_ArrayConstant()      { return this; }
  virtual jobject encoding() const;
};


class InstanceType: public ObjectType {
 public:
  virtual InstanceType* as_InstanceType()        { return this; }
};


class InstanceConstant: public InstanceType {
 private:
  ciInstance* _value;

 public:
  InstanceConstant(ciInstance* value)            { _value = value; }

  ciInstance* value() const                      { return _value; }

  virtual bool is_constant() const               { return true; }

  virtual InstanceConstant* as_InstanceConstant(){ return this; }
  virtual jobject encoding() const;
};


class ClassType: public ObjectType {
 public:
  virtual ClassType* as_ClassType()              { return this; }
};


class ClassConstant: public ClassType {
 private:
  ciInstanceKlass* _value;

 public:
  ClassConstant(ciInstanceKlass* value)          { _value = value; }

  ciInstanceKlass* value() const                 { return _value; }

  virtual bool is_constant() const               { return true; }

  virtual ClassConstant* as_ClassConstant()      { return this; }
  virtual jobject encoding() const;
};


class AddressType: public ValueType {
 public:
  virtual ValueType* base() const                { return addressType; }
  virtual ValueTag tag() const                   { return addressTag; }
  virtual int size() const                       { return 1; }
  virtual const char tchar() const               { return 'r'; }
  virtual const char* name() const               { return "address"; }
  virtual AddressType* as_AddressType()          { return this; }
};


class AddressConstant: public AddressType {
 private:
  jint _value;

 public:
  AddressConstant(jint value)                    { _value = value; }

  jint value() const                             { return _value; }

  virtual bool is_constant() const               { return true; }

  virtual AddressConstant* as_AddressConstant()  { return this; }
};


class IllegalType: public ValueType {
 public:
  virtual ValueType* base() const                { return illegalType; }
  virtual ValueTag tag() const                   { return illegalTag; }
  virtual int size() const                       { ShouldNotReachHere(); return -1; }
  virtual const char tchar() const               { return ' '; }
  virtual const char* name() const               { return "illegal"; }
  virtual IllegalType* as_IllegalType()          { return this; }
};


// conversion between ValueTypes, BasicTypes, and ciConstants
ValueType* as_ValueType(BasicType type);
ValueType* as_ValueType(ciConstant value);
BasicType  as_BasicType(ValueType* type);

inline ValueType* as_ValueType(ciType* type) { return as_ValueType(type->basic_type()); }
