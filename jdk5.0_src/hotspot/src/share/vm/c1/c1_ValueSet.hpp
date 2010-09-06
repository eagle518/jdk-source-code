#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_ValueSet.hpp	1.8 03/12/23 16:39:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A ValueSet is a simple abstraction on top of a BitMap representing
// a set of Instructions. Currently it assumes that the number of
// instructions is fixed during its lifetime; should make it
// automatically resizable.

class ValueSet: public CompilationResourceObj {
 private:
  BitMap _map;

 public:
  ValueSet();
  
  ValueSet* copy();
  bool contains(Value x);
  void put     (Value x);
  void remove  (Value x);
  bool set_intersect(ValueSet* other);
  void clear   ();
  void set_from(ValueSet* other);
  bool equals  (ValueSet* other);
};

inline ValueSet::ValueSet() : _map(Instruction::number_of_instructions()) {
  _map.clear();
}


inline ValueSet* ValueSet::copy() {
  ValueSet* res = new ValueSet();
  res->_map.set_from(_map);
  return res;
}


inline bool ValueSet::contains(Value x) {
  return _map.at(x->id());
}


inline void ValueSet::put(Value x) {
  _map.set_bit(x->id());
}


inline void ValueSet::remove(Value x) {
  _map.clear_bit(x->id());
}


inline bool ValueSet::set_intersect(ValueSet* other) {
  return _map.set_intersection_with_result(other->_map);
}


inline void ValueSet::clear() {
  _map.clear();
}

inline void ValueSet::set_from(ValueSet* other) {
  _map.set_from(other->_map);
}

inline bool ValueSet::equals(ValueSet* other) {
  return _map.is_same(other->_map);
}
