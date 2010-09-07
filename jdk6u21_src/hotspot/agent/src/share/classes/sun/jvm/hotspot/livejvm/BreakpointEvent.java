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

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;

public class BreakpointEvent extends Event {
  private Oop thread;
  private Oop clazz;
  private JNIid method;
  private int location;

  public BreakpointEvent(Oop thread,
                         Oop clazz,
                         JNIid method,
                         int location) {
    super(Event.Type.BREAKPOINT);
    this.thread = thread;
    this.clazz = clazz;
    this.method = method;
    this.location = location;
  }

  public Oop thread()     { return thread;   }
  public Oop clazz()      { return clazz;    }
  public JNIid methodID() { return method;   }
  public int location()   { return location; }
}
