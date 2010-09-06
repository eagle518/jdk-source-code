#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)classify.hpp	1.2 03/12/23 16:40:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

typedef enum oop_type {
  unknown_type,
  instance_type,
  instanceRef_type,
  objArray_type,
  symbol_type,
  klass_type,
  instanceKlass_type,
  method_type,
  constMethod_type,
#ifndef CORE
  methodData_type,
#endif // !CORE
  constantPool_type,
  constantPoolCache_type,
  typeArray_type,
  compiledICHolder_type,
  number_object_types
} object_type;


// Classify objects by type and keep counts.
// Print the count and space taken for each type.


class ClassifyObjectClosure : public ObjectClosure {
private:

  static const char* object_type_name[number_object_types];

  int total_object_count;
  size_t total_object_size;
  int object_count[number_object_types];
  size_t object_size[number_object_types];

public:
  ClassifyObjectClosure() { reset(); }
  void reset();
  void do_object(oop obj);
  static object_type classify_object(oop obj, bool count);
  size_t print();
};


// Count objects using the alloc_count field in the object's klass
// object.

class ClassifyInstanceKlassClosure : public ClassifyObjectClosure {
private:
  int total_instances;
public:
  ClassifyInstanceKlassClosure() { reset(); }
  void reset();
  void print();
  void do_object(oop obj);
};


// Clear the alloc_count fields in all classes so that the count can be
// restarted.

class ClearAllocCountClosure : public ObjectClosure {
public:
  void do_object(oop obj) {
    if (obj->is_klass()) {
      Klass* k = Klass::cast((klassOop)obj);
      k->set_alloc_count(0);
    }
  }
};
