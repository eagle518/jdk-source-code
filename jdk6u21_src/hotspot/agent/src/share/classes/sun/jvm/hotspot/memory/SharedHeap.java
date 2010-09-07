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

package sun.jvm.hotspot.memory;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.gc_interface.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public abstract class SharedHeap extends CollectedHeap {
  private static AddressField permGenField;
  private static VirtualConstructor ctor;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("SharedHeap");
    permGenField        = type.getAddressField("_perm_gen");
    ctor = new VirtualConstructor(db);
    ctor.addMapping("CompactingPermGen", CompactingPermGen.class);
    ctor.addMapping("CMSPermGen", CMSPermGen.class);

  }

  public SharedHeap(Address addr) {
    super(addr);
  }

  /** These functions return the "permanent" generation, in which
      reflective objects are allocated and stored.  Two versions, the
      second of which returns the view of the perm gen as a
      generation. (FIXME: this distinction is strange and seems
      unnecessary, and should be cleaned up.) */
  public PermGen perm() {
    return (PermGen) ctor.instantiateWrapperFor(permGenField.getValue(addr));
  }

  public CollectedHeapName kind() {
    return CollectedHeapName.SHARED_HEAP;
  }

  public Generation permGen() {
    return perm().asGen();
  }
}
