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
import sun.jvm.hotspot.gc_implementation.shared.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class PSPermGen extends PSOldGen {
   static {
      VM.registerVMInitializedObserver(new Observer() {
         public void update(Observable o, Object data) {
            initialize(VM.getVM().getTypeDataBase());
         }
      });
   }

   private static synchronized void initialize(TypeDataBase db) {
      // just checking type existence
      Type type = db.lookupType("PSPermGen");
   }

   public PSPermGen(Address addr) {
      super(addr);
   }

   public void printOn(PrintStream tty) {
      tty.print("PSPermGen [ ");
      objectSpace().printOn(tty);
      tty.print(" ] ");
   }

   // FIXME: no other stuff yet
}
