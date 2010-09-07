/*
 * @(#)BinaryTreeDictionary.java
 * Copyright (c) 2000, 2008, Oracle and/or its affiliates. All rights reserved.
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
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.*;

public class BinaryTreeDictionary extends VMObject {
   static {
      VM.registerVMInitializedObserver(new Observer() {
         public void update(Observable o, Object data) {
            initialize(VM.getVM().getTypeDataBase());
         }
      });
   }

   private static synchronized void initialize(TypeDataBase db) {
      Type type = db.lookupType("BinaryTreeDictionary");
      totalSizeField = type.getCIntegerField("_totalSize");
   }

   // Fields
   private static CIntegerField totalSizeField;

   // Accessors
   public long size() {
      return totalSizeField.getValue(addr);
   }

   // Constructor
   public BinaryTreeDictionary(Address addr) {
      super(addr);
   }
}
