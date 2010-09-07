/*
 * Copyright (c) 1999, 2006, Oracle and/or its affiliates. All rights reserved.
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


GrowableArray<FilteredField*> *FilteredFieldsMap::_filtered_fields =
  new (ResourceObj::C_HEAP) GrowableArray<FilteredField*>(3,true);


void FilteredFieldsMap::initialize() {
  int offset;
  offset = java_lang_Throwable::get_backtrace_offset();
  _filtered_fields->append(new FilteredField(SystemDictionary::Throwable_klass(), offset));
  // The latest version of vm may be used with old jdk.
  if (JDK_Version::is_gte_jdk16x_version()) {
    // The following class fields do not exist in
    // previous version of jdk.
    offset = sun_reflect_ConstantPool::cp_oop_offset();
    _filtered_fields->append(new FilteredField(SystemDictionary::reflect_ConstantPool_klass(), offset));
    offset = sun_reflect_UnsafeStaticFieldAccessorImpl::base_offset();
    _filtered_fields->append(new FilteredField(SystemDictionary::reflect_UnsafeStaticFieldAccessorImpl_klass(), offset));
  }
}

int FilteredFieldStream::field_count() {
  int numflds = 0;
  for (;!eos(); next()) {
    numflds++;
  }
  return numflds;
}
