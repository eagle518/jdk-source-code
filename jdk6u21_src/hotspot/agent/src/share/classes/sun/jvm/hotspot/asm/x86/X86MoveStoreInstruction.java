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

package sun.jvm.hotspot.asm.x86;

import sun.jvm.hotspot.asm.*;

public class X86MoveStoreInstruction extends X86MemoryInstruction
                                          implements StoreInstruction {
   final protected Register[] storeSources;

   public X86MoveStoreInstruction(String name, Address address, X86Register register, int dataType, int size, int prefixes) {
      super(name, address, register, dataType, size, prefixes);
      storeSources = new Register[1];
      storeSources[0] = register;
   }

   protected String initDescription() {
      StringBuffer buf = new StringBuffer();
      buf.append(getPrefixString());
      buf.append(getName());
      buf.append(spaces);
      buf.append(address.toString());
      buf.append(comma);
      buf.append(register.toString());
      return buf.toString();
   }

   public int getDataType() {
      return dataType;
   }

   public Address getStoreDestination() {
      return address;
   }

   public Register[] getStoreSources() {
      return storeSources;
   }

   public boolean isStore() {
      return true;
   }
}
