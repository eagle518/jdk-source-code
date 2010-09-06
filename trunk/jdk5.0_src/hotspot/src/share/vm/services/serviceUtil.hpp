#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)serviceUtil.hpp	1.2 04/07/29 16:36:24 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

//
// Serviceability utility functions.
// (Shared by MM and JVMTI).
//
class ServiceUtil : public AllStatic {
 public:
    
  // Return true if oop represents an object that is "visible"
  // to the java world.
  static inline bool visible_oop(oop o) {
    // the sentinel for deleted handles isn't visible
    if (o == JNIHandles::deleted_handle()) {
      return false;
    }

    // ignore KlassKlass
    if (o->is_klass()) {
      return false;
    }

    // instance
    if (o->is_instance()) {
      // instance objects are visible
      if (o->klass() != SystemDictionary::class_klass()) {
        return true;
      }
      if (java_lang_Class::is_primitive(o)) {
        return true;
      }
      // java.lang.Classes are visible
      o = java_lang_Class::as_klassOop(o);
      if (o->is_klass()) {
        // if it's a class for an object, an object array, or
        // primitive (type) array then it's visible.
        klassOop klass = (klassOop)o;
        if (Klass::cast(klass)->oop_is_instance()) {
          return true;
        }
        if (Klass::cast(klass)->oop_is_objArray()) {
          return true;
        }
        if (Klass::cast(klass)->oop_is_typeArray()) {
          return true;
        }
      }
      return false;
    }
    // object arrays are visible if they aren't system object arrays
    if (o->is_objArray()) {
      objArrayOop array = (objArrayOop)o;
      if (array->klass() != Universe::systemObjArrayKlassObj()) {
        return true;
      } else {
        return false;
      }
    }
    // type arrays are visible
    if (o->is_typeArray()) {
      return true;
    }
    // everything else (methodOops, ...) aren't visible
    return false;
  };   // end of visible_oop()
    
};
