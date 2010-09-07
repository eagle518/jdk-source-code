/*
 * Copyright (c) 1999, 2000, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

// ciSignature
//
// This class represents the signature of a method.
class ciSignature : public ResourceObj {
private:
  ciSymbol* _symbol;
  ciKlass*  _accessing_klass;

  GrowableArray<ciType*>* _types;
  int _size;
  int _count;

  friend class ciMethod;

  ciSignature(ciKlass* accessing_klass, ciSymbol* signature);

  void get_all_klasses();

  symbolOop get_symbolOop() const                { return _symbol->get_symbolOop(); }

public:
  ciSymbol* as_symbol() const                    { return _symbol; }

  ciType* return_type() const;
  ciType* type_at(int index) const;

  int       size() const                         { return _size; }
  int       count() const                        { return _count; }

  void print_signature();
  void print();
};
