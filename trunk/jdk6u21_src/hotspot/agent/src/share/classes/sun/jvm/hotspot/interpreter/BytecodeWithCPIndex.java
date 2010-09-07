/*
 * Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;

// any bytecode with constant pool index

public abstract class BytecodeWithCPIndex extends Bytecode {
  BytecodeWithCPIndex(Method method, int bci) {
    super(method, bci);
  }

  // the constant pool index for this bytecode
  public int index() { return 0xFFFF & javaShortAt(1); }

  protected int indexForFieldOrMethod() {
     ConstantPoolCache cpCache = method().getConstants().getCache();
     // get ConstantPool index from ConstantPoolCacheIndex at given bci
     int cpCacheIndex = index();
     if (cpCache == null) {
        return cpCacheIndex;
     } else {
        // change byte-ordering and go via cache
        return cpCache.getEntryAt((int) (0xFFFF & VM.getVM().getBytes().swapShort((short) cpCacheIndex))).getConstantPoolIndex();
     }
  }
}
