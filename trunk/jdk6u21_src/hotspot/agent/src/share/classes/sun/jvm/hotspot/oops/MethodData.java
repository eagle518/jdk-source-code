/*
 * Copyright (c) 2000, 2004, Oracle and/or its affiliates. All rights reserved.
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

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

// A MethodData provides interpreter profiling information

public class MethodData extends Oop {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type      = db.lookupType("methodDataOopDesc");
    baseOffset     = type.getSize();

    size           = new CIntField(type.getCIntegerField("_size"), 0);
    method         = new OopField(type.getOopField("_method"), 0);
    // FIXME: add more fields and accessors
  }

  MethodData(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  public boolean isMethodData()        { return true; }

  private static long baseOffset;
  private static CIntField size;
  private static OopField  method;

  public long getObjectSize() {
    return alignObjectSize(size.getValue(this));
  }

  public Method getMethod() {
    return (Method) method.getValue(this);
  }

  public void printValueOn(PrintStream tty) {
    Method m = getMethod();
    tty.print("MethodData for " + m.getName().asString() + m.getSignature().asString());
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(method, true);
      visitor.doCInt(size, true);
    }
  }
}
