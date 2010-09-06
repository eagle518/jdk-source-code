#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)reflectionUtils.hpp	1.9 03/12/23 16:44:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A KlassStream is an abstract stream for streaming over self, superclasses
// and (super)interfaces. Streaming is done in reverse order (subclasses first,
// interfaces last).
//
//    for (KlassStream st(k, false, false); !st.eos(); st.next()) {
//      klassOop k = st.klass();
//      ...
//    }

class KlassStream VALUE_OBJ_CLASS_SPEC {
 protected:
  instanceKlassHandle _klass;           // current klass/interface iterated over
  objArrayHandle      _interfaces;      // transitive interfaces for initial class
  int                 _interface_index; // current interface being processed
  bool                _local_only;      // process initial class/interface only
  bool                _classes_only;    // process classes only (no interfaces)
  int                 _index;

  virtual int length() const = 0;

 public:
  // constructor
  KlassStream(instanceKlassHandle klass, bool local_only, bool classes_only);

  // testing
  bool eos();

  // iterating
  virtual void next() = 0;

  // accessors
  instanceKlassHandle klass() const { return _klass; }
  int index() const                 { return _index; }
};


// A MethodStream streams over all methods in a class, superclasses and (super)interfaces.
// Streaming is done in reverse order (subclasses first, methods in reverse order)
// Usage:
//
//    for (MethodStream st(k, false, false); !st.eos(); st.next()) {
//      methodOop m = st.method();
//      ...
//    }

class MethodStream : public KlassStream {
 private:
  int length() const          { return methods()->length(); }
  objArrayOop methods() const { return _klass->methods(); }
 public:
  MethodStream(instanceKlassHandle klass, bool local_only, bool classes_only)
    : KlassStream(klass, local_only, classes_only) {
    _index = length();
    next();
  }

  void next() { _index--; }
  methodOop method() const { return methodOop(methods()->obj_at(index())); }
};


// A FieldStream streams over all fields in a class, superclasses and (super)interfaces.
// Streaming is done in reverse order (subclasses first, fields in reverse order)
// Usage:
//
//    for (FieldStream st(k, false, false); !st.eos(); st.next()) {
//      symbolOop field_name = st.name();
//      ...
//    }


class FieldStream : public KlassStream {
 private:
  int length() const                { return fields()->length(); }
  typeArrayOop fields() const       { return _klass->fields(); }
  constantPoolOop constants() const { return _klass->constants(); }
 public:
  FieldStream(instanceKlassHandle klass, bool local_only, bool classes_only)
    : KlassStream(klass, local_only, classes_only) {
    _index = length();
    next();
  }

  void next() { _index -= instanceKlass::next_offset; }

  // Accessors for current field
  AccessFlags access_flags() const { 
    AccessFlags flags;
    flags.set_flags(fields()->ushort_at(index() + instanceKlass::access_flags_offset));
    return flags;
  }
  symbolOop name() const {
    int name_index = fields()->ushort_at(index() + instanceKlass::name_index_offset);
    return constants()->symbol_at(name_index);
  }
  symbolOop signature() const {
    int signature_index = fields()->ushort_at(index() + instanceKlass::signature_index_offset);
    return constants()->symbol_at(signature_index);
  }  
  // missing: initval()
  int offset() const {
    return _klass->offset_from_fields( index() );
  }
  
};


