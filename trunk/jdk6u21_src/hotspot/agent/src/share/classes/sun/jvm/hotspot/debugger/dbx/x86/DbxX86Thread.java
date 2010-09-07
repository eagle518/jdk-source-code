/*
 * Copyright (c) 2000, 2002, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.debugger.dbx.x86;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.x86.*;
import sun.jvm.hotspot.debugger.dbx.*;
import sun.jvm.hotspot.utilities.*;

public class DbxX86Thread implements ThreadProxy {
  private DbxDebugger debugger;
  private int         id;

  public DbxX86Thread(DbxDebugger debugger, Address addr) {
    this.debugger = debugger;

    // FIXME: the size here should be configurable. However, making it
    // so would produce a dependency on the "types" package from the
    // debugger package, which is not desired.
    this.id       = (int) addr.getCIntegerAt(0, 4, true);
  }

  public DbxX86Thread(DbxDebugger debugger, long id) {
    this.debugger = debugger;
    this.id  = (int) id;
  }

  public boolean equals(Object obj) {
    if ((obj == null) || !(obj instanceof DbxX86Thread)) {
      return false;
    }

    return (((DbxX86Thread) obj).id == id);
  }

  public int hashCode() {
    return id;
  }

  public ThreadContext getContext() throws IllegalThreadStateException {
    DbxX86ThreadContext context = new DbxX86ThreadContext(debugger);
    long[] regs = debugger.getThreadIntegerRegisterSet(id);
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(regs.length == 19, "unknown size of register set -- adjust this code");
    }
    for (int i = 0; i < regs.length; i++) {
      context.setRegister(i, regs[i]);
    }
    return context;
  }

  public boolean canSetContext() throws DebuggerException {
    return false;
  }

  public void setContext(ThreadContext context)
    throws IllegalThreadStateException, DebuggerException {
    throw new DebuggerException("Unimplemented");
  }

  public String toString() {
    return "t@" + id;
  }
}
