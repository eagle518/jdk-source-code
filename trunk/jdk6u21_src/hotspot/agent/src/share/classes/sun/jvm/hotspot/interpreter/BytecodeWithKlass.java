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

public class BytecodeWithKlass extends BytecodeWithCPIndex {
  BytecodeWithKlass(Method method, int bci) {
    super(method, bci);
  }

  protected Klass getKlass() {
    return method().getConstants().getKlassRefAt(index());
  }

  public Symbol getClassName() {
    Oop obj = method().getConstants().getObjAt(index());
    if (obj instanceof Symbol) {
       return (Symbol)obj;
    } else {
       return ((Klass)obj).getName();
    }
  }

  public String toString() {
    StringBuffer buf = new StringBuffer();
    buf.append(getJavaBytecodeName());
    buf.append(spaces);
    buf.append('#');
    buf.append(Integer.toString(index()));
    buf.append(spaces);
    buf.append("[Class ");
    buf.append(getClassName().asString().replace('/', '.'));
    buf.append(']');
    if (code() != javaCode()) {
       buf.append(spaces);
       buf.append('[');
       buf.append(getBytecodeName());
       buf.append(']');
    }
    return buf.toString();
  }
}
