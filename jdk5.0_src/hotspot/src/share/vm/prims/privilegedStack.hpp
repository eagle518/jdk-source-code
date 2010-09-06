#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)privilegedStack.hpp	1.19 03/12/23 16:43:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class PrivilegedElement VALUE_OBJ_CLASS_SPEC {
 private:  
  klassOop  _klass;                // klass for method 
  oop       _privileged_context;   // context for operation  
  intptr_t*     _frame_id;             // location on stack
  PrivilegedElement* _next;        // Link to next one on stack
 public:
  void initialize(vframeStream* vf, oop context, PrivilegedElement* next, TRAPS);  
  void oops_do(OopClosure* f);    
  intptr_t* frame_id() const           { return _frame_id; }
  oop  privileged_context() const  { return _privileged_context; }  
  oop  class_loader() const        { return instanceKlass::cast(_klass)->class_loader(); }
  oop  protection_domain() const   { return instanceKlass::cast(_klass)->protection_domain(); }
  PrivilegedElement *next() const  { return _next; }

  // debugging (used for find)
  void print_on(outputStream* st) const   PRODUCT_RETURN;
  bool contains(address addr)             PRODUCT_RETURN0;
};

