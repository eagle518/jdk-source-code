/*
 * Copyright (c) 2001, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.memory;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class ProtectionDomainEntry extends VMObject {
  private static AddressField nextField;
  private static sun.jvm.hotspot.types.OopField protectionDomainField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("ProtectionDomainEntry");

    nextField = type.getAddressField("_next");
    protectionDomainField = type.getOopField("_protection_domain");
  }

  public ProtectionDomainEntry(Address addr) {
    super(addr);
  }

  public ProtectionDomainEntry next() {
    return (ProtectionDomainEntry) VMObjectFactory.newObject(ProtectionDomainEntry.class, addr);
  }

  public Oop protectionDomain() {
    return VM.getVM().getObjectHeap().newOop(protectionDomainField.getValue(addr));
  }
}
