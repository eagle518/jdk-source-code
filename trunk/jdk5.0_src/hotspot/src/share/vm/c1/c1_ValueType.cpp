#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_ValueType.cpp	1.15 03/12/23 16:39:22 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_ValueType.cpp.incl"


// predefined types
VoidType*       voidType     = NULL;
IntType*        intType      = NULL;
LongType*       longType     = NULL;
FloatType*      floatType    = NULL;
DoubleType*     doubleType   = NULL;
ObjectType*     objectType   = NULL;
ArrayType*      arrayType    = NULL;
InstanceType*   instanceType = NULL;
ClassType*      classType    = NULL;
AddressType*    addressType  = NULL;
IllegalType*    illegalType  = NULL;


// predefined constants
IntConstant*    intZero      = NULL;
IntConstant*    intOne       = NULL;
ObjectConstant* objectNull   = NULL;


void ValueType::initialize() {
  // Note: Must initialize all types for each compilation
  //       as they are allocated within a ResourceMark!

  // types
  voidType     = new VoidType();
  intType      = new IntType();
  longType     = new LongType();
  floatType    = new FloatType();
  doubleType   = new DoubleType();
  objectType   = new ObjectType();
  arrayType    = new ArrayType();
  instanceType = new InstanceType();
  classType    = new ClassType();
  addressType  = new AddressType();
  illegalType  = new IllegalType();
  
  // constants
  intZero     = new IntConstant(0);
  intOne      = new IntConstant(1);
  objectNull  = new ObjectConstant(ciNullObject::make());
};


ValueType* ValueType::meet(ValueType* y) const {
  // incomplete & conservative solution for now - fix this!
  assert(tag() == y->tag(), "types must match");
  return base();
}


ValueType* ValueType::join(ValueType* y) const {
  Unimplemented();
  return NULL;
}


jobject ObjectConstant::encoding() const                 { return _value->encoding(); }
jobject ArrayConstant::encoding() const                  { return _value->encoding(); }
jobject InstanceConstant::encoding() const               { return _value->encoding(); }
jobject ClassConstant::encoding() const                  { return _value->encoding(); }



ValueType* as_ValueType(BasicType type) {
  switch (type) {
    case T_VOID   : return voidType;
    case T_BYTE   : // fall through
    case T_CHAR   : // fall through
    case T_SHORT  : // fall through
    case T_BOOLEAN: // fall through
    case T_INT    : return intType;
    case T_LONG   : return longType;
    case T_FLOAT  : return floatType;
    case T_DOUBLE : return doubleType;
    case T_ARRAY  : return arrayType;
    case T_OBJECT : return objectType;
    case T_ADDRESS: return addressType;
    case T_ILLEGAL: return illegalType;
  }
  ShouldNotReachHere();
  return illegalType;
}


ValueType* as_ValueType(ciConstant value) {
  switch (value.basic_type()) {
    case T_BYTE   : // fall through
    case T_CHAR   : // fall through
    case T_SHORT  : // fall through
    case T_BOOLEAN: // fall through
    case T_INT    : return new IntConstant   (value.as_int   ());
    case T_LONG   : return new LongConstant  (value.as_long  ());
    case T_FLOAT  : return new FloatConstant (value.as_float ());
    case T_DOUBLE : return new DoubleConstant(value.as_double());
    case T_ARRAY  : // fall through (ciConstant doesn't have an array accessor)
    case T_OBJECT : return new ObjectConstant(value.as_object());
  }
  ShouldNotReachHere();
  return illegalType;
}


BasicType as_BasicType(ValueType* type) {
  switch (type->tag()) {
    case voidTag:    return T_VOID;
    case intTag:     return T_INT;
    case longTag:    return T_LONG;
    case floatTag:   return T_FLOAT;
    case doubleTag:  return T_DOUBLE;
    case objectTag:  return type->is_array() ? T_ARRAY : T_OBJECT;
    case addressTag: return T_ADDRESS;
    case illegalTag: return T_ILLEGAL;
  }
  ShouldNotReachHere();
  return T_ILLEGAL;
}
