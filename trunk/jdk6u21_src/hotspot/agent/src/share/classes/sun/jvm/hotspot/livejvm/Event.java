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

public class Event {
  public static class Type {
    private Type() {}
    public static final Type BREAKPOINT = new Type();
    public static final Type EXCEPTION  = new Type();
  }

  private Type type;

  public Event(Type type) {
    this.type = type;
  }

  public Type getType() { return type; }
}
