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

public interface Arithmetic extends Instruction, RTLOperations {
   public Operand[]  getArithmeticSources();
   public Operand    getArithmeticDestination();
   public int        getOperation(); // one of RTLOperations
}
