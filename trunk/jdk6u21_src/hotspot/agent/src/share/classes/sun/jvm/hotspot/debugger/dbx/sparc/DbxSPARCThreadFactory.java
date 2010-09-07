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
import sun.jvm.hotspot.debugger.dbx.*;

public class DbxSPARCThreadFactory implements DbxThreadFactory {
  private DbxDebugger debugger;

  public DbxSPARCThreadFactory(DbxDebugger debugger) {
    this.debugger = debugger;
  }

  public ThreadProxy createThreadWrapper(Address threadIdentifierAddr) {
    return new DbxSPARCThread(debugger, threadIdentifierAddr);
  }

  public ThreadProxy createThreadWrapper(long id) {
    return new DbxSPARCThread(debugger, id);
  }
}
