#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)shared_amd64.cpp	1.2 03/12/23 16:35:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_shared_amd64.cpp.incl"

void SharedInfo::set_regName()
{
  regName[rax->encoding()] = "RAX";
  regName[rcx->encoding()] = "RCX";
  regName[rbx->encoding()] = "RBX";
  regName[rdx->encoding()] = "RDX";
  regName[rdi->encoding()] = "RDI";
  regName[rsi->encoding()] = "RSI";
  regName[rbp->encoding()] = "RBP";
  regName[rsp->encoding()] = "RSP";
  regName[r8->encoding()]  = "R8";
  regName[r9->encoding()]  = "R9";
  regName[r10->encoding()] = "R10";
  regName[r11->encoding()] = "R11";
  regName[r12->encoding()] = "R12";
  regName[r13->encoding()] = "R13";
  regName[r14->encoding()] = "R14";
  regName[r15->encoding()] = "R15";
#ifdef COMPILER1
#error XXX fixme
  regName[ 8] = "FPU ST[0]lo";
  regName[ 9] = "FPU ST[0]hi";
  regName[10] = "FPU ST[1]lo";
  regName[11] = "FPU ST[1]hi";
  regName[12] = "FPU ST[2]lo";
  regName[13] = "FPU ST[2]hi";
  regName[14] = "FPU ST[3]lo";
  regName[15] = "FPU ST[3]hi";
  regName[16] = "FPU ST[4]lo";
  regName[17] = "FPU ST[4]hi";
  regName[18] = "FPU ST[5]lo";
  regName[19] = "FPU ST[5]hi";
  regName[20] = "FPU ST[6]lo";
  regName[21] = "FPU ST[6]hi";
  regName[22] = "FPU ST[7]lo";
  regName[23] = "FPU ST[7]hi";
#endif
}

