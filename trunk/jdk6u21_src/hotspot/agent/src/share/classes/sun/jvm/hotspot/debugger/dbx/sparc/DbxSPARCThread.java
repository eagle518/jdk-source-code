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

package sun.jvm.hotspot.debugger.dbx.sparc;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.sparc.*;
import sun.jvm.hotspot.debugger.dbx.*;
import sun.jvm.hotspot.utilities.*;

public class DbxSPARCThread implements ThreadProxy {
  private DbxDebugger debugger;
  private int         id;

  public DbxSPARCThread(DbxDebugger debugger, Address addr) {
    this.debugger = debugger;

    // FIXME: the size here should be configurable. However, making it
    // so would produce a dependency on the "types" package from the
    // debugger package, which is not desired.
    this.id       = (int) addr.getCIntegerAt(0, 4, true);
  }

  public DbxSPARCThread(DbxDebugger debugger, long id) {
    this.debugger = debugger;
    this.id = (int) id;
  }

  public boolean equals(Object obj) {
    if ((obj == null) || !(obj instanceof DbxSPARCThread)) {
      return false;
    }

    return (((DbxSPARCThread) obj).id == id);
  }

  public int hashCode() {
    return id;
  }

  public ThreadContext getContext() throws IllegalThreadStateException {
    DbxSPARCThreadContext context = new DbxSPARCThreadContext(debugger);
    long[] regs = debugger.getThreadIntegerRegisterSet(id);
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(regs.length == SPARCThreadContext.NPRGREG, "size of register set must match");
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
