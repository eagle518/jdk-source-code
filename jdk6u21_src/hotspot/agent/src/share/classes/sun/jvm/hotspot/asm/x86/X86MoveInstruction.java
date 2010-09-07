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

public class X86MoveInstruction extends X86Instruction
                          implements MoveInstruction, RTLOperations {
   private ImmediateOrRegister source;
   private X86Register destination;

   public X86MoveInstruction(String name, X86Register rd, ImmediateOrRegister oSrc, int size, int prefixes) {
      super(name, size, prefixes);
      this.source = oSrc;
      this.destination = rd;
   }
   public String asString(long currentPc, SymbolFinder symFinder) {
      StringBuffer buf = new StringBuffer();
      buf.append(getPrefixString());
      buf.append(getName());
      buf.append(spaces);
      buf.append(destination.toString());
      buf.append(comma);
      buf.append(getSourceString());
      return buf.toString();
   }

   protected String getSourceString() {
      StringBuffer buf = new StringBuffer();
      if ((source instanceof Register)) {
         buf.append(source.toString());
      } else {
         Number number = ((Immediate)source).getNumber();
         buf.append("0x");
         buf.append(Integer.toHexString(number.intValue()));
      }
      return buf.toString();
   }

   public ImmediateOrRegister getMoveSource() {
      return source;
   }

   public Register getMoveDestination() {
     return destination;
   }

   // for condition moves
   public boolean  isConditional() {
      return false;
   }

   public boolean isMove() {
      return true;
   }
}
