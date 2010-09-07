/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2008, 2009 Red Hat, Inc.
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

class ZeroEntry {
 public:
  ZeroEntry() {
    ShouldNotCallThis();
  }

 private:
  address _entry_point;

 public:
  address entry_point() const {
    return _entry_point;
  }
  void set_entry_point(address entry_point) {
    _entry_point = entry_point;
  }

 private:
  typedef void (*NormalEntryFunc)(methodOop method,
                                  intptr_t  base_pc,
                                  TRAPS);
  typedef void (*OSREntryFunc)(methodOop method,
                               address   osr_buf,
                               intptr_t  base_pc,
                               TRAPS);

 public:
  void invoke(methodOop method, TRAPS) const {
    ((NormalEntryFunc) entry_point())(method, (intptr_t) this, THREAD);
  }
  void invoke_osr(methodOop method, address osr_buf, TRAPS) const {
    ((OSREntryFunc) entry_point())(method, osr_buf, (intptr_t) this, THREAD);
  }

 public:
  static ByteSize entry_point_offset() {
    return byte_offset_of(ZeroEntry, _entry_point);
  }
};
