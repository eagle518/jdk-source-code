/*
 * Copyright (c) 2000, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.compiler;

import sun.jvm.hotspot.code.*;

public class OopMapStream {
  private CompressedReadStream stream;
  private OopMap oopMap;
  private int mask;
  private int size;
  private int position;
  private OopMapValue omv;
  private boolean omvValid;

  public OopMapStream(OopMap oopMap) {
    this(oopMap, (OopMapValue.OopTypes[]) null);
  }

  public OopMapStream(OopMap oopMap, OopMapValue.OopTypes type) {
    this(oopMap, (OopMapValue.OopTypes[]) null);
    mask = type.getValue();
  }

  public OopMapStream(OopMap oopMap, OopMapValue.OopTypes[] types) {
    if (oopMap.getOMVData() == null) {
      stream = new CompressedReadStream(oopMap.getWriteStream().getBuffer());
    } else {
      stream = new CompressedReadStream(oopMap.getOMVData());
    }
    mask = computeMask(types);
    size = (int) oopMap.getOMVCount();
    position = 0;
    omv = new OopMapValue();
    omvValid = false;
  }

  public boolean isDone() {
    if (!omvValid) {
      findNext();
    }
    return !omvValid;
  }

  public void next() {
    findNext();
  }

  public OopMapValue getCurrent() {
    return omv;
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private int computeMask(OopMapValue.OopTypes[] types) {
    mask = 0;
    if (types != null) {
      for (int i = 0; i < types.length; i++) {
        mask |= types[i].getValue();
      }
    }
    return mask;
  }

  private void findNext() {
    while (position++ < size) {
      omv.readFrom(stream);
      if ((omv.getType().getValue() & mask) > 0) {
        omvValid = true;
        return;
      }
    }
    omvValid = false;
  }
}
