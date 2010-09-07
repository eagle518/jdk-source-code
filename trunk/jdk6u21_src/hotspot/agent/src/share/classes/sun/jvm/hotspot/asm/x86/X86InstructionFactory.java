/*
 * Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
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

public interface X86InstructionFactory {
   public X86Instruction newCallInstruction(String name, Address addr, int size, int prefixes);

   public X86Instruction newJmpInstruction(String name, Address addr, int size, int prefixes);

   public X86Instruction newCondJmpInstruction(String name, X86PCRelativeAddress addr, int size, int prefixes);

   public X86Instruction newMoveInstruction(String name, X86Register rd, ImmediateOrRegister oSrc, int size, int prefixes);

   public X86Instruction newMoveLoadInstruction(String name, X86Register op1, Address op2, int dataType, int size, int prefixes);

   public X86Instruction newMoveStoreInstruction(String name, Address op1, X86Register op2, int dataType, int size, int prefixes);

   public X86Instruction newArithmeticInstruction(String name, int rtlOperation, Operand op1, Operand op2, Operand op3, int size, int prefixes);

   public X86Instruction newArithmeticInstruction(String name, int rtlOperation, Operand op1, Operand op2, int size, int prefixes);

   public X86Instruction newLogicInstruction(String name, int rtlOperation, Operand op1, Operand op2, int size, int prefixes);

   public X86Instruction newBranchInstruction(String name, X86PCRelativeAddress addr, int size, int prefixes);

   public X86Instruction newShiftInstruction(String name, int rtlOperation, Operand op1, ImmediateOrRegister op2, int size, int prefixes);

   public X86Instruction newRotateInstruction(String name, Operand op1, ImmediateOrRegister op2, int size, int prefixes);

   public X86Instruction newFPLoadInstruction(String name, Operand op, int size, int prefixes);

   public X86Instruction newFPStoreInstruction(String name, Operand op, int size, int prefixes);

   public X86Instruction newFPArithmeticInstruction(String name, int rtlOperation, Operand op1, Operand op2, int size, int prefixes);

   public X86Instruction newGeneralInstruction(String name, Operand op1, Operand op2, Operand op3, int size, int prefixes);

   public X86Instruction newGeneralInstruction(String name, Operand op1, Operand op2, int size, int prefixes);

   public X86Instruction newGeneralInstruction(String name, Operand op1, int size, int prefixes);

   public X86Instruction newIllegalInstruction();

}
