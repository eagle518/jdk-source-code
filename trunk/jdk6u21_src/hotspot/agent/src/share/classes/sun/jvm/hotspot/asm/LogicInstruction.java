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

public interface LogicInstruction extends Instruction, RTLOperations {
   public Operand[]  getLogicSources();
   public Operand    getLogicDestination();
   public int        getOperation(); // one of RTLOperations
}
