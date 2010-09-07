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

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public interface SPARCInstructionFactory {
    public SPARCInstruction newCallInstruction(PCRelativeAddress addr);

    public SPARCInstruction newNoopInstruction();

    public SPARCInstruction newSethiInstruction(int imm22, SPARCRegister rd);

    public SPARCInstruction newUnimpInstruction(int const22);

    public SPARCInstruction newBranchInstruction(String name, PCRelativeAddress addr, boolean isAnnuled, int conditionCode);

    public SPARCInstruction newSpecialLoadInstruction(String name, int specialReg, int cregNum,
                                      SPARCRegisterIndirectAddress addr);

    public SPARCInstruction newSpecialStoreInstruction(String name, int specialReg, int cregNum,
                                      SPARCRegisterIndirectAddress addr);

    public SPARCInstruction newLoadInstruction(String name, int opcode,
                                  SPARCRegisterIndirectAddress addr, SPARCRegister rd,
                                  int dataType);

    public SPARCInstruction newStoreInstruction(String name, int opcode,
                                  SPARCRegisterIndirectAddress addr, SPARCRegister rd,
                                  int dataType);

    public SPARCInstruction newStbarInstruction();

    public SPARCInstruction newReadInstruction(int specialReg, int asrRegNum, SPARCRegister rd);

    public SPARCInstruction newWriteInstruction(int specialReg, int asrRegNum, SPARCRegister rs1,
                                             ImmediateOrRegister operand2);

    public SPARCInstruction newIllegalInstruction(int instruction);

    public SPARCInstruction newIndirectCallInstruction(SPARCRegisterIndirectAddress addr,
                                  SPARCRegister rd);

    public SPARCInstruction newReturnInstruction(SPARCRegisterIndirectAddress addr,
                                  SPARCRegister rd, boolean isLeaf);

    public SPARCInstruction newJmplInstruction(SPARCRegisterIndirectAddress addr,
                                  SPARCRegister rd);

    public SPARCInstruction newFP2RegisterInstruction(String name, int opf, SPARCFloatRegister rs, SPARCFloatRegister rd);

    public SPARCInstruction newFPMoveInstruction(String name, int opf, SPARCFloatRegister rs, SPARCFloatRegister rd);

    public SPARCInstruction newFPArithmeticInstruction(String name, int opf, int rtlOperation,
                                                     SPARCFloatRegister rs1, SPARCFloatRegister rs2,
                                                     SPARCFloatRegister rd);

    public SPARCInstruction newFlushInstruction(SPARCRegisterIndirectAddress addr);

    public SPARCInstruction newSaveInstruction(SPARCRegister rs1, ImmediateOrRegister operand2, SPARCRegister rd);

    public SPARCInstruction newRestoreInstruction(SPARCRegister rs1, ImmediateOrRegister operand2, SPARCRegister rd);

    public SPARCInstruction newTrapInstruction(String name, int conditionCode);

    public SPARCInstruction newRettInstruction(SPARCRegisterIndirectAddress addr);

    public SPARCInstruction newArithmeticInstruction(String name, int opcode, int rtlOperation,
                                                     SPARCRegister rs1, ImmediateOrRegister operand2,
                                                     SPARCRegister rd);

    public SPARCInstruction newLogicInstruction(String name, int opcode, int rtlOperation,
                                                SPARCRegister rs1, ImmediateOrRegister operand2,
                                                SPARCRegister rd);

    public SPARCInstruction newMoveInstruction(String name, int opcode,
                                               ImmediateOrRegister operand2,
                                               SPARCRegister rd);

    public SPARCInstruction newShiftInstruction(String name, int opcode, int rtlOperation,
                                                     SPARCRegister rs1, ImmediateOrRegister operand2,
                                                     SPARCRegister rd);

    public SPARCInstruction newCoprocessorInstruction(int instruction, int cpopcode, int opc,
                                                     int rs1Num, int rs2Num, int rdNum);
    public SPARCInstruction newSwapInstruction(String name, SPARCRegisterIndirectAddress addr, SPARCRegister rd);
    public SPARCInstruction newLdstubInstruction(String name, SPARCRegisterIndirectAddress addr, SPARCRegister rd);
}
