#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciSignature.hpp	1.10 03/12/23 16:39:39 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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



