#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciField.hpp	1.12 03/12/23 16:39:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciField
//
// This class represents the result of a field lookup in the VM.
// The lookup may not succeed, in which case the information in
// the ciField will be incomplete.
class ciField : public ResourceObj {
  CI_PACKAGE_ACCESS
  friend class ciEnv;
  friend class ciInstanceKlass;

private:
  ciFlags          _flags;
  ciInstanceKlass* _holder;
  ciType*          _type;
  int              _offset;
  bool             _is_constant;
  ciInstanceKlass* _known_to_link_with;
  ciConstant       _constant_value;

  // Used for will_link
  int              _cp_index;

  ciField(ciInstanceKlass* klass, int index);
  ciField(fieldDescriptor* fd);

  // shared constructor code
  void initialize_from(fieldDescriptor* fd);

  // The implementation of the print method.
  void print_impl();

public:
  ciFlags flags() { return _flags; }

  // Of which klass is this field a member?
  //
  // Usage note: the declared holder of a field is the class
  // referenced by name in the bytecodes.  The canonical holder
  // is the most general class which holds the field.  This
  // method returns the canonical holder.  The declared holder
  // can be accessed via a method in ciByteCodeStream.
  //
  // Ex.
  //     class A {
  //       public int f = 7;
  //     }
  //     class B extends A {
  //       public void test() {
  //         System.out.println(f);
  //       }
  //     }
  //
  //   A java compiler is permitted to compile the access to
  //   field f as:
  //   
  //     getfield B.f
  //
  //   In that case the declared holder of f would be B and
  //   the canonical holder of f would be A.
  ciInstanceKlass* holder() { return _holder; }

  // Of what type is this field?
  ciType* type() { return _type; }

  // How is this field actually stored in memory?
  BasicType layout_type() { return type2field[type()->basic_type()]; }

  // What is the offset of this field?
  int offset() {
    assert(_offset >= 1, "illegal call to offset()");
    return _offset;
  }

  int offset_in_bytes() {
    return offset();
  }

  // Is this field a constant?
  //
  // Clarification: A field is considered constant if:
  //   1. The field is both static and final
  //   2. The canonical holder of the field has undergone
  //      static initialization.
  //   3. If the field is an object or array, then the oop
  //      in question is allocated in perm space.
  //   4. The field is not one of the special static/final
  //      non-constant fields.  These are java.lang.System.in
  //      and java.lang.System.out.  Abomination.
  //
  // Note: the check for case 4 is not yet implemented.
  bool is_constant() { return _is_constant; }

  // Get the constant value of this field.
  ciConstant constant_value() {
    assert(is_constant(), "illegal call to constant_value()");
    return _constant_value;
  }

  // Check for link time errors.  Accessing a field from a
  // certain class via a certain bytecode may or may not be legal.
  // This call checks to see if an exception may be raised by
  // an access of this field.
  //
  // Usage note: if the same field is accessed multiple times
  // in the same compilation, will_link will need to be checked
  // at each point of access.
  bool will_link(ciInstanceKlass* accessing_klass,
                 Bytecodes::Code bc);

  // Java access flags
  bool is_public      () { return flags().is_public(); }
  bool is_private     () { return flags().is_private(); }
  bool is_protected   () { return flags().is_protected(); }
  bool is_static      () { return flags().is_static(); }
  bool is_final       () { return flags().is_final(); }
  bool is_volatile    () { return flags().is_volatile(); }
  bool is_transient   () { return flags().is_transient(); }

  // Debugging output
  void print();
};
