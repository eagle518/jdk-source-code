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

package sun.jvm.hotspot.gc_implementation.parallelScavenge;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.gc_interface.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class ParallelScavengeHeap extends CollectedHeap {
   static {
      VM.registerVMInitializedObserver(new Observer() {
         public void update(Observable o, Object data) {
            initialize(VM.getVM().getTypeDataBase());
         }
      });
   }

   private static synchronized void initialize(TypeDataBase db) {
      Type type = db.lookupType("ParallelScavengeHeap");
      youngGenField = type.getAddressField("_young_gen");
      oldGenField    = type.getAddressField("_old_gen");
      permGenField    = type.getAddressField("_perm_gen");
   }

   public ParallelScavengeHeap(Address addr) {
      super(addr);
   }

   // Fields
   private static AddressField youngGenField;
   private static AddressField oldGenField;
   private static AddressField permGenField;

   // Accessors
   public PSYoungGen youngGen() {
      return (PSYoungGen) VMObjectFactory.newObject(PSYoungGen.class, youngGenField.getValue());
   }

   public PSOldGen oldGen() {
      return (PSOldGen) VMObjectFactory.newObject(PSOldGen.class, oldGenField.getValue());
   }

   public PSPermGen permGen() {
      return (PSPermGen) VMObjectFactory.newObject(PSPermGen.class, permGenField.getValue());
   }

   public long capacity() {
      return youngGen().capacity() + oldGen().capacity();
   }

   public long used() {
      return youngGen().used() + oldGen().used();
   }

   public boolean isIn(Address a) {
      if (youngGen().isIn(a)) {
         return true;
      }

      if (oldGen().isIn(a)) {
         return true;
      }

      if (permGen().isIn(a)) {
         return true;
      }

      return false;
   }

   public CollectedHeapName kind() {
      return CollectedHeapName.PARALLEL_SCAVENGE_HEAP;
   }

   public void printOn(PrintStream tty) {
      tty.print("ParallelScavengeHeap [ ");
      youngGen().printOn(tty);
      oldGen().printOn(tty);
      permGen().printOn(tty);
      tty.print(" ] ");
   }
}
