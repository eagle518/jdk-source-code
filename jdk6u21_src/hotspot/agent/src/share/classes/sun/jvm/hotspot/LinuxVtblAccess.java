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

package sun.jvm.hotspot;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.types.basic.*;

public class LinuxVtblAccess extends BasicVtblAccess {
  private String vt;

  public LinuxVtblAccess(SymbolLookup symbolLookup,
                         String[] dllNames) {
    super(symbolLookup, dllNames);

    if (symbolLookup.lookup("libjvm.so", "__vt_10JavaThread") != null ||
        symbolLookup.lookup("libjvm_g.so", "__vt_10JavaThread") != null) {
       // old C++ ABI
       vt = "__vt_";
    } else {
       // new C++ ABI
       vt = "_ZTV";
    }
  }

  protected String vtblSymbolForType(Type type) {
    return vt + type.getName().length() + type;
  }
}
