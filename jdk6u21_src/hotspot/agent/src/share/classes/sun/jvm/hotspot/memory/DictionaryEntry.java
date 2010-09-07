/*
 * Copyright (c) 2003, 2005, Oracle and/or its affiliates. All rights reserved.
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
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.utilities.*;

public class DictionaryEntry extends sun.jvm.hotspot.utilities.HashtableEntry {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("DictionaryEntry");
    pdSetField = type.getAddressField("_pd_set");
    loaderField = type.getOopField("_loader");
  }

  // Fields
  private static AddressField pdSetField;
  private static sun.jvm.hotspot.types.OopField loaderField;

  // Accessors

  public ProtectionDomainEntry pdSet() {
    Address tmp = pdSetField.getValue(addr);
    return (ProtectionDomainEntry) VMObjectFactory.newObject(
                          ProtectionDomainEntry.class, tmp);
  }

  public Oop loader() {
    return VM.getVM().getObjectHeap().newOop(loaderField.getValue(addr));
  }

  public Klass klass() {
    return (Klass) literal();
  }

  public DictionaryEntry(Address addr) {
    super(addr);
  }

  public boolean equals(Symbol className, Oop classLoader) {
    InstanceKlass ik = (InstanceKlass) klass();
    Oop loader = loader();
    if (! ik.getName().equals(className)) {
      return false;
    } else {
      return (loader == null)? (classLoader == null) :
                               (loader.equals(classLoader));
    }
  }

  public boolean isValidProtectionDomain(Oop protectionDomain) {
    if (protectionDomain == null) {
      return true;
    } else {
      return containsProtectionDomain(protectionDomain);
    }
  }

  public boolean containsProtectionDomain(Oop protectionDomain) {
    InstanceKlass ik = (InstanceKlass) klass();
    if (protectionDomain.equals(ik.getProtectionDomain())) {
      return true; // Succeeds trivially
    }
    for (ProtectionDomainEntry current = pdSet(); current != null;
                                       current = current.next()) {
      if (protectionDomain.equals(current.protectionDomain())) {
        return true;
      }
    }
    return false;
  }

  /* covariant return type :-(
  public DictionaryEntry next() {
    return (DictionaryEntry) super.next();
  }
  For now, let the caller cast it ..
  */
}
