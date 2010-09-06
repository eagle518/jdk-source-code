#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)constantTag.cpp	1.15 03/12/23 16:44:41 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_constantTag.cpp.incl"

#ifndef PRODUCT

void constantTag::print_on(outputStream* st) const {
  switch (_tag) {
    case JVM_CONSTANT_Class :
      st->print("Class");
      break;
    case JVM_CONSTANT_Fieldref :
      st->print("Field");
      break;
    case JVM_CONSTANT_Methodref :
      st->print("Method");
      break;
    case JVM_CONSTANT_InterfaceMethodref :
      st->print("InterfaceMethod");
      break;
    case JVM_CONSTANT_String :
      st->print("String");
      break;
    case JVM_CONSTANT_Integer :
      st->print("Integer");
      break;
    case JVM_CONSTANT_Float :
      st->print("Float");
      break;
    case JVM_CONSTANT_Long :
      st->print("Long");
      break;
    case JVM_CONSTANT_Double :
      st->print("Double");
      break;
    case JVM_CONSTANT_NameAndType :
      st->print("NameAndType");
      break;
    case JVM_CONSTANT_Utf8 :
      st->print("Utf8");
      break;
    case JVM_CONSTANT_UnresolvedClass :
      st->print("Unresolved class");
      break;
    case JVM_CONSTANT_ClassIndex :
      st->print("Unresolved class index");
      break;
    case JVM_CONSTANT_UnresolvedString :
      st->print("Unresolved string");
      break;
    case JVM_CONSTANT_StringIndex :
      st->print("Unresolved string index");
      break;
    default:
      ShouldNotReachHere();
      break;
  }         
}

#endif // PRODUCT
