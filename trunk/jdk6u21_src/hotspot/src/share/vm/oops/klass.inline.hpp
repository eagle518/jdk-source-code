/*
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
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

inline void Klass::set_prototype_header(markOop header) {
  assert(!header->has_bias_pattern() || oop_is_instance(), "biased locking currently only supported for Java instances");
  _prototype_header = header;
}
