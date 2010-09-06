#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciObjArrayKlassKlass.cpp	1.4 03/12/23 16:39:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciObjArrayKlassKlass.cpp.incl"

// ciObjArrayKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is an arrayKlassKlass.

// ------------------------------------------------------------------
// ciObjArrayKlassKlass::instance
//
// Return the distinguished instance of this class
ciObjArrayKlassKlass* ciObjArrayKlassKlass::make() {
  return CURRENT_ENV->_obj_array_klass_klass_instance;
}
