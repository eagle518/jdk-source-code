#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)stackValue.hpp	1.29 03/12/23 16:44:12 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class StackValue : public ResourceObj {
 private:
  BasicType _type;
  intptr_t  _i; // Blank java stack slot value
  Handle    _o; // Java stack slot value interpreted as a Handle
 public:

  StackValue(intptr_t value) {
    _type  = T_INT;
    _i     = value;
  }

  StackValue(Handle value) {
    _type    = T_OBJECT;
    _o       = value;
  }

  StackValue() {
    _type   = T_CONFLICT;
    _i      = 0;
  }

  Handle get_obj() const {
    assert(type() == T_OBJECT, "type check");
    return _o;
  }

  void set_obj(Handle value) {
    assert(type() == T_OBJECT, "type check");
    _o = value;
  }

  intptr_t get_int() const {
    assert(type() == T_INT, "type check");
    return _i;
  }

  void set_int(intptr_t value) {
    assert(type() == T_INT, "type check");
    _i = value;
  }

  BasicType type() const { return  _type; }

  bool equal(StackValue *value) {
    if (_type != value->_type) return false;
    if (_type == T_OBJECT) 
      return (_o == value->_o);
    else {
      assert(_type == T_INT, "sanity check");
      // [phh] compare only low addressed portions of intptr_t slots
      return (*(int *)&_i == *(int *)&value->_i);    
    }
  }

#ifndef PRODUCT
 public:
  // Printing
  void print_on(outputStream* st) const;
#endif
};

