/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_jvmtiUtil.cpp.incl"
//
// class JvmtiUtil
//

ResourceArea* JvmtiUtil::_single_threaded_resource_area = NULL;

ResourceArea* JvmtiUtil::single_threaded_resource_area() {
  if (_single_threaded_resource_area == NULL) {
    // lazily create the single threaded resource area
    // pick a size which is not a standard since the pools don't exist yet
    _single_threaded_resource_area = new ResourceArea(Chunk::non_pool_size);
  }
  return _single_threaded_resource_area;
}
