#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)reflectionUtils.cpp	1.6 03/12/23 16:44:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_reflectionUtils.cpp.incl"

KlassStream::KlassStream(instanceKlassHandle klass, bool local_only, bool classes_only) {
  _klass = klass;
  if (classes_only) {
    _interfaces = Universe::the_empty_system_obj_array();
  } else {
    _interfaces = klass->transitive_interfaces();
  }
  _interface_index = _interfaces->length();
  _local_only = local_only;
  _classes_only = classes_only;
}

bool KlassStream::eos() {
  if (index() >= 0) return false;
  if (_local_only) return true;
  if (!_klass->is_interface() && _klass->super() != NULL) {
    // go up superclass chain (not for interfaces)
    _klass = _klass->super();
  } else { 
    if (_interface_index > 0) {
      _klass = klassOop(_interfaces->obj_at(--_interface_index));
    } else {
      return true;
    }
  }
  _index = length();
  next();
  return eos();
}


