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

public abstract class X86MemoryInstruction extends X86Instruction
             implements MemoryInstruction {
   final protected Address address;
   final protected X86Register register;
   final protected int dataType;
   final protected String description;

   public X86MemoryInstruction(String name, Address address, X86Register register, int dataType, int size, int prefixes) {
      super(name, size, prefixes);
      this.address = address;
      this.register = register;
      this.dataType = dataType;
      description = initDescription();
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

   public String asString(long currentPc, SymbolFinder symFinder) {
      return description;
   }

   public int getDataType() {
      return dataType;
   }

   public boolean isConditional() {
      return false;
   }
}
