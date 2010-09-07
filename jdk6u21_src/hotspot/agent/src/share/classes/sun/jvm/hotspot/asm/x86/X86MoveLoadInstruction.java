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

public class X86MoveLoadInstruction extends X86MemoryInstruction
                                          implements LoadInstruction {
   public X86MoveLoadInstruction(String name, X86Register register, Address address, int dataType, int size, int prefixes) {
      super(name, address, register, dataType, size, prefixes);
   }

   public boolean isLoad() {
      return true;
   }

   public Register[] getLoadDestinations() {
      Register[] destinations = new Register[1];
      destinations[0] = register;
      return destinations;
   }

   protected String initDescription() {
      StringBuffer buf = new StringBuffer();
      buf.append(getPrefixString());
      buf.append(getName());
      buf.append(spaces);
      buf.append(register.toString());
      buf.append(comma);
      buf.append(address.toString());
      return buf.toString();
   }

   public Address getLoadSource() {
      return address;
   }

}
