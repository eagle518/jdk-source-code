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

public class X86JmpInstruction extends X86Instruction
                         implements BranchInstruction {
   final private Address addr;

   public X86JmpInstruction(String name, Address addr, int size, int prefixes) {
      super(name, size, prefixes);
      this.addr = addr;
      if(addr instanceof X86PCRelativeAddress) {
         ((X86PCRelativeAddress)addr).setInstructionSize(getSize());
      }
   }

   public String asString(long currentPc, SymbolFinder symFinder) {
      StringBuffer buf = new StringBuffer();
      buf.append(getPrefixString());
      buf.append(getName());
      buf.append(spaces);
      long address;
      if(addr instanceof X86PCRelativeAddress) {
         long disp = ((X86PCRelativeAddress)addr).getDisplacement();
         address = disp + currentPc;
         buf.append(symFinder.getSymbolFor(address));
      }
      else {
         buf.append(addr.toString());
      }
      return buf.toString();
   }

   public Address getBranchDestination() {
      return addr;
   }

   public boolean isBranch() {
      return true;
   }

   public boolean isConditional() {
      return false;
   }
}
