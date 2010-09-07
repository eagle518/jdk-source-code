/*
 * Copyright (c) 2000, 2001, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.debugger.*;

// The class for a boolean field simply provides access to the value.
public class BooleanField extends Field {
  public BooleanField(FieldIdentifier id, long offset, boolean isVMField) {
    super(id, offset, isVMField);
  }

  public BooleanField(sun.jvm.hotspot.types.JBooleanField vmField, long startOffset) {
    super(new NamedFieldIdentifier(vmField.getName()), vmField.getOffset() + startOffset, true);
  }

  public BooleanField(InstanceKlass holder, int fieldArrayIndex) {
    super(holder, fieldArrayIndex);
  }

  public boolean getValue(Oop obj) { return obj.getHandle().getJBooleanAt(getOffset()); }
  public void setValue(Oop obj, boolean value) throws MutationException {
    // Fix this: setJBooleanAt is missing in Address
  }
}
