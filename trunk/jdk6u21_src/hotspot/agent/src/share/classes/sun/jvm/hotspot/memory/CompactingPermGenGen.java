/*
 * Copyright (c) 2000, 2005, Oracle and/or its affiliates. All rights reserved.
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

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

/** This is the "generation" view of a CompactingPermGen. */
public class CompactingPermGenGen extends OneContigSpaceCardGeneration {
  private static AddressField unsharedBottomField;
  private static AddressField unsharedEndField;
  private static AddressField sharedBottomField;
  private static AddressField sharedEndField;
  private static AddressField readOnlyBottomField;
  private static AddressField readOnlyEndField;
  private static AddressField readWriteBottomField;
  private static AddressField readWriteEndField;
  private static AddressField roSpaceField;
  private static AddressField rwSpaceField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
     Type type = db.lookupType("CompactingPermGenGen");
     unsharedBottomField = type.getAddressField("unshared_bottom");
     unsharedEndField = type.getAddressField("unshared_end");
     sharedBottomField = type.getAddressField("shared_bottom");
     sharedEndField = type.getAddressField("shared_end");
     readOnlyBottomField = type.getAddressField("readonly_bottom");
     readOnlyEndField = type.getAddressField("readonly_end");
     readWriteBottomField = type.getAddressField("readwrite_bottom");
     readWriteEndField = type.getAddressField("readwrite_end");
     roSpaceField = type.getAddressField("_ro_space");
     rwSpaceField = type.getAddressField("_rw_space");
  }

  public boolean isSharingEnabled() {
    return VM.getVM().isSharingEnabled();
  }

  // NEEDS_CLEANUP
  public CompactingPermGenGen(Address addr) {
    super(addr);
  }

  public OffsetTableContigSpace roSpace() {
    return newOffsetTableContigSpace(roSpaceField.getValue(addr));
  }

  public OffsetTableContigSpace rwSpace() {
    return newOffsetTableContigSpace(rwSpaceField.getValue(addr));
  }

  public String name() {
    return "compacting permanent generation";
  }

  public static Address unsharedBottom() {
    return unsharedBottomField.getValue();
  }

  public static Address unsharedEnd() {
    return unsharedEndField.getValue();
  }

  public static Address sharedBottom() {
    return sharedBottomField.getValue();
  }

  public static Address sharedEnd() {
    return sharedEndField.getValue();
  }

  public static Address readOnlyBottom() {
    return readOnlyBottomField.getValue();
  }

  public static Address readOnlyEnd() {
    return readOnlyEndField.getValue();
  }

  public static Address readWriteBottom() {
    return readWriteBottomField.getValue();
  }

  public static Address readWriteEnd() {
    return readWriteEndField.getValue();
  }

  public static boolean isShared(Address p) {
    return sharedBottom().lessThanOrEqual(p) && sharedEnd().greaterThan(p);
  }

  public static boolean isSharedReadOnly(Address p) {
    return readOnlyBottom().lessThanOrEqual(p) && readOnlyEnd().greaterThan(p);
  }

  public static boolean isSharedReadWrite(Address p) {
    return readWriteBottom().lessThanOrEqual(p) && readWriteEnd().greaterThan(p);
  }

  public boolean isIn(Address p) {
    return unsharedBottom().lessThanOrEqual(p) && sharedEnd().greaterThan(p);
  }

  public void spaceIterate(SpaceClosure blk, boolean usedOnly) {
    super.spaceIterate(blk, usedOnly);
    if (isSharingEnabled()) {
      blk.doSpace(roSpace());
      blk.doSpace(rwSpace());
    }
  }

  public void printOn(PrintStream tty) {
    tty.print("  perm");
    theSpace().printOn(tty);
    if (isSharingEnabled()) {
      tty.print("  ro space: ");
      roSpace().printOn(tty);
      tty.print(", rw space: ");
      rwSpace().printOn(tty);
    }
  }

  private OffsetTableContigSpace newOffsetTableContigSpace(Address addr) {
    return (OffsetTableContigSpace) VMObjectFactory.newObject(
               OffsetTableContigSpace.class, addr);
  }
}
