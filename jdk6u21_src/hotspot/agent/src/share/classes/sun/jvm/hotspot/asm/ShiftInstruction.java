/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.asm;

public interface ShiftInstruction extends Instruction, RTLOperations {
   public Operand getShiftSource();
   public Operand  getShiftLength(); // number of bits to shift
   public Operand getShiftDestination();
}
