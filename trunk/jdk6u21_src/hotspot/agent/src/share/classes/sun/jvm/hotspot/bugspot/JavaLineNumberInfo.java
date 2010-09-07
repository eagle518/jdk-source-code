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

package sun.jvm.hotspot.bugspot;

import sun.jvm.hotspot.oops.*;

/** Wrapper class which describes line number information for Java
    class files. The line number table is converted into this
    representation on demand. These objects are then sorted by line
    number for fast lookup when setting breakpoints in a particular
    source file. */

public class JavaLineNumberInfo {
  private InstanceKlass klass;
  private Method method;
  private int startBCI;
  private int lineNumber;

  public JavaLineNumberInfo(InstanceKlass klass,
                            Method method,
                            int startBCI,
                            int lineNumber) {
    this.klass = klass;
    this.method = method;
    this.startBCI = startBCI;
    this.lineNumber = lineNumber;
  }

  public InstanceKlass getKlass()      { return klass; }
  public Method        getMethod()     { return method; }
  public int           getStartBCI()   { return startBCI; }
  public int           getLineNumber() { return lineNumber; }
}
