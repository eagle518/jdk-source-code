/*
 * @(#)FreeList.java
 *
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

public class FreeList extends VMObject {
   static {
      VM.registerVMInitializedObserver(new Observer() {
         public void update(Observable o, Object data) {
            initialize(VM.getVM().getTypeDataBase());
         }
      });
   }

   private static synchronized void initialize(TypeDataBase db) {
      Type type = db.lookupType("FreeList");
      sizeField = type.getCIntegerField("_size");
      countField = type.getCIntegerField("_count");
      headerSize = type.getSize();
   }

   // Fields
   private static CIntegerField sizeField;
   private static CIntegerField countField;
   private static long          headerSize;

   //Constructor
   public FreeList(Address address) {
     super(address);
   }

   // Accessors
   public long size() {
      return sizeField.getValue(addr);
   }

   public long count() {
      return  countField.getValue(addr);
   }

   public static long sizeOf() {
     return headerSize;
  }
}
