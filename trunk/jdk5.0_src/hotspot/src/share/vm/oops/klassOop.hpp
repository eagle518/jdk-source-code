#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)klassOop.hpp	1.13 03/12/23 16:41:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A klassOop is the C++ equivalent of a Java class.
// Part of a klassOopDesc is a Klass which handle the 
// dispatching for the C++ method calls.

//  klassOop object layout:
//    [header     ]
//    [klass_field]
//    [KLASS      ]

class klassOopDesc : public oopDesc {
 public:
  // size operation
  static int header_size()                       { return sizeof(klassOopDesc)/HeapWordSize; }

  // support for code generation
  static int klass_part_offset_in_bytes()        { return sizeof(klassOopDesc); }

  // returns the Klass part containing dispatching behavior
  Klass* klass_part()                            { return (Klass*)((address)this + klass_part_offset_in_bytes()); }
};



