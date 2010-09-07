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

public class ExceptionEvent extends Event {
  private Oop thread;
  private Oop clazz;
  private JNIid method;
  private int location;
  private Oop exception;
  private Oop catchClass;
  private JNIid catchMethod;
  private int catchLocation;

  public ExceptionEvent(Oop thread,
                        Oop clazz,
                        JNIid method,
                        int location,
                        Oop exception,
                        Oop catchClass,
                        JNIid catchMethod,
                        int catchLocation) {
    super(Event.Type.EXCEPTION);
    this.thread        = thread;
    this.clazz         = clazz;
    this.method        = method;
    this.location      = location;
    this.exception     = exception;
    this.catchClass    = catchClass;
    this.catchMethod   = catchMethod;
    this.catchLocation = catchLocation;
  }

  public Oop   thread()        { return thread;        }
  public Oop   clazz()         { return clazz;         }
  public JNIid methodID()      { return method;        }
  public int   location()      { return location;      }
  public Oop   exception()     { return exception;     }
  public Oop   catchClass()    { return catchClass;    }
  public JNIid catchMethodID() { return catchMethod;   }
  public int   catchLocation() { return catchLocation; }
}
