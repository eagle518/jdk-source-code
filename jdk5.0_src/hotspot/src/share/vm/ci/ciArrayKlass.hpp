#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciArrayKlass.hpp	1.7 03/12/23 16:39:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciArrayKlass
//
// This class, and its subclasses represent klassOops in the
// HotSpot virtual machine whose Klass part is an arrayKlass.
class ciArrayKlass : public ciKlass {
private:
  jint _dimension;

protected:
  ciArrayKlass(KlassHandle h_k);
  ciArrayKlass(ciSymbol* name, int dimension, ciKlass* klass);

  arrayKlass* get_arrayKlass() {
    return (arrayKlass*)get_Klass();
  }

  const char* type_string() { return "ciArrayKlass"; }

public:
  jint dimension() { return _dimension; }
  ciType* element_type();       // JLS calls this the "component type"
  ciType* base_element_type();  // JLS calls this the "element type"
  
  // What kind of vmObject is this?
  bool is_array_klass() { return true; }
  bool is_java_klass()  { return true; }

  static ciArrayKlass* make(ciType* element_type);
};
