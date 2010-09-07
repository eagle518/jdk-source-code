/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.livejvm;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;

class JNIHandleAccessor {
  private Address addr;
  private ObjectHeap heap;

  JNIHandleAccessor(Address addr, ObjectHeap heap) {
    this.addr = addr;
    this.heap = heap;
  }

  Oop getValue() {
    // Accessing the contents of the JNIHandle is a double dereference
    Address handle = addr.getAddressAt(0);
    if (handle == null) return null;
    return heap.newOop(handle.getOopHandleAt(0));
  }

  void setValue(Oop value) {
    Address handle = addr.getAddressAt(0);
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(handle != null, "Must have valid global JNI handle for setting");
    }
    handle.setOopHandleAt(0, value.getHandle());
  }
}
