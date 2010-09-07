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
import sun.jvm.hotspot.debugger.dbx.*;

public class DbxX86ThreadFactory implements DbxThreadFactory {
  private DbxDebugger debugger;

  public DbxX86ThreadFactory(DbxDebugger debugger) {
    this.debugger = debugger;
  }

  public ThreadProxy createThreadWrapper(Address threadIdentifierAddr) {
    return new DbxX86Thread(debugger, threadIdentifierAddr);
  }

  public ThreadProxy createThreadWrapper(long id) {
    return new DbxX86Thread(debugger, id);
  }
}
