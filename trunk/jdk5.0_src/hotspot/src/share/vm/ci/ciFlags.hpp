#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciFlags.hpp	1.8 03/12/23 16:39:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciFlags
//
// This class represents klass or method flags.
class ciFlags VALUE_OBJ_CLASS_SPEC {
private:
  friend class ciInstanceKlass;
  friend class ciField;
  friend class ciMethod;

  jint _flags;

  ciFlags()                  { _flags = 0; }
  ciFlags(AccessFlags flags) { _flags = flags.as_int(); }

public:
  // Java access flags
  bool is_public      () const         { return (_flags & JVM_ACC_PUBLIC      ) != 0; }
  bool is_private     () const         { return (_flags & JVM_ACC_PRIVATE     ) != 0; }
  bool is_protected   () const         { return (_flags & JVM_ACC_PROTECTED   ) != 0; }
  bool is_static      () const         { return (_flags & JVM_ACC_STATIC      ) != 0; }
  bool is_final       () const         { return (_flags & JVM_ACC_FINAL       ) != 0; }
  bool is_synchronized() const         { return (_flags & JVM_ACC_SYNCHRONIZED) != 0; }
  bool is_super       () const         { return (_flags & JVM_ACC_SUPER       ) != 0; }
  bool is_volatile    () const         { return (_flags & JVM_ACC_VOLATILE    ) != 0; }
  bool is_transient   () const         { return (_flags & JVM_ACC_TRANSIENT   ) != 0; }
  bool is_native      () const         { return (_flags & JVM_ACC_NATIVE      ) != 0; }
  bool is_interface   () const         { return (_flags & JVM_ACC_INTERFACE   ) != 0; }
  bool is_abstract    () const         { return (_flags & JVM_ACC_ABSTRACT    ) != 0; }
  bool is_strict      () const         { return (_flags & JVM_ACC_STRICT      ) != 0; }  
  
  // Conversion
  jint   as_int()                      { return _flags; }

  void print_klass_flags();
  void print_member_flags();
  void print();
};
