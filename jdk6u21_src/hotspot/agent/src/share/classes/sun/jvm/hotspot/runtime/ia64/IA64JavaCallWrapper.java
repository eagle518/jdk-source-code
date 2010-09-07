/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.runtime.ia64;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.*;

public class IA64JavaCallWrapper extends JavaCallWrapper {
  private static AddressField lastJavaIFrameField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("JavaCallWrapper");

    lastJavaIFrameField  = type.getAddressField("_last_Java_iframe");
  }

  public IA64JavaCallWrapper(Address addr) {
    super(addr);
  }

  public Address getPrevIFrame() {
    return lastJavaIFrameField.getValue(addr);
  }
}
